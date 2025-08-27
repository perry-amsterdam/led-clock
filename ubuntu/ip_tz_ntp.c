// build: gcc ip_tz_ntp.c -o ip_tz_ntp -lcurl -lcjson
// deps (Ubuntu): sudo apt update && sudo apt install -y libcurl4-openssl-dev libcjson-dev
// run: ./ip_tz_ntp

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <curl/curl.h>
#include <cjson/cJSON.h>

// ---------------------- util: dynamic buffer for libcurl ----------------------
typedef struct { char *data; size_t len; } mem_t;

static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t n = size * nmemb;
    mem_t *m = (mem_t*)userdata;
    char *p = realloc(m->data, m->len + n + 1);
    if (!p) return 0; // out of memory -> abort
    m->data = p;
    memcpy(m->data + m->len, ptr, n);
    m->len += n;
    m->data[m->len] = '\0';
    return n;
}

static bool http_get(const char *url, mem_t *out, long *http_code) {
    CURL *curl = curl_easy_init();
    if (!curl) return false;
    memset(out, 0, sizeof(*out));
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "ledclock-ubuntu/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);
    CURLcode rc = curl_easy_perform(curl);
    long code = 0; curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    if (http_code) *http_code = code;
    curl_easy_cleanup(curl);
    return (rc == CURLE_OK && code == 200);
}

// ---------------------- Step 1+2: public IP, country code, timezone ----------------------
// Uses ip-api.com (free, no key). We'll request only the fields we need.

static bool get_ip_and_geo(char *out_ip, size_t ip_sz,
                           char *out_cc, size_t cc_sz,
                           char *out_tz, size_t tz_sz) {
    const char *URL = "http://ip-api.com/json?fields=status,message,query,countryCode,timezone";
    mem_t m = {0}; long code=0;
    if (!http_get(URL, &m, &code)) {
        fprintf(stderr, "HTTP error getting %s (code %ld)\n", URL, code);
        free(m.data); return false;
    }
    cJSON *root = cJSON_Parse(m.data);
    if (!root) { fprintf(stderr, "JSON parse error\n"); free(m.data); return false; }
    cJSON *status = cJSON_GetObjectItemCaseSensitive(root, "status");
    if (!cJSON_IsString(status) || strcmp(status->valuestring, "success") != 0) {
        cJSON *msg = cJSON_GetObjectItemCaseSensitive(root, "message");
        fprintf(stderr, "API returned error: %s\n", cJSON_IsString(msg)? msg->valuestring:"unknown");
        cJSON_Delete(root); free(m.data); return false;
    }
    cJSON *q = cJSON_GetObjectItemCaseSensitive(root, "query");
    cJSON *cc = cJSON_GetObjectItemCaseSensitive(root, "countryCode");
    cJSON *tz = cJSON_GetObjectItemCaseSensitive(root, "timezone");
    if (cJSON_IsString(q) && out_ip) snprintf(out_ip, ip_sz, "%s", q->valuestring);
    if (cJSON_IsString(cc) && out_cc) snprintf(out_cc, cc_sz, "%s", cc->valuestring);
    if (cJSON_IsString(tz) && out_tz) snprintf(out_tz, tz_sz, "%s", tz->valuestring);
    cJSON_Delete(root); free(m.data); return true;
}

// ---------------------- Step 3: SNTP (UDP) to pool.ntp.org ----------------------
// Minimal SNTP client: send 48-byte packet (LI=0, VN=4, Mode=3), read Transmit Timestamp.
// Convert NTP epoch (1900) to Unix epoch (1970) by subtracting 2208988800 seconds.

#define NTP_UNIX_EPOCH_DIFF 2208988800UL

static bool sntp_get_time(const char *server, time_t *out_unix) {
    if (!server || !out_unix) return false;

    int sock = -1; struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // v4 or v6
    hints.ai_socktype = SOCK_DGRAM;

    int err = getaddrinfo(server, "123", &hints, &res);
    if (err != 0) { fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err)); return false; }

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) { perror("socket"); freeaddrinfo(res); return false; }

    // 2s recv timeout
    struct timeval tv = { .tv_sec = 2, .tv_usec = 0 };
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    unsigned char pkt[48] = {0};
    pkt[0] = 0x23; // LI=0, VN=4, Mode=3 (client)

    ssize_t n = sendto(sock, pkt, sizeof(pkt), 0, res->ai_addr, res->ai_addrlen);
    if (n != (ssize_t)sizeof(pkt)) { perror("sendto"); close(sock); freeaddrinfo(res); return false; }

    unsigned char reply[48];
    ssize_t r = recvfrom(sock, reply, sizeof(reply), 0, NULL, NULL);
    if (r < 48) { perror("recvfrom"); close(sock); freeaddrinfo(res); return false; }

    close(sock); freeaddrinfo(res);

    // Transmit Timestamp starts at byte 40: seconds (32-bit) + fraction (32-bit)
    uint32_t secs;
    memcpy(&secs, reply + 40, 4);
    secs = ntohl(secs);
    if (secs < NTP_UNIX_EPOCH_DIFF) return false; // sanity
    *out_unix = (time_t)(secs - NTP_UNIX_EPOCH_DIFF);
    return true;
}

// ---------------------- main ----------------------
int main(void) {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    char ip[64] = {0}, cc[8] = {0}, tz[128] = {0};
    if (!get_ip_and_geo(ip, sizeof ip, cc, sizeof cc, tz, sizeof tz)) {
        fprintf(stderr, "Failed to get public IP & geo info\n");
        return 1;
    }
    printf("Public IP    : %s\n", ip);
    printf("Country code : %s\n", cc);
    printf("Timezone     : %s\n", tz);

    // Query time from NTP pool (you could also use a regional pool like nl.pool.ntp.org)
    time_t t_utc;
    if (!sntp_get_time("pool.ntp.org", &t_utc)) {
        fprintf(stderr, "Failed to get NTP time\n");
        return 1;
    }

    // Show UTC time
    struct tm tm_utc; gmtime_r(&t_utc, &tm_utc);
    char buf[64]; strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S UTC", &tm_utc);
    printf("Time (UTC)   : %s\n", buf);

    // Apply timezone from API (IANA TZ name, e.g., "Europe/Amsterdam")
    if (tz[0]) { setenv("TZ", tz, 1); tzset(); }

    struct tm tm_local; localtime_r(&t_utc, &tm_local);
    char buf2[80]; strftime(buf2, sizeof buf2, "%Y-%m-%d %H:%M:%S %Z%z", &tm_local);
    printf("Time (local) : %s\n", buf2);

    curl_global_cleanup();
    return 0;
}


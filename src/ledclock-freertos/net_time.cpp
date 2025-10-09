#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <time.h>
#include "lwip/apps/sntp.h"

#include "net_time.h"
#include "hal_time_freertos.h"
#include "timezone_storage.h"

// --------- Configurable endpoints (overridable via -D defines) ----------
#ifndef URL_TIMEINFO
// IP-based fallback (detect timezone via uitgaande IP)
#define URL_TIMEINFO "https://worldtimeapi.org/api/ip"
#endif

#ifndef URL_TZ_IANA_BASE
// Handmatige TZ details (offsets) voor IANA string
#define URL_TZ_IANA_BASE "https://worldtimeapi.org/api/timezone/"
#endif

#ifndef URL_COUNTRYCODE
// Simpele country code resolver
#define URL_COUNTRYCODE "https://ipapi.co/country/"
#endif
// ------------------------------------------------------------------------

// Preview helper for HTTP payloads (guarded by DEBUG_TZ)
void dumpPreview(const String& payload)
{
	#ifdef DEBUG_TZ
	int n = payload.length();
	int k = n > 160 ? 160 : n;
	Serial.print("[HTTP PREVIEW] ");
	for (int i = 0; i < k; ++i)
	{
		char c = payload[i];
		if (c == '\n') { Serial.print("\\n"); }
		else if (c == '\r') { Serial.print("\\r"); }
		else { Serial.print(c); }
	}
	if (n > k) Serial.print(" ...");
	Serial.println();
	#else
	(void)payload;
	#endif
}


// Very light-weight JSON string extractor: looks for "key":"value"
String extractJsonString(const String& json, const String& key)
{
	String pat = "\"" + key + "\"";
	int i = json.indexOf(pat);
	if (i < 0) return "";
	i = json.indexOf(':', i);
	if (i < 0) return "";
	// skip whitespace
	while (i + 1 < (int)json.length() && (json[i + 1] == ' ' || json[i + 1] == '\t')) i++;
	// expect '"'
	int q1 = json.indexOf('\"', i + 1);
	if (q1 < 0) return "";
	int q2 = json.indexOf('\"', q1 + 1);
	if (q2 < 0) return "";
	return json.substring(q1 + 1, q2);
}


// Simple HTTP GET into String (TLS optional)
static bool httpGetToString(const String& url, String& out, bool acceptAllHttps)
{
	WiFiClientSecure client;
	if (acceptAllHttps) client.setInsecure();

	HTTPClient http;
	http.setConnectTimeout(5000);
	http.setTimeout(10000);
	http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

	if (!http.begin(client, url))
	{
		#ifdef DEBUG_TZ
		Serial.printf("[HTTP] begin() failed: %s\n", url.c_str());
		#endif
		return false;
	}

	// retry a few times  sommige hosts weigeren soms verbinding
	int code = -1;
	for (int attempt = 0; attempt < 3; ++attempt)
	{
		code = http.GET();
		if (code == HTTP_CODE_OK) break;
		hal_delay_ms(200);
	}

	if (code != HTTP_CODE_OK)
	{
		#ifdef DEBUG_TZ
		Serial.printf("[HTTP] GET %s -> %d\n", url.c_str(), code);
		#endif
		http.end();
		return false;
	}

	out = http.getString();
	http.end();
	return true;
}


// Fetch offsets for a given IANA TZ using WorldTimeAPI
static bool fetchOffsetsForIanaFromWorldTimeAPI(const String& ianaTz,
int& gmtOffsetSec,
int& daylightOffsetSec,
bool acceptAllHttps)
{
	String url = String(URL_TZ_IANA_BASE) + ianaTz;
	String body;
	if (!httpGetToString(url, body, acceptAllHttps))
	{
		return false;
	}
	dumpPreview(body);

	// WorldTimeAPI velden: raw_offset (sec), dst_offset (sec)
	// Gebruik string extractor (lichtgewicht) of parse ints snel:
	// we pakken string values en converteren (robust genoeg voor WTA format)
	String raw_s = extractJsonString(body, "raw_offset");
	String dst_s = extractJsonString(body, "dst_offset");

	// Sommige implementaties geven raw_offset/dst_offset als numeriek zonder quotes.
	// Fallback: simpele zoek-naar sleutel gevolgd door nummer.
	auto extractNumber = [&](const String& src, const String& k, long& outNum) -> bool
	{
		int p = src.indexOf("\"" + k + "\"");
		if (p < 0) return false;
		p = src.indexOf(':', p);
		if (p < 0) return false;
		// skip spaces
		while (p + 1 < (int)src.length() && (src[p + 1] == ' ' || src[p + 1] == '\t')) p++;
		int s = p + 1;
		int e = s;
		while (e < (int)src.length() && (isDigit(src[e]) || src[e] == '-' )) e++;
		if (e <= s) return false;
		outNum = src.substring(s, e).toInt();
		return true;
	};

	long raw = 0, dst = 0;
	bool okRaw = false, okDst = false;

	if (raw_s.length() > 0) { raw = raw_s.toInt(); okRaw = true; }
	if (dst_s.length() > 0) { dst = dst_s.toInt(); okDst = true; }

	if (!okRaw) okRaw = extractNumber(body, "raw_offset", raw);
	if (!okDst) okDst = extractNumber(body, "dst_offset", dst);

	if (!okRaw || !okDst)
	{
		#ifdef DEBUG_TZ
		Serial.println("[TZ] Missing raw_offset/dst_offset in WorldTimeAPI response");
		#endif
		return false;
	}

	gmtOffsetSec      = (int)raw;
	daylightOffsetSec = (int)dst;
	#ifdef DEBUG_TZ
	Serial.printf("[TZ] IANA '%s' raw=%d dst_off=%d\n", ianaTz.c_str(), gmtOffsetSec, daylightOffsetSec);
	#endif
	return true;
}


// --- Public API ---

// Haal country code op (2-letter) via eenvoudige endpoint
String fetchCountryCode()
{
	String body;
	if (!httpGetToString(URL_COUNTRYCODE, body, /*acceptAllHttps*/ true)) {
	return "";
}


// ipapi.co/country/ geeft letterlijk bv. "NL\n"
body.trim();
return body;
}


// Bepaalt tzIana en offsets.
// 1) Als user-TZ gezet is: die gebruiken, offsets via WorldTimeAPI proberen (fallback 0/0).
// 2) Anders: IP-based lookup via URL_TIMEINFO (WorldTimeAPI /api/ip), haal timezone+offsets daaruit.
bool fetchTimeInfo(String& tzIana, int& gmtOffsetSec, int& daylightOffsetSec, bool acceptAllHttps)
{
	// 1) Respecteer via-API ingestelde timezone
	{
		String manualTz;
		if (tz_user_is_set() && tz_user_get(manualTz) && manualTz.length() > 0)
		{
			tzIana = manualTz;

			// offsets via WorldTimeAPI voor exact deze IANA
			if (!fetchOffsetsForIanaFromWorldTimeAPI(tzIana, gmtOffsetSec, daylightOffsetSec, acceptAllHttps))
			{
				gmtOffsetSec = 0;
				daylightOffsetSec = 0;
				#ifdef DEBUG_TZ
				Serial.printf("[TZ] Using manual TZ without fetched offsets: %s\n", tzIana.c_str());
				#endif
			}
			else
			{
				#ifdef DEBUG_TZ
				Serial.printf("[TZ] Using manual TZ with fetched offsets: %s\n", tzIana.c_str());
				#endif
			}
			return true;
		}
	}

	// 2) Fallback: via uitgaande IP
	String body;
	if (!httpGetToString(URL_TIMEINFO, body, acceptAllHttps))
	{
		#ifdef DEBUG_TZ
		Serial.println("[TZ] IP-based timeinfo request failed");
		#endif
		return false;
	}

	dumpPreview(body);

	// WorldTimeAPI /api/ip velden: "timezone":"Europe/Amsterdam", "raw_offset":7200, "dst_offset":3600, "dst":true
	String tz = extractJsonString(body, "timezone");
	if (tz.length() == 0)
	{
		#ifdef DEBUG_TZ
		Serial.println("[TZ] Could not find 'timezone' in response");
		#endif
		return false;
	}

	tzIana = tz;

	long raw = 0, dst = 0;
	bool okRaw = false, okDst = false;

	// Probeert numeriek te pakken (zie helper hierboven)
	auto extractNumber = [&](const String& src, const String& k, long& outNum) -> bool
	{
		int p = src.indexOf("\"" + k + "\"");
		if (p < 0) return false;
		p = src.indexOf(':', p);
		if (p < 0) return false;
		while (p + 1 < (int)src.length() && (src[p + 1] == ' ' || src[p + 1] == '\t')) p++;
		int s = p + 1;
		int e = s;
		while (e < (int)src.length() && (isDigit(src[e]) || src[e] == '-' )) e++;
		if (e <= s) return false;
		outNum = src.substring(s, e).toInt();
		return true;
	};

	okRaw = extractNumber(body, "raw_offset", raw);
	okDst = extractNumber(body, "dst_offset", dst);

	// Als offsets ontbreken, val terug op 0/0 (TZ string dekt DST-regels)
	gmtOffsetSec      = okRaw ? (int)raw : 0;
	daylightOffsetSec = okDst ? (int)dst : 0;

	#ifdef DEBUG_TZ
	Serial.printf("[TZ] From IP: tz=%s raw=%d dst_off=%d\n",
		tzIana.c_str(), gmtOffsetSec, daylightOffsetSec);
	#endif

	return true;
}


// Configureer SNTP + TZ; Keert true terug als tijd plausibel gezet is
bool setupTimeFromInternet(bool acceptAllHttps)
{
	String tzIana;
	int gmtOffset = 0;
	int dstOffset = 0;

	if (!fetchTimeInfo(tzIana, gmtOffset, dstOffset, acceptAllHttps))
	{
		#ifdef DEBUG_TZ
		Serial.println("[TIME] fetchTimeInfo failed");
		#endif
		return false;
	}

	// Stel TZ in via IANA string  ESP32 libc ondersteunt POSIX TZ, maar
	// recentere IDF builds accepteren ook IANA via setenv("TZ", "...").
	// We vertrouwen op IANA; offsets worden door TZ-regels afgehandeld.
	if (tzIana.length() > 0)
	{
		setenv("TZ", tzIana.c_str(), 1);
		tzset();
		#ifdef DEBUG_TZ
		Serial.printf("[TIME] TZ set to IANA: %s\n", tzIana.c_str());
		#endif
	}

	// SNTP configuratie
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, (char*)"pool.ntp.org");
	sntp_setservername(1, (char*)"time.google.com");
	sntp_init();

	// Wacht even op tijd-sync (niet te lang)
	const uint32_t start = hal_millis();
	while ((time(nullptr) < 8 * 3600) && (hal_millis() - start < 5000))
	{
		hal_delay_ms(150);
	}

	time_t now = time(nullptr);
	#ifdef DEBUG_TZ
	struct tm tm_now;
	localtime_r(&now, &tm_now);
	char buf[64];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S %Z", &tm_now);
	Serial.printf("[TIME] now=%ld (%s)\n", (long)now, buf);
	#endif

	return (now >= 8 * 3600);
}


// Eenvoudig onderhoud: check 1x per minuut of tijd nog ok is en resync zo nodig
void netTimeMaintain()
{
	static uint32_t last = 0;
	uint32_t now_ms = hal_millis();

	// run roughly once per minute
	if (now_ms - last < 60000) return;
	last = now_ms;

	time_t now = time(nullptr);
	// If epoch looks invalid (< 8 hours since boot default), try to resync.
	if (now < 8 * 3600)
	{
		setupTimeFromInternet(true);
	}
}

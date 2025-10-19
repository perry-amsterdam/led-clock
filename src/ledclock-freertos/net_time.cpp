#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <time.h>
#include "esp_netif_sntp.h"
#include "esp_sntp.h"

#include "net_time.h"
#include "hal_time_freertos.h"
#include "config_storage.h"
#include <ArduinoJson.h>

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
	// Limit the debug output to maxium 160 characters.
	int n = payload.length();
	int k = n > 160 ? 160 : n;

	Serial.print("[HTTP PREVIEW] ");
	for (int i = 0; i < k; ++i)
	{
		char c = payload[i];
		if (c == '\n')
		{
			Serial.print("\\n");
		}
		else if (c == '\r')
		{
			Serial.print("\\r");
		}
		else
		{
			Serial.print(c);
		}
	}

	if (n > k)
	{
		Serial.print(" ...");
	}
	Serial.println();
	#else
	(void)payload;
	#endif
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
	for (int attempt = 0; attempt < 10; ++attempt)
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


// Fetch offsets using WorldTimeAPI
static bool fetchFromWorldTimeAPI(const String& ianaTz, String& timezone, int& gmtOffsetSec, int& daylightOffsetSec, bool acceptAllHttps)
{

	String url;
	if (ianaTz[0] == '\0')
	{
		url = String(URL_TIMEINFO);
	}
	else
	{
		url = String(URL_TZ_IANA_BASE) + ianaTz;
	}
	String body;

	if (!httpGetToString(url, body, acceptAllHttps))
	{
		return false;
	}

	//Serial.println(String(URL_TZ_IANA_BASE) + ianaTz);
	dumpPreview(body);

	// Parse JSON body
	StaticJsonDocument<1024> doc;
	DeserializationError err = deserializeJson(doc, body);
	if (err)
	{
		Serial.print("JSON parse error: ");
		Serial.println(err.f_str());
		return false;
	}

	// Controleer of beide keys bestaan
	if (!doc.containsKey("raw_offset") || !doc.containsKey("dst_offset"))
	{
		#ifdef DEBUG_TZ
		Serial.println("[TZ] Missing raw_offset/dst_offset in WorldTimeAPI response");
		#endif
		return false;
	}

	// Lees offsets direct als integers
	int raw = doc["raw_offset"] | 0;
	int dst = doc["dst_offset"] | 0;

	if (!doc.containsKey("timezone"))
	{
		Serial.printf("[JSON] Key not found: %s\n", "timezone");
		return false;
	}

	// veilig uitlezen als C-string, dan naar Arduino String
	const char* tz = doc["timezone"] | "";

	gmtOffsetSec      = (int)raw;
	daylightOffsetSec = (int)dst;
	timezone          = tz;

	#ifdef DEBUG_TZ
	Serial.printf("[TZ] '%s' raw=%d dst_off=%d\n", timezone.c_str(), gmtOffsetSec, daylightOffsetSec);
	#endif
	return true;
}


// --- Public API ---

// Configureer SNTP + TZ; Keert true terug als tijd plausibel gezet is
bool setupTimeFromInternet(bool acceptAllHttps)
{
	String tzIana;
	String timezone;
	int gmtOffset = 0;
	int dstOffset = 0;

	// Bepaalt tzIana en offsets.
	// 1) Als user-TZ gezet is: die gebruiken, offsets via WorldTimeAPI proberen (fallback 0/0).
	// 2) Anders: IP-based lookup via URL_TIMEINFO (WorldTimeAPI /api/ip), haal timezone+offsets daaruit.
	String manualTz;
	if (tz_user_is_set() && tz_user_get(manualTz) && manualTz.length() > 0)
	{
		tzIana = manualTz;
	}  else
	{
		tzIana = "";
	}

	// Timezone en offsets via WorldTimeAPI.
	if (!fetchFromWorldTimeAPI(tzIana, timezone, gmtOffset, dstOffset, acceptAllHttps))
	{
		gmtOffset = 0;
		dstOffset = 0;
		timezone = "";
		#ifdef DEBUG_TZ
		Serial.printf("[TZ] Using TZ without fetched offsets: %s\n", tzIana.c_str());
		#endif
	}
	else
	{
		#ifdef DEBUG_TZ
		Serial.printf("[TZ] Using TZ with fetched offsets: %s\n", tzIana.c_str());
		#endif
	}

	// Probeer IANA TZ te zetten (voor wie dit ondersteunt)
	if (timezone.length() > 0)
	{
		setenv("TZ", timezone.c_str(), 1);
		tzset();
		#ifdef DEBUG_TZ
		Serial.printf("[TIME] TZ set to IANA: %s\n", timezone.c_str());
		#endif
	}

	// **Belangrijk**: configureer SNTP inclusief offsets zodat tijd lokaal klopt
	//  (werkt ook wanneer IANA TZ niet wordt ondersteund).
	configTime(gmtOffset, dstOffset, "europe.pool.ntp.org", "time.google.com", "pool.ntp.org");

	// Wacht even op tijd-sync (niet te lang)
	const uint32_t start = hal_millis();
	while ((time(nullptr) < 8 * 3600) && (hal_millis() - start < 10000))
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

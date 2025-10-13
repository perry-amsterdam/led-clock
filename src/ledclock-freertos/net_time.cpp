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


// Fetch offsets for a given IANA TZ using WorldTimeAPI
static bool fetchOffsetsForIanaFromWorldTimeAPI(const String& ianaTz, int& gmtOffsetSec, int& daylightOffsetSec, bool acceptAllHttps)
{
	String url = String(URL_TZ_IANA_BASE) + ianaTz;
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

	// Debug (optioneel)
	Serial.printf("raw_offset=%d, dst_offset=%d\n", raw, dst);

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
	if (!httpGetToString(URL_COUNTRYCODE, body, true))
	{
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
			Serial.printf("[TZ] From IP: tz=%s raw=%d dst_off=%d\n", tzIana.c_str(), gmtOffsetSec, daylightOffsetSec);
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

	// Parse JSON body
	DynamicJsonDocument doc(4096);
	DeserializationError err = deserializeJson(doc, body);
	if (err)
	{
		Serial.print("JSON parse error: ");
		Serial.println(err.f_str());
		return false;
	}

	if (!doc.containsKey("timezone"))
	{
		Serial.printf("[JSON] Key not found: %s\n", "timezone");
		return false;
	}

	// veilig uitlezen als C-string, dan naar Arduino String
	const char* tz_c = doc["timezone"] | "";
	String tz = String(tz_c);

	if (tz.length() == 0)
	{
		#ifdef DEBUG_TZ
		Serial.println("[TZ] Could not find 'timezone' in response");
		#endif
		return false;
	}

	tzIana = tz;

	// Controleer of beide keys bestaan
	if (!doc.containsKey("raw_offset") || !doc.containsKey("dst_offset"))
	{
		#ifdef DEBUG_TZ
		Serial.println("[TZ] Missing raw_offset/dst_offset in WorldTimeAPI response");
		#endif
		return false;
	}

	// Lees offsets direct als integers
	gmtOffsetSec = doc["raw_offset"] | 0;
	daylightOffsetSec = doc["dst_offset"] | 0;

	#ifdef DEBUG_TZ
	Serial.printf("[TZ] From IP: tz=%s raw=%d dst_off=%d\n", tzIana.c_str(), gmtOffsetSec, daylightOffsetSec);
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

	// 1) Probeer IANA TZ te zetten (voor wie dit ondersteunt)
	if (tzIana.length() > 0)
	{
		setenv("TZ", tzIana.c_str(), 1);
		tzset();
		#ifdef DEBUG_TZ
		Serial.printf("[TIME] TZ set to IANA: %s\n", tzIana.c_str());
		#endif
	}

	// 2) **Belangrijk**: configureer SNTP inclusief offsets zodat tijd lokaal klopt
	//    (werkt ook wanneer IANA TZ niet wordt ondersteund).
	configTime(gmtOffset, dstOffset,
		"europe.pool.ntp.org",
		"time.google.com",
		"pool.ntp.org");

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

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include "net_time.h"
#include <WiFi.h>
#include "lwip/apps/sntp.h"
#include "hal_time_freertos.h"

// Preview helper for HTTP payloads (guarded by DEBUG_TZ)
void dumpPreview(const String& payload)
{
	#ifdef DEBUG_TZ
	int n = payload.length();
	int k = n > 160 ? 160 : n;
	String head = payload.substring(0, k);
	String tmp = head;
	tmp.replace("\n", "\\n");
	tmp.replace("\r", "\\r");
	Serial.printf("[HTTP] len=%d preview=%s%s\n", n, tmp.c_str(), (n > k ? "..." : ""));
	#endif
}


// Very small/robust extractor for flat JSON (string or numeric/bool token)
String extractJsonString(const String& json, const String& key)
{
	String pat = "\"" + key + "\":";
	int i = json.indexOf(pat);
	if (i < 0) return "";
	i += pat.length();
	while (i < (int)json.length() && (json[i] == ' ' || json[i] == '\t')) i++;

	// Case 1: quoted string value
	if (i < (int)json.length() && json[i] == '\"')
	{
		int s = i + 1;
		int e = json.indexOf('\"', s);
		if (e > s) return json.substring(s, e);
		return "";
	}

	// Case 2: raw token (number, true/false/null) until ',' or '}'
	int e = json.indexOf(',', i);
	if (e < 0) e = json.indexOf('}', i);
	if (e < 0) e = json.length();
	String v = json.substring(i, e);
	v.trim();
	return v;
}


// worldtimeapi.org: timezone (string), raw_offset (seconds), dst_offset (seconds)
// ip-api.com: countryCode (string)
bool fetchTimeInfo(String& tzIana, int& gmtOffsetSec, int& daylightOffsetSec, bool acceptAllHttps)
{
	WiFiClientSecure client;
	if (acceptAllHttps)
	{
		client.setInsecure();
	}

	HTTPClient http;
	if (!http.begin(client, URL_TIMEINFO))
	{
		#ifdef DEBUG_TZ
		Serial.println("[TZ] http.begin failed");
		#endif
		return false;
	}
	http.setConnectTimeout(5000);
	http.setTimeout(8000);
	http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

	// Retry time api to make sure it works. Some times it refuses connection.
	int code = -1;
	for (int i = 0;(i < 10 && code < 0);i++)
	{
		code = http.GET();

		// Wait 1 second for retry.
		if (code < 0)
		{
			hal_delay_ms(1000);
		}
	}

	if (code <= 0)
	{
		#ifdef DEBUG_TZ
		Serial.printf("[TZ] HTTP GET failed: %d\n", code);
		Serial.printf("[TZ] Reason: %s\n", HTTPClient::errorToString(code).c_str());
		Serial.printf("[TZ] URL: %s\n", URL_TIMEINFO);
		Serial.printf("[TZ] WiFi status: %d (WL_CONNECTED=%d)\n", WiFi.status(), WL_CONNECTED);
		Serial.printf("[TZ] Local IP: %s DNS: %s Gateway: %s\n",
			WiFi.localIP().toString().c_str(),
			WiFi.dnsIP().toString().c_str(),
			WiFi.gatewayIP().toString().c_str());
		#endif
		http.end();
		return false;
	}

	if (code != HTTP_CODE_OK)
	{
		#ifdef DEBUG_TZ
		Serial.printf("[TZ] HTTP status: %d (expected 200)\n", code);
		#endif
		http.end();
		return false;
	}

	//  Eerst payload ophalen, dan pas parsen
	String payload = http.getString();
	http.end();
	dumpPreview(payload);

	// JSON waarden parsen
	String tz  = extractJsonString(payload, "timezone");
	String raw = extractJsonString(payload, "raw_offset");
	String dst = extractJsonString(payload, "dst_offset");

	if (tz.length() == 0)
	{
		#ifdef DEBUG_TZ
		Serial.println("[TZ] timezone missing in payload");
		#endif
		return false;
	}

	// Output params vullen
	tzIana = tz;
	gmtOffsetSec = raw.length() ? raw.toInt() : 0;
	daylightOffsetSec = dst.length() ? dst.toInt() : 0;
	return true;
}


String fetchCountryCode()
{
	HTTPClient http;
	if (!http.begin(URL_COUNTRY))
	{
		return "";
	}
	int code = http.GET();
	if (code <= 0) { http.end(); return ""; }
	String payload = http.getString();
	http.end();
	dumpPreview(payload);
	String cc = extractJsonString(payload, "countryCode");
	return cc;
}


bool setupTimeFromInternet(bool acceptAllHttps)
{
	String tz;
	int gmt = 0, dst = 0;
	if (!fetchTimeInfo(tz, gmt, dst, acceptAllHttps))
	{
		#ifdef DEBUG_TZ
		Serial.println("[TZ] fetchTimeInfo failed");
		#endif
		#ifdef DEBUG_TZ
		Serial.println("[TZ] worldtimeapi failed \342\200\224 fallback to configTzTime for Europe/Amsterdam");
		#endif

		// POSIX TZ for Europe/Amsterdam (CET/CEST with EU DST rules)
		configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", NTP1, NTP2, NTP3);
	}

	// Configure NTP using offsets (ESP32 time.h)
	configTime(gmt, dst, NTP1, NTP2, NTP3);

	#ifdef DEBUG_TZ
	Serial.printf("\r[TZ] configTime(gmt=%d, dst=%d) with tz=%s\r\n", gmt, dst, tz.c_str());
	#endif

	time_t now = 0;
	for (int i = 0; i < 20; ++i)
	{
		now = time(nullptr);

		if (now > 8 * 3600)
		{
			// success: time synced
			break;
		}

		// Every 5th attempt (5 and 10), "restart" SNTP in a portable way
		if ((i + 1) % 5 == 0)
		{
			#ifdef DEBUG_TZ
			Serial.printf("\r[TZ] Re-kicking SNTP at attempt %d via configTime()\n", i + 1);
			#endif
			configTime(gmt, dst, NTP1, NTP2, NTP3);
		}

		hal_delay_ms(500);
	}

	#ifdef DEBUG_TZ
	if (now > 8 * 3600)
	{
		struct tm t; localtime_r(&now, &t);
		Serial.printf("\r[TZ] time synced: %04d-%02d-%02d %02d:%02d:%02d\r\n", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	}
	else
	{
		Serial.println("\r[TZ] time sync timeout\n");
	}
	#endif
	return now > 8 * 3600;
}


// Periodic time maintenance: if time seems unsynced (epoch too small) retry NTP setup.
// Called from task_time loop.
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

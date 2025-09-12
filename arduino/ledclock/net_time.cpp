#include <Arduino.h>
#include <HTTPClient.h>
#include <time.h>
#include "net_time.h"

String extractJsonString(const String& json, const String& key)
{
	String pat="\""+key+"\":";
	int i=json.indexOf(pat); if(i<0) return ""; i+=pat.length();
	while(i<(int)json.length() && json[i]==' ') i++;
	if(i<(int)json.length() && json[i]=='\"'){ int s=i+1; int e=json.indexOf('\"',s); if(e>s) return json.substring(s,e); }
	int e=json.indexOf(',',i); if(e<0) e=json.indexOf('}

void dumpPreview(const String& payload)
{
	if(!DEBUG_TZ) return;
								 // iets ruimer voor debug
	int n=payload.length(); int k = n>160 ? 160 : n;
	String head = payload.substring(0,k); head.replace("\n","\\n"); head.replace("\r","\\r");
	Serial.printf("[HTTP] len=%d preview=%s%s\n", n, head.c_str(), (n>k?"...":""));
}

bool fetchTimeInfo(String& tzIana, int& gmtOffsetSec, int& daylightOffsetSec)
{
	if(DEBUG_TZ) Serial.printf("[TZ] GET %s\n", URL_TIMEINFO);
	HTTPClient http;
	if(!http.begin(URL_TIMEINFO)){ Serial.println("[TZ] http.begin() fail"); return false; }
	int code=http.GET(); if(DEBUG_TZ) Serial.printf("[TZ] HTTP %d\n", code);
	if(code!=200){ http.end(); return false; }
	String payload=http.getString(); http.end(); dumpPreview(payload);

	String tz = extractJsonString(payload,"timezone");
								 // "+01:00"
	String utcOff = extractJsonString(payload,"utc_offset");
								 // true/false
	String dstRaw = extractJsonString(payload,"dst");
	if(tz.isEmpty() || utcOff.length()<3){ Serial.println("[TZ] incomplete response"); return false; }

	int sign = (utcOff[0]=='-')?-1:1;
	int hh = utcOff.substring(1,3).toInt();
	int mm = (utcOff.length()>=6 && utcOff[3]==':') ? utcOff.substring(4,6).toInt() : 0;
	gmtOffsetSec = sign*(hh*3600 + mm*60);
	bool dst = dstRaw.indexOf("true")>=0;
	daylightOffsetSec = dst ? 3600 : 0;

	tzIana = tz;
	if(DEBUG_TZ) Serial.printf("[TZ] Parsed tz=%s offset=%s => gmtOffsetSec=%d dst=%s(daylight=%d)\n",
			tz.c_str(), utcOff.c_str(), gmtOffsetSec, dst?"true":"false", daylightOffsetSec);
	return true;
}

String fetchCountryCode()
{
	if(DEBUG_TZ) Serial.printf("[Geo] GET %s\n", URL_COUNTRY);
	HTTPClient http;
	if(!http.begin(URL_COUNTRY)){ Serial.println("[Geo] http.begin() fail"); return ""; }
	int code=http.GET(); if(DEBUG_TZ) Serial.printf("[Geo] HTTP %d\n", code);
	if(code!=200){ http.end(); return ""; }
	String payload=http.getString(); http.end(); dumpPreview(payload);
	String cc = extractJsonString(payload,"countryCode"); cc.trim();
	if(DEBUG_TZ) Serial.printf("[Geo] countryCode=%s\n", cc.c_str());
	return cc;
}

bool setupTimeFromInternet()
{
	// 1) TZ ophalen
	if(!fetchTimeInfo(g_timezoneIANA, g_gmtOffsetSec, g_daylightSec)){ Serial.println("[NTP] TZ fetch failed"); return false; }

	// 2) NTP instellen met offsets  lokale tijd via getLocalTime()
	if(DEBUG_TZ) Serial.printf("[NTP] configTime(gmt=%d, dst=%d, servers=[%s,%s])\n",
			g_gmtOffsetSec, g_daylightSec, NTP1, NTP2);
	configTime(g_gmtOffsetSec, g_daylightSec, NTP1, NTP2);

	// 3) Wachten op sync
	struct tm t; g_timeReady=false;
	for(int i=1;i<=25;i++)
	{
		if(getLocalTime(&t, 400)){ g_timeReady=true; if(DEBUG_TZ) Serial.printf("[NTP] sync OK (try %d)\n", i); break; }
		delay(200); if(DEBUG_TZ) Serial.printf("[NTP] waiting... (%d)\n", i);
	}
	// 4) (Optioneel) country code
	g_countryCode = fetchCountryCode();

	if(DEBUG_TZ)
	{
		if(g_timeReady)
		{
			char buf[64]; strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",&t);
			Serial.printf("[Time] local=%s | TZ=%s | CC=%s\n", buf,
				g_timezoneIANA.c_str(),
				g_countryCode.length()? g_countryCode.c_str():"(?)");
		}
		else
		{
			Serial.println("[NTP] sync failed");
		}
	}
	return g_timeReady;
}
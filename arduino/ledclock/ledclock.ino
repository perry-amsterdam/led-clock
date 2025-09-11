Using configuration file at "/etc/bcpp/bcpp.cfg"
Using configuration file at "/etc/bcpp/bcpp.cfg"
// ==== ESP32: TZ bij opstart + datum/tijd via NTP ====
// - Verbindt met Wi-Fi of start captive portal
// - Bij succesvolle Wi-Fi: vraagt timezone op (HTTP) en stelt NTP in
// - Print datum/tijd periodiek (standaard 5 min) naar de Serial console
// - WS2812 status-LED (1 pixel): blauw=verbinden, rood=geen Wi-Fi/AP, groen=verbonden

#define DEBUG_TZ 1
#define DEBUG_NET 1

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <Adafruit_NeoPixel.h>
#include <HTTPClient.h>
#include <time.h>

// ---------- Instellingen ----------
#define LED_PIN    48			 // < PAS AAN (vaak 8 of 48)
#define LED_COUNT  1
const uint8_t LED_BRIGHTNESS = 50;
const unsigned long TIME_PRINT_INTERVAL_SEC = 300;

const char* PREF_NS  = "wifi";
const char* AP_SSID  = "ESP32-Setup";
const char* AP_PASS  = "configwifi";
const byte  DNS_PORT = 53;
IPAddress   AP_IP(192,168,4,1), AP_GW(192,168,4,1), AP_MASK(255,255,255,0);

// Endpoints en NTP servers
								 // levert timezone + UTC offset + DST
const char* URL_TIMEINFO = "http://worldtimeapi.org/api/ip";
								 // optioneel: countryCode
const char* URL_COUNTRY  = "http://ip-api.com/json";
const char* NTP1 = "pool.ntp.org";
const char* NTP2 = "time.nist.gov";

// ---------- Globals ----------
Adafruit_NeoPixel pixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Preferences prefs;
WebServer server(80);
DNSServer dns;

String savedSsid, savedPass;
String g_timezoneIANA = "";
String g_countryCode  = "";
int    g_gmtOffsetSec = 0;
int    g_daylightSec  = 0;
bool   g_timeReady    = false;
unsigned long lastPrintMs = 0;

// ---------- WS2812 helpers ----------
void ledBegin(){ pixel.begin(); pixel.setBrightness(LED_BRIGHTNESS); pixel.clear(); pixel.show(); }
void ledColor(uint8_t r,uint8_t g,uint8_t b){ pixel.setPixelColor(0, pixel.Color(r,g,b)); pixel.show(); }
void ledBlue(){   if(DEBUG_NET) Serial.println("[LED] Blauw (verbinden)");    ledColor(0,0,60); }
void ledRed(){    if(DEBUG_NET) Serial.println("[LED] Rood (geen WiFi/AP)");  ledColor(60,0,0); }
void ledGreen(){  if(DEBUG_NET) Serial.println("[LED] Groen (verbonden)");    ledColor(0,60,0); }
void ledOff(){    if(DEBUG_NET) Serial.println("[LED] Uit");                  ledColor(0,0,0); }

// ---------- Portal (simpel) ----------
String htmlWrap(const String& body)
{
	return String(F(
		"<!doctype html><html><head><meta charset='utf-8'/>"
		"<meta name='viewport' content='width=device-width,initial-scale=1'/>"
		"<title>ESP32 Wi-Fi setup</title>"
		"<style>body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial;"
		"max-width:520px;margin:2rem auto;padding:0 1rem}"
		"form{display:grid;gap:.75rem}input,button{font-size:1rem;padding:.6rem}"
		"button{cursor:pointer}</style></head><body>"
		)) + body + F("</body></html>");
}


void handleRoot()
{
	if(DEBUG_NET) Serial.println("[HTTP] GET /");
	server.send(200,"text/html", htmlWrap(
		"<h1>ESP32 Wi-Fi configuratie</h1>"
		"<form method='POST' action='/save'>"
		"<label>SSID<br><input name='ssid' required></label>"
		"<label>Wachtwoord<br><input name='pass' type='password' required></label>"
		"<button type='submit'>Opslaan</button></form>"
		"<p><a href='/reset'>Opgeslagen gegevens wissen</a></p>"));
}


void handleSave()
{
	if(DEBUG_NET) Serial.println("[HTTP] POST /save");
	if(!server.hasArg("ssid")||!server.hasArg("pass")){ server.send(400,"text/plain","Ontbrekende velden"); return; }
	prefs.begin(PREF_NS,false);
	prefs.putString("ssid", server.arg("ssid"));
	prefs.putString("pass", server.arg("pass"));
	prefs.end();
	if(DEBUG_NET) Serial.printf("[WiFi] Stored SSID='%s' len(pass)=%u\n", server.arg("ssid").c_str(), server.arg("pass").length());
	server.send(200,"text/html",htmlWrap("<h1>Opgeslagen \342\234\205</h1><p>Herstart...</p>"));
	delay(500); ESP.restart();
}


void handleReset()
{
	if(DEBUG_NET) Serial.println("[HTTP] GET /reset -> clear creds");
	prefs.begin(PREF_NS,false); prefs.clear(); prefs.end();
	server.send(200,"text/html",htmlWrap("<h1>Gewist \342\234\205</h1><p>Herstart...</p>"));
	delay(500); ESP.restart();
}


static bool isIpLike(const String& h)
{
	int dots=0; for(char c: h)
	{
		if(c=='.') dots++; else if(!isDigit(c)) return false;
	}
	return dots==3;
}


void handleNotFound()
{
	if(DEBUG_NET) Serial.printf("[HTTP] 404 %s -> redirect /\n", server.uri().c_str());
	if(!isIpLike(server.hostHeader()) && server.hostHeader()!=AP_IP.toString())
	{
		String loc = "http://" + AP_IP.toString(); server.sendHeader("Location", loc, true); server.send(302,"text/plain",""); return;
	}
	handleRoot();
}


void startPortal()
{
	if(DEBUG_NET) Serial.println("[Portal] Start");
	WiFi.mode(WIFI_AP);
	WiFi.softAPConfig(AP_IP,AP_GW,AP_MASK);
	WiFi.softAP(AP_SSID,AP_PASS);
	dns.start(DNS_PORT,"*",AP_IP);
	server.on("/",HTTP_GET,handleRoot);
	server.on("/save",HTTP_POST,handleSave);
	server.on("/reset",HTTP_GET,handleReset);
	server.onNotFound(handleNotFound);
	server.begin();
	ledRed();
	if(DEBUG_NET) Serial.printf("[Portal] SSID '%s' -> http://%s\n", AP_SSID, AP_IP.toString().c_str());
}


void stopPortal(){ server.stop(); dns.stop(); WiFi.softAPdisconnect(true); if(DEBUG_NET) Serial.println("[Portal] Gestopt"); }

// ---------- JSON mini helpers ----------
String extractJsonString(const String& json, const String& key)
{
	String pat="\""+key+"\":";
	int i=json.indexOf(pat); if(i<0) return ""; i+=pat.length();
	while(i<(int)json.length() && json[i]==' ') i++;
	if(i<(int)json.length() && json[i]=='\"'){ int s=i+1; int e=json.indexOf('\"',s); if(e>s) return json.substring(s,e); }
	int e=json.indexOf(',',i); if(e<0) e=json.indexOf('}',i); if(e>i) return json.substring(i,e);
	return "";
}


void dumpPreview(const String& payload)
{
	if(!DEBUG_TZ) return;
								 // iets ruimer voor debug
	int n=payload.length(); int k = n>160 ? 160 : n;
	String head = payload.substring(0,k); head.replace("\n","\\n"); head.replace("\r","\\r");
	Serial.printf("[HTTP] len=%d preview=%s%s\n", n, head.c_str(), (n>k?"...":""));
}


// ---------- Timezone + NTP (bij opstarten na Wi-Fi) ----------
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


// ---------- Wi-Fi ----------
bool connectWiFi(const String& ssid, const String& pass, uint32_t timeoutMs=15000)
{
	if(ssid.isEmpty()){ Serial.println("[WiFi] geen opgeslagen SSID"); return false; }
	if(DEBUG_NET) Serial.printf("[WiFi] verbinden met SSID='%s'...\n", ssid.c_str());
	ledBlue();
	WiFi.mode(WIFI_STA);
	WiFi.disconnect(true,true);
	delay(50);
	WiFi.begin(ssid.c_str(), pass.c_str());

	uint32_t start=millis(); bool blink=false;
	while(WiFi.status()!=WL_CONNECTED && (millis()-start)<timeoutMs)
	{
		blink=!blink; if(blink) ledBlue(); else ledOff();
		delay(300); Serial.print('.');
	}
	Serial.println();

	if(WiFi.status()==WL_CONNECTED)
	{
		if(DEBUG_NET){ Serial.print("[WiFi] OK IP="); Serial.println(WiFi.localIP()); }
		ledGreen(); return true;
	}
	Serial.println("[WiFi] FAILED"); ledRed(); return false;
}


// ---------- Setup/Loop ----------
void setup()
{
	Serial.begin(115200);
	delay(200);
	Serial.println("\n[Boot] Start");

	ledBegin(); ledBlue();

	prefs.begin(PREF_NS,true);
	savedSsid = prefs.getString("ssid","");
	savedPass = prefs.getString("pass","");
	prefs.end();
	if(DEBUG_NET) Serial.printf("[Boot] stored SSID='%s' len(pass)=%u\n", savedSsid.c_str(), savedPass.length());

	if(connectWiFi(savedSsid, savedPass))
	{
		// >>> HIER: TZ opvragen en NTP instellen bij opstart <<<
		if(setupTimeFromInternet())
		{
			lastPrintMs = millis();
		}
		else
		{
			Serial.println("[Boot] NTP/TZ setup mislukt");
		}
	}
	else
	{
		startPortal();
	}
}


void loop()
{
	// Portal afhandelen als AP actief is
	if(WiFi.getMode()==WIFI_AP || WiFi.getMode()==WIFI_AP_STA)
	{
		dns.processNextRequest(); server.handleClient();
	}

	// Periodiek tijd printen (alleen als NTP ok)
	if(WiFi.status()==WL_CONNECTED && g_timeReady)
	{
		unsigned long nowMs = millis();
		if(nowMs - lastPrintMs >= TIME_PRINT_INTERVAL_SEC*1000UL)
		{
			lastPrintMs = nowMs;
			struct tm t; if(getLocalTime(&t))
			{
				char buf[64]; strftime(buf,sizeof(buf),"%Y-%m-%d %H:%M:%S",&t);
				time_t epoch = time(nullptr);
				Serial.printf("[Time] %s | epoch=%ld | TZ=%s | CC=%s\n",
					buf, (long)epoch,
					g_timezoneIANA.c_str(),
					g_countryCode.length()? g_countryCode.c_str():"(?)");
			}
			else
			{
				Serial.println("[Time] getLocalTime() fail \342\200\224 probeer resync?");
			}
		}
	}
}

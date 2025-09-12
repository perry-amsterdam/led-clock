#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "portal.h"
#include "status_led.h"

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
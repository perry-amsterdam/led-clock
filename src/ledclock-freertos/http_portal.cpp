#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "status_led.h"
#include "hal_time_freertos.h"
#include "rtos.h"
#include "http_api.h"
#include "http_portal.h"

extern "C"
{
	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"
	#include "task_mdns.h"
}


static TaskHandle_t s_portalTask = nullptr;
static volatile bool s_portalOn = false;
static void portalTask(void* pvParameters);

String htmlWrap(const String& body)
{
	return String(F(
		"<!doctype html><html lang='nl'><head><meta charset='utf-8'/>"
		"<meta name='viewport' content='width=device-width,initial-scale=1'/>"
		"<title>ESP32 Wi-Fi setup</title>"
		"<style>"
		":root{--ikea-blue:#0058A3;--ikea-yellow:#FFDA1A;--text:#111;--muted:#666;--bg:#f6f7f9}"
		"*{box-sizing:border-box}html,body{height:100%}"
		"body{margin:0;font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial,sans-serif;"
		"background:var(--bg);color:var(--text);line-height:1.35}"
		"header{background:var(--ikea-blue);color:#fff;padding:.9rem 1rem}"
		".brand{display:flex;align-items:center;gap:.6rem;font-weight:700}"
		".badge{display:inline-block;background:var(--ikea-yellow);color:#111;"
		"padding:.15rem .45rem;border-radius:.3rem;font-size:.85rem}"
		"main{max-width:540px;margin:1.25rem auto;padding:0 1rem}"
		".card{background:#fff;border-radius:.8rem;box-shadow:0 8px 24px rgba(0,0,0,.06);"
		"padding:1rem .9rem}"
		"h1{margin:.2rem 0 1rem 0;font-size:1.4rem}"
		"p.lead{margin:.25rem 0 1rem 0;color:var(--muted);font-size:.95rem}"
		"form{display:grid;gap:.75rem}"
		"label{font-size:.9rem;font-weight:600}"
		".field{display:grid;gap:.35rem}"
		"input{width:100%;font-size:1rem;padding:.7rem .8rem;border:1px solid #dcdfe3;"
		"border-radius:.55rem;background:#fff}"
		"input:focus{outline:none;border-color:var(--ikea-blue);"
		"box-shadow:0 0 0 3px rgba(0,88,163,.15)}"
		".row{display:flex;gap:.6rem;flex-wrap:wrap}"
		"button,.btn{appearance:none;border:0;cursor:pointer;font-weight:700;"
		"padding:.75rem 1rem;border-radius:.6rem;font-size:1rem}"
		".btn-primary{background:var(--ikea-blue);color:#fff}"
		".btn-primary:active{transform:translateY(1px)}"
		".btn-ghost{background:transparent;color:var(--ikea-blue)}"
		".btn-ghost:active{background:rgba(0,88,163,.08)}"
		".danger{color:#b00020}"
		"a{color:var(--ikea-blue);text-decoration:none}"
		"a:hover{text-decoration:underline}"
		"footer{margin:1.25rem auto;max-width:540px;padding:0 1rem;color:var(--muted);font-size:.85rem}"
		".note{font-size:.85rem;color:var(--muted)}"
		".space{height:.2rem}"
		".kicker{display:inline-flex;align-items:center;gap:.4rem;"
		"font-size:.78rem;color:#fff;opacity:.9}"
		".kdot{width:.4rem;height:.4rem;border-radius:50%;background:var(--ikea-yellow)}"
		"@media (min-width:640px){.card{padding:1.2rem 1.1rem}}"
		"</style></head><body>"
		"<header><div class='brand'>"
		"<span class='badge'>IKEA-style</span>"
		"<span>ESP32 configuratie</span>"
		"</div>"
		"<div class='kicker'><span class='kdot'></span><span>Wi-Fi &amp; systeeminstellingen</span></div>"
		"</header>"
		"<main><div class='card'>"
		)) + body + F(
		"</div></main>"
		"<footer>ESP32 Wi-Fi setup &middot; <span class='note'>lokale configuratiepagina</span></footer>"
		"</body></html>"
		);
}


void handleRoot()
{
	if(DEBUG_NET) Serial.println("[HTTP] GET /");

	// Prefill from NVS
	prefs.begin(PREF_NS, true);
	String savedSsid = prefs.getString("ssid", "");
	prefs.end();

	String page;
	page.reserve(3000);

	page += F(
		"<h1>Wi-Fi configuratie</h1>"
		"<p class='lead'>Verbind de klok met je draadloze netwerk. Je gegevens worden lokaal opgeslagen (NVS).</p>"
		"<form method='POST' action='/save' autocomplete='on'>"

		"<div class='field'>"
		"<label for='ssid'>SSID</label>"
		"<input id='ssid' name='ssid' placeholder='Bijv. MijnNetwerk' autocapitalize='none'/>"
		"</div>"

		"<div class='field'>"
		"<label for='pass'>Wachtwoord</label>"
		"<input id='pass' name='pass' type='password' placeholder='\342\200\242\342\200\242\342\200\242\342\200\242\342\200\242\342\200\242\342\200\242\342\200\242\342\200\242\342\200\242\342\200\242\342\200\242\342\200\242\342\200\242\342\200\242\342\200\242'/>"
		"<div class='row note'>"
		"<label style='display:flex;align-items:center;gap:.4rem'>"
		"<input type='checkbox' id='showpw' style='width:auto'>"
		"Wachtwoord tonen"
		"</label>"
		"</div>"
		"</div>"
		);

	page += F("<div class='field'>"
		"</div>");

	// Action buttons
	page += F(
		"<div class='row'>"
		"<button class='btn btn-primary' type='submit'>Opslaan</button>"
		"<a class='btn btn-ghost' href='/reset'>Opgeslagen gegevens wissen</a>"
		"</div>"

		"<div class='space'></div>"
		"<p class='note'>Tip: na opslaan wordt de ESP32 opnieuw verbonden met het gekozen netwerk.</p>"
		"</form>"
		);

	// In de buttons-row:
	page += F("<button class='btn-ghost' type='button' id='rebootBtn'>Herstart apparaat</button>");

	// Script to prefill SSID and wire up UI logic
	page += F("<script>");

	// Prefill SSID (escape simple quotes/backslashes)
	String esc = savedSsid; esc.replace("\\", "\\\\"); esc.replace("'", "\\'");
	page += "var ss=document.getElementById('ssid'); if(ss){ ss.value='";
	page += esc;
	page += "'; }";

	// Password show/hide
	page += "var s=document.getElementById('showpw'),p=document.getElementById('pass');"
		"if(s&&p){s.addEventListener('change',function(){p.type=this.checked?'text':'password';});}";

	// Reboot-knop (bevestiging + POST)
	page += F("document.getElementById('rebootBtn').addEventListener('click',function(){"
		" if(!confirm('Zeker weten dat je het apparaat wilt herstarten?')) return;"
		" fetch('/reboot',{method:'POST'}).then(function(r){return r.text();}).then(function(html){"
		"   document.open(); document.write(html); document.close();"
		" }).catch(function(){ alert('Kon herstart niet triggeren'); });"
		"});");

	page += F("</script>");

	server.send(200,"text/html", htmlWrap(page));
}


void handleSave()
{
	if(DEBUG_NET) Serial.println("[HTTP] POST /save");

	if(!server.hasArg("ssid") || !server.hasArg("pass"))
	{
		server.send(400,"text/plain","Ontbrekende velden");
		return;
	}

	prefs.begin(PREF_NS,false);
	prefs.putString("ssid", server.arg("ssid"));
	prefs.putString("pass", server.arg("pass"));
	prefs.end();

	hal_delay_ms(500); ESP.restart();
}


void handleReset()
{
	if(DEBUG_NET) Serial.println("[HTTP] GET /reset -> clear creds");
	prefs.begin(PREF_NS,false); prefs.clear(); prefs.end();
	server.send(200,"text/html",htmlWrap("<h1>Gewist \342\234\205</h1><p>Herstart...</p>"));
	hal_delay_ms(500); ESP.restart();
}


void handleReboot()
{
	if (DEBUG_NET) Serial.println("[HTTP] POST /reboot -> restarting device");

	// Toon meteen feedback aan de gebruiker
	String body = F("<h1>Herstarten\342\200\246</h1><p>De klok start nu opnieuw op.</br>"
		"Je verliest zo de verbinding met dit netwerk (AP) of de portal.</p>");

	// Connection: close voorkomt dat de browser de socket open probeert te houden
	server.sendHeader("Connection", "close");
	server.send(200, "text/html", htmlWrap(body));

	// Restart in een korte, uitgestelde FreeRTOS-task zodat de response eerst de lucht in gaat
	vTaskDelay(pdMS_TO_TICKS(800));
	ESP.restart();
}


bool isIpLike(const String& h)
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
		String loc = "http://" + AP_IP.toString();
		server.sendHeader("Location", loc, true); 
		server.send(302,"text/plain",""); return;
	}
	handleRoot();
}


void startPortal()
{
	// --- Ensure portal task is running ---
	startPortalTask();

	/*__STARTPORTAL_MUTEX__*/
	stopApi();
	xEventGroupSetBits(g_sysEvents, EVT_PORTAL_ON);

	// --- mDNS (AP/portal) ---
	// Laat clients http://ledclock.local gebruiken tijdens portal
	mdnsStop();
	if (!mdnsStart("ledclock"))
	{
		Serial.println("[mDNS] AP mDNS start failed");
	}
	else
	{
		Serial.println("[mDNS] AP mDNS ledclock.local");
		mdnsAddHttpService(80);
	}

	if(DEBUG_NET) Serial.println("[Portal] Start");

	// Start wifi access point.
	WiFi.mode(WIFI_AP);
	WiFi.softAPConfig(AP_IP,AP_GW,AP_MASK);
	WiFi.softAP(AP_SSID,AP_PASS);

	// Start local dns server.
	dns.start(DNS_PORT,"*",AP_IP);

	// Start portal server to add wifi credentials.
	server.on("/",HTTP_GET,handleRoot);
	server.on("/save",HTTP_POST,handleSave);
	server.on("/reset",HTTP_GET,handleReset);
	server.on("/scan", HTTP_GET,handleScan);
	server.on("/reboot", HTTP_POST, handleReboot);
	server.onNotFound(handleNotFound);
	server.begin();

	if(DEBUG_NET) Serial.printf("[Portal] SSID '%s' -> http://%s\n", AP_SSID, AP_IP.toString().c_str());
}


void stopPortal()
{
	// --- Request portal task to stop and wait briefly ---
	s_portalOn = false;
	stopPortalTask();

	/*__STOPPORTAL_BITS__*/
	xEventGroupClearBits(g_sysEvents, EVT_PORTAL_ON);

	server.stop();
	dns.stop();
	mdnsStop();
	WiFi.softAPdisconnect(true);
	if(DEBUG_NET)
	{
		Serial.println("[Portal] Gestopt");
	}
}


// ---------- Nieuwe functie: handleScan ----------
// Retourneert JSON array: [{ "ssid":"...", "rssi":-60, "enc":"WPA2" }, ...]
void handleScan()
{
	if (DEBUG_NET) Serial.println("[HTTP] GET /scan -> starting wifi scan");

	// Zet gateway/AP mode tijdelijk niet nodig  de AP draait al wanneer portal on
	// Gebruik blocking scan (scheelt complexiteit). Dit kan een paar 100..1000 ms duren.
	// blocking, include hidden
	int n = WiFi.scanNetworks(false, true);

	// WiFi.scanNetworks() returns number of networks found
	String json = "[";
	for (int i = 0; i < n; ++i)
	{
		if (i) json += ",";
		String ssid = WiFi.SSID(i);
		int rssi = WiFi.RSSI(i);
		// ENC_TYPE_NONE, ENC_TYPE_WEP, ENC_TYPE_TKIP, ENC_TYPE_CCMP, ENC_TYPE_AUTO
		int enc = WiFi.encryptionType(i);
		const char *encstr = "UNKNOWN";
		switch (enc)
		{
			case WIFI_AUTH_OPEN: encstr = "OPEN"; break;
			case WIFI_AUTH_WEP: encstr = "WEP"; break;
			case WIFI_AUTH_WPA_PSK: encstr = "WPA-PSK"; break;
			case WIFI_AUTH_WPA2_PSK: encstr = "WPA2-PSK"; break;
			case WIFI_AUTH_WPA_WPA2_PSK: encstr = "WPA/WPA2-PSK"; break;
			case WIFI_AUTH_WPA2_ENTERPRISE: encstr = "WPA2-ENT"; break;
			default: encstr = "UNKNOWN"; break;
		}

		// Basic JSON escaping for SSID
		String esc = ssid;
		esc.replace("\\", "\\\\");
		esc.replace("\"", "\\\"");
		json += "{";
		json += "\"ssid\":\"" + esc + "\"";
		json += ",\"rssi\":" + String(rssi);
		json += ",\"enc\":\"" + String(encstr) + "\"";
		json += "}";
	}
	json += "]";

	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.send(200, "application/json", json);

	if (DEBUG_NET) Serial.printf("[HTTP] /scan returned %d AP(s)\n", n);
}

// ======================================================
// HTTP task (FreeRTOS)
// ======================================================
static void portalTask(void*)
{
	while (s_portalOn)
	{
		dns.processNextRequest();
		server.handleClient();
		vTaskDelay(pdMS_TO_TICKS(10));
	}

	s_portalTask = nullptr;
	vTaskDelete(nullptr);
}


void startPortalTask()
{	
	if (s_portalTask == nullptr)
	{
		BaseType_t ok = xTaskCreate( portalTask, "task_portal", 4096, nullptr, tskIDLE_PRIORITY + 2, &s_portalTask);
		if (ok != pdPASS)
		{
			s_portalTask = nullptr;
		}
		Serial.println("[Portal] Task gestart");
	}
}


void stopPortalTask()
{
	if (s_portalTask != nullptr)
	{
		TaskHandle_t t = s_portalTask;
		s_portalTask = nullptr;
		vTaskDelete(t);
		Serial.println("[Portal] Task gestopt");
	}
}

//	TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(250);
//	while (s_portalTask != nullptr && xTaskGetTickCount() < deadline)
//	{
//		vTaskDelay(pdMS_TO_TICKS(10));
//	}
//	if (s_portalTask != nullptr)
//	{
//		vTaskDelete(s_portalTask);
//		s_portalTask = nullptr;
//	}


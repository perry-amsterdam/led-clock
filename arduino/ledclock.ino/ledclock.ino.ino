// ==== ESP32: Minimal Wi-Fi Test met Captive Portal (zonder FreeRTOS) ====
// - Probeert verbinden met opgeslagen Wi-Fi
// - Faalt? Start Access Point + webportal om SSID/WPA in te voeren
// - Na opslaan -> reboot en verbindt

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

Preferences prefs;
WebServer server(80);
DNSServer dns;

const char* PREF_NS  = "wifi";
String savedSsid, savedPass;

const char* AP_SSID  = "ESP32-Setup";
const char* AP_PASS  = "configwifi"; // min. 8 chars
const byte  DNS_PORT = 53;
IPAddress   AP_IP(192,168,4,1), AP_GW(192,168,4,1), AP_MASK(255,255,255,0);

// -------- helpers --------
String htmlWrap(const String& body) {
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

void handleRoot() {
  String body = F(
    "<h1>ESP32 Wi-Fi configuratie</h1>"
    "<form method='POST' action='/save'>"
    "<label>SSID<br><input name='ssid' required placeholder='Bijv. MijnWiFi'></label>"
    "<label>Wachtwoord<br><input name='pass' type='password' required></label>"
    "<button type='submit'>Opslaan</button>"
    "</form>"
    "<p><a href='/reset'>Opgeslagen gegevens wissen</a></p>"
  );
  server.send(200, "text/html", htmlWrap(body));
}

void handleSave() {
  if (!server.hasArg("ssid") || !server.hasArg("pass")) {
    server.send(400, "text/plain", "Ontbrekende velden");
    return;
  }
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");

  prefs.begin(PREF_NS, false);
  prefs.putString("ssid", ssid);
  prefs.putString("pass", pass);
  prefs.end();

  server.send(200, "text/html", htmlWrap(
    "<h1>Opgeslagen ✅</h1><p>Herstart en verbinden met <b>" + ssid + "</b>...</p>"
  ));
  delay(600);
  ESP.restart();
}

void handleReset() {
  prefs.begin(PREF_NS, false);
  prefs.clear();
  prefs.end();
  server.send(200, "text/html", htmlWrap("<h1>Gewist ✅</h1><p>Herstart...</p>"));
  delay(600);
  ESP.restart();
}

static bool isIpLike(const String& h) {
  int dots=0;
  for (size_t i=0;i<h.length();i++){
    if (h[i]=='.'){ dots++; continue; }
    if (!isDigit(h[i])) return false;
  }
  return dots==3;
}

void handleNotFound() {
  // Captive portal: alles omleiden naar / als host niet ons AP-IP is
  if (!isIpLike(server.hostHeader()) && server.hostHeader() != AP_IP.toString()) {
    String loc = "http://" + AP_IP.toString();
    server.sendHeader("Location", loc, true);
    server.send(302, "text/plain", "");
    return;
  }
  handleRoot();
}

// -------- Wi-Fi --------
bool connectWiFi(const String& ssid, const String& pass, uint32_t timeoutMs=15000) {
  if (ssid.isEmpty()) return false;

  Serial.printf("Verbinden met SSID='%s'...\n", ssid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  delay(50);
  WiFi.begin(ssid.c_str(), pass.c_str());

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Verbonden! IP: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  Serial.println("Verbinden mislukt.");
  return false;
}

void startPortal() {
  Serial.println("Start AP + portal...");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_GW, AP_MASK);
  if (!WiFi.softAP(AP_SSID, AP_PASS)) {
    Serial.println("AP starten mislukt!");
    return;
  }
  dns.start(DNS_PORT, "*", AP_IP);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/reset", HTTP_GET, handleReset);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.printf("AP actief: %s  (http://%s)\n", AP_SSID, AP_IP.toString().c_str());
}

void stopPortal() {
  server.stop();
  dns.stop();
  WiFi.softAPdisconnect(true);
  Serial.println("Portal gestopt.");
}

// -------- Arduino lifecycle --------
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\nBoot Wi-Fi test...");

  // opgeslagen credentials
  prefs.begin(PREF_NS, true);
  savedSsid = prefs.getString("ssid", "");
  savedPass = prefs.getString("pass", "");
  prefs.end();

  // 1) Probeer Wi-Fi
  if (connectWiFi(savedSsid, savedPass)) {
    Serial.println("Klaar: STA verbonden.");
  } else {
    // 2) Start AP + portal
    startPortal();
  }
}

void loop() {
  // Draai portal alleen als AP actief is
  if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
    dns.processNextRequest();
    server.handleClient();
  }

  // Optioneel: als we intussen verbinding kregen, kun je portal uitzetten:
  if (WiFi.status() == WL_CONNECTED && WiFi.getMode() != WIFI_STA) {
    stopPortal();
    WiFi.mode(WIFI_STA);
    Serial.print("Verbonden! IP: ");
    Serial.println(WiFi.localIP());
  }
}

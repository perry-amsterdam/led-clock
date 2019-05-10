/*
  Simple.ino, Example for the AutoConnect library.
  Copyright (c) 2018, Hieromon Ikasamo
  https://github.com/Hieromon/AutoConnect

  This software is released under the MIT License.
  https://opensource.org/licenses/MIT
*/

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>
#include <ESP8266WebServer.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
#endif
#include <time.h>
#include <AutoConnect.h>
#include <NTPClient.h>
//#include <timezone.h>

const IPAddress remote_ip(8,8,8,8);

// Use these variables to set the initial time
int hours = 12;
int minutes = 0;
int seconds = 0;
bool time_ready = false;

unsigned long lastDraw = 0;

// How fast do you want the clock to spin? Set this to 1 for fun.
// Set this to 1000 to get _about_ 1 second timing.
const int CLOCK_SPEED = 1000;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "sg.pool.ntp.org", 2*60*60, 15*60*1000);  // offset +2 hours, update every 15 min.

static const char AUX_TIMEZONE[] PROGMEM = R"(
{
	"title": "TimeZone",
		"uri": "/timezone",
		"menu": true,
		"element": [
	{
		"name": "caption",
			"type": "ACText",
			"value": "Sets the time zone to get the current local time.",
			"style": "font-family:Arial;font-weight:bold;text-align:center;margin-bottom:10px;color:DarkSlateBlue"
	},
	{
		"name": "timezone",
			"type": "ACSelect",
			"label": "Select TZ name",
			"option": [],
			"selected": 10
	},
	{
		"name": "newline",
			"type": "ACElement",
			"value": "<br>"
	},
	{
		"name": "start",
			"type": "ACSubmit",
			"value": "OK",
			"uri": "/start"
	}
	]
}


)";

typedef struct
{
	const char* zone;
	const char* ntpServer;
	int8_t      tzoff;
} Timezone_t;

static const Timezone_t TZ[] =
{
	{ "Europe/London", "europe.pool.ntp.org", 0 },
	{ "Europe/Berlin", "europe.pool.ntp.org", 1 },
	{ "Europe/Helsinki", "europe.pool.ntp.org", 2 },
	{ "Europe/Moscow", "europe.pool.ntp.org", 3 },
	{ "Asia/Dubai", "asia.pool.ntp.org", 4 },
	{ "Asia/Karachi", "asia.pool.ntp.org", 5 },
	{ "Asia/Dhaka", "asia.pool.ntp.org", 6 },
	{ "Asia/Jakarta", "asia.pool.ntp.org", 7 },
	{ "Asia/Manila", "asia.pool.ntp.org", 8 },
	{ "Asia/Tokyo", "asia.pool.ntp.org", 9 },
	{ "Australia/Brisbane", "oceania.pool.ntp.org", 10 },
	{ "Pacific/Noumea", "oceania.pool.ntp.org", 11 },
	{ "Pacific/Auckland", "oceania.pool.ntp.org", 12 },
	{ "Atlantic/Azores", "europe.pool.ntp.org", -1 },
	{ "America/Noronha", "south-america.pool.ntp.org", -2 },
	{ "America/Araguaina", "south-america.pool.ntp.org", -3 },
	{ "America/Blanc-Sablon", "north-america.pool.ntp.org", -4},
	{ "America/New_York", "north-america.pool.ntp.org", -5 },
	{ "America/Chicago", "north-america.pool.ntp.org", -6 },
	{ "America/Denver", "north-america.pool.ntp.org", -7 },
	{ "America/Los_Angeles", "north-america.pool.ntp.org", -8 },
	{ "America/Anchorage", "north-america.pool.ntp.org", -9 },
	{ "Pacific/Honolulu", "north-america.pool.ntp.org", -10 },
	{ "Pacific/Samoa", "oceania.pool.ntp.org", -11 }
};

#if defined(ARDUINO_ARCH_ESP8266)
ESP8266WebServer Server;
#elif defined(ARDUINO_ARCH_ESP32)
WebServer Server;
#endif

AutoConnect Portal(Server);
AutoConnectConfig Config;
AutoConnectAux Timezone;

// Simple function to increment seconds and then increment minutes
// and hours if necessary.
void updateTime()
{
  if (timeClient.update()) {
    hours = timeClient.getHours();
    minutes = timeClient.getMinutes();
    seconds = timeClient.getSeconds();
    Serial.println(timeClient.getFormattedTime());
    time_ready = true;
  }
}

void rootPage()
{
	String  content =
	"<html>"
	"<head>"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
	"</head>"
	"<body>"
	"<h2 align=\"center\" style=\"color:blue;margin:20px;\">Setup page ledclock</h2>"
	"<h3 align=\"center\" style=\"color:gray;margin:10px;\">{{DateTime}}</h3>"
	"<p style=\"text-align:center;\">Reload the page to update the time.</p>"
	"<p></p><p style=\"padding-top:15px;text-align:center\">" AUTOCONNECT_LINK(COG_24) "</p>"
	"</body>"
	"</html>";

	static const char *wd[7] = { "Sun","Mon","Tue","Wed","Thr","Fri","Sat" };

	struct tm *tm;
	time_t t;
	char dateTime[26];

  // Local time info.
	t = time(NULL);
	tm = localtime(&t);
	sprintf(dateTime, "%04d/%02d/%02d(%s) %02d:%02d:%02d.", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, wd[tm->tm_wday], tm->tm_hour, tm->tm_min, tm->tm_sec);
	content.replace("{{DateTime}}", String(dateTime));
	Server.send(200, "text/html", content);
}

void startPage()
{
	// Retrieve the value of AutoConnectElement with arg function of WebServer class.
	// Values are accessible with the element name.
	String  tz = Server.arg("timezone");

	for (uint8_t n = 0; n < sizeof(TZ) / sizeof(Timezone_t); n++)
	{
		String  tzName = String(TZ[n].zone);
		if (tz.equalsIgnoreCase(tzName))
		{
			configTime(TZ[n].tzoff * 3600, 0, TZ[n].ntpServer);
			Serial.println("Time zone: " + tz);
			Serial.println("ntp server: " + String(TZ[n].ntpServer));
			break;
		}
	}

	// The /start page just constitutes timezone,
	// it redirects to the root page without the content response.
	Server.sendHeader("Location", String("http://") + Server.client().localIP().toString() + String("/"));
	Server.send(302, "text/plain", "");
	Server.client().flush();
	Server.client().stop();
}


void setup()
{
	delay(1000);
	Serial.begin(115200);
	Serial.println();

	// Enable saved past credential by autoReconnect option,
	// even once it is disconnected.
	Config.autoReconnect = true;
	Portal.config(Config);

	// Load aux. page
	Timezone.load(AUX_TIMEZONE);

	// Retrieve the select element that holds the time zone code and
	// register the zone mnemonic in advance.
	AutoConnectSelect& tz = Timezone["timezone"].as<AutoConnectSelect>();
	for (uint8_t n = 0; n < sizeof(TZ) / sizeof(Timezone_t); n++)
	{
		tz.add(String(TZ[n].zone));
	}

  // Register aux. page
	Portal.join(	
	{
		Timezone
	});

	// Behavior a root path of ESP8266WebServer.
	Server.on("/", rootPage);

	// Set NTP server trigger handler
	Server.on("/start", startPage);

	// Establish a connection with an autoReconnect option.
	if (Portal.begin())
	{
		Serial.println("WiFi connected: " + WiFi.localIP().toString());
	}
}


void loop()
{
	Portal.handleClient();

	Serial.println();
	Serial.print("WiFi connected with ip ");
	Serial.println(WiFi.localIP());

	Serial.print("Pinging ip ");
	Serial.println(remote_ip);

	if(Ping.ping(remote_ip))
	{
		Serial.println("Success!!");
	}
	else
	{
		Serial.println("Error :(");
	}
 
  // Check if we need to update seconds, minutes, hours:
  if (lastDraw + CLOCK_SPEED < millis())
  {
    
    lastDraw = millis();
    
    // Add a second, update minutes/hours if necessary:
    updateTime();
    if (time_ready) 
    {
      char buffer[100];
      sprintf(buffer, "Time hour %d, minuts %d, seconds %d", hours, minutes, seconds);
      Serial.println(buffer);
    }
  } 
}
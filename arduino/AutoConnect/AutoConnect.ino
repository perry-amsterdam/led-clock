/*
 led-clock.ino
 Copyright (c) 2019, Perry Couprie
 https://github.com/perry-amsterdam/led-clock
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
#include <SPIFFS.h>
#endif
#include <FS.h>
#include <AutoConnect.h>
#include <NTPClient.h>

#include <Adafruit_NeoPixel.h>

#include <ArduinoJson.h>

#include <iostream>
#include <string>

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        6		

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 84			 

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define PARAM_FILE	"/elements.json"

// Use these variables to set the initial time
int hours = 12;
int minutes = 0;
int seconds = 0;

// Create ntp client opject.
WiFiUDP ntpUDP;

//NTPClient timeClient(ntpUDP, "nl.pool.ntp.org", 2*60*60, 15*60*1000);
NTPClient timeClient(ntpUDP, "nl.pool.ntp.org");

String Tijdzone;

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
		"uri": "/save"
	}
	]
}
)";

static const char PAGE_SAVE[] PROGMEM = R"(
{
	"uri": "/save",
	"title": "TimeZoneSave",
	"menu": false,
	"element": [
	{
		"name": "caption",
		"type": "ACText",
		"format": "Elements have been saved to %s",
		"style": "font-family:Arial;font-size:18px;font-weight:400;color:#191970"
	},
	{
		"name": "validated",
		"type": "ACText",
		"style": "color:red"
	},
	{
		"name": "echo",
		"type": "ACText",
		"style": "font-family:monospace;font-size:small;white-space:pre;"
	},
	{
		"name": "ok",
		"type": "ACSubmit",
		"value": "OK",
		"uri": "/_ac"
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


AutoConnect Portal(Server);
AutoConnectConfig Config;
AutoConnectAux Timezone;
AutoConnectAux  saveAux;

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

	saveAux.load(FPSTR(PAGE_SAVE));

	saveAux.on([] (AutoConnectAux& aux, PageArgument& arg)
	{

		// The following line sets only the value, but it is HTMLified as
		// formatted text using the format attribute.
		aux["caption"].value = PARAM_FILE;

		SPIFFS.begin();
		File param = SPIFFS.open(PARAM_FILE, "w");
		if (param)
		{
			// Save as a loadable set for parameters.
			Timezone.saveElement(param, { "timezone" });
			param.close();

			// Read the saved elements again to display.
			param = SPIFFS.open(PARAM_FILE, "r");
			aux["echo"].value = param.readString();
			param.close();
		}
		else
		{
			aux["echo"].value = "<styleSPIFFS failed to open.";
		}

		SPIFFS.end();
		return String();
	
		// Set config ntpclient.
		init_ntp_client();
	});

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
		Timezone, saveAux
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

	// Set config ntpclient.
	init_ntp_client();
}

// Read config info and init ntp server time config.
void init_ntp_client()
{
	// Read the saved elements again to display.
	SPIFFS.begin();
	File param = SPIFFS.open(PARAM_FILE, "r");
	if (param)
	{
		String json = param.readString();
	
		const size_t capacity = JSON_ARRAY_SIZE(24) + JSON_OBJECT_SIZE(5) + json.length() + 10;
		DynamicJsonDocument doc(capacity);
		
		deserializeJson(doc, json.c_str());
		
		int selected = doc["selected"];

		int8_t tzoff = TZ[selected - 1].tzoff;
		char output[300];
		sprintf(output, "Selected : %d, tzoff : %d\n, ntpserver : %s", selected, tzoff, TZ[selected].ntpServer);
		Serial.println(output);
	
		// Set time offset.
		timeClient.setTimeOffset(3600 * tzoff);
		//timeClient.setPoolServerName(TZ[selected].ntpserver);
	}
	param.close();
	SPIFFS.end();
}

void loop()
{
	Portal.handleClient();

	Serial.println();
	Serial.print("WiFi connected with ip ");
	Serial.println(WiFi.localIP());

	timeClient.update();

	// Display time info.
	Serial.print("Clock time : ");
	Serial.println(timeClient.getFormattedTime());

	delay(1000);
}


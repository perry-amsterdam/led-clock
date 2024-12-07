#include <iostream>
#include <string>
#include <curl/curl.h>
#include <ctime>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <json/json.h>			 // Include the jsoncpp library for parsing JSON
#include <iomanip>				 // Include for std::setw and std::setfill

// Helper function to handle data received from HTTP response
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string *)userp)->append((char *)contents, size * nmemb);
	return size * nmemb;
}


class TimeInfo
{
	public:
		// Constructor takes a 'debug' flag to enable/disable debug output
		TimeInfo(bool debug = false) : debug(debug)
		{
			curl_global_init(CURL_GLOBAL_DEFAULT);
		}

		~TimeInfo()
		{
			curl_global_cleanup();
		}

		// Function to log debug messages if debug is enabled
		void debugLog(const std::string &message)
		{
			if (debug)
			{
				std::cout << "[DEBUG] " << message << std::endl;
			}
		}

		// Function to get the public IP address using an API
		std::string getPublicIP()
		{
			debugLog("Initializing cURL for public IP retrieval...");
			CURL *curl;
			CURLcode res;
			std::string ip;

			curl = curl_easy_init();
			if (curl)
			{
				debugLog("Fetching public IP from https://api.ipify.org/...");
				curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org/");
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ip);
				res = curl_easy_perform(curl);
				if (res != CURLE_OK)
				{
					std::cerr << "[ERROR] Failed to get IP address: " << curl_easy_strerror(res) << std::endl;
				}
				else
				{
					debugLog("Public IP retrieved successfully.");
				}
				curl_easy_cleanup(curl);
			}
			else
			{
				std::cerr << "[ERROR] cURL initialization failed for public IP retrieval." << std::endl;
			}

			return ip;
		}

		// Function to get timezone and DST information using IP address
		void getTimezoneAndDST(const std::string &ip, std::string &timezone, bool &dst)
		{
			debugLog("Initializing cURL for timezone and DST retrieval...");

			CURL *curl;
			CURLcode res;
			std::string response;
			std::string url = "http://ip-api.com/json/" + ip + "?fields=status,message,country,countryCode,city,timezone,offset,query";

			debugLog("cURL URL set to: " + url);

			curl = curl_easy_init();

			if (curl)
			{
				debugLog("Fetching timezone and DST information from " + url + "...");

				curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

				res = curl_easy_perform(curl);

				if (res != CURLE_OK)
				{
					std::cerr << "[ERROR] Failed to get timezone information: " << curl_easy_strerror(res) << std::endl;
				}
				else
				{
					debugLog("Timezone and DST information retrieved successfully.");
					debugLog("cURL response: " + response);
				}

				curl_easy_cleanup(curl);
			}
			else
			{
				std::cerr << "[ERROR] cURL initialization failed." << std::endl;
			}

			// Parse the response for timezone and DST
			if (!response.empty())
			{
				debugLog("Parsing the response...");

				Json::CharReaderBuilder readerBuilder;
				Json::Value jsonData;
				std::string errs;

				std::istringstream sstream(response);
				if (Json::parseFromStream(readerBuilder, sstream, &jsonData, &errs))
				{
					timezone = jsonData["timezone"].asString();
					dst = jsonData["dst"].asBool();
					debugLog("Parsed response successfully.");
					debugLog("Timezone: " + timezone + ", DST: " + (dst ? "Yes" : "No"));
				}
				else
				{
					std::cerr << "[ERROR] Failed to parse timezone and DST information." << std::endl;
								 // Show the JSON parsing error if any
					std::cerr << "[DEBUG] Parsing errors: " << errs << std::endl;
				}
			}
			else
			{
				std::cerr << "[ERROR] Failed to retrieve timezone and DST information. Empty response." << std::endl;
			}
		}

		// Function to get the current time from an NTP server
		std::string getCurrentTimeFromNTP()
		{
			debugLog("Initializing cURL for NTP time retrieval...");
			CURL *curl;
			CURLcode res;
			std::string timeData;

			curl = curl_easy_init();
			if (curl)
			{
				debugLog("Fetching current time from http://worldclockapi.com/api/json/utc/now...");
				curl_easy_setopt(curl, CURLOPT_URL, "http://worldclockapi.com/api/json/utc/now");
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &timeData);
				res = curl_easy_perform(curl);
				if (res != CURLE_OK)
				{
					std::cerr << "[ERROR] Failed to get current time: " << curl_easy_strerror(res) << std::endl;
				}
				else
				{
					debugLog("Current time retrieved successfully.");
				}
				curl_easy_cleanup(curl);
			}

			return timeData;
		}

		// Function to extract and adjust time details
		void extractTimeDetails(const std::string &timeData, const std::string &timezone, bool dst)
		{
			debugLog("Parsing time data for details...");
			Json::CharReaderBuilder readerBuilder;
			Json::Value jsonData;
			std::string errs;

			std::istringstream sstream(timeData);
			if (Json::parseFromStream(readerBuilder, sstream, &jsonData, &errs))
			{
				std::string currentDateTime = jsonData["currentDateTime"].asString();
				std::string date = currentDateTime.substr(0, 10);
				std::string time = currentDateTime.substr(11, 8);

				int hour, minute, second;
				sscanf(time.c_str(), "%d:%d:%d", &hour, &minute, &second);

				debugLog("Current UTC Time: " + time);
				debugLog("Extracted Hour: " + std::to_string(hour) + ", Minute: " + std::to_string(minute) + ", Second: " + std::to_string(second));

				int offset = 0;
				if (timezone == "Europe/Paris" || timezone == "America/New_York")
				{
					offset = dst ? 2 : 1;
				}
				hour = (hour + offset) % 24;

				debugLog("Adjusted Local Time: " + std::to_string(hour) + ":" + std::to_string(minute) + ":" + std::to_string(second));

				// Print formatted output
				printFormattedOutput(date, time, timezone, dst, hour, minute, second);
			}
			else
			{
				std::cerr << "[ERROR] Failed to parse time data." << std::endl;
			}
		}

		// Function to print formatted output
		void printFormattedOutput(const std::string &date, const std::string &time, const std::string &timezone, bool dst, int hour, int minute, int second)
		{
			std::cout << "\n========== Information ==========\n";
			std::cout << "Public IP Address: " << getPublicIP() << "\n";
			std::cout << "Timezone: " << timezone << "\n";
			std::cout << "DST: " << (dst ? "Yes" : "No") << "\n";
			std::cout << "\nUTC Time: " << date << " " << time << "\n";
			std::cout << "Adjusted Local Time: " << date << " " << std::setw(2) << std::setfill('0') << hour << ":"
				<< std::setw(2) << std::setfill('0') << minute << ":" << std::setw(2) << std::setfill('0') << second << "\n";
			std::cout << "================================\n";
		}

	private:
		bool debug;				 // Flag to enable or disable debug output
		CURL *curl;
};

int main()
{
	// Set the debug flag to true or false as needed
	bool enableDebug = true;	 // Change this to false to disable debug output
	std::cout << "[DEBUG] Program started. Fetching public IP..." << std::endl;

	// Pass the debug flag to the TimeInfo object
	TimeInfo timeInfo(enableDebug);

	std::string publicIP = timeInfo.getPublicIP();
	std::cout << "[INFO] Public IP Address: " << publicIP << std::endl;

	std::cout << "[DEBUG] Fetching timezone and DST information..." << std::endl;
	std::string timezone;
	bool dst;
	timeInfo.getTimezoneAndDST(publicIP, timezone, dst);

	std::cout << "[DEBUG] Fetching current time from NTP..." << std::endl;
	std::string timeData = timeInfo.getCurrentTimeFromNTP();

	std::cout << "[DEBUG] Extracting and adjusting time details..." << std::endl;
	timeInfo.extractTimeDetails(timeData, timezone, dst);

	return 0;
}

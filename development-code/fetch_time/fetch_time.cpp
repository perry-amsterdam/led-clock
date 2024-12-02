#include <iostream>
#include <string>
#include <curl/curl.h>
#include <ctime>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <json/json.h>			 // Include the jsoncpp library for parsing JSON

// Helper function to handle data received from HTTP response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}


// Function to get the public IP address using an API
std::string getPublicIP()
{
	CURL* curl;
	CURLcode res;
	std::string ip;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org/");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ip);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			std::cerr << "Failed to get IP address: " << curl_easy_strerror(res) << std::endl;
		}
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
	return ip;
}


// Function to get timezone and DST information using IP address
void getTimezoneAndDST(const std::string& ip, std::string& timezone, bool& dst)
{
	CURL* curl;
	CURLcode res;
	std::string response;
	std::string url = "http://ip-api.com/json/" + ip + "?fields=timezone,dst";

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			std::cerr << "Failed to get timezone information: " << curl_easy_strerror(res) << std::endl;
		}
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();

	// Parse the response for timezone and DST
	if (!response.empty())
	{
		Json::CharReaderBuilder readerBuilder;
		Json::Value jsonData;
		std::string errs;

		std::istringstream sstream(response);
		if (Json::parseFromStream(readerBuilder, sstream, &jsonData, &errs))
		{
			timezone = jsonData["timezone"].asString();
			dst = jsonData["dst"].asBool();
			std::cout << "Timezone: " << timezone << ", DST: " << (dst ? "Yes" : "No") << std::endl;
		}
		else
		{
			std::cerr << "Failed to parse timezone and DST information." << std::endl;
		}
	}
	else
	{
		std::cerr << "Failed to retrieve timezone and DST information." << std::endl;
	}
}


// Function to get the current time from an NTP server (simplified version)
void extractTimeDetails(const std::string& timeData, const std::string& timezone, bool dst)
{
	Json::CharReaderBuilder readerBuilder;
	Json::Value jsonData;
	std::string errs;

	std::istringstream sstream(timeData);
	if (Json::parseFromStream(readerBuilder, sstream, &jsonData, &errs))
	{
		std::string currentDateTime = jsonData["currentDateTime"].asString();
								 // Extract date (yyyy-mm-dd)
		std::string date = currentDateTime.substr(0, 10);
								 // Extract time (hh:mm:ss)
		std::string time = currentDateTime.substr(11, 8);

		// Extract hour, minute, and second
		int hour, minute, second;
		sscanf(time.c_str(), "%d:%d:%d", &hour, &minute, &second);

		std::cout << "Current UTC Time: " << time << std::endl;
		std::cout << "Hour: " << hour << ", Minute: " << minute << ", Second: " << second << std::endl;

		// Adjust for timezone and DST
		int offset = 0;
								 // Example timezone
		if (timezone == "Europe/Paris" || timezone == "America/New_York")
		{
			// Adjust time based on DST
			if (dst)
			{
				offset = 2;		 // Example: Paris during DST (UTC + 2)
			}
			else
			{
				offset = 1;		 // Example: Paris standard time (UTC + 1)
			}
		}

		// Apply the time zone offset
								 // Adjust for DST or winter time
		hour = (hour + offset) % 24;
		std::cout << "Adjusted Local Time: " << hour << ":" << minute << ":" << second << std::endl;
	}
	else
	{
		std::cerr << "Failed to parse time data." << std::endl;
	}
}


// Function to get the current time from an NTP server (simplified version)
std::string getCurrentTimeFromNTP()
{
	CURL* curl;
	CURLcode res;
	std::string timeData;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, "http://worldclockapi.com/api/json/utc/now");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &timeData);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			std::cerr << "Failed to get current time: " << curl_easy_strerror(res) << std::endl;
		}
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();

	return timeData;
}


int main()
{
	// Step 1: Get the public IP address
	std::string publicIP = getPublicIP();
	std::cout << "Public IP Address: " << publicIP << std::endl;

	// Step 2: Get the timezone and DST information
	std::string timezone;
	bool dst;
	getTimezoneAndDST(publicIP, timezone, dst);

	// Step 3: Get the current time from the internet (UTC)
	std::string currentTime = getCurrentTimeFromNTP();
	std::cout << "Current Time (UTC): " << currentTime << std::endl;

	// Step 4: Extract hour, minute, and second, and adjust for time zone and DST
	extractTimeDetails(currentTime, timezone, dst);

	return 0;
}

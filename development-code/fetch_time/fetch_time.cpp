#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <ctime>
#include <iomanip>

using json = nlohmann::json;

// Global variables to store time info
std::string timeZoneName = "";	 // Renamed from 'timezone' to 'timeZoneName'
int gmtOffset_sec = 0;
int daylightOffset_sec = 0;

// CURL write callback to capture HTTP response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* out)
{
	size_t totalSize = size * nmemb;
	out->append((char*)contents, totalSize);
	return totalSize;
}


#include <ctime>
#include <iostream>

void calculateTimeOffsetFromTimezone(const std::string& timezone)
{
	// Set the timezone environment variable
	setenv("TZ", timezone.c_str(), 1);

	// Get the current time
	std::time_t now = std::time(nullptr);
	std::tm localTime = *std::localtime(&now);

	// Calculate GMT offset in seconds
	gmtOffset_sec = localTime.tm_gmtoff;

	// Check if DST is active
	daylightOffset_sec = localTime.tm_isdst > 0 ? 3600 : 0;

	// Debug: Print calculated values
	//std::cout << "Timezone: " << timezone << std::endl;
	//std::cout << "GMT Offset (seconds): " << gmtOffset_sec << std::endl;
	//std::cout << "Daylight Offset (seconds): " << daylightOffset_sec << std::endl;
}


bool fetchTimezoneData()
{
	CURL* curl;
	CURLcode res;
	std::string readBuffer;

	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, "http://ip-api.com/json");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
			curl_easy_cleanup(curl);
			return false;
		}

		curl_easy_cleanup(curl);

		// Debug: Print the raw response
		//std::cout << "API Response: " << readBuffer << std::endl;

		// Parse the JSON response
		auto jsonResponse = json::parse(readBuffer);

		// Check for the timezone field
		if (jsonResponse.contains("timezone"))
		{
			timeZoneName = jsonResponse["timezone"];
		}
		else
		{
			std::cerr << "Response missing 'timezone' field." << std::endl;
			return false;
		}

		// Calculate GMT offset and DST from timezone
		calculateTimeOffsetFromTimezone(timeZoneName);

		return true;
	}

	return false;
}


void printTime()
{
	time_t rawTime = time(NULL) + gmtOffset_sec + daylightOffset_sec;
	struct tm* localTime = gmtime(&rawTime);
	char timeBuffer[80];
	strftime(timeBuffer, 80, "%Y-%m-%d %H:%M:%S", localTime);

	std::cout << "Current Time: " << timeBuffer << " (" << timeZoneName << ")" << std::endl;
}


int main()
{
	std::cout << "Fetching timezone and DST information..." << std::endl;
	if (fetchTimezoneData())
	{
								 // Updated variable name
		std::cout << "Timezone: " << timeZoneName << std::endl;
		std::cout << "GMT Offset: " << gmtOffset_sec << " seconds" << std::endl;
		std::cout << "Daylight Offset: " << daylightOffset_sec << " seconds" << std::endl;

		std::cout << "\nLocal Time:" << std::endl;
		printTime();
	}
	else
	{
		std::cerr << "Failed to fetch timezone information." << std::endl;
	}
	return 0;
}

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <ctime>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <json/json.h> // Include the jsoncpp library for parsing JSON

// Helper function to handle data received from HTTP response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to get the public IP address using an API
std::string getPublicIP()
{
    std::cout << "[DEBUG] Initializing cURL for public IP retrieval..." << std::endl;
    CURL* curl;
    CURLcode res;
    std::string ip;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl)
    {
        std::cout << "[DEBUG] Fetching public IP from https://api.ipify.org/..." << std::endl;
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
            std::cout << "[DEBUG] Public IP retrieved successfully." << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    else
    {
        std::cerr << "[ERROR] cURL initialization failed for public IP retrieval." << std::endl;
    }
    curl_global_cleanup();
    return ip;
}

// Function to get timezone and DST information using IP address
void getTimezoneAndDST(const std::string& ip, std::string& timezone, bool& dst)
{
    std::cout << "[DEBUG] Initializing cURL for timezone and DST retrieval..." << std::endl;
    CURL* curl;
    CURLcode res;
    std::string response;
    std::string url = "http://ip-api.com/json/" + ip + "?fields=timezone,dst";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl)
    {
        std::cout << "[DEBUG] Fetching timezone and DST information from " << url << "..." << std::endl;
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
            std::cout << "[DEBUG] Timezone and DST information retrieved successfully." << std::endl;
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
            std::cout << "[DEBUG] Timezone: " << timezone << ", DST: " << (dst ? "Yes" : "No") << std::endl;
        }
        else
        {
            std::cerr << "[ERROR] Failed to parse timezone and DST information." << std::endl;
        }
    }
    else
    {
        std::cerr << "[ERROR] Failed to retrieve timezone and DST information." << std::endl;
    }
}

// Function to get the current time from an NTP server
std::string getCurrentTimeFromNTP()
{
    std::cout << "[DEBUG] Initializing cURL for NTP time retrieval..." << std::endl;
    CURL* curl;
    CURLcode res;
    std::string timeData;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl)
    {
        std::cout << "[DEBUG] Fetching current time from http://worldclockapi.com/api/json/utc/now..." << std::endl;
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
            std::cout << "[DEBUG] Current time retrieved successfully." << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return timeData;
}

// Function to extract and adjust time details
void extractTimeDetails(const std::string& timeData, const std::string& timezone, bool dst)
{
    std::cout << "[DEBUG] Parsing time data for details..." << std::endl;
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

        std::cout << "[DEBUG] Current UTC Time: " << time << std::endl;
        std::cout << "[DEBUG] Extracted Hour: " << hour << ", Minute: " << minute << ", Second: " << second << std::endl;

        int offset = 0;
        if (timezone == "Europe/Paris" || timezone == "America/New_York")
        {
            offset = dst ? 2 : 1;
        }
        hour = (hour + offset) % 24;

        std::cout << "[DEBUG] Adjusted Local Time: " << hour << ":" << minute << ":" << second << std::endl;
    }
    else
    {
        std::cerr << "[ERROR] Failed to parse time data." << std::endl;
    }
}

int main()
{
    std::cout << "[DEBUG] Program started. Fetching public IP..." << std::endl;
    std::string publicIP = getPublicIP();
    std::cout << "[INFO] Public IP Address: " << publicIP << std::endl;

    std::cout << "[DEBUG] Fetching timezone and DST information..." << std::endl;
    std::string timezone;
    bool dst;
    getTimezoneAndDST(publicIP, timezone, dst);

    std::cout << "[DEBUG] Fetching current time (UTC)..." << std::endl;
    std::string currentTime = getCurrentTimeFromNTP();
    std::cout << "[INFO] Current Time (UTC): " << currentTime << std::endl;

    std::cout << "[DEBUG] Extracting and adjusting time details..." << std::endl;
    extractTimeDetails(currentTime, timezone, dst);

    std::cout << "[DEBUG] Program completed successfully." << std::endl;
    return 0;
}


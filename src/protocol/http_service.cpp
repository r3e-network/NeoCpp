#include "neocpp/protocol/http_service.hpp"
#include "neocpp/exceptions.hpp"
#include <curl/curl.h>
#include <sstream>

namespace neocpp {

// Callback for CURL to write response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

HttpService::HttpService(const std::string& baseUrl) 
    : baseUrl_(baseUrl) {
    // Initialize CURL globally (only needs to be done once)
    static bool curlInitialized = false;
    if (!curlInitialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curlInitialized = true;
    }
}

HttpService::~HttpService() {
    // Cleanup is handled globally
}

nlohmann::json HttpService::post(const nlohmann::json& data, const std::string& endpoint) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw RpcException("Failed to initialize CURL");
    }
    
    std::string url = baseUrl_ + endpoint;
    std::string jsonStr = data.dump();
    std::string response;
    
    // Set CURL options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonStr.length());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    // Set headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        throw RpcException("HTTP request failed: " + std::string(curl_easy_strerror(res)));
    }
    
    // Parse response
    try {
        return nlohmann::json::parse(response);
    } catch (const nlohmann::json::exception& e) {
        throw RpcException("Failed to parse JSON response: " + std::string(e.what()));
    }
}

nlohmann::json HttpService::get(const std::string& endpoint) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw RpcException("Failed to initialize CURL");
    }
    
    std::string url = baseUrl_ + endpoint;
    std::string response;
    
    // Set CURL options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    // Set headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        throw RpcException("HTTP request failed: " + std::string(curl_easy_strerror(res)));
    }
    
    // Parse response
    try {
        return nlohmann::json::parse(response);
    } catch (const nlohmann::json::exception& e) {
        throw RpcException("Failed to parse JSON response: " + std::string(e.what()));
    }
}

} // namespace neocpp
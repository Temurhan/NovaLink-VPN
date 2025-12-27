#include "fetcher.hpp"
#include <curl/curl.h>
#include <iostream>

// Если в hpp есть namespace, то и здесь он должен быть
namespace NovaLink {

    size_t DiscoveryService::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t totalSize = size * nmemb;
        ((std::string*)userp)->append((char*)contents, totalSize);
        return totalSize;
    }

    std::string DiscoveryService::get_node_ip(const std::string& api_url) {
        CURL* curl;
        CURLcode res;
        std::string readBuffer;

        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());

            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "Authorization: my_super_secret_key_123");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3L);

            res = curl_easy_perform(curl);

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);

            if(res == CURLE_OK) return readBuffer;

            std::cerr << "[-] Discovery Error: " << curl_easy_strerror(res) << std::endl;
        }
        return "";
    }

} // namespace NovaLink
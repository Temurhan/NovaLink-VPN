#ifndef DISCOVERY_FETCHER_HPP
#define DISCOVERY_FETCHER_HPP

#include <string>
namespace NovaLink {
    class DiscoveryService {
    public:
        // Мы оставляем только прототип метода
        std::string get_node_ip(const std::string& api_url);

    private:
        // Статическому колбэку тоже место в реализации
        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    };
}

#endif
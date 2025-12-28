#ifndef DISCOVERY_FETCHER_HPP
#define DISCOVERY_FETCHER_HPP

#include <string>
namespace NovaLink {
    class DiscoveryService {
    public:
        std::string get_node_ip(const std::string& api_url);

    private:
        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    };
}

#endif
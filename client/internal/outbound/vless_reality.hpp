#pragma once
#include <string>
#include <openssl/ssl.h>

namespace NovaLink {

    struct OutboundConfig {
        std::string server_ip;
        int server_port;
        std::string uuid;
        std::string sni;
    };

    class VlessRealityHandler {
    private:
        SSL* ssl = nullptr;
        int server_fd = -1;
        bool connected = false;

    public:
        bool connect(const OutboundConfig& config);
    };

}
#pragma once
#include <string>
#include <cstdint>
#include <sys/types.h>

namespace NovaLink {
    struct OutboundConfig {
        std::string address;
        int port = 443;
        std::string uuid;
        std::string sni;
        std::string publicKey;
        std::string shortId;
    };
    class OutboundHandler {
    public:
        virtual ~OutboundHandler() = default;
        virtual bool connect(const OutboundConfig& config) = 0;
        virtual bool process_and_send(const uint8_t* data, size_t length) = 0;
        virtual ssize_t receive_and_unpack(uint8_t* buffer, size_t max_length) = 0;
        virtual int get_socket_fd() const = 0;
    };

}
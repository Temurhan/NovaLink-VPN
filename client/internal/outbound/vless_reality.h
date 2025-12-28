#pragma once
#include <openssl/ssl.h>
#include <vector>
#include <cstdint>

#include "protocol.h"

namespace NovaLink {

    class VlessRealityHandler {
    protected:
        SSL* ssl = nullptr;
        SSL_CTX* ctx = nullptr;

    public:
        VlessRealityHandler();
        virtual ~VlessRealityHandler();
        bool connect(const OutboundConfig& config);
        virtual ssize_t process_and_send(const uint8_t* data, size_t length);
        virtual ssize_t receive_and_unpack(uint8_t* buffer, size_t size);
        int get_socket_fd();
    };
}

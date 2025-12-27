#pragma once
#include "protocol.h"
#include <openssl/ssl.h>
#include <vector>

namespace NovaLink {

    class VlessRealityHandler : public OutboundHandler {
    private:
        int sock_fd = -1;
        SSL_CTX* ssl_ctx = nullptr;
        SSL* ssl_conn = nullptr;
        OutboundConfig current_config;
        std::vector<uint8_t> cached_uuid;
        bool header_sent = false; // Флаг: отправлен ли заголовок VLESS

        void uuid_to_bytes(const std::string& uuid_str);

    public:
        VlessRealityHandler();
        ~VlessRealityHandler() override;

        bool connect(const OutboundConfig& config) override;
        bool process_and_send(const uint8_t* data, size_t length) override;
        ssize_t receive_and_unpack(uint8_t* buffer, size_t max_length) override;
        int get_socket_fd() const override { return sock_fd; }
    };

}
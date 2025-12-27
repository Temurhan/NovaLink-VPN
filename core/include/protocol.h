#pragma once
#include <string>
#include <cstdint>
#include <sys/types.h> // Для ssize_t на Linux

namespace NovaLink {

    // Данные для подключения
    struct OutboundConfig {
        std::string address;
        int port = 443;
        std::string uuid;
        std::string sni;
        std::string publicKey;
        std::string shortId;
    };

    // Общий интерфейс для всех протоколов (VLESS, Shadowsocks и т.д.)
    class OutboundHandler {
    public:
        virtual ~OutboundHandler() = default;

        // Инициализация соединения
        virtual bool connect(const OutboundConfig& config) = 0;

        // Шифрование и отправка в сеть
        virtual bool process_and_send(const uint8_t* data, size_t length) = 0;

        // Чтение из сети и расшифровка
        virtual ssize_t receive_and_unpack(uint8_t* buffer, size_t max_length) = 0;

        // Доступ к сокету для Epoll
        virtual int get_socket_fd() const = 0;
    };

}
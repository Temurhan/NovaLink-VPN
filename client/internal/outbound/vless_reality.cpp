#include "vless_reality.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <vector>
#include <random>

namespace NovaLink {

    // Генератор случайных чисел (статический, чтобы не инициализировать каждый раз)
    static std::mt19937& get_rng() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        return gen;
    }


    // 1. КОНСТРУКТОР (Линковщик ругался на него)
    VlessRealityHandler::VlessRealityHandler() {
        ssl = nullptr;
        ctx = nullptr;
        // Здесь должна быть инициализация SSL_CTX
    }

    // 2. ДЕСТРУКТОР
    VlessRealityHandler::~VlessRealityHandler() {
        if (ssl) SSL_free(ssl);
        if (ctx) SSL_CTX_free(ctx);
    }

    // 3. МЕТОД CONNECT
    bool VlessRealityHandler::connect(const OutboundConfig& config) {
        // Твоя реализация подключения...
        return true;
    }

    // 4. МЕТОД GET_SOCKET_FD
    int VlessRealityHandler::get_socket_fd() {
        if (ssl) return SSL_get_fd(ssl);
        return -1;
    }

    const uint8_t MY_UUID[16] = {
        0xde, 0xad, 0xbe, 0xef, 0x12, 0x34, 0x56, 0x78,
        0x9a, 0xbc, 0xde, 0xf0, 0x11, 0x22, 0x33, 0x44
    };
    static std::vector<uint8_t> wrap_vless(const uint8_t* payload, size_t payload_len) {
        std::vector<uint8_t> packet;

        // 1. Стандартный заголовок VLESS
        packet.push_back(0x00); // Version
        packet.insert(packet.end(), MY_UUID, MY_UUID + 16);

        // 2. Добавляем информацию о Padding (набивке)
        // В VLESS протоколе мы можем использовать поле addons для передачи длины набивки
        std::uniform_int_distribution<> dis(1, 64); // Случайный размер от 1 до 64 байт
        uint8_t padding_len = static_cast<uint8_t>(dis(get_rng()));

        packet.push_back(padding_len); // Записываем длину набивки в поле Addons
        packet.push_back(0x01);        // Command

        // 3. Основные данные (Payload)
        packet.insert(packet.end(), payload, payload + payload_len);

        // 4. Генерируем саму набивку (случайные байты)
        for (int i = 0; i < padding_len; ++i) {
            packet.push_back(static_cast<uint8_t>(dis(get_rng())));
        }

        return packet;
    }
    // 5. PROCESS_AND_SEND
    ssize_t VlessRealityHandler::process_and_send(const uint8_t* buffer, size_t size) {
        if (!this->ssl) return -1;
        auto vless_packet = wrap_vless(buffer, size);
        int n = SSL_write(this->ssl, vless_packet.data(), (int)vless_packet.size());
        return (n > 0) ? (ssize_t)size : (ssize_t)n;
    }

    // 6. RECEIVE_AND_UNPACK
    ssize_t VlessRealityHandler::receive_and_unpack(uint8_t* buffer, size_t size) {
        if (!this->ssl) return -1;
        return SSL_read(this->ssl, buffer, (int)size);
    }

} // namespace NovaLink
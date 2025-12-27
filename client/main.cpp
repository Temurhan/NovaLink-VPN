#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <curl/curl.h>
#include "json.hpp"
#include "vless_reality.h"
#include "TunHandler.hpp"

using json = nlohmann::json;
using namespace NovaLink;

// --- 1. ВСПОМОГАТЕЛЬНЫЙ КОД ДЛЯ API ---

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

struct VpnConfig {
    std::string server_ip;
    int port;
    std::string uuid;
    bool valid = false;
};

VpnConfig fetch_vpn_config(const std::string& api_url) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    VpnConfig config;

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // Тайм-аут 10 секунд

        CURLcode res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
            try {
                auto j = json::parse(readBuffer);
                config.server_ip = j.at("server_ip").get<std::string>();
                config.port = j.at("server_port").get<int>();
                config.uuid = j.at("uuid").get<std::string>();
                config.valid = true;
            } catch (...) {
                std::cerr << "[!] Ошибка парсинга JSON настроек" << std::endl;
            }
        }
        curl_easy_cleanup(curl);
    }
    return config;
}

// --- 2. ГЛАВНЫЙ ЦИКЛ ПРОГРАММЫ ---

int main() {
    std::cout << "=== NovaLink Client Starting ===" << std::endl;

    // ШАГ 1: Получаем настройки от Manager-API
    // Замени IP на адрес своего VPS, где запущен Manager-API
    VpnConfig config = fetch_vpn_config("http://твой_vps_ip:8080/api/v1/config");

    if (!config.valid) {
        std::cerr << "[!] Не удалось получить конфигурацию. Проверьте интернет или API." << std::endl;
        return 1;
    }

    // ШАГ 2: Инициализация TUN интерфейса
    TunHandler tun;
    if (!tun.open_tun("nova_cli")) {
        std::cerr << "[!] Ошибка создания TUN интерфейса (нужен sudo!)" << std::endl;
        return 1;
    }

    // Настройка IP для TUN (вручную для теста)
    system("ip addr add 10.8.0.2/24 dev nova_cli");
    system("ip link set dev nova_cli up");

    // Здесь можно вызвать нашу функцию prevent_dns_leaks("nova_cli")

    // ШАГ 3: Инициализация VLESS обработчика
    VlessRealityHandler vless;
    OutboundConfig outbound;
    outbound.address = config.server_ip;
    outbound.port = config.port;
    outbound.uuid = config.uuid;

    if (!vless.connect(outbound)) {
        std::cerr << "[!] Ошибка TLS подключения к серверу" << std::endl;
        return 1;
    }

    std::cout << "[+] Туннель установлен! Трафик шифруется." << std::endl;

    // ШАГ 4: Потоки передачи данных

    // Поток: TUN -> VLESS (Отправка)
    std::thread writer([&]() {
        uint8_t buf[4096];
        while (true) {
            int n = tun.read_packet(buf, sizeof(buf));
            if (n > 0) {
                vless.process_and_send(buf, n);
            }
        }
    });

    // Поток: VLESS -> TUN (Прием)
    std::thread reader([&]() {
        uint8_t buf[4096];
        while (true) {
            ssize_t n = vless.receive_and_unpack(buf, sizeof(buf));
            if (n > 0) {
                tun.write_packet(buf, n);
            }
        }
    });

    writer.join();
    reader.join();

    return 0;
}
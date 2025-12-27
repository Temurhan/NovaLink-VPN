#pragma once
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h" // Нужно будет скачать этот один файл
#include <json.hpp> // Для удобной работы с JSON
#include <iostream>

using json = nlohmann::json;

namespace NovaLink{
    class ManagerAPI {
    public:
        void start(int port) {
            httplib::Server svr;

            // Эндпоинт для выдачи конфига клиенту
            svr.Get("/api/v1/config", [](const httplib::Request&, httplib::Response& res) {
                json config;
                config["server_ip"] = "Ваш_IP_VPS";
                config["server_port"] = 443;
                config["uuid"] = "de-ad-be-ef-12-34-56-78-9a-bc-de-f0-11-22-33-44";
                config["dns"] = "1.1.1.1";
                config["obfuscation"] = "vless-reality";

                res.set_content(config.dump(), "application/json");
            });

            // Эндпоинт для мониторинга (сколько клиентов онлайн)
            svr.Get("/status", [](const httplib::Request&, httplib::Response& res) {
                res.set_content("{\"status\": \"running\"}", "application/json");
            });

            std::cout << "[+] Manager-API started on port " << port << std::endl;
            svr.listen("0.0.0.0", port);
        }
    };
}
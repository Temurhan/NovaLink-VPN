#pragma once
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <json.hpp>
#include <iostream>
#include <string>

using json = nlohmann::json;

namespace NovaLink {
    class ManagerAPI {
    public:
        void start(const std::string& ip_address, const std::string& uuid_str, int port) {
            httplib::Server svr;
            svr.Get("/api/v1/config", [ip_address, uuid_str](const httplib::Request&, httplib::Response& res) {
                   json config;
                   config["server_ip"] = ip_address;
                   config["server_port"] = 443;
                   config["uuid"] = uuid_str;

                   config["dns"] = "1.1.1.1";
                   config["obfuscation"] = "vless-reality";

                   res.set_content(config.dump(), "application/json");
               });
            svr.Get("/status", [](const httplib::Request&, httplib::Response& res) {
                res.set_content("{\"status\": \"running\"}", "application/json");
            });

            std::cout << "[+] Manager-API started on port " << port << std::endl;
            std::cout << "[+] Configured with IP: " << ip_address << std::endl;

            svr.listen("0.0.0.0", port);
        }
    };
}
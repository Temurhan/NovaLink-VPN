#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <algorithm> // для std::find_if
#include "protocol.h"

namespace NovaLink {

    // Вспомогательная функция для удаления пробелов по краям
    inline std::string trim(const std::string& s) {
        auto start = std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        });
        auto end = std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base();
        return (start < end ? std::string(start, end) : "");
    }

    inline OutboundConfig parse_config(const std::string& filename) {
        OutboundConfig config;
        // Установим значения по умолчанию
        config.port = 443;

        std::ifstream file(filename);
        if (!file.is_open()) {
            // Полезно для отладки в CLion:
            std::cerr << "[!] Warning: Config not found at " << filename << std::endl;
            return config;
        }

        std::string line;
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty() || line[0] == '#') continue;

            size_t delimiter_pos = line.find('=');
            if (delimiter_pos != std::string::npos) {
                std::string key = trim(line.substr(0, delimiter_pos));
                std::string value = trim(line.substr(delimiter_pos + 1));

                if (key == "server_ip") config.address = value;
                else if (key == "server_port") {
                    try {
                        if(!value.empty()) config.port = std::stoi(value);
                    } catch (...) { config.port = 443; }
                }
                else if (key == "uuid") config.uuid = value;
                else if (key == "sni") config.sni = value;
                else if (key == "public_key") config.publicKey = value;
                else if (key == "short_id") config.shortId = value;
            }
        }
        return config;
    }
}
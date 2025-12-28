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
                std::cerr << "[!] Error parsing JSON settings" << std::endl;
            }
        }
        curl_easy_cleanup(curl);
    }
    return config;
}
int main() {
    std::cout << "=== NovaLink Client Starting ===" << std::endl;
    //This's an IP address for testing, not the original one, bro :)
    // VpnConfig config = fetch_vpn_config("http://188.68.222.128:8080/api/v1/config");
    VpnConfig config = fetch_vpn_config("http://your_vps_ip:8080/api/v1/config");
    if (!config.valid) {
        std::cerr << "[!] Failed to retrieve configuration. Please check internet or API connection.." << std::endl;
        return 1;
    }
    TunHandler tun;
    if (!tun.open_tun("nova_cli")) {
        std::cerr << "[!] Error creating TUN interface (sudo required!)" << std::endl;
        return 1;
    }
    system("ip addr add 10.8.0.2/24 dev nova_cli");
    system("ip link set dev nova_cli up");


    VlessRealityHandler vless;
    OutboundConfig outbound;
    outbound.address = config.server_ip;
    outbound.port = config.port;
    outbound.uuid = config.uuid;

    if (!vless.connect(outbound)) {
        std::cerr << "[!] TLS connection error with server" << std::endl;
        return 1;
    }

    std::cout << "[+] The tunnel is established! Traffic is encrypted." << std::endl;
    std::thread writer([&]() {
        uint8_t buf[4096];
        while (true) {
            int n = tun.read_packet(buf, sizeof(buf));
            if (n > 0) {
                vless.process_and_send(buf, n);
            }
        }
    });

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
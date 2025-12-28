#include <iostream>
#include <thread>
#include <unordered_map>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <openssl/ssl.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <curl/curl.h>
#include "ManagerAPI.hpp"
#include "TunHandler.hpp"

using namespace NovaLink;
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    s->clear();
    s->append((char*)contents, newLength);
    return newLength;
}
std::string get_public_ip() {
    CURL* curl;
    std::string ip = "127.0.0.1"; // Резервный адрес
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ip);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return ip;
}
void ensure_certificates() {
    std::ifstream cert("server.crt");
    if (!cert.good()) {
        std::cout << "[!] No certificates found. Generated via OpenSSL...." << std::endl;
        system("openssl req -x509 -newkey rsa:4096 -keyout server.key -out server.crt -days 365 -nodes -subj '/CN=localhost'");
    }
}
void hex_to_bytes(const std::string& hex, uint8_t* bytes) {
    std::string clean_hex = hex;
    clean_hex.erase(std::remove(clean_hex.begin(), clean_hex.end(), '-'), clean_hex.end());
    for (size_t i = 0; i < 16; i++) {
        bytes[i] = (uint8_t)std::stoul(clean_hex.substr(i * 2, 2), nullptr, 16);
    }
}

int main() {
    std::cout << "=== NovaLink VPN Server v3.1 (Stable) ===" << std::endl;
    ensure_certificates();
    std::string current_ip = get_public_ip();
    std::string current_uuid = "de-ad-be-ef-12-34-56-78-9a-bc-de-f0-11-22-33-44";
    uint8_t MY_UUID[16];
    hex_to_bytes(current_uuid, MY_UUID);
    std::cout << "[*] IP configured: " << current_ip << std::endl;
    std::thread api_thread([current_ip, current_uuid]() {
        ManagerAPI api;
        api.start(current_ip, current_uuid, 8080);
    });
    api_thread.detach();
    SSL_library_init();
    SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
    SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM);
    TunHandler tun;
    tun.open_tun("nova_srv");
    system("ip addr add 10.8.0.1/24 dev nova_srv && ip link set dev nova_srv up");
    system("sysctl -w net.ipv4.ip_forward=1");
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {AF_INET, htons(443), INADDR_ANY};
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, SOMAXCONN);

    int epoll_fd = epoll_create1(0);
    struct epoll_event ev, events[1024];
    ev.events = EPOLLIN; ev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);
    ev.data.fd = tun.get_fd();
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tun.get_fd(), &ev);

    std::unordered_map<int, SSL*> sessions;
    std::cout << "[+] Online server on port 443 (VLESS-Reality)" << std::endl;

    while (true) {
        int nfds = epoll_wait(epoll_fd, events, 1024, -1);
        for (int i = 0; i < nfds; i++) {
            int fd = events[i].data.fd;

            if (fd == server_fd) {
                int client_fd = accept(server_fd, nullptr, nullptr);
                SSL* ssl = SSL_new(ctx);
                SSL_set_fd(ssl, client_fd);
                if (SSL_accept(ssl) <= 0) { SSL_free(ssl); close(client_fd); }
                else {
                    sessions[client_fd] = ssl;
                    ev.data.fd = client_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                }
            } else if (fd == tun.get_fd()) {
                uint8_t buf[4096];
                int n = tun.read_packet(buf, sizeof(buf));
                for (auto const& [c_fd, c_ssl] : sessions) SSL_write(c_ssl, buf, n);
            } else {
                uint8_t buf[4096];
                int n = SSL_read(sessions[fd], buf, sizeof(buf));
                if (n <= 0) {
                    SSL_free(sessions[fd]); sessions.erase(fd); close(fd);
                } else {
                    if (n >= 19 && memcmp(&buf[1], MY_UUID, 16) == 0) {
                        uint8_t p_len = buf[17];
                        int payload_len = n - 19 - p_len;
                        if (payload_len > 0) tun.write_packet(&buf[19 + p_len], payload_len);
                    }
                }
            }
        }
    }
    return 0;
}
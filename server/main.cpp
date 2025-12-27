#include <iostream>
#include <thread>
#include <unordered_map>
#include <openssl/ssl.h>
#include <sys/epoll.h>
#include "ManagerAPI.hpp" // Наш API
#include "TunHandler.hpp"

using namespace NovaLink;

// Глобальный UUID (в реальности лучше грузить из файла/БД)
const std::string SERVER_UUID_STR = "de-ad-be-ef-12-34-56-78-9a-bc-de-f0-11-22-33-44";
uint8_t MY_UUID[16];

// Конвертация строки UUID в массив байт
void hex_to_bytes(const std::string& hex, uint8_t* bytes) {
    // Упрощенная логика парсинга (убираем тире и конвертируем)
    std::string clean_hex = hex;
    clean_hex.erase(std::remove(clean_hex.begin(), clean_hex.end(), '-'), clean_hex.end());
    for (size_t i = 0; i < 16; i++) {
        bytes[i] = std::stoul(clean_hex.substr(i * 2, 2), nullptr, 16);
    }
}

int main() {
    // 1. Инициализация ключей
    hex_to_bytes(SERVER_UUID_STR, MY_UUID);

    // 2. ЗАПУСК MANAGER-API (в фоновом потоке)
    std::thread api_thread([]() {
        ManagerAPI api;
        api.start(8080); // Порт для выдачи конфигов
    });
    api_thread.detach();

    // 3. НАСТРОЙКА VPN-ЯДРА
    SSL_library_init();
    SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
    // Загружаем сертификаты (убедись, что они лежат в папке с программой)
    SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM);

    TunHandler tun;
    tun.open_tun("nova_srv");
    system("ip addr add 10.8.0.1/24 dev nova_srv && ip link set dev nova_srv up");
    system("sysctl -w net.ipv4.ip_forward=1");

    // 4. EPOLL СЕРВЕР (High-Load)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
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

    std::cout << "[*] NovaLink Core & API are online!" << std::endl;

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
                // Трафик из интернета -> клиентам
                uint8_t buf[4096];
                int n = tun.read_packet(buf, sizeof(buf));
                for (auto const& [c_fd, c_ssl] : sessions) SSL_write(c_ssl, buf, n);
            } else {
                // Трафик от клиента (VLESS)
                uint8_t buf[4096];
                int n = SSL_read(sessions[fd], buf, sizeof(buf));
                if (n <= 0) {
                    SSL_free(sessions[fd]); sessions.erase(fd); close(fd);
                    continue;
                }

                // ВАЛИДАЦИЯ UUID И РАСПАКОВКА
                if (n >= 19 && memcmp(&buf[1], MY_UUID, 16) == 0) {
                    uint8_t p_len = buf[17];
                    int payload_len = n - 19 - p_len;
                    if (payload_len > 0) tun.write_packet(&buf[19], payload_len);
                }
            }
        }
    }
    return 0;
}
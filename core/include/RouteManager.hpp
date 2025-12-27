#pragma once

#include <string>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/if.h>     // Для структур ifreq и констант IFF_UP
#include "NetlinkCommander.hpp"

namespace NovaLink { // Обязательно добавляем пространство имен

class RouteManager {
private:
    NetlinkCommander nl_cmd;
    void prevent_dns_leaks(const std::string& tun_if_name, const std::string& dns_server = "1.1.1.1") {
        // 1. Устанавливаем системный DNS на безопасный (Cloudflare)
        std::string set_dns_cmd = "echo \"nameserver " + dns_server + "\" > /etc/resolv.conf";
        std::system(set_dns_cmd.c_str());

        // 2. Направляем трафик DNS в туннель
        // Мы создаем маршрут для DNS-сервера через наш TUN интерфейс
        std::string route_dns_cmd = "ip route add " + dns_server + " dev " + tun_if_name;
        std::system(route_dns_cmd.c_str());

        std::cout << "[+] DNS Leak Protection enabled via " << dns_server << std::endl;
    }
public:
    // Установка IP адреса интерфейсу
    void set_ip(const std::string& if_name, const std::string& ip_addr) {
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0) throw std::runtime_error("RouteManager: Socket failed");

        struct ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        std::strncpy(ifr.ifr_name, if_name.c_str(), IFNAMSIZ - 1);

        struct sockaddr_in* addr = (struct sockaddr_in*)&ifr.ifr_addr;
        addr->sin_family = AF_INET;
        if (inet_pton(AF_INET, ip_addr.c_str(), &addr->sin_addr) <= 0) {
            close(fd);
            throw std::runtime_error("RouteManager: Invalid IP format");
        }

        if (ioctl(fd, SIOCSIFADDR, &ifr) < 0) {
            close(fd);
            throw std::runtime_error("RouteManager: Failed to set IP (run as sudo?)");
        }
        close(fd);
    }

    // Установка MTU
    void set_mtu(const std::string& if_name, int mtu) {
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0) throw std::runtime_error("RouteManager: Socket failed");

        struct ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        std::strncpy(ifr.ifr_name, if_name.c_str(), IFNAMSIZ - 1);
        ifr.ifr_mtu = mtu;

        if (ioctl(fd, SIOCSIFMTU, &ifr) < 0) {
            close(fd);
            throw std::runtime_error("RouteManager: Failed to set MTU");
        }
        close(fd);
    }

    // Поднятие интерфейса (UP)
    void set_up(const std::string& if_name) {
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd < 0) throw std::runtime_error("RouteManager: Socket failed");

        struct ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        std::strncpy(ifr.ifr_name, if_name.c_str(), IFNAMSIZ - 1);

        // Сначала получаем текущие флаги
        if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
            close(fd);
            throw std::runtime_error("RouteManager: Failed to get flags");
        }

        // Используем стандартные константы вместо магических чисел
        ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);

        if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
            close(fd);
            throw std::runtime_error("RouteManager: Failed to set UP");
        }
        close(fd);
    }

    // Добавление маршрута через Netlink
    void add_route(const std::string& if_name, const std::string& dest_ip) {
        nl_cmd.add_route(if_name, dest_ip);
    }
};

} // namespace NovaLink
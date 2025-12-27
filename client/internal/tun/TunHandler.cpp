#include "TunHandler.hpp"
#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <algorithm>

namespace NovaLink { // <-- ДОБАВЬ ЭТО

TunHandler::TunHandler() : tun_fd(-1), actual_name("") {}

TunHandler::~TunHandler() {
    if (tun_fd >= 0) {
        close(tun_fd);
        std::cout << "NovaLink: Interface " << actual_name << " closed." << std::endl;
    }
}

bool TunHandler::is_root() {
    return getuid() == 0;
}

bool TunHandler::open_tun(std::string device_name) {
    if (!is_root()) {
        std::cerr << "Error: Root privileges required to create TUN interface!" << std::endl;
        return false;
    }

    struct ifreq ifr;
    const char *clonedev = "/dev/net/tun";

    if ((tun_fd = open(clonedev, O_RDWR)) < 0) {
        perror("NovaLink Error (open /dev/net/tun)");
        return false;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    if (!device_name.empty()) {
        size_t name_len = std::min(device_name.length(), (size_t)IFNAMSIZ - 1);
        strncpy(ifr.ifr_name, device_name.c_str(), name_len);
        ifr.ifr_name[name_len] = '\0';
    }

    if (ioctl(tun_fd, TUNSETIFF, (void *)&ifr) < 0) {
        perror("NovaLink Error (ioctl TUNSETIFF)");
        close(tun_fd);
        tun_fd = -1;
        return false;
    }

    actual_name = ifr.ifr_name;
    std::cout << "NovaLink: Interface [" << actual_name << "] created successfully." << std::endl;
    return true;
}

bool TunHandler::configure_network(const std::string& ip_addr, int mtu) {
    if (tun_fd == -1) return false;

    auto run_cmd = [](const std::string& cmd) {
        int res = std::system(cmd.c_str());
        return res == 0;
    };

    std::string cmd_ip  = "ip addr add " + ip_addr + "/24 dev " + actual_name;
    std::string cmd_mtu = "ip link set dev " + actual_name + " mtu " + std::to_string(mtu);
    std::string cmd_up  = "ip link set dev " + actual_name + " up";

    return run_cmd(cmd_ip) && run_cmd(cmd_mtu) && run_cmd(cmd_up);
}

ssize_t TunHandler::read_packet(uint8_t* buffer, size_t size) {
    if (tun_fd == -1) return -1;
    return read(tun_fd, buffer, size);
}

ssize_t TunHandler::write_packet(const uint8_t* buffer, size_t size) {
    if (tun_fd == -1) return -1;
    return write(tun_fd, buffer, size);
}

} // <-- И ЭТУ СКОБКУ В КОНЦЕ
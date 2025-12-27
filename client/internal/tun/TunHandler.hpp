#ifndef TUN_HANDLER_HPP
#define TUN_HANDLER_HPP

#include <string>
#include <cstdint>
#include <sys/types.h>

namespace NovaLink{

class TunHandler {

private:
    int tun_fd;
    std::string actual_name;

    bool is_root();

public:
    TunHandler();
    ~TunHandler();

    bool open_tun(std::string device_name);
    bool configure_network(const std::string& ip_addr, int mtu = 1500);

    ssize_t read_packet(uint8_t* buffer, size_t size);
    ssize_t write_packet(const uint8_t* buffer, size_t size);

    std::string get_name() const { return actual_name; }
    int get_fd() const { return tun_fd; }
};
}
#endif
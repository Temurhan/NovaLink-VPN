#pragma once
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>

namespace NovaLink { // ДОБАВЛЕНО: теперь RouteManager его увидит

class NetlinkCommander {
private:
    void add_attr(struct nlmsghdr* nlh, int max_size, int type, const void* data, int len) {
        struct rtattr* rta = (struct rtattr*)((char*)nlh + NLMSG_ALIGN(nlh->nlmsg_len));
        rta->rta_type = type;
        rta->rta_len = RTA_LENGTH(len);
        std::memcpy(RTA_DATA(rta), data, len);
        nlh->nlmsg_len = NLMSG_ALIGN(nlh->nlmsg_len) + RTA_ALIGN(rta->rta_len);
    }

    int get_interface_index(const std::string& if_name) {
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) return 0;

        struct ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        std::strncpy(ifr.ifr_name, if_name.c_str(), IFNAMSIZ - 1);

        if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
            close(sock);
            return 0;
        }
        close(sock);
        return ifr.ifr_ifindex;
    }

public:
    void add_route(const std::string& if_name, const std::string& dest_ip) {
        int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
        if (sock < 0) throw std::runtime_error("NL socket failed");

        int if_index = get_interface_index(if_name);
        if (if_index == 0) {
            close(sock);
            throw std::runtime_error("Interface " + if_name + " index not found");
        }

        struct {
            struct nlmsghdr nlh;
            struct rtmsg rtm;
            char buf[512];
        } req;

        std::memset(&req, 0, sizeof(req));
        req.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
        req.nlh.nlmsg_type = RTM_NEWROUTE;
        req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;

        req.rtm.rtm_family = AF_INET;
        req.rtm.rtm_dst_len = 32; // /32 маршрут (одиночный IP)
        req.rtm.rtm_table = RT_TABLE_MAIN;
        req.rtm.rtm_protocol = RTPROT_STATIC;
        req.rtm.rtm_scope = RT_SCOPE_LINK;
        req.rtm.rtm_type = RTN_UNICAST;

        uint32_t addr;
        if (inet_pton(AF_INET, dest_ip.c_str(), &addr) <= 0) {
            close(sock);
            throw std::runtime_error("Invalid destination IP: " + dest_ip);
        }

        add_attr(&req.nlh, sizeof(req), RTA_DST, &addr, 4);
        add_attr(&req.nlh, sizeof(req), RTA_OIF, &if_index, 4);

        if (send(sock, &req, req.nlh.nlmsg_len, 0) < 0) {
            close(sock);
            throw std::runtime_error("NL send failed (permissions?)");
        }

        close(sock);
        // Примечание: для 100% надежности здесь можно добавить recv() для проверки ACK от ядра
    }
};

} // namespace NovaLink
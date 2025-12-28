#include "vless_reality.hpp"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/err.h>

namespace NovaLink {
bool VlessRealityHandler::connect(const OutboundConfig& config) {
    std::cout << "[*] Connecting to " << config.server_ip << ":" << config.server_port << "..." << std::endl;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "[-] Error: Could not create socket" << std::endl;
        return false;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(config.server_port);
    inet_pton(AF_INET, config.server_ip.c_str(), &server_addr.sin_addr);

    if (::connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "[-] Error: TCP connection failed" << std::endl;
        close(sock);
        return false;
    }

    SSL_library_init();
    const SSL_METHOD* method = TLS_client_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);

    this->ssl = SSL_new(ctx);
    SSL_set_fd(this->ssl, sock);
    SSL_set_tlsext_host_name(this->ssl, config.sni.c_str());
    if (SSL_connect(this->ssl) <= 0) {
        std::cerr << "[-] Error: TLS Handshake failed (Reality rejection)" << std::endl;
        ERR_print_errors_fp(stderr);
        return false;
    }

    std::cout << "[+] Connected to NovaLink Server via VLESS-Reality!" << std::endl;
    this->connected = true;
    this->server_fd = sock;

    return true;
}

}
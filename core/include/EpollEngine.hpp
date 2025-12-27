#pragma once
#include <csignal>
#include <sys/epoll.h>
#include <vector>
#include <functional>
#include <stdexcept>
#include <unistd.h>

namespace NovaLink { // ОБЯЗАТЕЛЬНО: оберни в namespace

    class ShadeEngine {
    private:
        int epoll_fd;
        int tun_fd;
        int sock_fd;
        static const int MAX_EVENTS = 10;

    public:
        // Конструктор принимает дескрипторы TUN и Сокета
        ShadeEngine(int tun_f, int sock_f) : tun_fd(tun_f), sock_fd(sock_f) {
            epoll_fd = epoll_create1(0);
            if (epoll_fd == -1) {
                throw std::runtime_error("[-] Failed to create epoll instance");
            }

            // Регистрируем TUN (чтение)
            add_event(tun_fd, EPOLLIN);
            // Регистрируем Сокет (чтение)
            add_event(sock_fd, EPOLLIN);
        }

        ~ShadeEngine() {
            if (epoll_fd != -1) close(epoll_fd);
        }

        // Вспомогательная функция для добавления FD в epoll
        void add_event(int fd, uint32_t events) {
            struct epoll_event ev{};
            ev.events = events;
            ev.data.fd = fd;
            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
                throw std::runtime_error("[-] epoll_ctl failed for fd: " + std::to_string(fd));
            }
        }

        // Основной цикл обработки
        void run(volatile sig_atomic_t& keep_running,
                 std::function<void()> on_tun_ready,
                 std::function<void()> on_sock_ready) {

            struct epoll_event events[MAX_EVENTS];

            while (keep_running) {
                // Ждем событий (таймаут 100мс, чтобы проверять флаг keep_running)
                int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 100);

                for (int n = 0; n < nfds; ++n) {
                    if (events[n].data.fd == tun_fd) {
                        on_tun_ready(); // Вызываем лямбду для обработки TUN
                    } else if (events[n].data.fd == sock_fd) {
                        on_sock_ready(); // Вызываем лямбду для обработки Сокета
                    }
                }
            }
        }
    };

} // namespace NovaLink

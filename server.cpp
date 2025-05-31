#include "server.hpp"
#include "hasher.hpp"

#include <iostream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace
{

constexpr uint16_t POLL_TIMEOUT = 500;
constexpr size_t BUFFER_SIZE = 1024;

} // unnamed

namespace hashd
{

void Server::run(uint16_t port)
{
    if (!m_server_fd.configure_server(port)) {
        return;
    }

    std::thread accept_clients_thread(&Server::accept_clients, this);
    std::vector<uint8_t> buff(BUFFER_SIZE);

    while(true) {
        const auto fds = m_epoll_fd.wait(POLL_TIMEOUT);

        for (const auto fd : fds) {
            while (true) {
                ssize_t count = read(fd, buff.data(), BUFFER_SIZE);

                if (count == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        break;
                    }

                    std::cerr << "error while reading data" << std::endl;
                    std::lock_guard lock(m_fds_mutex);
                    m_epoll_fd.remove_watching_fd(fd);
                    m_fds.erase(fd);
                    break;
                }
                else if (count == 0) {
                    std::cout << "client disconnected" << std::endl;
                    std::lock_guard lock(m_fds_mutex);
                    m_epoll_fd.remove_watching_fd(fd);
                    m_fds.erase(fd);
                    break;
                }

                const auto hash = Hasher(buff, count).to_string();
                count = send(fd, hash.data(), hash.size(), 0);

                if (count != hash.size()) {
                    std::cerr << "data send error, disconnecting client" << std::endl;
                    std::lock_guard lock(m_fds_mutex);
                    m_epoll_fd.remove_watching_fd(fd);
                    m_fds.erase(fd);
                    break;
                }
            }
        }
    }

    accept_clients_thread.join();
}

void Server::accept_clients() {
    while (true) {
        SocketDescriptor client_fd(accept(m_server_fd, nullptr, nullptr));

        if(client_fd < 0) {
            //std::cerr << "unable to accept connection" << std::endl;
            continue;
        }

        if (!client_fd.set_non_blocking()) {
            std::cerr << "unable to configure client to non blocking mode" << std::endl;
            continue;
        }

        std::cout << "a new client connected" << std::endl;

        std::lock_guard lock(m_fds_mutex);
        m_epoll_fd.add_watching_fd(client_fd);
        m_fds.emplace(client_fd, std::move(client_fd));
    }
}

} // hashd

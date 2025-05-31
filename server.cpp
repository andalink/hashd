#include "server.hpp"
#include "hasher.hpp"

#include <iostream>
#include <netinet/in.h>
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

Server::Server()
    : m_server_fd(socket(AF_INET, SOCK_STREAM, 0))
{}

Server::~Server()
{
    if (m_server_fd != -1) {
        close(m_server_fd);
    }
}

void Server::run(uint16_t port)
{
    if (!prepare_server_socket(port)) {
        return;
    }

    std::thread accept_clients_thread(&Server::accept_clients, this);
    std::vector<uint8_t> buff(BUFFER_SIZE);

    while(true) {
        std::lock_guard lock(m_fds_mutex);
        int count = poll(m_fds.data(), m_fds.size(), POLL_TIMEOUT);

        if (count < 0) {
            std::cerr << "error while poll socket" << std::endl;
            break;
        }
        else if (count == 0) {
            continue;
        }

        for (auto pfd = m_fds.begin(); pfd != m_fds.end(); ) {
            if (pfd->revents & POLLIN) {
                if (int valread = read(pfd->fd, buff.data(), BUFFER_SIZE); valread > 0) {
                    const auto hash = Hasher(buff, valread).to_string();
                    send(pfd->fd, hash.data(), hash.size(), 0); // Echo back
                }
                else {
                    close(pfd->fd);
                    pfd = m_fds.erase(pfd);
                    continue;
                }
            }
            ++pfd;
        }
    }

    accept_clients_thread.join();
    for (const auto& pfd : m_fds) {
        close(pfd.fd);
    }
}

bool Server::prepare_server_socket(uint16_t port)
{
    if (m_server_fd < 0) {
        std::cerr << "error while creating socket" << std::endl;
        return false;
    }

    const int enable = 1;
    if (setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        std::cerr << "error while setting options for socket" << std::endl;
        return false;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if(bind(m_server_fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) < 0) {
        std::cerr << "error while binding socket" << std::endl;
        return false;
    }

    if(listen(m_server_fd, 3) < 0) {
        std::cerr << "unable to listen socket" << std::endl;
        return false;
    }

    return true;
}

void Server::accept_clients() {
    while (true) {
        int client_fd = accept(m_server_fd, nullptr, nullptr);

        if(client_fd < 0) {
            std::cerr << "unable to accept connection" << std::endl;
            continue;
        }

        std::cout << "a new client connected" << std::endl;

        std::lock_guard lock(m_fds_mutex);
        m_fds.emplace_back(pollfd{client_fd, POLLIN, 0});
    }
}

} // hashd

#include "server.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace hashd
{

Server::Server(uint16_t timeout, uint8_t workers_count)
    : m_timeout(timeout), m_workers_count(workers_count)
{}

bool Server::run(uint16_t port, bool &terminate)
{
    if (m_epoll_fd < 0 || m_server_fd < 0) {
        std::cerr << "unable to create descriptor" << std::endl;
        return false;
    }

    if (!m_server_fd.configure_server(port)) {
        return false;
    }

    m_epoll_fd.add_watching_fd(m_server_fd);
    for (uint8_t i = 0; i < m_workers_count; ++i) {
        m_workers.emplace_back(m_timeout);
    }

    for (auto& handler : m_workers) {
        if (!handler.run()) {
            std::cerr << "unable to run worker" << std::endl;
            return false;
        }
        m_epoll_fd.add_watching_fd(handler.event_fd());
    }

    accept_clients(terminate);

    for (auto& handler : m_workers) {
        handler.terminate();
    }

    return true;
}

void Server::accept_clients(bool& terminate) {
    std::multimap<size_t, size_t> clients_queue;
    for (size_t i = 0; i < m_workers.size(); ++i) {
        clients_queue.emplace(0u, i);
    }

    while (!terminate) {
        const auto fds = m_epoll_fd.wait(m_timeout);
        for (const auto fd : fds) {
            if (fd == m_server_fd) {
                SocketDescriptor client_fd(accept(m_server_fd, nullptr, nullptr));

                if(client_fd < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                        continue;
                    }

                    std::cerr << "unable to accept connection" << std::endl;
                    continue;
                }

                if (!client_fd.set_non_blocking()) {
                    std::cerr << "unable to configure client to non blocking mode" << std::endl;
                    continue;
                }

                std::cout << "a new client connected" << std::endl;
                const auto item = clients_queue.begin();
                m_workers[item->second].epoll_fd().add_watching_fd(client_fd);
                m_fds.emplace(client_fd, std::move(client_fd));
                clients_queue.erase(clients_queue.begin());
                clients_queue.emplace(item->first + 1, item->second);
            }
            else {
                uint64_t value;
                read(fd, &value, sizeof(value));

                auto handler = std::find_if(m_workers.begin(), m_workers.end(),
                    [fd](const auto& h){ return h.event_fd() == fd; });

                if (handler == m_workers.end()) {
                    std::cerr << "unable to find worker for received event" << std::endl;
                    continue;
                }

                const auto disconnected = handler->disconnected();
                for (const auto fd : disconnected) {
                    handler->epoll_fd().remove_watching_fd(fd);
                    m_fds.erase(fd);
                }

                size_t idx = std::distance(m_workers.begin(), handler);
                for (auto it = clients_queue.begin(); it != clients_queue.end(); ++it) {
                    if (it->second == idx) {
                        size_t count = it->first - disconnected.size();
                        clients_queue.erase(it);
                        clients_queue.emplace(count, idx);
                    }
                }
            }
        }
    }
}

} // hashd

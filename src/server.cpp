#include "server.hpp"

#include <algorithm>
#include <iostream>
#include <queue>
#include <sys/socket.h>
#include <unistd.h>

namespace
{

using clients_queue_item = std::pair<size_t, size_t>;

class ClientsQueueCompare {
public:
    bool operator()(const clients_queue_item& lhs, const clients_queue_item& rhs) const
    {
        return lhs.first < rhs.first;
    }
};

} // unnamed


namespace hashd
{

Server::Server(uint16_t timeout, uint8_t workers_count)
    : m_timeout(timeout), m_workers_count(workers_count)
{}

bool Server::run(uint16_t port, const bool &terminate)
{
    if (m_epoll_fd < 0 || m_server_fd < 0) {
        std::cerr << "unable to create descriptor" << std::endl;
        return false;
    }

    if (!m_server_fd.configure_server(port)) {
        return false;
    }

    if (!m_epoll_fd.add_watching_fd(m_server_fd)) {
        std::cerr << "unable to configure server description" << std::endl;
        return false;
    }

    for (uint8_t i = 0; i < m_workers_count; ++i) {
        m_workers.emplace_back(m_timeout);
    }

    for (auto& handler : m_workers) {
        if (!handler.run()) {
            std::cerr << "unable to run worker" << std::endl;
            return false;
        }
    }

    accept_clients(terminate);

    for (auto& handler : m_workers) {
        handler.terminate();
    }

    return true;
}

void Server::accept_clients(const bool& terminate) {
    std::vector<SocketDescriptor> new_fds;
    std::priority_queue<clients_queue_item, std::vector<clients_queue_item>, ClientsQueueCompare> clients_queue;

    for (size_t i = 0; i < m_workers.size(); ++i) {
        clients_queue.emplace(0u, i);
    }

    while (!terminate) {
        const auto fds = m_epoll_fd.wait(m_timeout);

        if (fds.empty()) {
            continue;
        }

        while (true) {
            SocketDescriptor client_fd(accept(m_server_fd, nullptr, nullptr), true);

            if(client_fd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                    break;
                }

                std::cerr << "unable to accept connection" << std::endl;
                break;
            }

            if (!client_fd.set_non_blocking()) {
                std::cerr << "unable to configure client to non blocking mode" << std::endl;
                continue;
            }

            std::cout << "a new client connected" << std::endl;
            new_fds.emplace_back(std::move(client_fd));
        }

        if (!new_fds.empty()) {
            const auto [count, worker_id] = clients_queue.top();
            auto& handler = m_workers[worker_id];

            handler.push_new_clients(new_fds);
            constexpr uint64_t value = 1;
            write(handler.event_fd(), &value, sizeof(value));

            clients_queue.pop();
            clients_queue.emplace(handler.clients_count() + new_fds.size(), worker_id);

            new_fds.clear();
        }
    }
}

} // hashd

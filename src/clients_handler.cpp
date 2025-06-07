#include "clients_handler.hpp"
#include "hasher.hpp"

#include <iostream>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <unistd.h>

constexpr size_t C_BUFFER_SIZE = 1024;

namespace hashd
{

ClientsHandler::ClientsHandler(uint16_t timeout)
    : m_timeout(timeout),
    m_terminated(true),
    m_event_fd(eventfd(0, EFD_NONBLOCK)),
    m_buffer(C_BUFFER_SIZE)
{}

ClientsHandler::~ClientsHandler()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

ClientsHandler::ClientsHandler(ClientsHandler&& other)
    : m_timeout(other.m_timeout),
    m_terminated(other.m_terminated),
    m_epoll_fd(std::move(other.m_epoll_fd)),
    m_event_fd(std::move(other.m_event_fd)),
    m_disconnected(std::move(other.m_disconnected)),
    m_thread(std::move(other.m_thread)),
    m_buffer(std::move(other.m_buffer))
{}

bool ClientsHandler::run()
{
    if (m_epoll_fd < 0 | m_event_fd < 0) {
        std::cerr << "unable to create descriptor" << std::endl;
        return false;
    }

    m_terminated = false;
    m_thread = std::thread(&ClientsHandler::process_clients, this);
    return true;
}

void ClientsHandler::terminate()
{
    m_terminated = true;
}

const EpollDescriptor& ClientsHandler::epoll_fd() const
{
    return m_epoll_fd;
}

const SocketDescriptor& ClientsHandler::event_fd() const
{
    return m_event_fd;
}

std::vector<int> ClientsHandler::disconnected()
{
    std::lock_guard lock(m_disconnected_mutex);
    const auto res = m_disconnected;
    m_disconnected.clear();
    return res;
}

void ClientsHandler::process_clients()
{
    while(!m_terminated) {
        const auto fds = m_epoll_fd.wait(m_timeout);

        for (const auto fd : fds) {
            ssize_t count = read(fd, m_buffer.data(), m_buffer.size());

            if (count == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    continue;
                }

                std::cerr << "error while reading data" << std::endl;
                disconnect(fd);
                continue;
            }
            else if (count == 0) {
                std::cout << "client disconnected" << std::endl;
                disconnect(fd);
                continue;
            }

            Hasher sha256_hash;
            sha256_hash.update(m_buffer, count);

            while (true) {
                count = read(fd, m_buffer.data(), m_buffer.size());

                if (count <= 0) {
                    break;
                }

                sha256_hash.update(m_buffer, count);
            }

            const auto hash = sha256_hash.to_string();
            count = send(fd, hash.data(), hash.size(), 0);

            if (count != hash.size()) {
                std::cerr << "data send error, disconnecting client" << std::endl;
                disconnect(fd);
                break;
            }
        }
    }
}

void ClientsHandler::disconnect(int fd)
{
    {
        std::lock_guard lock(m_disconnected_mutex);
        m_disconnected.push_back(fd);
    }
    uint64_t value = 1;
    write(m_event_fd, &value, sizeof(value));
}

} // hashd

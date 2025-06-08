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
    : m_event_fd(eventfd(0, EFD_NONBLOCK)),
    m_client_count(0u),
    m_buffer(C_BUFFER_SIZE),
    m_timeout(timeout),
    m_terminated(true)
{}

ClientsHandler::~ClientsHandler()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

ClientsHandler::ClientsHandler(ClientsHandler&& other) noexcept
    : m_clients(std::move(other.m_clients)),
    m_epoll_fd(std::move(other.m_epoll_fd)),
    m_event_fd(std::move(other.m_event_fd)),
    m_client_count(m_clients.size()),
    m_new_fds(std::move(other.m_new_fds)),
    m_thread(std::move(other.m_thread)),
    m_buffer(std::move(other.m_buffer)),
    m_timeout(other.m_timeout),
    m_terminated(other.m_terminated)
{}

bool ClientsHandler::run()
{
    if (m_epoll_fd < 0 | m_event_fd < 0) {
        std::cerr << "unable to create descriptor" << std::endl;
        return false;
    }

    if (!m_epoll_fd.add_watching_fd(m_event_fd)) {
        std::cerr << "unable setup event descriptor" << std::endl;
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

size_t ClientsHandler::clients_count() const
{
    return m_client_count.load(std::memory_order_acquire);
}

void ClientsHandler::push_new_clients(std::vector<SocketDescriptor>& fds)
{
    std::lock_guard lock(m_new_fds_mutex);
    for (auto& fd : fds) {
        m_new_fds.push_back(std::move(fd));
    }
}

void ClientsHandler::process_clients()
{
    while(!m_terminated) {
        const auto fds = m_epoll_fd.wait(m_timeout);

        for (const auto fd : fds) {
            if (fd == m_event_fd) {
                uint64_t val;
                read(m_event_fd, &val, sizeof(val));
                config_new_clients();
                continue;
            }

            ssize_t count = read(fd, m_buffer.data(), m_buffer.size());

            if (count == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    continue;
                }

                std::cerr << "error while reading data" << std::endl;
                disconnect(fd);
                continue;
            }
            if (count == 0) {
                std::cout << "client disconnected" << std::endl;
                disconnect(fd);
                continue;
            }

            auto& ctx = m_clients[fd];

            while (count > 0) {
                auto processed = m_buffer.begin();
                for (auto it = m_buffer.begin(); it < m_buffer.begin() + count; ++it) {
                    if (*it == '\n') {
                        ctx.m_hasher.update(&*processed, std::distance(processed, it));
                        const auto hash = ctx.m_hasher.to_string();

                        if (!hash.empty()) {
                            if (send(fd, hash.data(), hash.size(), 0) != hash.size()) {
                                std::cerr << "data send error, disconnecting client" << std::endl;
                                disconnect(fd);
                                count = -1;
                                break;
                            }
                        }

                        processed = it + 1;
                    }
                }

                if (const size_t remains = count - std::distance(m_buffer.begin(), processed)) {
                    ctx.m_hasher.update(&*processed, remains);
                }

                count = read(fd, m_buffer.data(), m_buffer.size());
            }
        }
    }
}

void ClientsHandler::config_new_clients()
{
    std::vector<SocketDescriptor> fds;

    {
        std::lock_guard lock(m_new_fds_mutex);
        std::swap(fds, m_new_fds);
    }

    for (auto& fd : fds) {
        if (!m_epoll_fd.add_watching_fd(fd)) {
            std::cerr << "unable to configure client" << std::endl;
            continue;
        }

        m_clients.emplace(std::move(fd), ClientContext());
    }

    m_client_count.store(m_clients.size(), std::memory_order_release);
}

void ClientsHandler::disconnect(int fd)
{
    if (!m_epoll_fd.remove_watching_fd(fd)) {
        std::cerr << "error while disconnecting client" << std::endl;
    }
    m_clients.erase(fd);
}

ClientsHandler::ClientContext::ClientContext(ClientContext&& other) noexcept
    : m_hasher(std::move(other.m_hasher))
{}

ClientsHandler::ClientContext& ClientsHandler::ClientContext::operator=(ClientContext&& other) noexcept
{
    if (this != &other) {
        m_hasher = std::move(other.m_hasher);
    }
    return *this;
}

} // hashd

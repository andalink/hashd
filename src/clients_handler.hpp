#ifndef CLIENTS_HANDLER_H
#define CLIENTS_HANDLER_H

#include "epoll_descriptor.hpp"
#include "hasher.hpp"
#include "socket_descriptor.hpp"

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

namespace hashd
{

class ClientsHandler
{
public:
    ClientsHandler(uint16_t timeout);
    ~ClientsHandler();
    ClientsHandler(const ClientsHandler&) = delete;
    ClientsHandler& operator=(const ClientsHandler&) = delete;
    ClientsHandler(ClientsHandler&& other) noexcept;

    bool run();
    void terminate();
    const EpollDescriptor& epoll_fd() const;
    const SocketDescriptor& event_fd() const;
    size_t clients_count() const;
    void push_new_clients(std::vector<SocketDescriptor>& fds);

private:
    struct ClientContext {
        ClientContext() = default;
        ClientContext(const ClientContext&) = delete;
        ClientContext& operator=(const ClientContext&) = delete;
        ClientContext(ClientContext&& other) noexcept;
        ClientContext& operator=(ClientContext&& other) noexcept;

        Hasher m_hasher;
    };

    void process_clients();
    void config_new_clients();
    void disconnect(int fd);

    std::unordered_map<SocketDescriptor, ClientContext> m_clients;
    EpollDescriptor m_epoll_fd;
    SocketDescriptor m_event_fd;
    std::atomic<size_t> m_client_count;
    std::vector<SocketDescriptor> m_new_fds;
    std::mutex m_new_fds_mutex;
    std::thread m_thread;
    std::vector<uint8_t> m_buffer;
    uint16_t m_timeout;
    bool m_terminated;
};

} // hashd

#endif // CLIENTS_HANDLER_H

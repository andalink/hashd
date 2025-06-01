#ifndef CLIENTS_HANDLER_H
#define CLIENTS_HANDLER_H

#include "epoll_descriptor.hpp"
#include "socket_descriptor.hpp"

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
    ClientsHandler(ClientsHandler&& other);

    bool run();
    void terminate();
    const EpollDescriptor& epoll_fd() const;
    const SocketDescriptor& event_fd() const;
    std::vector<int> disconnected();

private:
    void process_clients();
    void disconnect(int fd);

    uint16_t m_timeout;
    bool m_terminated;
    EpollDescriptor m_epoll_fd;
    SocketDescriptor m_event_fd;
    std::vector<int> m_disconnected;
    std::mutex m_disconnected_mutex;
    std::thread m_thread;
    std::vector<uint8_t> m_buffer;
};

} // hashd

#endif // CLIENTS_HANDLER_H

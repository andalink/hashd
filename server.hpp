#ifndef SERVER_HPP
#define SERVER_HPP

#include "clients_handler.hpp"
#include "epoll_descriptor.hpp"
#include "socket_descriptor.hpp"

#include <mutex>
#include <stdint.h>
#include <unordered_map>

namespace hashd
{

class Server
{
public:
    bool run(uint16_t port, bool& terminate);

private:
    void accept_clients(bool& terminate);

    std::unordered_map<int, const SocketDescriptor> m_fds;
    std::vector<ClientsHandler> m_workers;
    const EpollDescriptor m_epoll_fd;
    const SocketDescriptor m_server_fd;
};

} // hashd

#endif // SERVER_HPP

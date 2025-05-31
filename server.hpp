#ifndef SERVER_HPP
#define SERVER_HPP

#include "epoll_descriptor.hpp"
#include "socket_descriptor.hpp"

#include <mutex>
#include <stdint.h>
#include <unordered_map>
#include <vector>

namespace hashd
{

class Server
{
public:
    void run(uint16_t port);

private:
    void accept_clients();

    std::unordered_map<int, const SocketDescriptor> m_fds;
    std::mutex m_fds_mutex;
    const SocketDescriptor m_server_fd;
    const EpollDescriptor m_epoll_fd;
};

} // hashd

#endif // SERVER_HPP

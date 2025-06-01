#include "socket_descriptor.hpp"

#include <iostream>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace hashd
{

SocketDescriptor::SocketDescriptor()
    : m_fd(socket(AF_INET, SOCK_STREAM, 0))
{}

SocketDescriptor::SocketDescriptor(int fd)
    : m_fd(fd)
{}

SocketDescriptor::~SocketDescriptor()
{
    if (m_fd != -1) {
        close(m_fd);
    }
}

SocketDescriptor::SocketDescriptor(SocketDescriptor&& other)
    : m_fd(other)
{
    other.m_fd = -1;
}

SocketDescriptor& SocketDescriptor::operator=(SocketDescriptor&& other)
{
    if (this != &other) {
        m_fd = other.m_fd;
        other.m_fd = -1;
    }
    return *this;
}

bool SocketDescriptor::configure_server(uint16_t port) const
{
    if (m_fd < 0) {
        std::cerr << "error while creating socket" << std::endl;
        return false;
    }

    const int enable = 1;
    if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        std::cerr << "error while setting options for socket" << std::endl;
        return false;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if(bind(m_fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) < 0) {
        std::cerr << "error while binding socket" << std::endl;
        return false;
    }

    if(listen(m_fd, 3) < 0) {
        std::cerr << "unable to listen socket" << std::endl;
        return false;
    }

    if (!set_non_blocking()) {
        std::cerr << "unable to configure server to non blocking mode" << std::endl;
        return false;
    }

    return true;
}

bool SocketDescriptor::set_non_blocking() const
{
    int flags = fcntl(m_fd, F_GETFL, 0);
    if (flags < 0) {
        std::cerr << "unable to get soket flags" << std::endl;
        return false;
    }

    if (fcntl(m_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        std::cerr << "unable to set soket flags" << std::endl;
        return false;
    }

    return true;
}

SocketDescriptor::operator int() const
{
    return m_fd;
}

} // hashd

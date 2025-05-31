#ifndef SOCKET_DESCRIPTOR_H
#define SOCKET_DESCRIPTOR_H

#include <stdint.h>

namespace hashd
{

class SocketDescriptor
{
public:
    SocketDescriptor();
    SocketDescriptor(int fd);
    ~SocketDescriptor();
    SocketDescriptor(const SocketDescriptor&) = delete;
    SocketDescriptor& operator=(const SocketDescriptor&) = delete;
    SocketDescriptor(SocketDescriptor&& other);
    SocketDescriptor& operator=(SocketDescriptor&& other);

    bool configure_server(uint16_t port) const;
    bool set_non_blocking() const;
    operator int() const;

private:
    int m_fd;
};

} // hashd

#endif // SOCKET_DESCRIPTOR_H

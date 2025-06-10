#ifndef SOCKET_DESCRIPTOR_H
#define SOCKET_DESCRIPTOR_H

#include <cstdint>
#include <functional>
#include <limits>

namespace hashd
{

class SocketDescriptor
{
public:
    SocketDescriptor();
    SocketDescriptor(int fd, bool inuse = false);
    ~SocketDescriptor();
    SocketDescriptor(const SocketDescriptor&) = delete;
    SocketDescriptor& operator=(const SocketDescriptor&) = delete;
    SocketDescriptor(SocketDescriptor&& other) noexcept;
    SocketDescriptor& operator=(SocketDescriptor&& other) noexcept;

    bool configure_server(uint16_t port) const;
    bool set_non_blocking() const;
    operator int() const;

private:
    int m_fd;
    bool m_inuse;
};

} // hashd

namespace std
{

template <>
struct hash<hashd::SocketDescriptor>
{
    size_t operator()(const hashd::SocketDescriptor& fd) const noexcept
    {
        return fd == -1 ? numeric_limits<size_t>::max() : static_cast<size_t>(fd);
    }
};

}

#endif // SOCKET_DESCRIPTOR_H

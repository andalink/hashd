#ifndef EPOLL_DESCRIPTOR_HPP
#define EPOLL_DESCRIPTOR_HPP

#include <stdint.h>
#include <vector>

namespace hashd
{

class EpollDescriptor
{
public:
    static constexpr int C_MAX_EVENTS = 10;

    EpollDescriptor();
    ~EpollDescriptor();
    EpollDescriptor(const EpollDescriptor&) = delete;
    EpollDescriptor& operator=(const EpollDescriptor&) = delete;

    bool add_watching_fd(int fd) const;
    bool remove_watching_fd(int fd) const;
    std::vector<int> wait(uint16_t timeout) const;
    operator int() const;

private:
    const int m_fd;
};

} // hashd

#endif // EPOLL_DESCRIPTOR_HPP

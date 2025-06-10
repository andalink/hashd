#include "epoll_descriptor.hpp"

#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>

namespace hashd
{

EpollDescriptor::EpollDescriptor()
    : m_fd(epoll_create1(0))
{}

EpollDescriptor::~EpollDescriptor()
{
    if (m_fd != -1) {
        close(m_fd);
    }
}

EpollDescriptor::EpollDescriptor(EpollDescriptor&& other) noexcept
    : m_fd(other)
{
    other.m_fd = -1;
}

EpollDescriptor& EpollDescriptor::operator=(EpollDescriptor&& other) noexcept
{
    if (this != &other) {
        if (m_fd != -1) {
            close(m_fd);
        }

        m_fd = other.m_fd;
        other.m_fd = -1;
    }
    return *this;
}

bool EpollDescriptor::add_watching_fd(int fd) const
{
    epoll_event event{};
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = fd;
    if (epoll_ctl(m_fd, EPOLL_CTL_ADD, fd, &event) < 0) {
        std::cerr << "unable to add fd for polling" << std::endl;
        return false;
    }

    return true;
}

bool EpollDescriptor::remove_watching_fd(int fd) const
{
    if (epoll_ctl(m_fd, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        std::cerr << "unable to remove fd from polling" << std::endl;
        return false;
    }

    return true;
}

std::vector<int> EpollDescriptor::wait(uint16_t timeout) const
{
    epoll_event events[C_MAX_EVENTS];
    int count = epoll_wait(m_fd, events, C_MAX_EVENTS, timeout);

    if (count < 0) {
        if (errno != EINTR) {
            std::cerr << "error while polling socket" << std::endl;
        }
        return {};
    }

    std::vector<int> res;
    for (int i = 0; i < count; ++i) {
        if (events[i].events & EPOLLIN) {
            res.push_back(events[i].data.fd);
        }
    }

    return res;
}

EpollDescriptor::operator int() const
{
    return m_fd;
}

} // hashd

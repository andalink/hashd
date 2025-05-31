#ifndef SERVER_HPP
#define SERVER_HPP

#include <poll.h>
#include <stdint.h>
#include <vector>

namespace hashd
{

class Server
{
public:
    Server();
    ~Server();
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    void run(uint16_t port);

private:
    bool prepare_server_socket(uint16_t port);

    std::vector<pollfd> m_fds;
    const int m_server_fd;
};

} // hashd

#endif // SERVER_HPP

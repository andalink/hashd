#ifndef SERVER_HPP
#define SERVER_HPP

#include <mutex>
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
    void accept_clients();

    std::vector<pollfd> m_fds;
    std::mutex m_fds_mutex;
    const int m_server_fd;
};

} // hashd

#endif // SERVER_HPP

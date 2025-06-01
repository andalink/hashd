#include "server.hpp"

#include <csignal>

static bool terminate = false;

void sig_handler(int signal __attribute__((unused)))
{
    terminate = true;

}

int main()
{
    std::signal(SIGINT, sig_handler);
    std::signal(SIGTERM, sig_handler);


    hashd::Server s;
    bool res = s.run(11011, terminate);

    return res ? 0 : 1;
}

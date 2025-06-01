#include "server.hpp"

#include <csignal>
#include <iostream>

constexpr uint16_t C_DEFAULT_PORT = 11111;
constexpr uint16_t C_DEFAULT_TIMEOUT = 100;
constexpr uint8_t C_DEFAULT_WORKERS = 4;
static bool terminate = false;

void sig_handler(int signal __attribute__((unused)))
{
    terminate = true;
}

int main(int argc, char *argv[])
{
    std::signal(SIGINT, sig_handler);
    std::signal(SIGTERM, sig_handler);

    uint16_t port = C_DEFAULT_PORT;
    uint16_t timeout = C_DEFAULT_TIMEOUT;
    uint8_t workers = C_DEFAULT_WORKERS;

    int opt;
    while((opt = getopt(argc, argv, "p:t:w:h")) != -1) {
        switch(opt) {
        case 'p':
            try {
                port = static_cast<uint16_t>(std::stoi(std::string(optarg)));
            }
            catch (...) {
                std::cerr << "invalid value of port argument -p: " << optarg << std::endl;
                return 1;
            }
            break;
        case 't':
            try {
                timeout = static_cast<uint16_t>(std::stoi(std::string(optarg)));
            }
            catch (...) {
                std::cerr << "invalid value of timeout argument -t: " << optarg << std::endl;
                return 1;
            }
            break;
        case 'w':
            try {
                workers = static_cast<uint16_t>(std::stoi(std::string(optarg)));
            }
            catch (...) {
                std::cerr << "invalid value of workers argument -w: " << optarg << std::endl;
                return 1;
            }
            break;
        case 'h':
            std::cout <<
                "-p port number (default " << C_DEFAULT_PORT << ")\n" <<
                "-e poll timeout in milliseconds (default " << C_DEFAULT_TIMEOUT << ")\n" <<
                "-w workers count (default " << static_cast<int>(C_DEFAULT_WORKERS) << ")" << std::endl;
            return 0;
        }
    }

    std::cout << "starting server with parameters:\n" <<
        "\tport=" << port << "\n" <<
        "\ttimeout=" << timeout << "\n" <<
        "\tworkers count=" << static_cast<int>(workers) << std::endl;

    hashd::Server s(timeout, workers);
    bool res = s.run(port, terminate);

    return res ? 0 : 1;
}

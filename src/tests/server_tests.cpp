#include "../server.hpp"

#include <gtest/gtest.h>
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <poll.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

auto create_client(uint16_t port)
{
    const int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        std::cerr << "client: unable to create socket" << std::endl;
        return std::unique_ptr<int, void(*)(const int*)>(nullptr, [](const int*){});
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "client: error while converting address" << std::endl;
        return std::unique_ptr<int, void(*)(const int*)>(nullptr, [](const int*){});
    }

    if (connect(fd, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) < 0) {
        std::cerr << "client: error while converting address" << std::endl;
        return std::unique_ptr<int, void(*)(const int*)>(nullptr, [](const int*){});
    }

    std::unique_ptr<int, void(*)(const int*)> client_ptr(new int(fd),
        [](const int* s){close(*s);});

    return client_ptr;
}

auto create_server(uint16_t port, const bool& terminate)
{
    std::unique_ptr<std::thread, void(*)(std::thread*)> server_ptr(
        new std::thread([port, &terminate]() {
                hashd::Server s(100, 1);
                s.run(port, terminate);
            }),
        [](std::thread* t){t->join();});

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return server_ptr;
}

bool check_buffer(const uint8_t* buffer, size_t buffer_size, const std::string& expected)
{
    if (expected.size() > buffer_size) {
        return false;
    }

    for (size_t i = 0; i < expected.size(); ++i) {
        if (buffer[i] != expected[i]) {
            return false;
        }
    }

    if (buffer[expected.size()] != '\n') {
        return false;
    }

    return true;
}

TEST(ServerTest, ClientConnection)
{
    bool terminate = false;
    auto srv = create_server(11111, terminate);

    {
        const auto fd = create_client(11111);
        ASSERT_TRUE(fd);
        ASSERT_GE(*fd, 0);
    }

    terminate = true;
}

TEST(ServerTest, EmptyData)
{
    bool terminate = false;
    auto srv = create_server(11111, terminate);
    const auto fd = create_client(11111);

    const std::vector<uint8_t> test = {'\n'};
    send(*fd, test.data(), test.size(), 0);

    pollfd fds = {};
    fds.fd = *fd;
    fds.events = POLLIN;
    ASSERT_EQ(poll(&fds, 1, 100), 0);

    terminate = true;
}

TEST(ServerTest, SimpleData)
{
    bool terminate = false;
    auto srv = create_server(11111, terminate);
    const auto fd = create_client(11111);

    const std::vector<uint8_t> test = {'f', 'o', 'o', '\n'};
    send(*fd, test.data(), test.size(), 0);

    std::vector<uint8_t> buffer(1024);
    const ssize_t len = read(*fd, buffer.data(), buffer.size());
    ASSERT_EQ(len, 65);
    ASSERT_TRUE(check_buffer(buffer.data(), len, "2c26b46b68ffc68ff99b453c1d30413413422d706483bfa0f98a5e886266e7ae"));

    terminate = true;
}

TEST(ServerTest, DataInSeveralMessages)
{
    bool terminate = false;
    auto srv = create_server(11111, terminate);
    const auto fd = create_client(11111);

    const std::vector<uint8_t> test1 = {'f', 'o'};
    send(*fd, test1.data(), test1.size(), 0);
    const std::vector<uint8_t> test2 = {'o', '\n'};
    send(*fd, test2.data(), test2.size(), 0);

    std::vector<uint8_t> buffer(1024);
    const ssize_t len = read(*fd, buffer.data(), buffer.size());
    ASSERT_EQ(len, 65);
    ASSERT_TRUE(check_buffer(buffer.data(), len, "2c26b46b68ffc68ff99b453c1d30413413422d706483bfa0f98a5e886266e7ae"));

    terminate = true;
}

TEST(ServerTest, DataWithRedundantLines)
{
    bool terminate = false;
    auto srv = create_server(11111, terminate);
    const auto fd = create_client(11111);

    const std::vector<uint8_t> test = {'\n', 'f', 'o', 'o', '\n', '\n'};
    send(*fd, test.data(), test.size(), 0);

    std::vector<uint8_t> buffer(1024);
    const ssize_t len = read(*fd, buffer.data(), buffer.size());
    ASSERT_EQ(len, 65);
    ASSERT_TRUE(check_buffer(buffer.data(), len, "2c26b46b68ffc68ff99b453c1d30413413422d706483bfa0f98a5e886266e7ae"));

    terminate = true;
}

TEST(ServerTest, TwoStringsInOneMessage)
{
    bool terminate = false;
    auto srv = create_server(11111, terminate);
    const auto fd = create_client(11111);

    const std::vector<uint8_t> test = {'f', 'o', 'o', '\n', 'b', 'a', 'r', '\n'};
    send(*fd, test.data(), test.size(), 0);

    pollfd fds = {};
    fds.fd = *fd;
    fds.events = POLLIN;

    std::vector<uint8_t> buffer(1024);
    size_t total_len = 0;
    while (poll(&fds, 1, 100) > 0) {
        if (const ssize_t len = read(*fd, buffer.data() + total_len, buffer.size() - total_len); len > 0) {
            total_len += len;
        }
    }

    ASSERT_EQ(total_len, 130);

    ASSERT_TRUE(check_buffer(buffer.data(), 65, "2c26b46b68ffc68ff99b453c1d30413413422d706483bfa0f98a5e886266e7ae"));
    ASSERT_TRUE(check_buffer(buffer.data() + 65, 65, "fcde2b2edba56bf408601fb721fe9b5c338d10ee429ea04fae5511b68fbf8fb9"));

    terminate = true;
}

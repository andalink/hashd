#include "../socket_descriptor.hpp"

#include <gtest/gtest.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

TEST(SocketDescriptorTest, DefaultConstructor) {
    const hashd::SocketDescriptor sd;
    const int fd = sd;
    EXPECT_GE(fd, 0);
}

TEST(SocketDescriptorTest, ConstructWithFd) {
    const int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(socket_fd, 0);

    {
        const hashd::SocketDescriptor sd(socket_fd);
        const int fd = sd;
        EXPECT_EQ(fd, socket_fd);
    }

    EXPECT_EQ(close(socket_fd), 0);
}

TEST(SocketDescriptorTest, ConstructWithFdInuse) {
    const int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(socket_fd, 0);

    {
        const hashd::SocketDescriptor sd(socket_fd, true);
        const int fd = sd;
        EXPECT_EQ(fd, socket_fd);
    }

    EXPECT_LT(close(socket_fd), 0);
}


TEST(SocketDescriptorTest, MoveConstructorCall) {
    const int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(socket_fd, 0);

    {
        hashd::SocketDescriptor sd1(socket_fd, true);
        const hashd::SocketDescriptor sd2 = std::move(sd1);

        const int sd2_fd = sd2;
        EXPECT_EQ(sd2_fd, socket_fd);
    }

    EXPECT_LT(close(socket_fd), 0);
}

TEST(SocketDescriptorTest, MoveAssignmentCall) {
    const int socket_fd1 = socket(AF_INET, SOCK_STREAM, 0);
    const int socket_fd2 = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(socket_fd1, 0);
    ASSERT_GE(socket_fd2, 0);

    {
        hashd::SocketDescriptor sd1(socket_fd1, true);
        hashd::SocketDescriptor sd2(socket_fd2, true);

        sd2 = std::move(sd1);

        const int sd2_fd = sd2;
        EXPECT_EQ(sd2_fd, socket_fd1);
    }

    EXPECT_LT(close(socket_fd1), 0);
    EXPECT_LT(close(socket_fd2), 0);
}

TEST(SocketDescriptorTest, SetNonBlocking) {
    const int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(socket_fd, 0);

    const hashd::SocketDescriptor sd(socket_fd, true);
    EXPECT_TRUE(sd.set_non_blocking());

    const int flags = fcntl(socket_fd, F_GETFL, 0);
    EXPECT_TRUE(flags & O_NONBLOCK);
}
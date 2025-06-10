#include "../hasher.hpp"

#include <gtest/gtest.h>
#include <iomanip>
#include <openssl/sha.h>
#include <sstream>
#include <string>
#include <vector>

using namespace hashd;

const std::vector<uint8_t> test_foo = {'f', 'o', 'o'};
const std::vector<uint8_t> test_bar = {'b', 'a', 'r'};

std::string sha256_to_hex(const std::vector<uint8_t>& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(data.data(), data.size(), hash);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const uint8_t c : hash) {
        ss << std::setw(2) << static_cast<int>(c);
    }
    ss << '\n';
    return ss.str();
}

TEST(HasherTest, HashSingleUpdate) {
    Hasher h;

    h.update(test_foo.data(), test_foo.size());
    const std::string hash = h.to_string();

    EXPECT_EQ(hash, sha256_to_hex(test_foo));
}

TEST(HasherTest, HashMultipleUpdates) {
    std::vector<uint8_t> data = test_foo;
    data.insert(data.end(), test_bar.cbegin(), test_bar.cend());

    Hasher h;
    h.update(test_foo.data(), test_foo.size());
    h.update(test_bar.data(), test_bar.size());
    const std::string hash = h.to_string();

    EXPECT_EQ(hash, sha256_to_hex(data));
}

TEST(HasherTest, MoveConstructorCall) {
    Hasher h1;
    h1.update(test_foo.data(), test_foo.size());

    Hasher h2 = std::move(h1);
    const std::string hash = h2.to_string();

    EXPECT_EQ(hash, sha256_to_hex(test_foo));
}

TEST(HasherTest, MoveAssignmentCall) {
    Hasher h1;
    h1.update(test_foo.data(), test_foo.size());

    Hasher h2;
    h2.update(test_bar.data(), test_bar.size());
    h2 = std::move(h1);
    const std::string hash = h2.to_string();

    EXPECT_EQ(h1.to_string(), "");
    EXPECT_EQ(hash, sha256_to_hex(test_foo));
}


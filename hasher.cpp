#include "hasher.hpp"

#include <iomanip>
#include <openssl/sha.h>
#include <sstream>

namespace hashd {

Hasher::Hasher(std::vector<uint8_t> data, size_t size)
    : m_hash(SHA256_DIGEST_LENGTH)
{
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.data(), size);
    SHA256_Final(m_hash.data(), &sha256);
}

std::string Hasher::to_string()
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t c : m_hash) {
        ss << std::setw(2) << static_cast<int>(c);
    }
    ss << '\n';
    return ss.str();
}

} // hashd

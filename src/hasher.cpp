#include "hasher.hpp"

#include <iomanip>
#include <openssl/sha.h>
#include <sstream>

namespace hashd {

struct Hasher::Context
{
    SHA256_CTX sha256;
};

Hasher::Hasher()
    : m_hash(SHA256_DIGEST_LENGTH)
{}

Hasher::~Hasher() = default;

Hasher::Hasher(Hasher&& other) noexcept
    : m_hash(std::move(other.m_hash)),
    m_context(std::move(other.m_context))
{}

Hasher& Hasher::operator=(Hasher&& other) noexcept
{
    if (this != &other) {
        m_hash = std::move(other.m_hash);
        m_context = std::move(other.m_context);
    }
    return *this;
}

void Hasher::update(const std::vector<uint8_t>& data, size_t size)
{
    if (!m_context) {
        m_context = std::make_unique<Context>();
        SHA256_Init(&m_context->sha256);
    }

    SHA256_Update(&m_context->sha256, data.data(), size);
}

std::string Hasher::to_string()
{
    if (!m_context) {
        return {};
    }

    auto ctx = m_context.release();
    SHA256_Final(m_hash.data(), &ctx->sha256);
    delete ctx;

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const uint8_t c : m_hash) {
        ss << std::setw(2) << static_cast<int>(c);
    }
    ss << '\n';
    return ss.str();
}

} // hashd

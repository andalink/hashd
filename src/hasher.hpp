#ifndef HASHER_HPP
#define HASHER_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace hashd
{

class Hasher
{
public:
    Hasher();
    ~Hasher();
    Hasher(const Hasher&) = delete;
    Hasher& operator=(const Hasher&) = delete;
    Hasher(Hasher&& other) noexcept;
    Hasher& operator=(Hasher&& other) noexcept;

    void update(const std::vector<uint8_t>& data, size_t size);
    std::string to_string();

private:
    struct Context;

    std::vector<uint8_t> m_hash;
    std::unique_ptr<Context> m_context;
};

} // hashd

#endif // HASHER_HPP

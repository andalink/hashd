#ifndef HASHER_HPP
#define HASHER_HPP

#include <stdint.h>
#include <string>
#include <vector>

namespace hashd
{

class Hasher
{
public:
    Hasher(const std::vector<uint8_t>& data, size_t size);
    std::string to_string();

private:
    std::vector<uint8_t> m_hash;
};

std::vector<uint8_t> sha256_hash(std::vector<uint8_t> data, size_t size);

} // hashd

#endif // HASHER_HPP

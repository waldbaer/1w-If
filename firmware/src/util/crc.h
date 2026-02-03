#ifndef OWIF_UTIL_CRC_H
#define OWIF_UTIL_CRC_H

#include <cstdint>

namespace owif {
namespace util {

auto crc8(std::uint8_t const* data, std::uint8_t len) -> std::uint8_t;

}  // namespace util
}  // namespace owif

#endif  // OWIF_UTIL_CRC_H

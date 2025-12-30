#ifndef OWIF_UTIL_BIT_OPERATIONS_H
#define OWIF_UTIL_BIT_OPERATIONS_H

// ---- Includes ----

#include <cstdint>

namespace owif {
namespace util {

inline auto bswap64(uint64_t v) -> std::uint64_t {
  return ((v & 0x00000000000000FFULL) << 56) | ((v & 0x000000000000FF00ULL) << 40) |
         ((v & 0x0000000000FF0000ULL) << 24) | ((v & 0x00000000FF000000ULL) << 8) | ((v & 0x000000FF00000000ULL) >> 8) |
         ((v & 0x0000FF0000000000ULL) >> 24) | ((v & 0x00FF000000000000ULL) >> 40) |
         ((v & 0xFF00000000000000ULL) >> 56);
}

}  // namespace util
}  // namespace owif

#endif  // OWIF_UTIL_BIT_OPERATIONS_H

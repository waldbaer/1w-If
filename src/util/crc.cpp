#include "util/crc.h"

namespace owif {
namespace one_wire {

auto crc8(std::uint8_t const* data, std::uint8_t len) -> std::uint8_t {
  std::uint8_t crc{0};

  while ((len--) != 0u) {
    std::uint8_t inbyte{*data++};
    for (std::uint8_t i{8}; i != 0u; i--) {
      bool mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix) {
        crc ^= 0x8C;
      }
      inbyte >>= 1;
    }
  }
  return crc;
}

}  // namespace one_wire
}  // namespace owif

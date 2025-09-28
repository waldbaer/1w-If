
#include "logging/logutil.h"

#include <Arduino.h>

#include <cstdint>

#include "logging/logger.h"
#include "one_wire/one_wire_address.h"

namespace owif {
namespace logging {

auto FormatHexChar(std::uint8_t v) -> char { return v >= 10 ? 'A' + (v - 10) : '0' + v; }

auto LogUInt64Hex(std::uint64_t value) -> void {
  char buffer[17];  // 16 Hex-Characters + Null-Terminator
  for (int i = 15; i >= 0; --i) {
    std::uint8_t nibble{static_cast<std::uint8_t>(value & 0x0F)};
    buffer[i] = FormatHexChar(nibble);
    value >>= 4;
  }
  buffer[16] = '\0';

  logger_g.Verbose(buffer);
}

}  // namespace logging
}  // namespace owif

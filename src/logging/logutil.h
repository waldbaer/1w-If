
#ifndef OWIF_LOGGING_LOG_UTIL_H
#define OWIF_LOGGING_LOG_UTIL_H

#include <cstdint>

#include "one_wire/one_wire_address.h"

namespace owif {
namespace logging {

auto FormatHexChar(std::uint8_t v) -> char;

auto LogUInt64Hex(std::uint64_t value) -> void;

}  // namespace logging
}  // namespace owif

#endif  // OWIF_LOGGING_LOG_UTIL_H

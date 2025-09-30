#include "one_wire/one_wire_address.h"

#include <Arduino.h>
#include <WString.h>
#include <stdint.h>

#include <cstdint>
#include <memory>
#include <string>

#include "logging/logger.h"
#include "logging/logutil.h"
#include "util/crc.h"
#include "util/language.h"

namespace owif {
namespace one_wire {

auto OneWireAddress::FromOwfsFormat(String address) -> std::unique_ptr<OneWireAddress> {
  // no CRC, but with separation '.' -> 16 - 2 + 1 = 15
  if (address.length() == 15) {
    char hex[16];  // max. length: 15 characters + null termination
    std::uint8_t pos{0};

    for (std::size_t i{0}; i < address.length() && pos < 16; i++) {
      // Skip '.' in formatted address OWFS (e.g. "28.8F0945161302")
      if (address[i] != '.') {
        hex[pos++] = address[i];
      }
    }
    hex[pos] = '\0';  // add null-termination

    std::uint64_t parsed_address{std::stoull(hex, nullptr, 16)};
    parsed_address = __builtin_bswap64(parsed_address);

    // Calc CRC missing in the input string format.
    std::uint8_t const crc{util::crc8(reinterpret_cast<std::uint8_t*>(&parsed_address) + 1, 7)};
    parsed_address = (parsed_address >> 8) | (static_cast<std::uint64_t>(crc) << 56);

    return std::make_unique<OneWireAddress>(parsed_address);
  } else {
    logging::logger_g.Error(F("[OneWireAddress] Failed to parse 1-Wire address '%s'"), address);
    return std::unique_ptr<OneWireAddress>{nullptr};
  }
}

OneWireAddress::OneWireAddress(std::uint64_t addr) : address_{addr} {}

auto OneWireAddress::GetFullAddress() const -> std::uint64_t { return address_; }

auto OneWireAddress::GetFamilyCode() const -> std::uint8_t { return static_cast<std::uint8_t>(address_ & 0xFF); }

auto OneWireAddress::GetCrc() const -> std::uint8_t { return static_cast<std::uint8_t>(address_ >> 56); }

/*!
 * Formatting of OWFS  | std::uint64 of DS2484
 * 01.FEF49A1A0000     | 3F00001A9AF4FE01
 * 01.D2C79A1A0000     | 3E00001A9AC7D201
 * format
 * | 8bit CRC | 48 bit (9 byte) ID | 8-bit family code |
 */

auto OneWireAddress::Format() const -> String {
  String buffer{"00.000000000000"};  // 16 Hex-Characters + '.'

  uint8_t const* ptr = reinterpret_cast<const uint8_t*>(&address_);

  std::uint8_t pos{0};
  for (std::uint8_t i{0}; i < 7; ++i) {                   // skip last byte (CRC)
    std::uint8_t byte{ptr[i]};                            // LSB first
    buffer[pos++] = logging::FormatHexChar(byte >> 4);    // high nibble
    buffer[pos++] = logging::FormatHexChar(byte & 0x0F);  // low nibble

    if (i == 0) {
      buffer[pos++] = '.';
    }
  }

  buffer[pos] = '\0';

  return buffer;
}

auto OneWireAddress::operator==(OneWireAddress const& other) const -> bool { return address_ == other.address_; }

auto OneWireAddress::operator==(std::uint64_t const& other) const -> bool { return address_ == other; }

auto OneWireAddress::operator!=(OneWireAddress const& other) const -> bool { return address_ != other.address_; }

auto OneWireAddress::operator!=(std::uint64_t const& other) const -> bool { return address_ != other; }

auto OneWireAddress::operator<(OneWireAddress const& other) const -> bool { return address_ < other.address_; }

}  // namespace one_wire
}  // namespace owif

#ifndef OWIF_UTIL_FIXED_STRING_H
#define OWIF_UTIL_FIXED_STRING_H

#include <Arduino.h>

#include <cstdint>
#include <cstdlib>
#include <limits>

#include "logging/logger.h"

namespace owif {
namespace util {

template <std::uint16_t kStorageSize = 64>
class FixedString {
 public:
  using LengthType = std::uint16_t;
  using BufferType = char;
  using BufferConstPtrType = char const*;
  static constexpr std::size_t kCapacity{kStorageSize};

  static constexpr std::size_t kMinStorageSize{0};
  // Maximum: limited by LengthField - 1 byte null termination => 254
  static constexpr std::size_t kMaxStorage{std::numeric_limits<LengthType>::max()};
  static_assert(kStorageSize >= kMinStorageSize && kStorageSize <= kMaxStorage, "FixedString length limit violation");

  // ---- Constructor / Destructor -------------------------------------------------------------------------------------
  FixedString() { buffer_[0] = '\0'; }

  FixedString(char const* value) {
    std::size_t const intput_string_length{strlen(value)};
    if (intput_string_length > kStorageSize) {
      logging::logger_g.Abort(F("FixedString length '%d' exceeds max. limit of %d characters"), length_, kStorageSize);
    }
    length_ = static_cast<LengthType>(intput_string_length);
    strcpy(buffer_, value);
  }

  FixedString(String const& value) {
    std::size_t const intput_string_length{value.length()};
    if (length_ > kStorageSize) {
      logging::logger_g.Abort(F("FixedString length '%d' exceeds max. limit of %d characters"), length_, kStorageSize);
    }
    length_ = static_cast<LengthType>(intput_string_length);
    strcpy(buffer_, value.c_str());
  }

  FixedString(FixedString const&) = default;
  auto operator=(FixedString const&) -> FixedString& = default;
  FixedString(FixedString&&) = default;
  auto operator=(FixedString&&) -> FixedString& = default;
  ~FixedString() = default;

  // ---- Public APIs --------------------------------------------------------------------------------------------------

  auto length() const -> LengthType { return length_; }
  auto c_str() const -> BufferConstPtrType { return &buffer_[0]; }
  auto capacity() const -> std::size_t { return kStorageSize; };

  auto to_string() const -> String { return String{buffer_, length_}; }

 private:
  LengthType length_{0};
  char buffer_[kStorageSize + 1];  // +1 for null-termination
};

}  // namespace util
}  // namespace owif

#endif  // OWIF_UTIL_FIXED_STRING_H

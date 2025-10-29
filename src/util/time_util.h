#ifndef OWIF_UTIL_TIME_UTIL_H
#define OWIF_UTIL_TIME_UTIL_H

// ---- Includes ----
#include <Arduino.h>

#include <cstdint>

#include "cmd/command.h"

namespace owif {
namespace util {

using TimeStampMs = std::uint32_t;  // Time stamp in milliseconds (millis())

class TimeUtil final {
 public:
  TimeUtil(TimeUtil const&) = delete;
  TimeUtil(TimeUtil&&) = delete;
  auto operator=(TimeUtil const&) -> TimeUtil& = delete;
  auto operator=(TimeUtil&&) -> TimeUtil& = delete;

  ~TimeUtil() = delete;

  static auto Format(TimeStampMs time_stamp_ms) -> String;
  static auto Format(TimeStampMs time_stamp_ms, char (&formatted_string)[20]) -> void;

 private:
  // Division constants
  static constexpr std::uint32_t MSECS_PER_SEC{1000};
  static constexpr std::uint32_t SECS_PER_MIN{60};
  static constexpr std::uint32_t SECS_PER_HOUR{3600};
  static constexpr std::uint32_t SECS_PER_DAY{86400};

  TimeUtil() = delete;
};

}  // namespace util
}  // namespace owif

#endif  // OWIF_UTIL_TIME_UTIL_H

#ifndef OWIF_TIME_TIME_UTIL_H
#define OWIF_TIME_TIME_UTIL_H

// ---- Includes ----
#include <Arduino.h>

#include <cstdint>

namespace owif {
namespace time {

using TimeStampMs = std::uint64_t;  // Time stamp in milliseconds (millis())
using FormattedTimeString = char[25];

class TimeUtil final {
 public:
  TimeUtil(TimeUtil const&) = delete;
  TimeUtil(TimeUtil&&) = delete;
  auto operator=(TimeUtil const&) -> TimeUtil& = delete;
  auto operator=(TimeUtil&&) -> TimeUtil& = delete;

  ~TimeUtil() = delete;

  static auto TimeSinceStartup() -> TimeStampMs;

  static auto Format(TimeStampMs time_stamp_ms) -> String;
  static auto Format(TimeStampMs time_stamp_ms, FormattedTimeString& formatted_string) -> void;

 private:
  // Division constants
  static constexpr std::uint64_t MSECS_PER_SEC{1000};
  static constexpr std::uint64_t SECS_PER_MIN{60};
  static constexpr std::uint64_t SECS_PER_HOUR{3600};
  static constexpr std::uint64_t SECS_PER_DAY{86400};

  TimeUtil() = delete;
};

}  // namespace time
}  // namespace owif

#endif  // OWIF_UTIL_TIME_UTIL_H


#include "time/time_util.h"

namespace owif {
namespace time {

auto TimeUtil::TimeSinceStartup() -> TimeStampMs {
  TimeStampMs const now_ms{static_cast<TimeStampMs>(esp_timer_get_time() / 1000)};
  return now_ms;
}

auto TimeUtil::Format(TimeStampMs const time_stamp_ms) -> String {
  FormattedTimeString formatted_string;
  Format(time_stamp_ms, formatted_string);
  return formatted_string;
}

auto TimeUtil::Format(TimeStampMs time_stamp_ms, FormattedTimeString& formatted_string) -> void {
  // Total time
  std::uint64_t const time_stamp_secs{time_stamp_ms / MSECS_PER_SEC};

  // Time in components
  std::uint16_t const milliseconds{static_cast<std::uint16_t>(time_stamp_ms % MSECS_PER_SEC)};
  std::uint8_t const seconds{static_cast<std::uint8_t>(time_stamp_secs % SECS_PER_MIN)};
  std::uint8_t const minutes{static_cast<std::uint8_t>((time_stamp_secs / SECS_PER_MIN) % SECS_PER_MIN)};
  std::uint8_t const hours{static_cast<std::uint8_t>((time_stamp_secs % SECS_PER_DAY) / SECS_PER_HOUR)};
  std::uint16_t const days{static_cast<std::uint16_t>(time_stamp_secs / SECS_PER_DAY)};

  if (days > 0) {
    // Format with days
    sprintf(formatted_string, "%ud %02u:%02u:%02u.%03u", days, hours, minutes, seconds, milliseconds);
  } else {
    // Format without days
    sprintf(formatted_string, "%02u:%02u:%02u.%03u", hours, minutes, seconds, milliseconds);
  }
}

}  // namespace time
}  // namespace owif

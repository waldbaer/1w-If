
#include "time/time_util.h"

#include <time.h>

namespace owif {
namespace time {

auto TimeUtil::TimeSinceStartup() -> TimeStampMs {
  TimeStampMs const now_ms{static_cast<TimeStampMs>(esp_timer_get_time() / 1000)};
  return now_ms;
}

auto TimeUtil::Now() -> DateTime {
  struct timeval tv;
  gettimeofday(&tv, nullptr);

  struct tm local_time;
  localtime_r(&tv.tv_sec, &local_time);

  return DateTime{
      static_cast<std::uint16_t>(local_time.tm_year + 1900),  // year
      static_cast<std::uint8_t>(local_time.tm_mon + 1),       // month
      static_cast<std::uint8_t>(local_time.tm_mday),          // day of month
      static_cast<std::uint8_t>(local_time.tm_hour),          // hour
      static_cast<std::uint8_t>(local_time.tm_min),           // minute
      static_cast<std::uint8_t>(local_time.tm_sec),           // second of minute
      static_cast<std::uint16_t>(tv.tv_usec / 1000)           // milliseconds
  };
}

auto TimeUtil::Format(TimeStampMs const& time_stamp_ms) -> String {
  FormattedTimeString formatted_string;
  Format(time_stamp_ms, formatted_string);
  return formatted_string;
}

auto TimeUtil::Format(TimeStampMs const& time_stamp_ms, FormattedTimeString& formatted_string) -> void {
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

auto TimeUtil::Format(DateTime const& date_time, FormattedTimeString& formatted_string) -> void {
  sprintf(formatted_string, "%04u-%02u-%02u %02u:%02u:%02u.%03u", date_time.year, date_time.month, date_time.day,
          date_time.hour, date_time.minute, date_time.second, date_time.millisecond);
}

}  // namespace time
}  // namespace owif


#include "util/time_util.h"

namespace owif {
namespace util {

auto TimeUtil::Format(TimeStampMs const time_stamp_ms) -> String {
  char formatted_string[20];
  Format(time_stamp_ms, formatted_string);
  return formatted_string;
}

auto TimeUtil::Format(TimeStampMs time_stamp_ms, char (&formatted_string)[20]) -> void {
  // Total time
  std::uint32_t const time_stamp_secs{time_stamp_ms / MSECS_PER_SEC};

  // Time in components
  std::uint32_t const milliseconds{time_stamp_ms % MSECS_PER_SEC};
  std::uint32_t const seconds{time_stamp_secs % SECS_PER_MIN};
  std::uint32_t const minutes{(time_stamp_secs / SECS_PER_MIN) % SECS_PER_MIN};
  std::uint32_t const hours{(time_stamp_secs % SECS_PER_DAY) / SECS_PER_HOUR};

  sprintf(formatted_string, "%02d:%02d:%02d.%03d ", hours, minutes, seconds, milliseconds);
}

}  // namespace util
}  // namespace owif

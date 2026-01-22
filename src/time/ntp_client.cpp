#include "time/ntp_client.h"

namespace owif {
namespace time {

auto NtpClient::Begin() -> bool {
  logger_.Debug(F("[Ntp] Setup..."));

  setenv("TZ", kTimezone, 1);
  tzset();
  configTime(kGmtOffsetSec, kDaylightOffset, kNtpServer);

  return true;
}

auto NtpClient::Loop() -> void {
  // Nothing to do.
}

/*!
 * \brief Definition of global NTP client instance.
 */
NtpClient ntp_client_g{};

}  // namespace time
}  // namespace owif

#ifndef OWIF_TIME_NTP_CLIENT_H
#define OWIF_TIME_NTP_CLIENT_H

#include <Arduino.h>

#include "ethernet/ethernet.h"
#include "logging/logger.h"

namespace owif {
namespace time {

enum class ResponseCode : int {
  OK = 200,          //
  BadRequest = 400,  //
  Unauthorized = 401
};

class NtpClient {
 public:
  NtpClient() = default;
  NtpClient(NtpClient const&) = default;
  auto operator=(NtpClient const&) -> NtpClient& = default;
  NtpClient(NtpClient&&) = default;
  auto operator=(NtpClient&&) -> NtpClient& = default;
  ~NtpClient() = default;

  // ---- Public APIs --------------------------------------------------------------------------------------------------

  auto Begin() -> bool;
  auto Loop() -> void;

 private:
  static constexpr char const* kNtpServer{"pool.ntp.org"};
  static constexpr long kGmtOffsetSec{3600};
  static constexpr int kDaylightOffset{3600};

  static constexpr char const* kTimezone{"CET-1CEST,M3.5.0/2,M10.5.0/3"};

  logging::Logger& logger_{logging::logger_g};
};

/*!
 * \brief Declaration of global NTP client instance.
 */
extern NtpClient ntp_client_g;

}  // namespace time
}  // namespace owif

#endif  // OWIF_TIME_NTP_CLIENT_H

#include "time/ntp_client.h"

#include "config/ntp_config.h"
#include "ethernet/ethernet.h"

namespace owif {
namespace time {

auto NtpClient::Begin(config::NtpConfig const& ntp_config) -> bool {
  logger_.Debug(F("[Ntp] Setup..."));

  // NTP ServerAddress string must be valid after call to configTzTime. Therefore create a copy of the reference.
  config_ = ntp_config;

  configTzTime(config_.GetTimezone().c_str(), config_.GetServerAddr().c_str());

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

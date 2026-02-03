#include "config/ntp_config.h"

#include <string>

namespace owif {
namespace config {

auto NtpConfig::GetServerAddr() const -> String const& { return server_addr_; }
auto NtpConfig::SetServerAddr(String server_addr) -> void { server_addr_ = std::move(server_addr); }

auto NtpConfig::GetTimezone() const -> String const& { return timezone_; }
auto NtpConfig::SetTimezone(String timezone) -> void { timezone_ = std::move(timezone); }

}  // namespace config
}  // namespace owif

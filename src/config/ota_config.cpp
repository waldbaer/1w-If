#include "config/ota_config.h"

#include <cstdint>

namespace owif {
namespace config {

auto OtaConfig::GetPort() const -> std::uint16_t { return port_; }
auto OtaConfig::SetPort(std::uint16_t port) -> void { port_ = port; }

auto OtaConfig::GetPassword() const -> String const& { return password_; }
auto OtaConfig::SetPassword(String password) -> void { password_ = std::move(password); }

}  // namespace config
}  // namespace owif

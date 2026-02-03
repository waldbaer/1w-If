#ifndef OWIF_CONFIG_OTA_CONFIG_H
#define OWIF_CONFIG_OTA_CONFIG_H

#include <Preferences.h>

#include <cstdint>

namespace owif {
namespace config {

class OtaConfig {
 public:
  static constexpr std::uint16_t kDefaultPort{3232};  // common default port used for espota protocol
  static constexpr char const* kDefaultPassword{"1w-If"};

  OtaConfig() = default;

  OtaConfig(OtaConfig const&) = default;
  OtaConfig(OtaConfig&&) = default;
  auto operator=(OtaConfig const&) -> OtaConfig& = default;
  auto operator=(OtaConfig&&) -> OtaConfig& = default;

  ~OtaConfig() = default;

  // ---- Public APIs ----

  auto GetPort() const -> std::uint16_t;
  auto SetPort(std::uint16_t port) -> void;

  auto GetPassword() const -> String const&;
  auto SetPassword(String password) -> void;

 private:
  std::uint16_t port_{kDefaultPort};
  String password_{"1w-If"};
};

}  // namespace config
}  // namespace owif

#endif  // OWIF_CONFIG_OTA_CONFIG_H

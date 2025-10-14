#ifndef OWIF_CONFIG_ETHERNET_CONFIG_H
#define OWIF_CONFIG_ETHERNET_CONFIG_H

#include <Preferences.h>

#include <string>

namespace owif {
namespace config {

class EthernetConfig {
 public:
  static constexpr char const* kDefaultHostname{"owif"};

  EthernetConfig() = default;
  EthernetConfig(EthernetConfig const&) = default;
  EthernetConfig(EthernetConfig&&) = default;
  auto operator=(EthernetConfig const&) -> EthernetConfig& = default;
  auto operator=(EthernetConfig&&) -> EthernetConfig& = default;

  ~EthernetConfig() = default;

  // ---- Public APIs ----

  auto GetHostname() const -> String const&;
  auto SetHostname(String hostname) -> void;

 private:
  String hostname_{"owif"};
};

}  // namespace config
}  // namespace owif

#endif  // OWIF_CONFIG_ETHERNET_CONFIG_H

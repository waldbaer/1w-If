#ifndef OWIF_CONFIG_PERSISTENCY_H
#define OWIF_CONFIG_PERSISTENCY_H

#include <Preferences.h>

#include "config/ethernet_config.h"
#include "config/mqtt_config.h"
#include "config/webserver_config.h"

namespace owif {
namespace config {

class Persistency {
 public:
  Persistency() = default;

  Persistency(Persistency const&) = delete;
  Persistency(Persistency&&) = delete;
  auto operator=(Persistency const&) -> Persistency& = delete;
  auto operator=(Persistency&&) -> Persistency& = delete;

  ~Persistency() = default;

  // ---- Public APIs ----

  auto LoadEthernetConfig() -> EthernetConfig;
  auto StoreEthernetConfig(EthernetConfig const& ethernet_config) -> void;

  auto LoadWebServerConfig() -> WebServerConfig;
  auto StoreWebServerConfig(WebServerConfig const& webserver_config) -> void;

  auto LoadMqttConfig() -> MqttConfig;
  auto StoreMqttConfig(MqttConfig const& mqtt_config) -> void;

 private:
  Preferences preferences_;
};

extern Persistency persistency_g;

}  // namespace config
}  // namespace owif

#endif  // #define OWIF_CONFIG_PERSISTENCY_H

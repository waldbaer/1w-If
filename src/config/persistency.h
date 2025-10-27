#ifndef OWIF_CONFIG_PERSISTENCY_H
#define OWIF_CONFIG_PERSISTENCY_H

#include <Preferences.h>

#include "config/ethernet_config.h"
#include "config/logging_config.h"
#include "config/mqtt_config.h"
#include "config/ota_config.h"
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
  auto LoadLoggingConfig() -> LoggingConfig;
  auto StoreLoggingConfig(LoggingConfig const& logging_config) -> void;

  auto LoadEthernetConfig() -> EthernetConfig;
  auto StoreEthernetConfig(EthernetConfig const& ethernet_config) -> void;

  auto LoadOtaConfig() -> OtaConfig;
  auto StoreOtaConfig(OtaConfig const& ota_config) -> void;

  auto LoadWebServerConfig() -> WebServerConfig;
  auto StoreWebServerConfig(WebServerConfig const& webserver_config) -> void;

  auto LoadMqttConfig() -> MqttConfig;
  auto StoreMqttConfig(MqttConfig const& mqtt_config) -> void;

 private:
  static constexpr char const* kLoggingKey{"log"};
  static constexpr char const* kLoggingKeyLogLevel{"loglevel"};
  static constexpr char const* kLoggingKeySerialLog{"serial"};
  static constexpr char const* kLoggingKeyWebLog{"web"};

  static constexpr char const* kEthKey{"eth"};
  static constexpr char const* kEthKeyHostname{"hostname"};

  static constexpr char const* kOtaKey{"ota"};
  static constexpr char const* kOtaKeyPort{"port"};
  static constexpr char const* kOtaKeyPassword{"password"};

  static constexpr char const* kWebServerKey{"webserver"};
  static constexpr char const* kWebServerKeyUser{"user"};
  static constexpr char const* kWebServerKeyPassword{"password"};

  static constexpr char const* kMqttKey{"mqtt"};
  static constexpr char const* kMqttKeyServerAddr{"server_addr"};
  static constexpr char const* kMqttKeyServerPort{"server_port"};
  static constexpr char const* kMqttKeyUser{"user"};
  static constexpr char const* kMqttKeyPassword{"password"};
  static constexpr char const* kMqttKeyReconnectTime{"reconnect_t"};
  static constexpr char const* kMqttKeyTopic{"topic"};

  Preferences preferences_;
};

extern Persistency persistency_g;

}  // namespace config
}  // namespace owif

#endif  // #define OWIF_CONFIG_PERSISTENCY_H

#include "config/persistency.h"

#include <cstdint>
#include <string>

#include "config/ethernet_config.h"
#include "config/mqtt_config.h"
#include "config/ota_config.h"
#include "config/webserver_config.h"

namespace owif {
namespace config {

// ---- Ethernet ----
auto Persistency::LoadEthernetConfig() -> EthernetConfig {
  EthernetConfig config{};

  preferences_.begin("eth", false);

  config.SetHostname(preferences_.getString("hostname", "owif"));

  preferences_.end();

  return config;
}

auto Persistency::StoreEthernetConfig(EthernetConfig const& ethernet_config) -> void {
  preferences_.begin("eth", false);

  preferences_.putString("hostname", ethernet_config.GetHostname());

  preferences_.end();
}

// ---- Ethernet ----
auto Persistency::LoadOtaConfig() -> OtaConfig {
  OtaConfig config{};

  preferences_.begin("ota", false);

  config.SetPort(preferences_.getUInt(kOtaKeyPort, OtaConfig::kDefaultPort));
  config.SetPassword(preferences_.getString(kOtaKeyPassword, OtaConfig::kDefaultPassword));

  preferences_.end();

  return config;
}

auto Persistency::StoreOtaConfig(OtaConfig const& ota_config) -> void {
  preferences_.begin("ota", false);

  preferences_.putUInt(kOtaKeyPort, ota_config.GetPort());
  preferences_.putString(kOtaKeyPassword, ota_config.GetPassword());

  preferences_.end();
}

// ---- WebServer ----

auto Persistency::LoadWebServerConfig() -> WebServerConfig {
  WebServerConfig config{};

  preferences_.begin("webserver", false);

  config.SetUser(preferences_.getString("user", "admin"));
  config.SetPassword(preferences_.getString("password", "1w-If"));

  preferences_.end();

  return config;
}

auto Persistency::StoreWebServerConfig(WebServerConfig const& webserver_config) -> void {
  preferences_.begin("webserver", false);

  preferences_.putString("user", webserver_config.GetUser());
  preferences_.putString("password", webserver_config.GetPassword());

  preferences_.end();
}

// ---- MQTT ----

auto Persistency::LoadMqttConfig() -> MqttConfig {
  MqttConfig config{};

  preferences_.begin("mqtt", false);

  config.SetServerAddr(preferences_.getString("server_addr", "192.168.0.10"));
  config.SetServerPort(preferences_.getUInt("server_port", 1883));
  config.SetUser(preferences_.getString("user", "user"));
  config.SetPassword(preferences_.getString("password", "password"));
  config.SetReconnectTimeout(preferences_.getUInt("reconnect_t", 30 * 1000));
  config.SetTopic(preferences_.getString("topic", "1wIf"));

  preferences_.end();

  return config;
}

auto Persistency::StoreMqttConfig(MqttConfig const& mqtt_config) -> void {
  preferences_.begin("mqtt", false);

  preferences_.putString("server_addr", mqtt_config.GetServerAddr());
  preferences_.putUInt("server_port", mqtt_config.GetServerPort());
  preferences_.putString("user", mqtt_config.GetUser());
  preferences_.putString("password", mqtt_config.GetPassword());
  preferences_.putUInt("reconnect_t", mqtt_config.GetReconnectTimeout());
  preferences_.putString("topic", mqtt_config.GetTopic());

  preferences_.end();
}

// ---- Global Persistency Instance ----
Persistency persistency_g{};

}  // namespace config
}  // namespace owif

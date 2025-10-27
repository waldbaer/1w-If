#include "config/persistency.h"

#include <cstdint>
#include <string>

#include "config/ethernet_config.h"
#include "config/logging_config.h"
#include "config/mqtt_config.h"
#include "config/ota_config.h"
#include "config/webserver_config.h"

namespace owif {
namespace config {

// ---- Logging ----
auto Persistency::LoadLoggingConfig() -> LoggingConfig {
  LoggingConfig config{};

  preferences_.begin(kLoggingKey, false);

  config.SetLogLevel(static_cast<LoggingConfig::LogLevel>(
      preferences_.getChar(kLoggingKeyLogLevel, static_cast<char>(LoggingConfig::kDefaultLogLevel))));
  config.SetSerialLogEnabled(preferences_.getBool(kLoggingKeySerialLog, LoggingConfig::kDefaultSerialLogEnabled));
  config.SetWebLogEnabled(preferences_.getBool(kLoggingKeyWebLog, LoggingConfig::kDefaultWebLogEnabled));

  preferences_.end();

  return config;
}

auto Persistency::StoreLoggingConfig(LoggingConfig const& logging_config) -> void {
  preferences_.begin(kLoggingKey, false);

  preferences_.putChar(kLoggingKeyLogLevel, static_cast<char>(logging_config.GetLogLevel()));
  preferences_.putBool(kLoggingKeySerialLog, logging_config.GetSerialLogEnabled());
  preferences_.putBool(kLoggingKeyWebLog, logging_config.GetWebLogEnabled());

  preferences_.end();
}

// ---- Ethernet ----
auto Persistency::LoadEthernetConfig() -> EthernetConfig {
  EthernetConfig config{};

  preferences_.begin(kEthKey, false);

  config.SetHostname(preferences_.getString(kEthKeyHostname, EthernetConfig::kDefaultHostname));

  preferences_.end();

  return config;
}

auto Persistency::StoreEthernetConfig(EthernetConfig const& ethernet_config) -> void {
  preferences_.begin(kEthKey, false);

  preferences_.putString(kEthKeyHostname, ethernet_config.GetHostname());

  preferences_.end();
}

// ---- Ethernet ----
auto Persistency::LoadOtaConfig() -> OtaConfig {
  OtaConfig config{};

  preferences_.begin(kOtaKey, false);

  config.SetPort(preferences_.getUInt(kOtaKeyPort, OtaConfig::kDefaultPort));
  config.SetPassword(preferences_.getString(kOtaKeyPassword, OtaConfig::kDefaultPassword));

  preferences_.end();

  return config;
}

auto Persistency::StoreOtaConfig(OtaConfig const& ota_config) -> void {
  preferences_.begin(kOtaKey, false);

  preferences_.putUInt(kOtaKeyPort, ota_config.GetPort());
  preferences_.putString(kOtaKeyPassword, ota_config.GetPassword());

  preferences_.end();
}

// ---- WebServer ----

auto Persistency::LoadWebServerConfig() -> WebServerConfig {
  WebServerConfig config{};

  preferences_.begin(kWebServerKey, false);

  config.SetUser(preferences_.getString(kWebServerKeyUser, WebServerConfig::kDefaultUser));
  config.SetPassword(preferences_.getString(kWebServerKeyPassword, WebServerConfig::kDefaultPassword));

  preferences_.end();

  return config;
}

auto Persistency::StoreWebServerConfig(WebServerConfig const& webserver_config) -> void {
  preferences_.begin(kWebServerKey, false);

  preferences_.putString(kWebServerKeyUser, webserver_config.GetUser());
  preferences_.putString(kWebServerKeyPassword, webserver_config.GetPassword());

  preferences_.end();
}

// ---- MQTT ----

auto Persistency::LoadMqttConfig() -> MqttConfig {
  MqttConfig config{};

  preferences_.begin(kMqttKey, false);

  config.SetServerAddr(preferences_.getString(kMqttKeyServerAddr, MqttConfig::kDefaultServerAddr));
  config.SetServerPort(preferences_.getUInt(kMqttKeyServerPort, MqttConfig::kDefaultServerPort));
  config.SetUser(preferences_.getString(kMqttKeyUser, MqttConfig::kDefaultUser));
  config.SetPassword(preferences_.getString(kMqttKeyPassword, MqttConfig::kDefaultPassword));
  config.SetReconnectTimeout(preferences_.getUInt(kMqttKeyReconnectTime, MqttConfig::kDefaultReconnectTimeout));
  config.SetTopic(preferences_.getString(kMqttKeyTopic, MqttConfig::kDefaultTopic));

  preferences_.end();

  return config;
}

auto Persistency::StoreMqttConfig(MqttConfig const& mqtt_config) -> void {
  preferences_.begin(kMqttKey, false);

  preferences_.putString(kMqttKeyServerAddr, mqtt_config.GetServerAddr());
  preferences_.putUInt(kMqttKeyServerPort, mqtt_config.GetServerPort());
  preferences_.putString(kMqttKeyUser, mqtt_config.GetUser());
  preferences_.putString(kMqttKeyPassword, mqtt_config.GetPassword());
  preferences_.putUInt(kMqttKeyReconnectTime, mqtt_config.GetReconnectTimeout());
  preferences_.putString(kMqttKeyTopic, mqtt_config.GetTopic());

  preferences_.end();
}

// ---- Global Persistency Instance ----
Persistency persistency_g{};

}  // namespace config
}  // namespace owif

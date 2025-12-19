#include "config/persistency.h"

#include <cstdint>
#include <string>

#include "config/ethernet_config.h"
#include "config/logging_config.h"
#include "config/mqtt_config.h"
#include "config/onewire_config.h"
#include "config/ota_config.h"
#include "config/webserver_config.h"

namespace owif {
namespace config {

// ---- Public APIs ----------------------------------------------------------------------------------------------------

auto Persistency::PrettyPrint(logging::Logger& logger) -> void {
  LoggingConfig const& logging_config{LoadLoggingConfig()};
  OneWireConfig const& one_wire_config{LoadOneWireConfig()};
  EthernetConfig const& ethernet_config{LoadEthernetConfig()};
  OtaConfig const& ota_config{LoadOtaConfig()};
  MqttConfig const& mqtt_config{LoadMqttConfig()};

  logger.Info(F("[Persistency] +- Configuration ---------------------------------"));
  logger.Info(F("[Persistency] | Logging:"));
  logger.Info(F("[Persistency] |   Log-Level: %s"), logging_config.GetLogLevelAsString());
  logger.Info(F("[Persistency] |   Serial:    %s"), FormatOnOff(logging_config.GetSerialLogEnabled()));
  logger.Info(F("[Persistency] |   Web:       %s"), FormatOnOff(logging_config.GetWebLogEnabled()));
  logger.Info(F("[Persistency] | 1-Wire:"));
  logger.Info(F("[Persistency] |   Channels:  %s %s %s %s"),
              one_wire_config.GetChannelConfig(config::OneWireConfig::kOneWireChannel1).GetEnabled() ? "[1]" : "[ ]",
              one_wire_config.GetChannelConfig(config::OneWireConfig::kOneWireChannel2).GetEnabled() ? "[2]" : "[ ]",
              one_wire_config.GetChannelConfig(config::OneWireConfig::kOneWireChannel3).GetEnabled() ? "[3]" : "[ ]",
              one_wire_config.GetChannelConfig(config::OneWireConfig::kOneWireChannel4).GetEnabled() ? "[4]" : "[ ]");
  logger.Info(F("[Persistency] | Ethernet"));
  logger.Info(F("[Persistency] |   Hostname:  %s"), ethernet_config.GetHostname());
  logger.Info(F("[Persistency] | OTA"));
  logger.Info(F("[Persistency] |   Port:      %u"), ota_config.GetPort());
  logger.Info(F("[Persistency] | MQTT:"));
  logger.Info(F("[Persistency] |   Server:    %s:%u"), mqtt_config.GetServerAddr(), mqtt_config.GetServerPort());
  logger.Info(F("[Persistency] |   User:      %s"), mqtt_config.GetUser());
  logger.Info(F("[Persistency] |   Reconnect: %u ms"), mqtt_config.GetReconnectTimeout());
  logger.Info(F("[Persistency] |   Topic:     %s"), mqtt_config.GetTopic());
  logger.Info(F("[Persistency] +-------------------------------------------------"));
}

// ---- Domain Configs -------------------------------------------------------------------------------------------------

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

// ---- OneWire ----
auto Persistency::LoadOneWireConfig() -> OneWireConfig {
  OneWireConfig config{};

  preferences_.begin(kOneWireKey, false);

  config.GetChannelConfig(OneWireConfig::kOneWireChannel1)
      .SetEnabled(preferences_.getBool(kOneWireKeyCh1Enabled, OneWireChannelConfig::kDefaultEnabled));
  config.GetChannelConfig(OneWireConfig::kOneWireChannel2)
      .SetEnabled(preferences_.getBool(kOneWireKeyCh2Enabled, OneWireChannelConfig::kDefaultEnabled));
  config.GetChannelConfig(OneWireConfig::kOneWireChannel3)
      .SetEnabled(preferences_.getBool(kOneWireKeyCh3Enabled, OneWireChannelConfig::kDefaultEnabled));
  config.GetChannelConfig(OneWireConfig::kOneWireChannel4)
      .SetEnabled(preferences_.getBool(kOneWireKeyCh4Enabled, OneWireChannelConfig::kDefaultEnabled));

  preferences_.end();

  return config;
}

auto Persistency::StoreOneWireConfig(OneWireConfig const& onewire_config) -> void {
  preferences_.begin(kOneWireKey, false);

  preferences_.putBool(kOneWireKeyCh1Enabled,
                       onewire_config.GetChannelConfig(OneWireConfig::kOneWireChannel1).GetEnabled());
  preferences_.putBool(kOneWireKeyCh2Enabled,
                       onewire_config.GetChannelConfig(OneWireConfig::kOneWireChannel2).GetEnabled());
  preferences_.putBool(kOneWireKeyCh3Enabled,
                       onewire_config.GetChannelConfig(OneWireConfig::kOneWireChannel3).GetEnabled());
  preferences_.putBool(kOneWireKeyCh4Enabled,
                       onewire_config.GetChannelConfig(OneWireConfig::kOneWireChannel4).GetEnabled());

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

// ---- Private APIS ---------------------------------------------------------------------------------------------------

auto Persistency::FormatOnOff(bool enabled) -> char const* { return enabled ? "on " : "off"; }

// ---- Global Persistency Instance ----
Persistency persistency_g{};

}  // namespace config
}  // namespace owif

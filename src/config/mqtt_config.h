#ifndef OWIF_CONFIG_MQTT_CONFIG_H
#define OWIF_CONFIG_MQTT_CONFIG_H

#include <Preferences.h>

namespace owif {
namespace config {

class MqttConfig {
 public:
  MqttConfig() = default;
  MqttConfig(MqttConfig const&) = default;
  MqttConfig(MqttConfig&&) = default;
  auto operator=(MqttConfig const&) -> MqttConfig& = default;
  auto operator=(MqttConfig&&) -> MqttConfig& = default;

  ~MqttConfig() = default;

  // ---- Public APIs ----
  auto GetServerAddr() const -> String const&;
  auto SetServerAddr(String addr) -> void;

  auto GetServerPort() const -> std::uint16_t;
  auto SetServerPort(std::uint16_t port) -> void;

  auto GetUser() const -> String const&;
  auto SetUser(String user) -> void;

  auto GetPassword() const -> String const&;
  auto SetPassword(String password) -> void;

  auto GetReconnectTimeout() const -> std::uint32_t;
  auto SetReconnectTimeout(std::uint32_t reconnect_timeout) -> void;

  auto GetTopic() const -> String const&;
  auto SetTopic(String topic) -> void;

 private:
  String server_addr_{"192.168.0.10"};
  std::uint16_t server_port_{1883};
  String user_{"user"};
  String password_{"password"};

  std::uint32_t reconnect_timeout_{30 * 1000};  // ms

  String topic_{"1wIf"};
};

}  // namespace config
}  // namespace owif

#endif  // OWIF_CONFIG_MQTT_CONFIG_H

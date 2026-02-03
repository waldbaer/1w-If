#ifndef OWIF_CONFIG_MQTT_CONFIG_H
#define OWIF_CONFIG_MQTT_CONFIG_H

#include <Preferences.h>

namespace owif {
namespace config {

class MqttConfig {
 public:
  static constexpr char const* kDefaultServerAddr{"192.168.0.10"};
  static constexpr std::uint16_t kDefaultServerPort{1883};
  static constexpr char const* kDefaultUser{"admin"};
  static constexpr char const* kDefaultPassword{"1w-If"};
  static constexpr std::uint32_t kDefaultReconnectTimeout{30 * 1000};  // ms
  static constexpr char const* kDefaultTopic{"1wIf"};
  static constexpr char const* kDefaultClientId{"1wIf"};

  MqttConfig() = default;
  MqttConfig(MqttConfig const&) = default;
  MqttConfig(MqttConfig&&) = default;
  auto operator=(MqttConfig const&) -> MqttConfig& = default;
  auto operator=(MqttConfig&&) -> MqttConfig& = default;

  ~MqttConfig() = default;

  // ---- Public APIs ----
  auto GetServerAddr() const -> String const&;
  auto SetServerAddr(String const& addr) -> void;

  auto GetServerPort() const -> std::uint16_t;
  auto SetServerPort(std::uint16_t port) -> void;

  auto GetUser() const -> String const&;
  auto SetUser(String const& user) -> void;

  auto GetPassword() const -> String const&;
  auto SetPassword(String const& password) -> void;

  auto GetTopic() const -> String const&;
  auto SetTopic(String const& topic) -> void;

  auto GetClientId() const -> String const&;
  auto SetClientId(String const& client_id) -> void;

  auto GetReconnectTimeout() const -> std::uint32_t;
  auto SetReconnectTimeout(std::uint32_t reconnect_timeout) -> void;

 private:
  String server_addr_{kDefaultServerAddr};
  std::uint16_t server_port_{kDefaultServerPort};
  String user_{kDefaultUser};
  String password_{kDefaultPassword};
  std::uint32_t reconnect_timeout_{kDefaultReconnectTimeout};  // ms
  String topic_{kDefaultTopic};
  String client_id_{kDefaultClientId};
};

}  // namespace config
}  // namespace owif

#endif  // OWIF_CONFIG_MQTT_CONFIG_H

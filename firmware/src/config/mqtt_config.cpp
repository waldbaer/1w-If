#include "config/mqtt_config.h"

#include <cstdint>

namespace owif {
namespace config {

auto MqttConfig::GetServerAddr() const -> String const& { return server_addr_; }
auto MqttConfig::SetServerAddr(String const& addr) -> void { server_addr_ = addr; }

auto MqttConfig::GetServerPort() const -> std::uint16_t { return server_port_; }
auto MqttConfig::SetServerPort(std::uint16_t port) -> void { server_port_ = port; }

auto MqttConfig::GetUser() const -> String const& { return user_; }
auto MqttConfig::SetUser(String const& user) -> void { user_ = user; }

auto MqttConfig::GetPassword() const -> String const& { return password_; }
auto MqttConfig::SetPassword(String const& password) -> void { password_ = password; }

auto MqttConfig::GetTopic() const -> String const& { return topic_; }
auto MqttConfig::SetTopic(String const& topic) -> void { topic_ = topic; }

auto MqttConfig::GetClientId() const -> String const& { return client_id_; }
auto MqttConfig::SetClientId(String const& client_id) -> void { client_id_ = client_id; }

auto MqttConfig::GetReconnectTimeout() const -> std::uint32_t { return reconnect_timeout_; }
auto MqttConfig::SetReconnectTimeout(std::uint32_t reconnect_timeout) -> void {
  reconnect_timeout_ = reconnect_timeout;
}

}  // namespace config
}  // namespace owif

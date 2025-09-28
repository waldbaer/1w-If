#include "config/mqtt_config.h"

#include <cstdint>

namespace owif {
namespace config {

auto MqttConfig::GetServerAddr() const -> String const& { return server_addr_; }
auto MqttConfig::SetServerAddr(String addr) -> void { server_addr_ = std::move(addr); }

auto MqttConfig::GetServerPort() const -> std::uint16_t { return server_port_; }
auto MqttConfig::SetServerPort(std::uint16_t port) -> void { server_port_ = std::move(port); }

auto MqttConfig::GetUser() const -> String const& { return user_; }
auto MqttConfig::SetUser(String user) -> void { user_ = user; }

auto MqttConfig::GetPassword() const -> String const& { return password_; }
auto MqttConfig::SetPassword(String password) -> void { password_ = std::move(password); }

auto MqttConfig::GetReconnectTimeout() const -> std::uint32_t { return reconnect_timeout_; }
auto MqttConfig::SetReconnectTimeout(std::uint32_t reconnect_timeout) -> void {
  reconnect_timeout_ = reconnect_timeout;
}

auto MqttConfig::GetTopic() const -> String const& { return topic_; }
auto MqttConfig::SetTopic(String topic) -> void { topic_ = std::move(topic); }

}  // namespace config
}  // namespace owif

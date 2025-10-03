#ifndef OWIF_MQTT_MQTT_CLIENT_H
#define OWIF_MQTT_MQTT_CLIENT_H

#include <ArduinoJson.h>
#include <AsyncMqttClient.h>

#include <map>
#include <memory>

#include "config/mqtt_config.h"
#include "ethernet/ethernet.h"
#include "logging/logger.h"

namespace owif {
namespace mqtt {

enum class MqttQoS : std::uint8_t {
  kQoS0 = 0,  // At most once:  QoS 0 offers "fire and forget" messaging with no acknowledgment from the receiver.
  kQoS1 = 1,  // At least once: QoS 1 ensures that messages are delivered at least once by requiring a PUBACK
              // acknowledgment.
  kQoS2 = 2   // Exactly once: QoS 2 guarantees that each message is delivered exactly once by using a four-step
              // handshake (PUBLISH, PUBREC, PUBREL, PUBCOMP).
};

enum class MqttRetain : std::uint8_t {
  kNoRetain = 0,  // Do not retain the message.
  kRetain = 1     // Broker stores last retained message. New subscribers will immediately get the retained message.
};

struct MqttMsgId {
  using type = std::uint16_t;
  type value;
};

struct MqttMsgProps {
  MqttQoS qos;
  bool dup;
  bool retain;
};

enum class ConnectionState : std::uint8_t { kDisconnected = 0, kConnected = 1 };

using ConnectionStateChangeHandler = std::function<void(ConnectionState connection_state)>;
using MessageHandler = std::function<void(String topic, String payload, MqttMsgProps properties)>;

class MqttClient {
 public:
  MqttClient() = default;
  MqttClient(MqttClient const&) = delete;
  MqttClient(MqttClient&&) = delete;
  auto operator=(MqttClient const&) -> MqttClient& = delete;
  auto operator=(MqttClient&&) -> MqttClient& = delete;

  ~MqttClient() = default;

  // ---- Public APIs ----

  auto Begin() -> bool;
  auto End() -> void;
  auto Loop() -> void;

  auto IsConnected() -> bool;
  auto OnConnectionStateChange(ConnectionStateChangeHandler handler) -> void;

  auto Publish(char const* topic, char const* payload, MqttQoS qos = MqttQoS::kQoS0,
               MqttRetain retain = MqttRetain::kNoRetain) -> MqttMsgId;
  auto Subscribe(String topic, MessageHandler handler, MqttQoS qos = MqttQoS::kQoS0) -> MqttMsgId;

 private:
  auto OnConnectionStateChange(ethernet::ConnectionState connection_state) -> void;

  auto Connect() -> void;
  auto Disconnect() -> void;

  auto OnConnected(bool session_resent) -> void;
  auto OnDisconnect(AsyncMqttClientDisconnectReason reason) -> void;
  auto OnMqttSubscribe(MqttMsgId msg_id, MqttQoS qos) -> void;
  auto OnMqttUnsubscribe(MqttMsgId msg_id) -> void;
  auto OnMqttMessage(char const* topic, char const* payload, AsyncMqttClientMessageProperties properties, size_t len,
                     size_t index, size_t total) -> void;

  auto NotifyConnectionStateChangeHandlers() -> void;

  logging::Logger& logger_{logging::logger_g};

  config::MqttConfig config_;
  ::AsyncMqttClient mqtt_client_{};

  std::size_t reconnect_time_{0};
  static constexpr std::size_t kReconnectDelay{10 * 1000};  // ms

  ConnectionState connection_state_{ConnectionState::kDisconnected};
  std::vector<ConnectionStateChangeHandler> connection_state_change_handlers_{};
  std::map<String, MessageHandler> topic_handlers_{};
};

extern MqttClient mqtt_client_g;

}  // namespace mqtt
}  // namespace owif

#endif  // OWIF_MQTT_MQTT_CLIENT_H

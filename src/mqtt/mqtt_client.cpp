#include "mqtt/mqtt_client.h"

#include <ArduinoJson.h>

#include <string>
#include <type_traits>

#include "config/persistency.h"
#include "ethernet/ethernet.h"
#include "util/language.h"

namespace owif {
namespace mqtt {

auto MqttClient::Begin() -> bool {
  logger_.Debug(F("[MQTTClient] Setup..."));

  config_ = config::persistency_g.LoadMqttConfig();

  // Configure MQTT client
  mqtt_client_.onConnect([this](bool session_preset) { OnConnected(session_preset); });
  mqtt_client_.onDisconnect([this](AsyncMqttClientDisconnectReason reason) { OnDisconnect(reason); });

  mqtt_client_.onSubscribe([this](MqttMsgId::type msg_id, std::uint8_t qos) {
    OnMqttSubscribe(MqttMsgId{msg_id}, static_cast<MqttQoS>(qos));
  });
  mqtt_client_.onUnsubscribe([this](MqttMsgId::type msg_id) { OnMqttUnsubscribe(MqttMsgId{msg_id}); });
  mqtt_client_.onMessage([this](char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len,
                                size_t index,
                                size_t total) { OnMqttMessage(topic, payload, properties, len, index, total); });

  mqtt_client_.setCredentials(config_.GetUser().c_str(), config_.GetPassword().c_str());
  mqtt_client_.setServer(config_.GetServerAddr().c_str(), config_.GetServerPort());

  // Register Ethernet connection state change handler starting / stopping the WebServer
  ethernet::ethernet_g.OnConnectionStateChange(
      [this](ethernet::ConnectionState connection_state) { OnConnectionStateChange(connection_state); });

  return true;
}

auto MqttClient::End() -> void { mqtt_client_.disconnect(); }

auto MqttClient::Loop() -> void {
  if (reconnect_time_ != 0 && millis() >= reconnect_time_) {
    reconnect_time_ = 0;
    Connect();
  }
}

auto MqttClient::IsConnected() -> bool { return mqtt_client_.connected(); }

auto MqttClient::OnConnectionStateChange(ConnectionStateChangeHandler handler) -> void {
  if (handler) {
    connection_state_change_handlers_.push_back(handler);
  } else {
    logger_.Error(F("[MQTTClient] Invalid MQTT connection state change handler"));
  }
}

auto MqttClient::Publish(char const* topic, char const* payload, MqttQoS qos, MqttRetain retain) -> MqttMsgId {
  logger_.Verbose("[MQTTClient] Publishing to MQTT (qos: %u retain: %u) payload: %s", qos, retain, payload);

  return MqttMsgId{mqtt_client_.publish(topic, static_cast<std::underlying_type_t<MqttQoS>>(qos),
                                        retain == MqttRetain::kRetain, payload)};
}

auto MqttClient::Subscribe(String topic, MessageHandler handler, MqttQoS qos) -> MqttMsgId {
  logger_.Verbose("[MQTTClient] Subscribing to MQTT topic '%s'", topic);
  if (topic_handlers_.find(topic) == topic_handlers_.end()) {
    if (handler) {
      topic_handlers_[topic] = std::move(handler);
    } else {
      logger_.Warn(F("[MQTTClient] Skip registration of invalid message handler for MQTT topic '%s'"), topic);
    }
  }

  return MqttMsgId{mqtt_client_.subscribe(topic.c_str(), static_cast<std::underlying_type_t<MqttQoS>>(qos))};
}

// ---- Private APIs ---------------------------------------------------------------------------------------------------

auto MqttClient::Connect() -> void {
  logger_.Debug(F("[MQTTClient] connecting..."));
  mqtt_client_.connect();
}

auto MqttClient::Disconnect() -> void {
  logger_.Debug(F("[MQTTClient] disconnecting..."));
  mqtt_client_.disconnect();
}

auto MqttClient::OnConnectionStateChange(ethernet::ConnectionState connection_state) -> void {
  if (connection_state == ethernet::ConnectionState::kConnected) {
    Connect();
  } else if (connection_state == ethernet::ConnectionState::kDisconnected) {
    Disconnect();
  }
}

auto MqttClient::OnConnected(bool session_present) -> void {
  logger_.Debug(F("[MQTTClient] connected. | session present: %T"), session_present);

  connection_state_ = ConnectionState::kConnected;
  NotifyConnectionStateChangeHandlers();
}

auto MqttClient::OnDisconnect(AsyncMqttClientDisconnectReason reason) -> void {
  logger_.DebugNoLn(F("[MQTTClient] disconnected. | reason: %u"), reason);

  connection_state_ = ConnectionState::kDisconnected;

  if (ethernet::ethernet_g.IsConnected()) {
    logger_.Debug(F(" | Reconnect in %u ms"), config_.GetReconnectTimeout());
    reconnect_time_ = millis() + config_.GetReconnectTimeout();
  } else {
    logger_.Debug(F(" | No auto-reconnect as ethernet is disconnected."));
  }

  NotifyConnectionStateChangeHandlers();
}

auto MqttClient::OnMqttSubscribe(MqttMsgId msg_id, MqttQoS qos) -> void {
  logger_.Verbose("[MQTTClient] Subscribe confirmed for msg %u", msg_id.value);
}

auto MqttClient::OnMqttUnsubscribe(MqttMsgId msg_id) -> void {
  logger_.Verbose("[MQTTClient] Unsubscribe confirmed for msg %u", msg_id.value);
}

auto MqttClient::OnMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len,
                               size_t index, size_t total) -> void {
  // Dispatch to topic-specific message handler
  if (topic_handlers_.find(topic) != topic_handlers_.end()) {
    MessageHandler& message_handler{topic_handlers_.at(topic)};
    message_handler(String{topic}, String{payload, len},
                    MqttMsgProps{static_cast<MqttQoS>(properties.qos), properties.dup, properties.retain});
  } else {
    logger_.Warn(F("[MQTTClient] No message handler for MQTT topic '%s' found"), topic);
  }
}

auto MqttClient::NotifyConnectionStateChangeHandlers() -> void {
  for (ConnectionStateChangeHandler& handler : connection_state_change_handlers_) {
    handler(connection_state_);
  }
}

// ---- Global Instance ----
MqttClient mqtt_client_g{};

}  // namespace mqtt
}  // namespace owif

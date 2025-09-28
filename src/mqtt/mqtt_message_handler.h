#ifndef OWIF_MQTT_MQTT_MESSAGE_HANDLER_H
#define OWIF_MQTT_MQTT_MESSAGE_HANDLER_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "cmd/command_handler.h"
#include "logging/logger.h"
#include "mqtt/mqtt_client.h"

namespace owif {
namespace mqtt {

class MqttMessageHandler {
 public:
  MqttMessageHandler() = default;
  MqttMessageHandler(MqttMessageHandler const&) = delete;
  MqttMessageHandler(MqttMessageHandler&&) = delete;
  auto operator=(MqttMessageHandler const&) -> MqttMessageHandler& = delete;
  auto operator=(MqttMessageHandler&&) -> MqttMessageHandler& = delete;

  ~MqttMessageHandler() = default;

  // ---- Public APIs ----

  auto Begin(MqttClient* mqtt_client, cmd::CommandHandler* command_handler) -> bool;
  auto Loop() -> void;

  static auto HandleCommandResponse(void* ctx, JsonDocument const& command_result) -> void;
  static auto HandleErrorResponse(void* ctx, char const* error_message, char const* request_json) -> void;

 private:
  auto ProcessMessage(String topic, String payload, MqttMsgProps props) -> void;

  auto ProcessActionScan(JsonDocument json) -> void;
  auto ProcessActionRead(JsonDocument json) -> void;
  auto ProcessActionSubscribe(JsonDocument json) -> void;
  auto ProcessActionUnsubscribe(JsonDocument json) -> void;

  auto SendCommandResponse(JsonDocument const& command_result) -> void;
  auto SendErrorResponse(char const* error_message, char const* request_json = "") -> void;
  auto SendErrorResponse(char const* error_message, JsonDocument* request_json) -> void;

  auto InitEmptyCommand(cmd::Action action) -> cmd::Command;

  logging::Logger& logger_{logging::logger_g};

  MqttClient* mqtt_client_;
  cmd::CommandHandler* command_handler_;
  config::MqttConfig mqtt_config_;

  String mqtt_topic_cmd_;
  String mqtt_topic_stat_;
};

extern MqttMessageHandler mqtt_msg_handler_g;

}  // namespace mqtt
}  // namespace owif

#endif  // OWIF_MQTT_MQTT_MESSAGE_HANDLER_H

#include "mqtt/mqtt_message_handler.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#include "cmd/command.h"
#include "cmd/command_handler.h"
#include "cmd/json_constants.h"
#include "cmd/json_parser.h"
#include "config/mqtt_config.h"
#include "config/persistency.h"
#include "mqtt/mqtt_client.h"
#include "time/time_util.h"

namespace owif {
namespace mqtt {

auto MqttMessageHandler::Begin(MqttClient* mqtt_client, cmd::CommandHandler* command_handler) -> bool {
  bool result{true};

  mqtt_client_ = mqtt_client;
  mqtt_config_ = config::persistency_g.LoadMqttConfig();
  mqtt_topic_cmd_ = mqtt_config_.GetTopic() + "/cmd";
  mqtt_topic_stat_ = mqtt_config_.GetTopic() + "/stat";

  command_handler_ = command_handler;

  mqtt_client_->OnConnectionStateChange([this](ConnectionState connection_state) {
    if (connection_state == ConnectionState::kConnected) {
      MqttMsgId msg_id{mqtt_client_->Subscribe(
          mqtt_topic_cmd_.c_str(),
          [this](String topic, String payload, MqttMsgProps props) { ProcessMessage(topic, payload, props); })};
    }
  });

  return result;
}

auto MqttMessageHandler::Loop() -> void {
  // Nothing to be done
}

// ---- Private APIs ---------------------------------------------------------------------------------------------------

// ---- Request Handling ----

auto MqttMessageHandler::ProcessMessage(String topic, String payload, MqttMsgProps props) -> void {
  logger_.Verbose("[MqttMessageHandler] Msg received | topic: %s payload: %s", topic.c_str(), payload.c_str());

  JsonDocument json{};
  DeserializationError deserialization_result{deserializeJson(json, payload.c_str())};
  if (deserialization_result == DeserializationError::Ok) {
    JsonVariant action_json{json[cmd::json::kRootAction]};
    String action{action_json.as<String>()};

    if (action == cmd::json::kActionRestart) {
      ProcessActionRestart(json);
    } else if (action == cmd::json::kActionScan) {
      ProcessActionScan(json);
    } else if (action == cmd::json::kActionRead) {
      ProcessActionRead(json);
    } else if (action == cmd::json::kActionSubscribe) {
      ProcessActionSubscribe(json);
    } else if (action == cmd::json::kActionUnsubscribe) {
      ProcessActionUnsubscribe(json);
    } else {
      SendErrorResponse("Unknown/Unsupported action.", payload.c_str());
    }
  } else {
    SendErrorResponse("Failed to deserialize MQTT message.", payload.c_str());
  }
}

/*!
 * no parameters
 */
auto MqttMessageHandler::ProcessActionRestart(JsonDocument json) -> void {
  logger_.Debug("[MqttMessageHandler] Process action 'restart'");

  cmd::Command const cmd{InitEmptyCommand(cmd::Action::Restart)};
  command_handler_->EnqueueCommand(cmd);
}

/*!
 * param1: [Optional] device_id
 * param2: [Optional] family_code
 */
auto MqttMessageHandler::ProcessActionScan(JsonDocument json) -> void {
  logger_.Debug("[MqttMessageHandler] Process action 'scan'");

  cmd::Command cmd{InitEmptyCommand(cmd::Action::Scan)};

  bool address_parsing_result{
      cmd::json::JsonParser::ParseAddressing(json, cmd.param1, cmd.param2, /* any_address_info_mandatory:*/ false)};

  if (address_parsing_result) {
    command_handler_->EnqueueCommand(cmd);
  } else {
    String request_json{};
    serializeJson(json, request_json);
    SendErrorResponse("Missing or invalid JSON attributes 'device_id' or 'family_code'.", request_json.c_str());
  }
}

/*!
 * param1: [Optional] device_id
 * param2: [Optional] family_code
 * param3: device_attribute
 */
auto MqttMessageHandler::ProcessActionRead(JsonDocument json) -> void {
  logger_.Debug("[MqttMessageHandler] Process action 'read'");

  cmd::Command cmd{InitEmptyCommand(cmd::Action::Read)};

  bool address_parsing_result{
      cmd::json::JsonParser::ParseAddressing(json, cmd.param1, cmd.param2, /* any_address_info_mandatory:*/ true)};

  if (address_parsing_result) {
    bool const has_attribute_param{cmd::json::JsonParser::ParseDeviceAttribute(json, cmd.param3)};

    if (has_attribute_param) {
      command_handler_->EnqueueCommand(cmd);
    } else {
      String request_json{};
      serializeJson(json, request_json);
      SendErrorResponse("Missing or invalid JSON attribute 'attribute'.", request_json.c_str());
    }
  } else {
    String request_json{};
    serializeJson(json, request_json);
    SendErrorResponse("Missing or invalid JSON attributes 'device_id' or 'family_code'.", request_json.c_str());
  }
}

/*!
 * param1: [Optional] device_id
 * param2: [Optional] family_code
 * param3: device_attribute
 * param4: interval
 */
auto MqttMessageHandler::ProcessActionSubscribe(JsonDocument json) -> void {
  logger_.Debug("[MqttMessageHandler] Process action 'subscribe'");

  cmd::Command cmd{InitEmptyCommand(cmd::Action::Subscribe)};
  bool address_parsing_result{
      cmd::json::JsonParser::ParseAddressing(json, cmd.param1, cmd.param2, /* any_address_info_mandatory:*/ true)};

  if (address_parsing_result) {
    bool const has_attribute_param{cmd::json::JsonParser::ParseDeviceAttribute(json, cmd.param3)};
    bool const has_interval_param{json[cmd::json::kActionSubscribeInterval].is<cmd::TimeIntervalType::type>()};

    if (has_attribute_param && has_interval_param) {
      cmd.param4.param_available = true;  // interval.
      cmd.param4.param_value.interval.value =
          json[cmd::json::kActionSubscribeInterval].as<cmd::TimeIntervalType::type>();

      command_handler_->EnqueueCommand(cmd);
    } else {
      String request_json{};
      serializeJson(json, request_json);
      SendErrorResponse("Missing or invalid JSON attributes 'attribute' or 'interval'.", request_json.c_str());
    }
  } else {
    String request_json{};
    serializeJson(json, request_json);
    SendErrorResponse("Missing or invalid JSON attributes 'device_id' or 'family_code'.", request_json.c_str());
  }
}

/*!
 * param1: [Optional] device_id
 * param2: [Optional] family_code
 * param3: device_attribute
 */
auto MqttMessageHandler::ProcessActionUnsubscribe(JsonDocument json) -> void {
  logger_.Debug("[MqttMessageHandler] Process action 'unsubscribe'");

  cmd::Command cmd{InitEmptyCommand(cmd::Action::Unsubscribe)};
  bool address_parsing_result{
      cmd::json::JsonParser::ParseAddressing(json, cmd.param1, cmd.param2, /* any_address_info_mandatory:*/ true)};

  if (address_parsing_result) {
    bool const has_attribute_param{cmd::json::JsonParser::ParseDeviceAttribute(json, cmd.param3)};
    if (has_attribute_param) {
      command_handler_->EnqueueCommand(cmd);
    } else {
      String request_json{};
      serializeJson(json, request_json);
      SendErrorResponse("Missing or invalid JSON attribute 'attribute'.", request_json.c_str());
    }
  } else {
    String request_json{};
    serializeJson(json, request_json);
    SendErrorResponse("Missing or invalid JSON attributes 'device_id' or 'family_code'.", request_json.c_str());
  }
}

// ---- Response Handling ----

auto MqttMessageHandler::HandleCommandResponse(void* ctx, JsonDocument& command_result) -> void {
  static_cast<MqttMessageHandler*>(ctx)->SendCommandResponse(command_result);
}

auto MqttMessageHandler::HandleErrorResponse(void* ctx, char const* error_message, char const* request_json) -> void {
  static_cast<MqttMessageHandler*>(ctx)->SendErrorResponse(error_message, request_json);
}

auto MqttMessageHandler::SendCommandResponse(JsonDocument& command_result) -> void {
  AddTimestamp(command_result);

  String command_result_serialized{};
  serializeJson(command_result, command_result_serialized);

  mqtt_client_->Publish(mqtt_topic_stat_.c_str(), command_result_serialized.c_str());
}

auto MqttMessageHandler::SendErrorResponse(char const* error_message, char const* request_json) -> void {
  if (request_json != "") {
    // Try to deserialize the original request json string
    JsonDocument request_json_deserialized{};
    DeserializationError const deserialize_result{deserializeJson(request_json_deserialized, request_json)};
    if (deserialize_result == DeserializationError::Ok) {
      SendErrorResponse(error_message, &request_json_deserialized);
    } else {
      SendErrorResponse(error_message, static_cast<JsonDocument*>(nullptr));
    }
  } else {
    SendErrorResponse(error_message, static_cast<JsonDocument*>(nullptr));
  }
}

auto MqttMessageHandler::SendErrorResponse(char const* error_message, JsonDocument* request_json) -> void {
  JsonDocument json{};
  JsonObject json_error{json[cmd::json::kRootError].to<JsonObject>()};
  json_error[cmd::json::kErrorMessage] = error_message;

  if (request_json != nullptr) {
    json_error[cmd::json::kErrorRequest] = request_json->as<JsonObject>();
  } else {
    json_error[cmd::json::kErrorRequest] = nullptr;
  }

  AddTimestamp(json);

  String error_result_serialized{};
  serializeJson(json, error_result_serialized);
  mqtt_client_->Publish(mqtt_topic_stat_.c_str(), error_result_serialized.c_str());
}

// ---- Utilities ----

auto MqttMessageHandler::InitEmptyCommand(cmd::Action const action) -> cmd::Command {
  return cmd::Command{// Timer (no delay)
                      cmd::Timer{},
                      // Action and Sub-Action
                      action, cmd::SubAction::None,
                      // Parameters
                      cmd::CommandParam{false}, cmd::CommandParam{false}, cmd::CommandParam{false},
                      cmd::CommandParam{false},
                      // Result Callback
                      cmd::CommandResultCallback{&MqttMessageHandler::HandleCommandResponse, this},
                      // Error Result Callback
                      cmd::ErrorResultCallback{&MqttMessageHandler::HandleErrorResponse, this}};
}

auto MqttMessageHandler::AddTimestamp(JsonDocument& json) -> void {
  time::DateTime const now{time::TimeUtil::Now()};
  time::FormattedTimeString formatted_timestamp{};
  time::TimeUtil::Format(now, formatted_timestamp);

  json[cmd::json::kTime] = formatted_timestamp;
}

// ---- Global Instance ----
MqttMessageHandler mqtt_msg_handler_g{};

}  // namespace mqtt
}  // namespace owif

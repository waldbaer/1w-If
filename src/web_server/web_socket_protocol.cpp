#include "web_server/web_socket_protocol.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#include "cmd/json_builder.h"
#include "cmd/json_constants.h"

namespace owif {
namespace web_server {
namespace web_socket {

auto WebSocketProtocol::SerializeLog(String const& log_message) -> String {
  JsonDocument log_json{};
  log_json[kWebSocketKeyLogging] = log_message;

  String serialized_json;
  serializeJson(log_json, serialized_json);

  serialized_json += "\n";

  return serialized_json;
}

auto WebSocketProtocol::SerializeOneWireDeviceMap(one_wire::OneWireSystem& one_wire_system) -> String {
  JsonDocument json{};
  JsonArray devices_array{json.createNestedArray(cmd::json::kDevices)};

  one_wire::OneWireSystem::DeviceMap const& devices_map{one_wire_system.GetAvailableDevices()};

  for (std::pair<one_wire::OneWireAddress, std::shared_ptr<one_wire::OneWireDevice>> const& device_entry :
       devices_map) {
    JsonObject device_json = devices_array.createNestedObject();
    device_json[cmd::json::kDeviceId] = device_entry.first.Format();
    cmd::json::JsonBuilder::AddDeviceAttributes(&one_wire_system, device_json, device_entry.first);
  }

  String serialized_json;
  serializeJson(json, serialized_json);

  return serialized_json;
}

}  // namespace web_socket
}  // namespace web_server
}  // namespace owif

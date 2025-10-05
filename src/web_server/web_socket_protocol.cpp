#include "web_server/web_socket_protocol.h"

#include <Arduino.h>
#include <ArduinoJson.h>

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

}  // namespace web_socket
}  // namespace web_server
}  // namespace owif

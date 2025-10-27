#ifndef OWIF_WEB_SERVER_WEB_SOCKET_PROTOCOL_H
#define OWIF_WEB_SERVER_WEB_SOCKET_PROTOCOL_H

#include <Arduino.h>

namespace owif {
namespace web_server {
namespace web_socket {

class WebSocketProtocol {
 public:
  static auto SerializeLog(String const& log_message) -> String;

 private:
  static constexpr char const* kWebSocketKeyLogging{"LOG"};

  WebSocketProtocol() = delete;
  WebSocketProtocol(WebSocketProtocol const&) = delete;
  auto operator=(WebSocketProtocol const&) -> WebSocketProtocol& = delete;
  WebSocketProtocol(WebSocketProtocol&&) = delete;
  auto operator=(WebSocketProtocol&&) -> WebSocketProtocol& = delete;
  ~WebSocketProtocol() = delete;
};

}  // namespace web_socket
}  // namespace web_server
}  // namespace owif

#endif  // OWIF_WEB_SERVER_WEB_SOCKET_PROTOCOL_H

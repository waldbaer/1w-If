#ifndef OWIF_WEB_SERVER_WEB_SERVER_H
#define OWIF_WEB_SERVER_WEB_SERVER_H

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <map>

#include "ethernet/ethernet.h"
#include "logging/logger.h"

namespace owif {
namespace web_server {

class WebServer {
 public:
  WebServer() = default;

  auto Begin() -> bool;
  auto Loop() -> void;

 private:
  static const std::uint16_t kWebServerPort{80};

  auto OnConnectionStateChange(ethernet::ConnectionState connection_state) -> void;

  auto CheckAuthentication(AsyncWebServerRequest* request) -> bool;
  auto GenerateAuthenticationToken() -> String;
  auto RedirectTo(AsyncWebServerRequest* request, char const* location) -> void;

  auto HandleLoginGet(AsyncWebServerRequest* request) -> void;
  auto HandleLoginPost(AsyncWebServerRequest* request) -> void;
  auto HandleLogout(AsyncWebServerRequest* request) -> void;

  auto HandleRoot(AsyncWebServerRequest* request) -> void;
  auto HandleSave(AsyncWebServerRequest* request) -> void;
  auto HandleRestart(AsyncWebServerRequest* request) -> void;

  auto HandleOta(AsyncWebServerRequest* request) -> void;
  auto HandleOtaRequest(AsyncWebServerRequest* request) -> void;
  auto HandleOtaUpdate(AsyncWebServerRequest* request, String const& filename, std::size_t index, std::uint8_t* data,
                       std::size_t len, bool final) -> void;

  logging::Logger& logger_{logging::logger_g};

  bool is_running_{false};
  ::AsyncWebServer web_server_{kWebServerPort};
  std::map<String, std::size_t> sessions;  // token -> timestamp

  std::size_t restart_time_{0};                      // ms
  static constexpr std::size_t kRestartDelay{1000};  // ms
};

/*!
 * \brief Declaration of global Ethernet instance.
 */
extern WebServer web_server_g;

}  // namespace web_server
}  // namespace owif

#endif  // OWIF_WEB_SERVER_WEB_SERVER_H

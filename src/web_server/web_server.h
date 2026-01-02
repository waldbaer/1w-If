#ifndef OWIF_WEB_SERVER_WEB_SERVER_H
#define OWIF_WEB_SERVER_WEB_SERVER_H

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <map>

#include "ethernet/ethernet.h"
#include "logging/logger.h"
#include "one_wire/one_wire_subsystem.h"
#include "util/time_util.h"

namespace owif {
namespace web_server {

enum class ResponseCode : int {
  OK = 200,          //
  BadRequest = 400,  //
  Unauthorized = 401
};

class WebServer {
 public:
  WebServer() = default;
  WebServer(WebServer const&) = default;
  auto operator=(WebServer const&) -> WebServer& = default;
  WebServer(WebServer&&) = default;
  auto operator=(WebServer&&) -> WebServer& = default;
  ~WebServer() = default;

  // ---- Public APIs --------------------------------------------------------------------------------------------------

  auto Begin(one_wire::OneWireSystem& one_wire_system) -> bool;
  auto Loop() -> void;

  auto GetWebSocket() -> ::AsyncWebSocket&;

 private:
  static constexpr std::uint16_t kWebServerPort{80};
  static constexpr char const* kWebSocketUrl{"/"};  // reachable via root URL ws://

  static constexpr std::uint32_t kSessionTimeoutMin{30};
  static constexpr std::uint32_t kSessionTimeoutSec{kSessionTimeoutMin * 60};
  static constexpr std::uint32_t kSessionTimeoutMs{kSessionTimeoutSec * 1000};

  static constexpr char const* kHeaderLocation{"Location"};
  static constexpr char const* kHeaderCookie{"Cookie"};
  static constexpr char const* kHeaderSetCookie{"Set-Cookie"};
  static constexpr char const* kSessionCookieName{"OWIF_SESSION_ID="};

  static constexpr char const* kLoginParamUser{"user"};
  static constexpr char const* kLoginParamPassword{"pass"};

  static constexpr char const* kContextTypePlain{"text/plain"};
  static constexpr char const* kContextTypeHtml{"text/html"};

  static constexpr char const* kConfigSaveLogLevel{"log_level"};
  static constexpr char const* kConfigSaveSerialLog{"serial_log"};
  static constexpr char const* kConfigSaveWebLog{"web_log"};
  static constexpr char const* kConfigSaveOwCh1Enabled{"ow_ch1_enabled"};
  static constexpr char const* kConfigSaveOwCh2Enabled{"ow_ch2_enabled"};
  static constexpr char const* kConfigSaveOwCh3Enabled{"ow_ch3_enabled"};
  static constexpr char const* kConfigSaveOwCh4Enabled{"ow_ch4_enabled"};
  static constexpr char const* kConfigSaveEthHostname{"eth_hostname"};
  static constexpr char const* kConfigSaveOtaPort{"ota_port"};
  static constexpr char const* kConfigSaveOtaPass{"ota_pass"};
  static constexpr char const* kConfigSaveWebServerUser{"webserver_user"};
  static constexpr char const* kConfigSaveWebServerPass{"webserver_pass"};
  static constexpr char const* kConfigSaveMqttServer{"mqtt_server"};
  static constexpr char const* kConfigSaveMqttPort{"mqtt_port"};
  static constexpr char const* kConfigSaveMqttUser{"mqtt_user"};
  static constexpr char const* kConfigSaveMqttPass{"mqtt_pass"};
  static constexpr char const* kConfigSaveMqttTopic{"mqtt_topic"};
  static constexpr char const* kConfigSaveMqttClientId{"mqtt_client_id"};
  static constexpr char const* kConfigSaveMqttReconTimeout{"mqtt_recon_timeout"};

  auto OnConnectionStateChange(ethernet::ConnectionState connection_state) -> void;

  auto CheckAuthentication(AsyncWebServerRequest* request) -> bool;
  auto ConstTimeEquals(String const& left, String const& right) -> bool;
  auto GenerateAuthenticationToken() -> String;
  auto RedirectTo(AsyncWebServerRequest* request, char const* location) -> void;

  auto HandleLoginGet(AsyncWebServerRequest* request) -> void;
  auto HandleLoginPost(AsyncWebServerRequest* request) -> void;
  auto HandleLogout(AsyncWebServerRequest* request) -> void;

  auto HandleDashboard(AsyncWebServerRequest* request) -> void;
  auto HandleRestart(AsyncWebServerRequest* request) -> void;

  auto HandleConfig(AsyncWebServerRequest* request) -> void;
  auto HandleSave(AsyncWebServerRequest* request) -> void;

  auto HandleOta(AsyncWebServerRequest* request) -> void;
  auto HandleOtaRequest(AsyncWebServerRequest* request) -> void;
  auto HandleOtaUpdate(AsyncWebServerRequest* request, String const& filename, std::size_t index, std::uint8_t* data,
                       std::size_t len, bool final) -> void;

  auto HandleConsole(AsyncWebServerRequest* request) -> void;

  auto ToTemplateCheckOption(bool check_option) -> String;

  logging::Logger& logger_{logging::logger_g};
  one_wire::OneWireSystem* one_wire_system_{nullptr};

  bool is_running_{false};

  ::AsyncWebServer web_server_{kWebServerPort};
  ::AsyncWebSocket web_socket_{kWebSocketUrl};

  struct SessionInfo {
    util::TimeStampMs last_activity_ms;
    util::TimeStampMs expires_at_ms;  // millis() + TTL
  };
  using SessionsMap = std::map<String, SessionInfo>;
  SessionsMap sessions_;  // token -> timestamp

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

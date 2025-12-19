#include "web_server/web_server.h"

#include <LittleFS.h>
#include <Update.h>

#include <string>

#include "config/persistency.h"
#include "ethernet/ethernet.h"
#include "logging/web_socket_logger.h"
#include "util/language.h"
#include "util/time_util.h"
#include "web_server/web_socket_protocol.h"

namespace owif {
namespace web_server {

auto WebServer::Begin(one_wire::OneWireSystem& one_wire_system) -> bool {
  logger_.Debug(F("[WebServer] Setup..."));

  one_wire_system_ = &one_wire_system;

  if (!LittleFS.begin()) {
    logger_.Error(F("[WebServer] LittleFS setup failed"));
    return false;
  }

  // Register static handlers
  web_server_.serveStatic("/css/style.css", LittleFS, "/css/style.css");
  web_server_.serveStatic("/js/menu.js", LittleFS, "/js/menu.js");
  web_server_.serveStatic("/js/config-util.js", LittleFS, "/js/config-util.js");
  web_server_.serveStatic("/img/logo-emblem.svg", LittleFS, "/img/logo-emblem.svg");

  // Register path/page handlers
  web_server_.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) { HandleDashboard(request); });
  web_server_.on("/login", HTTP_GET, [this](AsyncWebServerRequest* request) { HandleLoginGet(request); });
  web_server_.on("/login", HTTP_POST, [this](AsyncWebServerRequest* request) { HandleLoginPost(request); });
  web_server_.on("/logout", HTTP_GET, [this](AsyncWebServerRequest* request) { HandleLogout(request); });
  web_server_.on("/save", HTTP_POST, [this](AsyncWebServerRequest* request) { HandleSave(request); });
  web_server_.on("/restart", HTTP_GET, [this](AsyncWebServerRequest* request) { HandleRestart(request); });

  web_server_.on("/config", HTTP_GET, [this](AsyncWebServerRequest* request) { HandleConfig(request); });
  web_server_.on("/ota", HTTP_GET, [this](AsyncWebServerRequest* request) { HandleOta(request); });
  web_server_.on("/console", HTTP_GET, [this](AsyncWebServerRequest* request) { HandleConsole(request); });
  web_server_.on(
      "/ota_update", HTTP_POST,
      // RequestHandler
      [this](AsyncWebServerRequest* request) { HandleOtaRequest(request); },
      // UpdateHandler
      [this](AsyncWebServerRequest* request, String const& filename, std::size_t index, std::uint8_t* data,
             std::size_t len, bool final) { HandleOtaUpdate(request, filename, index, data, len, final); }

  );

  // Register additional handlers
  web_socket_.onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg,
                             uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      owif::logging::web_socket_logger_g.LogFullHistory(client);
      logger_.Debug(F("[WebServer] client %u connected to WebSocket"), client->id());
    } else if (type == WS_EVT_DISCONNECT) {
      logger_.Debug(F("[WebServer] client %u disconnected from WebSocket"), client->id());
    }
  });
  web_server_.addHandler(&web_socket_);

  // Register Ethernet connection state change handler starting / stopping the WebServer
  ethernet::ethernet_g.OnConnectionStateChange(
      [this](ethernet::ConnectionState connection_state) { OnConnectionStateChange(connection_state); });

  return true;
}

auto WebServer::Loop() -> void {
  // Update sessions
  util::TimeStampMs const now{millis()};

  for (SessionsMap::iterator session = sessions_.begin(); session != sessions_.end();) {
    if (session->second.expires_at_ms <= now) {
      logger_.Verbose(F("[WebServer] session expired. Last activity: %s"),
                      util::TimeUtil::Format(session->second.last_activity_ms));
      session = sessions_.erase(session);
    } else {
      ++session;
    }
  }

  // Execute restart command
  if (restart_time_ != 0 && now >= restart_time_) {
    restart_time_ = 0;
    logger_.Info(F("[WebServer] >> RESTART Hardware << (requested by WebServer)"));
    ESP.restart();
  }

  // WebServer is handled asynchronously by base OS

  // Cleanup WebSocket clients not properly closed by WebBrowsers
  web_socket_.cleanupClients();
}

auto WebServer::OnConnectionStateChange(ethernet::ConnectionState connection_state) -> void {
  if (connection_state == ethernet::ConnectionState::kConnected) {
    logger_.Debug(F("[WebServer] Ethernet state changed: Starting Webserver..."));

    web_server_.begin();
    is_running_ = true;
  } else if (connection_state == ethernet::ConnectionState::kDisconnected) {
    logger_.Debug(F("[WebServer] Ethernet state changed: Stopping Webserver..."));
    web_server_.end();
  }
}

auto WebServer::GetWebSocket() -> ::AsyncWebSocket& { return web_socket_; }

// ---- Authentication -------------------------------------------------------------------------------------------------
auto WebServer::GenerateAuthenticationToken() -> String {
  uint8_t buf[20];
  esp_fill_random(buf, sizeof(buf));  // cryptographic safe random bytes

  const char hex[] = "0123456789abcdef";
  String token{};
  token.reserve(sizeof(buf) * 2);
  for (size_t i = 0; i < sizeof(buf); ++i) {
    token += hex[(buf[i] >> 4) & 0x0F];
    token += hex[buf[i] & 0x0F];
  }
  return token;
}

auto WebServer::CheckAuthentication(AsyncWebServerRequest* request) -> bool {
  bool result{false};
  util::TimeStampMs const now{millis()};

  if (request->hasHeader("Cookie")) {
    String cookie = request->header("Cookie");
    int pos{cookie.indexOf(kSessionCookieName)};
    if (pos != -1) {
      String token{cookie.substring(pos + strlen(kSessionCookieName))};

      SessionsMap::iterator found_session{sessions_.find(token)};
      if (found_session != sessions_.end()) {
        // Check if session is still valid or already expired
        if (found_session->second.expires_at_ms > now) {
          // Update session
          found_session->second.last_activity_ms = now;
          found_session->second.expires_at_ms = now + kSessionTimeoutMs;
          result = true;
        } else {
          logger_.Verbose(F("[WebServer] session expired. Last activity: %s"),
                          util::TimeUtil::Format(found_session->second.last_activity_ms));
          sessions_.erase(found_session);  // Session expired
        }
      }
    }
  }
  return result;
}

auto WebServer::ConstTimeEquals(const String& left, const String& right) -> bool {
  bool result{false};

  if (left.length() == right.length()) {
    std::uint8_t character_xor{0};
    for (std::size_t i{0}; i < left.length(); ++i) {
      character_xor |= static_cast<std::uint8_t>(left[i]) ^ static_cast<std::uint8_t>(right[i]);
    }

    result = (character_xor == 0);
  }

  return result;
}

auto WebServer::RedirectTo(AsyncWebServerRequest* request, char const* location) -> void {
  AsyncWebServerResponse* response{request->beginResponse(302, kContextTypePlain, "Please login")};
  response->addHeader(kHeaderLocation, location);
  request->send(response);
}

auto WebServer::HandleLoginGet(AsyncWebServerRequest* request) -> void {
  request->send(LittleFS, "/login.html", kContextTypeHtml);
}

auto WebServer::HandleLoginPost(AsyncWebServerRequest* request) -> void {
  if (!request->hasParam(kLoginParamUser, true) || !request->hasParam(kLoginParamPassword, true)) {
    logger_.Debug(F("[WebServer] Missing/Inconsistent login parameters"));
    request->send(ToUnderlying(ResponseCode::BadRequest), kContextTypePlain, "Missing parameters");
    return;
  }

  AsyncWebParameter const* param_user{request->getParam(kLoginParamUser, true)};
  AsyncWebParameter const* param_pass{request->getParam(kLoginParamPassword, true)};
  if (param_user == nullptr || param_pass == nullptr) {
    logger_.Debug(F("[WebServer] Missing/Inconsistent login parameters"));
    request->send(ToUnderlying(ResponseCode::BadRequest), kContextTypePlain, "Bad request");
    return;
  }

  String const& user{param_user->value()};
  String const& pass{param_pass->value()};
  logger_.Debug(F("[WebServer] Checking login from user '%s'"), user);

  config::WebServerConfig const webserver_config{config::persistency_g.LoadWebServerConfig()};
  if (ConstTimeEquals(user, webserver_config.GetUser()) && ConstTimeEquals(pass, webserver_config.GetPassword())) {
    util::TimeStampMs const now{millis()};

    String const token{GenerateAuthenticationToken()};
    sessions_[token] = SessionInfo{/*last_activity_ms=*/now, /*expires_at_ms=*/now + kSessionTimeoutMs};

    AsyncWebServerResponse* response = request->beginResponse(200, "text/plain", "LOGIN_SUCCESS");
    String const cookie_header{kSessionCookieName + token +
                               "; Path=/; HttpOnly; SameSite=Lax; Max-Age=" + String(kSessionTimeoutSec)};
    response->addHeader("Set-Cookie", cookie_header);
    request->send(response);
  } else {
    request->send(ToUnderlying(ResponseCode::Unauthorized), kContextTypePlain, "Login failed");
  }
}

auto WebServer::HandleLogout(AsyncWebServerRequest* request) -> void {
  if (request->hasHeader(kHeaderCookie)) {
    String const cookie{request->header(kHeaderCookie)};
    int const pos{cookie.indexOf(kSessionCookieName)};
    if (pos != -1) {
      String const token{cookie.substring(pos + strlen(kSessionCookieName))};
      sessions_.erase(token);
    }
  }

  AsyncWebServerResponse* response{request->beginResponse(302, kContextTypePlain, "Logged out.")};
  response->addHeader(kHeaderLocation, "/login");
  response->addHeader(kHeaderSetCookie, String{kSessionCookieName} + "deleted; Max-Age=0");
  request->send(response);
}

// ---- Sites ----------------------------------------------------------------------------------------------------------

auto WebServer::HandleDashboard(AsyncWebServerRequest* request) -> void {
  if (!CheckAuthentication(request)) {
    RedirectTo(request, "/login");
    return;
  }

  request->send(LittleFS, "/index.html", kContextTypeHtml, false, [this](String const& var) {
    if (var == "OW_DEVICES") {
      return web_socket::WebSocketProtocol::SerializeOneWireDeviceMap(*one_wire_system_);
    } else {
      logger_.Warn(F("[WebServer] Ignoring HTML template variable: %s"), var);
      return String{};
    }
  });
}

auto WebServer::HandleRestart(AsyncWebServerRequest* request) -> void {
  request->send(ToUnderlying(ResponseCode::OK), kContextTypeHtml, "Restarting... (Reloading page in 5sec)");

  restart_time_ = millis() + kRestartDelay;
}

auto WebServer::HandleSave(AsyncWebServerRequest* request) -> void {
  if (!CheckAuthentication(request)) {
    RedirectTo(request, "/login");
    return;
  }

  logger_.Debug(F("[WebServer] Saving config..."));

  // ---- Store LoggingConfig ----
  config::LoggingConfig logging_config{config::persistency_g.LoadLoggingConfig()};

  if (request->hasParam(kConfigSaveLogLevel, true)) {
    logging_config.SetLogLevel(
        static_cast<logging::LogLevel>(request->getParam(kConfigSaveLogLevel, true)->value().toInt()));
  }
  // checkboxes: value 'on' only sent in form data if checkbox is set (HTML standard)
  logging_config.SetSerialLogEnabled(request->hasParam(kConfigSaveSerialLog, true));
  logging_config.SetWebLogEnabled(request->hasParam(kConfigSaveWebLog, true));

  config::persistency_g.StoreLoggingConfig(logging_config);

  // ---- Store OneWireConfig ----
  config::OneWireConfig onewire_config{config::persistency_g.LoadOneWireConfig()};

  // checkboxes: value 'on' only sent in form data if checkbox is set (HTML standard)
  onewire_config.GetChannelConfig(config::OneWireConfig::kOneWireChannel1)
      .SetEnabled(request->hasParam(kConfigSaveOwCh1Enabled, true));
  onewire_config.GetChannelConfig(config::OneWireConfig::kOneWireChannel2)
      .SetEnabled(request->hasParam(kConfigSaveOwCh2Enabled, true));
  onewire_config.GetChannelConfig(config::OneWireConfig::kOneWireChannel3)
      .SetEnabled(request->hasParam(kConfigSaveOwCh3Enabled, true));
  onewire_config.GetChannelConfig(config::OneWireConfig::kOneWireChannel4)
      .SetEnabled(request->hasParam(kConfigSaveOwCh4Enabled, true));

  config::persistency_g.StoreOneWireConfig(onewire_config);

  // ---- Store EthernetConfig ----
  config::EthernetConfig ethernet_config{config::persistency_g.LoadEthernetConfig()};

  if (request->hasParam(kConfigSaveEthHostname, true)) {
    ethernet_config.SetHostname(request->getParam(kConfigSaveEthHostname, true)->value());
  }

  config::persistency_g.StoreEthernetConfig(ethernet_config);

  // ---- Store OTA Config ----

  config::OtaConfig ota_config{config::persistency_g.LoadOtaConfig()};

  if (request->hasParam(kConfigSaveOtaPort, true)) {
    ota_config.SetPort(request->getParam(kConfigSaveOtaPort, true)->value().toInt());
  }
  if (request->hasParam(kConfigSaveOtaPass, true)) {
    String const password{request->getParam(kConfigSaveOtaPass, true)->value()};
    if (password != "") {
      ota_config.SetPassword(password);
    }
  }

  config::persistency_g.StoreOtaConfig(ota_config);

  // ---- Store WebServer Config ----

  config::WebServerConfig webserver_config{config::persistency_g.LoadWebServerConfig()};

  if (request->hasParam(kConfigSaveWebServerUser, true)) {
    webserver_config.SetUser(request->getParam(kConfigSaveWebServerUser, true)->value());
  }
  if (request->hasParam(kConfigSaveWebServerPass, true)) {
    String const password{request->getParam(kConfigSaveWebServerPass, true)->value()};
    if (password != "") {
      webserver_config.SetPassword(password);
    }
  }

  config::persistency_g.StoreWebServerConfig(webserver_config);

  // ---- Store Mqtt Config ----

  config::MqttConfig mqtt_config{config::persistency_g.LoadMqttConfig()};

  if (request->hasParam(kConfigSaveMqttServer, true)) {
    mqtt_config.SetServerAddr(request->getParam(kConfigSaveMqttServer, true)->value());
  }
  if (request->hasParam(kConfigSaveMqttPort, true)) {
    mqtt_config.SetServerPort(request->getParam(kConfigSaveMqttPort, true)->value().toInt());
  }
  if (request->hasParam(kConfigSaveMqttUser, true)) {
    mqtt_config.SetUser(request->getParam(kConfigSaveMqttUser, true)->value());
  }
  if (request->hasParam(kConfigSaveMqttPass, true)) {
    String const password{request->getParam(kConfigSaveMqttPass, true)->value()};
    if (password != "") {
      mqtt_config.SetPassword(password);
    }
  }
  if (request->hasParam(kConfigSaveMqttReconTimeout, true)) {
    String const reconnect_timeout_str{request->getParam(kConfigSaveMqttReconTimeout, true)->value()};
    mqtt_config.SetReconnectTimeout(std::strtoul(reconnect_timeout_str.c_str(), nullptr, 10));
  }
  if (request->hasParam(kConfigSaveMqttTopic, true)) {
    mqtt_config.SetTopic(request->getParam(kConfigSaveMqttTopic, true)->value());
  }

  config::persistency_g.StoreMqttConfig(mqtt_config);

  // ---- All parts updated: Send response ----
  logging::logger_g.Info(F("[WebServer] Updated configuration:"));
  config::persistency_g.PrettyPrint(logging::logger_g);

  request->send(ToUnderlying(ResponseCode::OK), kContextTypeHtml, "Stored! Restart necessary...");
}

auto WebServer::HandleConfig(AsyncWebServerRequest* request) -> void {
  if (!CheckAuthentication(request)) {
    RedirectTo(request, "/login");
    return;
  }

  config::LoggingConfig const logging_config{config::persistency_g.LoadLoggingConfig()};
  config::OneWireConfig const onewire_config{config::persistency_g.LoadOneWireConfig()};
  config::EthernetConfig const ethernet_config{config::persistency_g.LoadEthernetConfig()};
  config::OtaConfig const ota_config{config::persistency_g.LoadOtaConfig()};
  config::WebServerConfig const webserver_config{config::persistency_g.LoadWebServerConfig()};
  config::MqttConfig const mqtt_config{config::persistency_g.LoadMqttConfig()};

  request->send(LittleFS, "/config.html", kContextTypeHtml, false,
                [this, logging_config, onewire_config, ethernet_config, ota_config, webserver_config,
                 mqtt_config](String const& var) {
                  // LoggingConfig
                  if (var == "LOG_LEVEL") {
                    return String{static_cast<std::underlying_type_t<logging::LogLevel>>(logging_config.GetLogLevel())};
                  } else if (var == "LOG_SERIAL") {
                    return ToTemplateCheckOption(logging_config.GetSerialLogEnabled());
                  } else if (var == "LOG_WEB") {
                    return ToTemplateCheckOption(logging_config.GetWebLogEnabled());
                  }
                  // OneWireConfig
                  else if (var == "OW_CH1_ENABLED") {
                    return ToTemplateCheckOption(
                        onewire_config.GetChannelConfig(config::OneWireConfig::kOneWireChannel1).GetEnabled());
                  } else if (var == "OW_CH2_ENABLED") {
                    return ToTemplateCheckOption(
                        onewire_config.GetChannelConfig(config::OneWireConfig::kOneWireChannel2).GetEnabled());
                  } else if (var == "OW_CH3_ENABLED") {
                    return ToTemplateCheckOption(
                        onewire_config.GetChannelConfig(config::OneWireConfig::kOneWireChannel3).GetEnabled());
                  } else if (var == "OW_CH4_ENABLED") {
                    return ToTemplateCheckOption(
                        onewire_config.GetChannelConfig(config::OneWireConfig::kOneWireChannel4).GetEnabled());
                  }
                  // EthernetConfig
                  else if (var == "ETH_HOSTNAME") {
                    return ethernet_config.GetHostname();
                  }
                  // OtaConfig
                  else if (var == "OTA_PORT") {
                    return String{ota_config.GetPort()};
                  }
                  // WebServerConfig
                  else if (var == "WEBSERVER_USER") {
                    return webserver_config.GetUser();
                  }
                  // MqttConfig
                  else if (var == "MQTT_SERVER") {
                    return mqtt_config.GetServerAddr();
                  } else if (var == "MQTT_PORT") {
                    return String{mqtt_config.GetServerPort()};
                  } else if (var == "MQTT_USER") {
                    return mqtt_config.GetUser();
                  } else if (var == "MQTT_RECON_TIMEOUT") {
                    return String{mqtt_config.GetReconnectTimeout()};
                  } else if (var == "MQTT_TOPIC") {
                    return mqtt_config.GetTopic();
                  }
                  // Unknown
                  else {
                    logger_.Warn(F("[WebServer] Ignoring HTML template variable: %s"), var);
                    return String();
                  }
                });
}

auto WebServer::HandleOta(AsyncWebServerRequest* request) -> void {
  if (!CheckAuthentication(request)) {
    RedirectTo(request, "/login");
    return;
  }
  request->send(LittleFS, "/ota.html", kContextTypeHtml);
}

auto WebServer::HandleOtaRequest(AsyncWebServerRequest* request) -> void {
  if (!CheckAuthentication(request)) {
    RedirectTo(request, "/login");
    return;
  }

  bool const update_result{!Update.hasError()};
  request->send(ToUnderlying(ResponseCode::OK), kContextTypePlain,
                update_result ? "Update success! Restarting..." : "Update failed!");

  if (update_result) {
    restart_time_ = millis() + kRestartDelay;
  }
}

auto WebServer::HandleOtaUpdate(AsyncWebServerRequest* request, String const& filename, std::size_t index,
                                std::uint8_t* data, std::size_t len, bool final) -> void {
  if (!index) {
    logger_.Debug(F("[WebServer] OTA update start: %s"), filename);
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  }
  if (!Update.hasError()) {
    if (Update.write(data, len) != len) {
      Update.printError(Serial);
    }
  }
  if (final) {
    if (Update.end(true)) {
      logger_.Debug(F("[WebServer] OTA update completed: %u bytes"), index + len);
    } else {
      Update.printError(Serial);
    }
  }
}

auto WebServer::HandleConsole(AsyncWebServerRequest* request) -> void {
  if (!CheckAuthentication(request)) {
    RedirectTo(request, "/login");
    return;
  }
  request->send(LittleFS, "/console.html", kContextTypeHtml);
}

auto WebServer::ToTemplateCheckOption(bool check_option) -> String {
  return check_option ? String{"checked"} : String{""};
}

/*!
 * \brief Definition of global Ethernet instance.
 */
WebServer web_server_g{};

}  // namespace web_server
}  // namespace owif

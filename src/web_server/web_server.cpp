#include "web_server/web_server.h"

#include <LittleFS.h>
#include <Update.h>

#include <string>

#include "config/persistency.h"
#include "ethernet/ethernet.h"
#include "logging/web_socket_logger.h"
#include "util/language.h"
#include "util/time_util.h"

namespace owif {
namespace web_server {

auto WebServer::Begin() -> bool {
  logger_.Debug(F("[WebServer] Setup..."));

  if (!LittleFS.begin()) {
    logger_.Error(F("[WebServer] LittleFS setup failed"));
    return false;
  }

  // Register static handlers
  web_server_.serveStatic("/style.css", LittleFS, "/style.css");

  // Register path/page handlers
  web_server_.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) { HandleRoot(request); });
  web_server_.on("/login", HTTP_GET, [this](AsyncWebServerRequest* request) { HandleLoginGet(request); });
  web_server_.on("/login", HTTP_POST, [this](AsyncWebServerRequest* request) { HandleLoginPost(request); });
  web_server_.on("/logout", HTTP_GET, [this](AsyncWebServerRequest* request) { HandleLogout(request); });
  web_server_.on("/save", HTTP_POST, [this](AsyncWebServerRequest* request) { HandleSave(request); });
  web_server_.on("/restart", HTTP_GET, [this](AsyncWebServerRequest* request) { HandleRestart(request); });

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
      logger_.Debug(F("[WebServer] client %u connected to WebSocket"), client->id());
      owif::logging::web_socket_logger_g.LogFullHistory(client);
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

    AsyncWebServerResponse* response{request->beginResponse(302, kContextTypePlain, "Login successful")};
    response->addHeader(kHeaderLocation, "/");
    response->addHeader(kHeaderSetCookie,
                        kSessionCookieName + token + "; Path=/; HttpOnly; SameSite=Lax; Max-Age=" + kSessionTimeoutSec);
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

auto WebServer::HandleRoot(AsyncWebServerRequest* request) -> void {
  if (!CheckAuthentication(request)) {
    RedirectTo(request, "/login");
    return;
  }
  logger_.Verbose(F("[WebServer] Handle root request"));

  config::LoggingConfig const logging_config{config::persistency_g.LoadLoggingConfig()};
  config::EthernetConfig const ethernet_config{config::persistency_g.LoadEthernetConfig()};
  config::OtaConfig const ota_config{config::persistency_g.LoadOtaConfig()};
  config::WebServerConfig const webserver_config{config::persistency_g.LoadWebServerConfig()};
  config::MqttConfig const mqtt_config{config::persistency_g.LoadMqttConfig()};

  request->send(LittleFS, "/index.html", kContextTypeHtml, false,
                [this, logging_config, ethernet_config, ota_config, webserver_config, mqtt_config](String const& var) {
                  // LoggingConfig
                  if (var == "LOG_LEVEL") {
                    return String{static_cast<std::underlying_type_t<logging::LogLevel>>(logging_config.GetLogLevel())};
                  } else if (var == "LOG_SERIAL") {
                    return logging_config.GetSerialLogEnabled() ? String{"checked"} : String{""};
                  } else if (var == "LOG_WEB") {
                    return logging_config.GetWebLogEnabled() ? String{"checked"} : String{""};
                  }
                  // EthernetConfig
                  if (var == "ETH_HOSTNAME") {
                    return ethernet_config.GetHostname();
                  }
                  // OtaConfig
                  else if (var == "OTA_PORT") {
                    return String{ota_config.GetPort()};
                  } else if (var == "OTA_PASS") {
                    return ota_config.GetPassword();
                  }
                  // WebServerConfig
                  else if (var == "WEBSERVER_USER") {
                    return webserver_config.GetUser();
                  } else if (var == "WEBSERVER_PASS") {
                    return webserver_config.GetPassword();
                  }
                  // MqttConfig
                  else if (var == "MQTT_SERVER") {
                    return mqtt_config.GetServerAddr();
                  } else if (var == "MQTT_PORT") {
                    return String{mqtt_config.GetServerPort()};
                  } else if (var == "MQTT_USER") {
                    return mqtt_config.GetUser();
                  } else if (var == "MQTT_PASS") {
                    return mqtt_config.GetPassword();
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

auto WebServer::HandleSave(AsyncWebServerRequest* request) -> void {
  if (!CheckAuthentication(request)) {
    RedirectTo(request, "/login");
    return;
  }

  logger_.Debug(F("[WebServer] Saving config..."));

  // ---- Store LoggingConfig ----
  config::LoggingConfig logging_config{};

  if (request->hasParam(kConfigSaveLogLevel, true)) {
    logging_config.SetLogLevel(
        static_cast<logging::LogLevel>(request->getParam(kConfigSaveLogLevel, true)->value().toInt()));
  }
  if (request->hasParam(kConfigSaveSerialLog, true)) {
    logging_config.SetSerialLogEnabled(request->getParam(kConfigSaveSerialLog, true)->value() == "on");
  }
  if (request->hasParam(kConfigSaveWebLog, true)) {
    logging_config.SetWebLogEnabled(request->getParam(kConfigSaveWebLog, true)->value() == "on");
  }

  config::persistency_g.StoreLoggingConfig(logging_config);

  // ---- Store EthernetConfig ----
  config::EthernetConfig ethernet_config{};

  if (request->hasParam(kConfigSaveEthHostname, true)) {
    ethernet_config.SetHostname(request->getParam(kConfigSaveEthHostname, true)->value());
  }

  config::persistency_g.StoreEthernetConfig(ethernet_config);

  // ---- Store OTA Config ----

  config::OtaConfig ota_config{};

  if (request->hasParam(kConfigSaveOtaPort, true)) {
    ota_config.SetPort(request->getParam(kConfigSaveOtaPort, true)->value().toInt());
  }
  if (request->hasParam(kConfigSaveOtaPass, true)) {
    ota_config.SetPassword(request->getParam(kConfigSaveOtaPass, true)->value());
  }

  config::persistency_g.StoreOtaConfig(ota_config);

  // ---- Store WebServer Config ----

  config::WebServerConfig webserver_config{};

  if (request->hasParam(kConfigSaveWebServerUser, true)) {
    webserver_config.SetUser(request->getParam(kConfigSaveWebServerUser, true)->value());
  }
  if (request->hasParam(kConfigSaveWebServerPass, true)) {
    webserver_config.SetPassword(request->getParam(kConfigSaveWebServerPass, true)->value());
  }

  config::persistency_g.StoreWebServerConfig(webserver_config);

  // ---- Store Mqtt Config ----

  config::MqttConfig mqtt_config{};

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
    mqtt_config.SetPassword(request->getParam(kConfigSaveMqttPass, true)->value());
  }
  if (request->hasParam(kConfigSaveMqttReconTimeout, true)) {
    String const reconnect_timeout_str{request->getParam(kConfigSaveMqttReconTimeout, true)->value()};
    mqtt_config.SetReconnectTimeout(std::strtoul(reconnect_timeout_str.c_str(), nullptr, 10));
  }
  if (request->hasParam(kConfigSaveMqttTopic, true)) {
    mqtt_config.SetTopic(request->getParam(kConfigSaveMqttTopic, true)->value());
  }

  config::persistency_g.StoreMqttConfig(mqtt_config);

  // Send positive response
  request->send(ToUnderlying(ResponseCode::OK), kContextTypeHtml, "Stored! Restart necessary...");
}

auto WebServer::HandleRestart(AsyncWebServerRequest* request) -> void {
  request->send(ToUnderlying(ResponseCode::OK), kContextTypeHtml, "Restarting... (Reloading page in 5sec)");

  restart_time_ = millis() + kRestartDelay;
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

/*!
 * \brief Definition of global Ethernet instance.
 */
WebServer web_server_g{};

}  // namespace web_server
}  // namespace owif

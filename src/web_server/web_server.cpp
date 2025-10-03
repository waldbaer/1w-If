#include "web_server/web_server.h"

#include <LittleFS.h>
#include <Update.h>

#include <string>

#include "config/persistency.h"
#include "ethernet/ethernet.h"

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
  web_server_.serveStatic("/ota", LittleFS, "/ota.html");

  // Register path/page handlers
  web_server_.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) { HandleRoot(request); });
  web_server_.on("/login", HTTP_GET, [this](AsyncWebServerRequest* request) { HandleLoginGet(request); });
  web_server_.on("/login", HTTP_POST, [this](AsyncWebServerRequest* request) { HandleLoginPost(request); });
  web_server_.on("/logout", HTTP_GET, [this](AsyncWebServerRequest* request) { HandleLogout(request); });
  web_server_.on("/save", HTTP_POST, [this](AsyncWebServerRequest* request) { HandleSave(request); });
  web_server_.on("/restart", HTTP_GET, [this](AsyncWebServerRequest* request) { HandleRestart(request); });

  web_server_.on("/ota", HTTP_GET, [this](AsyncWebServerRequest* request) { HandleOta(request); });
  web_server_.on(
      "/ota_update", HTTP_POST,
      // RequestHandler
      [this](AsyncWebServerRequest* request) { HandleOtaRequest(request); },
      // UpdateHandler
      [this](AsyncWebServerRequest* request, String const& filename, std::size_t index, std::uint8_t* data,
             std::size_t len, bool final) { HandleOtaUpdate(request, filename, index, data, len, final); }

  );

  // Register Ethernet connection state change handler starting / stopping the WebServer
  ethernet::ethernet_g.OnConnectionStateChange(
      [this](ethernet::ConnectionState connection_state) { OnConnectionStateChange(connection_state); });

  return true;
}

auto WebServer::Loop() -> void {
  // Execute restart command
  if (restart_time_ != 0 && millis() >= restart_time_) {
    restart_time_ = 0;
    logger_.Info(F("[WebServer] >> RESTART Hardware << (requested by WebServer)"));
    ESP.restart();
  }

  // WebServer is handled asynchronously by base OS
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

// ---- Authentication -------------------------------------------------------------------------------------------------
auto WebServer::GenerateAuthenticationToken() -> String {
  const char chars[]{"abcdefghijklmnopqrstuvwxyz0123456789"};
  String token;
  for (int i = 0; i < 20; i++) {
    token += chars[random(0, sizeof(chars) - 1)];
  }
  return token;
}

auto WebServer::CheckAuthentication(AsyncWebServerRequest* request) -> bool {
  if (request->hasHeader("Cookie")) {
    String cookie = request->header("Cookie");
    int pos{cookie.indexOf("ESPSESSIONID=")};
    if (pos != -1) {
      String token{cookie.substring(pos + strlen("ESPSESSIONID="))};
      if (sessions.count(token)) {
        // Optional: check session timeout
        sessions[token] = millis();  // Update activity
        return true;
      }
    }
  }
  return false;
}
auto WebServer::RedirectTo(AsyncWebServerRequest* request, char const* location) -> void {
  AsyncWebServerResponse* response{request->beginResponse(302, "text/plain", "Please login")};
  response->addHeader("Location", location);
  request->send(response);
}

auto WebServer::HandleLoginGet(AsyncWebServerRequest* request) -> void {
  request->send(LittleFS, "/login.html", "text/html");
}

auto WebServer::HandleLoginPost(AsyncWebServerRequest* request) -> void {
  String user{request->getParam("user", true)->value()};
  String pass{request->getParam("pass", true)->value()};

  config::WebServerConfig webserver_config{config::persistency_g.LoadWebServerConfig()};

  logger_.Debug(F("[WebServer] Checking login from user '%s' with password '<protected>'"), user.c_str());

  if (user == String{webserver_config.GetUser().c_str()} && pass == String{webserver_config.GetPassword().c_str()}) {
    String token{GenerateAuthenticationToken()};
    sessions[token] = millis();

    AsyncWebServerResponse* response{request->beginResponse(302, "text/plain", "Login successful")};
    response->addHeader("Location", "/");
    response->addHeader("Set-Cookie", "ESPSESSIONID=" + token);
    request->send(response);
  } else {
    request->send(401, "text/plain", "Login failed");
  }
}

auto WebServer::HandleLogout(AsyncWebServerRequest* request) -> void {
  if (request->hasHeader("Cookie")) {
    String cookie{request->header("Cookie")};
    int pos{cookie.indexOf("ESPSESSIONID=")};
    if (pos != -1) {
      String token{cookie.substring(pos + strlen("ESPSESSIONID="))};
      sessions.erase(token);
    }
  }
  AsyncWebServerResponse* response{request->beginResponse(302, "text/plain", "Logged out.")};
  response->addHeader("Location", "/login");
  response->addHeader("Set-Cookie", "ESPSESSIONID=deleted; Max-Age=0");
  request->send(response);
}

// ---- Sites ----------------------------------------------------------------------------------------------------------

auto WebServer::HandleRoot(AsyncWebServerRequest* request) -> void {
  if (!CheckAuthentication(request)) {
    RedirectTo(request, "/login");
    return;
  }
  logger_.Verbose(F("[WebServer] Handle root request"));

  config::EthernetConfig ethernet_config{config::persistency_g.LoadEthernetConfig()};
  config::WebServerConfig webserver_config{config::persistency_g.LoadWebServerConfig()};
  config::MqttConfig mqtt_config{config::persistency_g.LoadMqttConfig()};

  request->send(LittleFS, "/index.html", "text/html", false,
                [this, ethernet_config, webserver_config, mqtt_config](String const& var) {
                  // EthernetConfig
                  if (var == "ETH_HOSTNAME") {
                    return ethernet_config.GetHostname();
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
                    logger_.Warn(F("[WebServer] Ignoring HTML template variable: %s"), var.c_str());
                    return String();
                  }
                });
}

auto WebServer::HandleSave(AsyncWebServerRequest* request) -> void {
  if (!CheckAuthentication(request)) {
    RedirectTo(request, "/login");
    return;
  }

  logger_.Verbose(F("[WebServer] saving config..."));

  // ---- Store EthernetConfig ----
  config::EthernetConfig ethernet_config{};

  if (request->hasParam("eth_hostname", true)) {
    ethernet_config.SetHostname(request->getParam("eth_hostname", true)->value().c_str());
  }

  config::persistency_g.StoreEthernetConfig(ethernet_config);

  // ---- Store WebServer Config ----

  config::WebServerConfig webserver_config{};

  if (request->hasParam("webserver_user", true)) {
    webserver_config.SetUser(request->getParam("webserver_user", true)->value().c_str());
  }
  if (request->hasParam("webserver_pass", true)) {
    webserver_config.SetPassword(request->getParam("webserver_pass", true)->value().c_str());
  }

  config::persistency_g.StoreWebServerConfig(webserver_config);

  // ---- Store Mqtt Config ----

  config::MqttConfig mqtt_config{};

  if (request->hasParam("mqtt_server", true)) {
    mqtt_config.SetServerAddr(request->getParam("mqtt_server", true)->value().c_str());
  }
  if (request->hasParam("mqtt_port", true)) {
    mqtt_config.SetServerPort(request->getParam("mqtt_port", true)->value().toInt());
  }
  if (request->hasParam("mqtt_user", true)) {
    mqtt_config.SetUser(request->getParam("mqtt_user", true)->value().c_str());
  }
  if (request->hasParam("mqtt_pass", true)) {
    mqtt_config.SetPassword(request->getParam("mqtt_pass", true)->value().c_str());
  }
  if (request->hasParam("MQTT_RECON_TIMEOUT", true)) {
    std::string reconnect_timeout_str{request->getParam("MQTT_RECON_TIMEOUT", true)->value().c_str()};
    std::uint32_t reconnect_timeout{static_cast<std::uint32_t>(std::stoul(reconnect_timeout_str, nullptr, 10))};
    mqtt_config.SetReconnectTimeout(reconnect_timeout);
  }
  if (request->hasParam("mqtt_topic", true)) {
    mqtt_config.SetTopic(request->getParam("mqtt_topic", true)->value().c_str());
  }

  config::persistency_g.StoreMqttConfig(mqtt_config);

  // Send positive response
  request->send(200, "text/html", "Stored! Restart necessary...");
}

auto WebServer::HandleRestart(AsyncWebServerRequest* request) -> void {
  request->send(200, "text/html", "Restarting... (Reloading page in 5sec)");

  restart_time_ = millis() + kRestartDelay;
}

auto WebServer::HandleOta(AsyncWebServerRequest* request) -> void {
  if (!CheckAuthentication(request)) {
    RedirectTo(request, "/login");
    return;
  }
  request->send(LittleFS, "/ota.html", "text/html");
}

auto WebServer::HandleOtaRequest(AsyncWebServerRequest* request) -> void {
  if (!CheckAuthentication(request)) {
    RedirectTo(request, "/login");
    return;
  }

  bool const update_result{!Update.hasError()};
  request->send(200, "text/plain", update_result ? "Update success! Restarting..." : "Update failed!");

  if (update_result) {
    restart_time_ = millis() + kRestartDelay;
  }
}

auto WebServer::HandleOtaUpdate(AsyncWebServerRequest* request, String const& filename, std::size_t index,
                                std::uint8_t* data, std::size_t len, bool final) -> void {
  if (!index) {
    logger_.Debug(F("[WebServer] OTA update start: %s"), filename.c_str());
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
      logger_.Debug(F("[WebServer] OTA update completed: %u Bytes"), index + len);
    } else {
      Update.printError(Serial);
    }
  }
}

/*!
 * \brief Definition of global Ethernet instance.
 */
WebServer web_server_g{};

}  // namespace web_server
}  // namespace owif

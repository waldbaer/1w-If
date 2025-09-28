#ifndef OWIF_CONFIG_WEBSERVER_CONFIG_H
#define OWIF_CONFIG_WEBSERVER_CONFIG_H

#include <Preferences.h>

#include <string>

namespace owif {
namespace config {

class WebServerConfig {
 public:
  WebServerConfig() = default;

  WebServerConfig(WebServerConfig const&) = default;
  WebServerConfig(WebServerConfig&&) = default;
  auto operator=(WebServerConfig const&) -> WebServerConfig& = default;
  auto operator=(WebServerConfig&&) -> WebServerConfig& = default;

  ~WebServerConfig() = default;

  // ---- Public APIs ----

  auto GetUser() const -> String const&;
  auto SetUser(String user) -> void;

  auto GetPassword() const -> String const&;
  auto SetPassword(String password) -> void;

 private:
  String user_{"admin"};
  String password_{"1w-If"};
};

}  // namespace config
}  // namespace owif

#endif  // OWIF_CONFIG_WEBSERVER_CONFIG_H

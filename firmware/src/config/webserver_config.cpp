#include "config/webserver_config.h"

#include <string>

namespace owif {
namespace config {

auto WebServerConfig::GetUser() const -> String const& { return user_; }
auto WebServerConfig::SetUser(String user) -> void { user_ = std::move(user); }

auto WebServerConfig::GetPassword() const -> String const& { return password_; }
auto WebServerConfig::SetPassword(String password) -> void { password_ = std::move(password); }

}  // namespace config
}  // namespace owif

#include "config/logging_config.h"

#include <cstdint>

namespace owif {
namespace config {

auto LoggingConfig::GetLogLevel() const -> LogLevel { return log_level_; }
auto LoggingConfig::SetLogLevel(LogLevel log_level) -> void { log_level_ = log_level; }

auto LoggingConfig::GetSerialLogEnabled() const -> bool { return serial_log_enabled_; }
auto LoggingConfig::SetSerialLogEnabled(bool enabled) -> void { serial_log_enabled_ = enabled; }

auto LoggingConfig::GetWebLogEnabled() const -> bool { return web_log_enabled_; }
auto LoggingConfig::SetWebLogEnabled(bool enabled) -> void { web_log_enabled_ = enabled; }

}  // namespace config
}  // namespace owif

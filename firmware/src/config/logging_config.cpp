#include "config/logging_config.h"

#include <cstdint>

namespace owif {
namespace config {

auto LoggingConfig::GetLogLevel() const -> LogLevel { return log_level_; }
auto LoggingConfig::GetLogLevelAsString() const -> char const* {
  switch (log_level_) {
    case LogLevel::Off:
      return "Off    ";
    case LogLevel::Fatal:
      return "Fatal  ";
    case LogLevel::Error:
      return "Error  ";
    case LogLevel::Warning:
      return "Warn   ";
    case LogLevel::Info:
      return "Info   ";
    case LogLevel::Debug:
      return "Debug  ";
    case LogLevel::Verbose:
      return "Verbose";
    default:
      return "Unknown";
  }
}
auto LoggingConfig::SetLogLevel(LogLevel log_level) -> void { log_level_ = log_level; }

auto LoggingConfig::GetSerialLogEnabled() const -> bool { return serial_log_enabled_; }
auto LoggingConfig::SetSerialLogEnabled(bool enabled) -> void { serial_log_enabled_ = enabled; }

auto LoggingConfig::GetWebLogEnabled() const -> bool { return web_log_enabled_; }
auto LoggingConfig::SetWebLogEnabled(bool enabled) -> void { web_log_enabled_ = enabled; }

}  // namespace config
}  // namespace owif

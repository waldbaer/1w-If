#ifndef OWIF_CONFIG_LOGGING_CONFIG_H
#define OWIF_CONFIG_LOGGING_CONFIG_H

#include <Preferences.h>

#include <cstdint>

#include "logging/log_level.h"

namespace owif {
namespace config {

class LoggingConfig {
 public:
  using LogLevel = logging::LogLevel;

  static constexpr LogLevel kDefaultLogLevel{LogLevel::Info};
  static constexpr bool kDefaultSerialLogEnabled{false};
  static constexpr bool kDefaultWebLogEnabled{false};

  LoggingConfig() = default;

  LoggingConfig(LoggingConfig const&) = default;
  LoggingConfig(LoggingConfig&&) = default;
  auto operator=(LoggingConfig const&) -> LoggingConfig& = default;
  auto operator=(LoggingConfig&&) -> LoggingConfig& = default;

  ~LoggingConfig() = default;

  // ---- Public APIs ----

  auto GetLogLevel() const -> LogLevel;
  auto GetLogLevelAsString() const -> char const*;
  auto SetLogLevel(LogLevel log_level) -> void;

  auto GetSerialLogEnabled() const -> bool;
  auto SetSerialLogEnabled(bool enabled) -> void;

  auto GetWebLogEnabled() const -> bool;
  auto SetWebLogEnabled(bool enabled) -> void;

 private:
  LogLevel log_level_{kDefaultLogLevel};
  bool serial_log_enabled_;
  bool web_log_enabled_;
};

}  // namespace config
}  // namespace owif

#endif  // OWIF_CONFIG_LOGGING_CONFIG_H

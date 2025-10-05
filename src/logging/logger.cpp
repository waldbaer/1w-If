#include "logging/logger.h"

#include <Arduino.h>
#include <ArduinoLog.h>

#include "util/language.h"

namespace owif {
namespace logging {

// ---- Public APIs ----------------------------------------------------------------------------------------------------

auto Logger::Begin(std::uint32_t baud_rate, LogLevel log_level) -> bool {
  Serial.begin(baud_rate);

  Log.setPrefix(Logger::PrintPrefix);  // set logger output prefix
  Log.begin(static_cast<int>(log_level), &Serial);
  Log.setShowLevel(false);  // Do not show loglevel, we will do this in the prefix

  return true;
}

auto Logger::SetLogLevel(LogLevel const log_level) -> void { Log.setLevel(static_cast<int>(log_level)); }

auto Logger::GetLogLevel() -> LogLevel { return static_cast<LogLevel>(Log.getLevel()); }

// ---- Private APIs ---------------------------------------------------------------------------------------------------
auto Logger::PrintPrefix(Print* log_output, int log_level) -> void {
  PrintTimestamp(log_output);
  PrintLogLevel(log_output, log_level);
}

auto Logger::PrintTimestamp(Print* log_output) -> void {
  // Total time
  std::uint32_t const msecs{millis()};
  std::uint32_t const secs{msecs / MSECS_PER_SEC};

  // Time in components
  std::uint32_t const milliseconds{msecs % MSECS_PER_SEC};
  std::uint32_t const seconds{secs % SECS_PER_MIN};
  std::uint32_t const minutes{(secs / SECS_PER_MIN) % SECS_PER_MIN};
  std::uint32_t const hours{(secs % SECS_PER_DAY) / SECS_PER_HOUR};

  // Time as string
  char timestamp[20];
  sprintf(timestamp, "%02d:%02d:%02d.%03d ", hours, minutes, seconds, milliseconds);
  log_output->print(timestamp);
}

auto Logger::PrintLogLevel(Print* log_output, int log_level) -> void {
  /// Show log description based on log level
  switch (log_level) {
    default:
    case 0:
      log_output->print("SILENT ");
      break;
    case 1:
      log_output->print("FATAL ");
      break;
    case 2:
      log_output->print("ERROR ");
      break;
    case 3:
      log_output->print("WARN  ");
      break;
    case 4:
      log_output->print("INFO  ");
      break;
    case 5:
      log_output->print("DEBUG ");
      break;
    case 6:
      log_output->print("VERB  ");
      break;
  }
}

// ---- Global Logger Instance ----
Logger logger_g{};

}  // namespace logging
}  // namespace owif

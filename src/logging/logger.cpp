#include "logging/logger.h"

#include <Arduino.h>
#include <ArduinoLog.h>

#include "time/time_util.h"
#include "util/language.h"

namespace owif {
namespace logging {

// ---- Public APIs ----------------------------------------------------------------------------------------------------

auto Logger::Begin(Print& output_sink, LogLevel log_level) -> bool {
  Log.setPrefix(Logger::PrintPrefix);  // set logger output prefix
  Log.begin(static_cast<int>(log_level), &output_sink);
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
  time::FormattedTimeString formatted_time{};
  time::TimeUtil::Format(time::TimeUtil::TimeSinceStartup(), formatted_time);

  log_output->print(formatted_time);
  log_output->print(' ');
}

auto Logger::PrintLogLevel(Print* log_output, int log_level) -> void {
  /// Show log description based on log level
  switch (log_level) {
    default:
    case 0:
      log_output->print("[S] ");
      break;
    case 1:
      log_output->print("[F] ");
      break;
    case 2:
      log_output->print("[E] ");
      break;
    case 3:
      log_output->print("[W] ");
      break;
    case 4:
      log_output->print("[I] ");
      break;
    case 5:
      log_output->print("[D] ");
      break;
    case 6:
      log_output->print("[V] ");
      break;
  }
}

// ---- Global Logger Instance ----
Logger logger_g{};

}  // namespace logging
}  // namespace owif

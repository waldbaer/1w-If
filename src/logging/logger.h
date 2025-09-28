#ifndef OWIF_LOGGER_LOGGER_H
#define OWIF_LOGGER_LOGGER_H

// ---- Includes ----
#include <ArduinoLog.h>

#include <cstdint>

namespace owif {
namespace logging {

/*!
 * \brief Available I2C commands
 */
enum class LogLevel : std::uint8_t {
  Silent = 0,
  Fatal = 1,
  Error = 2,
  Warning = 3,
  Info = 4,
  Debug = 5,
  Verbose = 6,
};

class Logger {
 public:
  Logger() = default;
  Logger(Logger const&) = default;
  Logger(Logger&&) = default;
  auto operator=(Logger const&) -> Logger& = default;
  auto operator=(Logger&&) -> Logger& = default;

  // ---- Public APIs --------------------------------------------------------------------------------------------------

  auto Begin(std::uint32_t baud_rate, LogLevel log_level = LogLevel::Info) -> bool;

  auto SetLogLevel(LogLevel log_level) -> void;
  auto GetLogLevel() -> LogLevel;

  template <class T, typename... Args>
  inline auto Fatal(T msg, Args... args) -> void {
    Log.fatalln(std::forward<T>(msg), std::forward<Args>(args)...);
  }

  template <class T, typename... Args>
  inline auto FatalNoLn(T msg, Args... args) -> void {
    Log.fatal(std::forward<T>(msg), std::forward<Args>(args)...);
  }

  template <class T, typename... Args>
  inline auto Error(T msg, Args... args) -> void {
    Log.errorln(std::forward<T>(msg), std::forward<Args>(args)...);
  }

  template <class T, typename... Args>
  inline auto ErrorNoLn(T msg, Args... args) -> void {
    Log.error(std::forward<T>(msg), std::forward<Args>(args)...);
  }

  template <class T, typename... Args>
  inline auto Warn(T msg, Args... args) -> void {
    Log.warningln(std::forward<T>(msg), std::forward<Args>(args)...);
  }

  template <class T, typename... Args>
  inline auto WarnNoLn(T msg, Args... args) -> void {
    Log.warning(std::forward<T>(msg), std::forward<Args>(args)...);
  }

  template <class T, typename... Args>
  inline auto Info(T msg, Args... args) -> void {
    Log.infoln(std::forward<T>(msg), std::forward<Args>(args)...);
  }

  template <class T, typename... Args>
  inline auto InfoNoLn(T msg, Args... args) -> void {
    Log.info(std::forward<T>(msg), std::forward<Args>(args)...);
  }

  template <class T, typename... Args>
  inline auto Debug(T msg, Args... args) -> void {
    Log.traceln(std::forward<T>(msg), std::forward<Args>(args)...);
  }

  template <class T, typename... Args>
  inline auto DebugNoLn(T msg, Args... args) -> void {
    Log.trace(std::forward<T>(msg), std::forward<Args>(args)...);
  }

  template <class T, typename... Args>
  inline auto Verbose(T msg, Args... args) -> void {
    Log.verboseln(std::forward<T>(msg), std::forward<Args>(args)...);
  }

  template <class T, typename... Args>
  inline auto VerboseNoLn(T msg, Args... args) -> void {
    Log.verbose(std::forward<T>(msg), std::forward<Args>(args)...);
  }

 private:
  static auto PrintPrefix(Print* log_output, int log_level) -> void;
  static auto PrintTimestamp(Print* log_output) -> void;
  static auto PrintLogLevel(Print* log_output, int log_level) -> void;
};

extern Logger logger_g;

}  // namespace logging
}  // namespace owif

#endif  // OWIF_LOGGER_LOGGER_H

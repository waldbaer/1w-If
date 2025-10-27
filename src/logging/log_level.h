#ifndef OWIF_LOGGING_LOG_LEVEL_H
#define OWIF_LOGGING_LOG_LEVEL_H

// ---- Includes ----
#include <cstdint>

namespace owif {
namespace logging {

/*!
 * \brief Available log levels
 */
enum class LogLevel : std::uint8_t {
  Off = 0,
  Fatal = 1,
  Error = 2,
  Warning = 3,
  Info = 4,
  Debug = 5,
  Verbose = 6,
};

}  // namespace logging
}  // namespace owif

#endif  // OWIF_LOGGING_LOG_LEVEL_H

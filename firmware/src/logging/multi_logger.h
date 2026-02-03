#ifndef OWIF_LOGGER_MULTI_LOGGER_H
#define OWIF_LOGGER_MULTI_LOGGER_H

// ---- Includes ----
#include <Arduino.h>

#include <array>
#include <cstdint>

namespace owif {
namespace logging {

class MultiLogger : public Print {
 public:
  static constexpr std::uint8_t kMaxNumberOfSinks{4};

  MultiLogger() = default;
  MultiLogger(MultiLogger const&) = default;
  MultiLogger(MultiLogger&&) = default;
  auto operator=(MultiLogger const&) -> MultiLogger& = default;
  auto operator=(MultiLogger&&) -> MultiLogger& = default;
  ~MultiLogger() = default;

  // ---- Public APIs --------------------------------------------------------------------------------------------------
  auto RegisterLogSink(Print& sink) -> bool;

  // ---- Public APIs: Print Interface ---------------------------------------------------------------------------------

  auto write(std::uint8_t character) -> std::size_t final;
  auto write(std::uint8_t const* buffer, std::size_t size) -> std::size_t final;

 private:
  std::array<Print*, kMaxNumberOfSinks> sinks_{};
  std::uint8_t registered_sinks;
};

/*!
 * \brief Declaration of global MultiLogger instance.
 */
extern MultiLogger multi_logger_g;

}  // namespace logging
}  // namespace owif

#endif  // OWIF_LOGGER_MULTI_LOGGER_H

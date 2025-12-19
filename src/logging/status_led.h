#ifndef OWIF_LOGGING_STATUS_LED_H
#define OWIF_LOGGING_STATUS_LED_H

// ---- Includes ----
#include <Arduino.h>

#include <cstdint>

namespace owif {
namespace logging {

class StatusLed {
 public:
  StatusLed() = default;

  StatusLed(StatusLed const&) = default;
  StatusLed(StatusLed&&) = default;
  auto operator=(StatusLed const&) -> StatusLed& = default;
  auto operator=(StatusLed&&) -> StatusLed& = default;
  ~StatusLed() = default;

  // ---- Public APIs --------------------------------------------------------------------------------------------------
  auto Begin(std::uint8_t pin) -> bool;

  auto On() -> void;
  auto Off() -> void;
  auto Set(bool on_off) -> void;
  auto Get() -> bool;

  auto Toggle() -> void;
  auto Flash(std::uint32_t iterations, std::uint32_t duration_on = 70, std::uint32_t duration_off = 70) -> void;

 private:
  std::uint8_t pin_{0};
};

/*!
 * \brief Declaration of global StatusLed instance.
 */
extern StatusLed status_led_g;

}  // namespace logging
}  // namespace owif

#endif  // OWIF_LOGGING_STATUS_LED_H

#include "logging/status_led.h"

#include <Arduino.h>

namespace owif {
namespace logging {

// ---- Public APIs ----------------------------------------------------------------------------------------------------

auto StatusLed::Begin(std::uint8_t pin) -> bool {
  bool const result{pin_ == 0};

  pin_ = pin;

  pinMode(pin_, OUTPUT);
  Off();  // Turn off initially
  return result;
}

auto StatusLed::On() -> void {
  digitalWrite(pin_, LOW);  // active low
}

auto StatusLed::Off() -> void {
  digitalWrite(pin_, HIGH);  // active low
}

auto StatusLed::Set(bool on_off) -> void {
  digitalWrite(pin_, static_cast<std::uint8_t>(not on_off));  // active low
}

auto StatusLed::Flash(std::uint32_t iterations, std::uint32_t duration_on, std::uint32_t duration_off) -> void {
  Off();

  while (iterations > 0) {
    iterations--;

    delay(duration_off);
    On();
    delay(duration_on);
    Off();
  }
}

// ---- Global StatusLed Instance ----
StatusLed status_led_g{};

}  // namespace logging
}  // namespace owif

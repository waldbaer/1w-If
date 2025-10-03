
// ---- Includes ----
#include "cmd/timer.h"

#include <Arduino.h>

#include <cstdint>

namespace owif {
namespace cmd {

Timer::Timer() : Timer(0) {}

Timer::Timer(std::uint32_t delay) : delay_{delay} { Reset(); }

// ---- Public APIs --------------------------------------------------------------------------------------------------

auto Timer::Reset() -> void { Reset(delay_); }

auto Timer::Reset(std::uint32_t delay) -> void {
  delay_ = delay;
  if (delay_ > 0) {
    minimum_abs_execution_time_ = millis() + delay_;
  } else {
    minimum_abs_execution_time_ = 0;
  }
}

auto Timer::IsExpired() const -> bool {
  return (minimum_abs_execution_time_ == 0) || (minimum_abs_execution_time_ <= millis());
}

auto Timer::GetDelay() const -> std::uint32_t { return delay_; }

}  // namespace cmd
}  // namespace owif

#ifndef OWIF_CMD_TIMER_H
#define OWIF_CMD_TIMER_H

// ---- Includes ----

#include <Arduino.h>

#include <cstdint>

namespace owif {
namespace cmd {

class Timer {
 public:
  Timer();
  explicit Timer(std::uint32_t delay);

  Timer(Timer const&) = default;
  auto operator=(Timer const&) -> Timer& = default;
  Timer(Timer&&) = default;
  auto operator=(Timer&&) -> Timer& = default;

  // ---- Public APIs --------------------------------------------------------------------------------------------------

  auto Reset() -> void;
  auto Reset(std::uint32_t delay) -> void;
  auto IsExpired() const -> bool;

  auto GetDelay() const -> std::uint32_t;

 private:
  std::uint32_t delay_{0};
  std::uint32_t minimum_abs_execution_time_{0};
};

}  // namespace cmd
}  // namespace owif

#endif  // OWIF_CMD_TIMER_H

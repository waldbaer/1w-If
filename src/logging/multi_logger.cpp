#include "logging/multi_logger.h"

#include <Arduino.h>

namespace owif {
namespace logging {

auto MultiLogger::RegisterLogSink(Print& sink) -> bool {
  bool result{true};
  if (registered_sinks < kMaxNumberOfSinks - 1) {
    sinks_[registered_sinks] = &sink;
    registered_sinks++;
  } else {
    result = false;
  }
  return result;
}

auto MultiLogger::write(std::uint8_t character) -> std::size_t {
  for (std::uint8_t sink_index{0}; sink_index < registered_sinks; sink_index++) {
    sinks_[sink_index]->write(character);
  }
  return 1;
}

auto MultiLogger::write(std::uint8_t const* buffer, std::size_t size) -> std::size_t {
  for (std::uint8_t sink_index{0}; sink_index < registered_sinks; sink_index++) {
    sinks_[sink_index]->write(buffer, size);
  }
  return size;
}

// ---- Global MultiLogger Instance ----
MultiLogger multi_logger_g{};

}  // namespace logging
}  // namespace owif

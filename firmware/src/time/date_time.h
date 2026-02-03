#ifndef OWIF_TIME_DATE_TIME_H
#define OWIF_TIME_DATE_TIME_H

// ---- Includes ----
#include <Arduino.h>

#include <cstdint>

namespace owif {
namespace time {

struct DateTime {
  std::uint16_t year;
  std::uint8_t month;
  std::uint8_t day;
  std::uint8_t hour;
  std::uint8_t minute;
  std::uint8_t second;
  std::uint16_t millisecond;
};

}  // namespace time
}  // namespace owif

#endif  // OWIF_TIME_DATE_TIME_H

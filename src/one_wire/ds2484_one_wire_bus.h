#ifndef OWIF_ONE_WIRE_DS2484_ONE_WIRE_BUS_H
#define OWIF_ONE_WIRE_DS2484_ONE_WIRE_BUS_H

#include <cstdint>

#include "i2c/ds2484_device.h"
#include "logging/logger.h"
#include "one_wire/one_wire_address.h"
#include "one_wire/one_wire_bus.h"
#include "util/language.h"

namespace owif {
namespace one_wire {

/*!
 * \brief Representation of the DS2484 One-Wire Busmaster chip as I2C
 */
class Ds2484OneWireBus : public OneWireBus {
 public:
  // Inherit I2CDevice constructor
  Ds2484OneWireBus(OneWireBus::BusId bus_id, i2c::Ds2484Device& bus_master);

  // OneWireBus Interface
  auto Read8(std::uint8_t& value) -> bool override;
  auto Read64(std::uint64_t& value) -> bool override;
  auto Write8(std::uint8_t value) -> bool override;
  auto Write64(std::uint64_t value) -> bool override;

 protected:
  // OneWireBus Interface
  auto ResetBus() -> bool override;
  auto ResetSearch(std::uint64_t start_address, std::uint8_t last_discrepancy) -> void override;
  auto SearchNextDevice() -> std::uint64_t override;

 private:
  logging::Logger& logger_{logging::logger_g};

  std::uint64_t address_;
  std::uint8_t last_discrepancy_{0};
  bool last_device_flag_{false};

  i2c::Ds2484Device& bus_master_;
};

}  // namespace one_wire
}  // namespace owif

#endif  // OWIF_ONE_WIRE_DS2484_ONE_WIRE_BUS_H

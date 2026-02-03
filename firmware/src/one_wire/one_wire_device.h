#ifndef OWIF_ONE_WIRE_ONE_WIRE_DEVICE_H
#define OWIF_ONE_WIRE_ONE_WIRE_DEVICE_H

// ---- Includes ----
#include <cstdint>

#include "logging/logger.h"
#include "one_wire/one_wire_address.h"
#include "one_wire/one_wire_bus.h"

namespace owif {
namespace one_wire {

class OneWireDevice {
 public:
  OneWireDevice(OneWireBus& bus, OneWireAddress const& address);

  OneWireDevice(OneWireDevice const&) = default;
  auto operator=(OneWireDevice const&) -> OneWireDevice& = default;
  OneWireDevice(OneWireDevice&&) = default;
  auto operator=(OneWireDevice&&) -> OneWireDevice& = default;

  virtual ~OneWireDevice() = default;

  // ---- Public APIs ----------
  virtual auto Begin() -> bool = 0;

  auto GetAddress() const -> OneWireAddress;
  auto GetFamilyCode() const -> OneWireAddress::FamilyCode;
  auto GetBusId() const -> OneWireBus::BusId;

 protected:
  auto CheckAddress() -> bool;

  /*!
   * \brief Send command to the 1-wire bus
   * \param[in] cmd Command byte to be sent
   * \param[in] skip_rom_select If set command is sent to all 1-wire devices using the 'Skip ROM' command. Otherwise the
   *                            specific slave is addressed with 'Match ROM' command.
   */
  auto SendCommand(std::uint8_t cmd, bool skip_rom_select = false) -> bool;

  logging::Logger& logger_{logging::logger_g};

  OneWireBus& bus_;

  OneWireAddress address_;
};

}  // namespace one_wire
}  // namespace owif

#endif  // OWIF_ONE_WIRE_ONE_WIRE_DEVICE_H

#ifndef OWIF_ONE_WIRE_ONE_WIRE_BUS_H
#define OWIF_ONE_WIRE_ONE_WIRE_BUS_H

// ---- Includes ----
#include <cstdint>
#include <vector>

#include "logging/logger.h"
#include "one_wire/one_wire_address.h"

namespace owif {
namespace one_wire {

class OneWireBus {
 public:
  using BusId = std::uint8_t;

  OneWireBus(BusId bus_id);
  virtual ~OneWireBus() = default;

  auto GetId() const -> BusId;

  // Search for 1-Wire devices on the bus.
  auto Search() -> bool;
  auto Search(OneWireAddress::FamilyCode family_code) -> bool;
  auto Search(OneWireAddress address, bool& is_present) -> bool;

  // Return the list of found devices.
  auto GetDevices() -> std::vector<OneWireAddress> const&;

  // Select a specific address on the bus for the following command.
  auto Select(OneWireAddress address) -> bool;

  // Write a command to the bus that addresses all devices by skipping the ROM.
  auto Skip() -> bool;

  virtual auto Read8(std::uint8_t& value) -> bool = 0;
  virtual auto Read64(std::uint64_t& value) -> bool = 0;
  virtual auto Write8(std::uint8_t value) -> bool = 0;
  virtual auto Write64(std::uint64_t value) -> bool = 0;

 protected:
  /**
   * Bus Reset
   * @return -1: signal fail, 0: no device detected, 1: device detected
   */
  virtual auto ResetBus() -> bool = 0;

  // Reset the device search.
  virtual auto ResetSearch(std::uint64_t start_address = 0, std::uint8_t last_discrepancy = 0) -> void = 0;

  // Search for a 1-Wire device on the bus. Returns 0 if all devices have been found.
  virtual auto SearchNextDevice() -> std::uint64_t = 0;

  BusId bus_id_;
  std::vector<OneWireAddress> devices_{};

 private:
  // Common 1-wire commands
  enum class Commands : std::uint8_t {
    SearchRom = 0xF0,
    // This command can only be used when there is one slave on the bus. It allows the bus master to read the slave’s
    // 64-bit ROM code without using the Search ROM procedure.
    ReadRom = 0x33,
    // The match ROM command followed by a 64-bit ROM code sequence allows the bus master
    // to address a specific slave device
    MatchRom = 0x55,
    // The master can use this command to address all devices on the bus simultaneously
    // without sending out any ROM code information.
    SkipRom = 0xCC
  };

  logging::Logger& logger_{logging::logger_g};
};

}  // namespace one_wire
}  // namespace owif

#endif  // OWIF_ONE_WIRE_ONE_WIRE_BUS_H

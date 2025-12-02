#ifndef OWIF_ONE_WIRE_ONE_WIRE_SUBSYSTEM_H
#define OWIF_ONE_WIRE_ONE_WIRE_SUBSYSTEM_H

// ---- Includes ----
#include <array>
#include <map>
#include <memory>
#include <vector>

#include "i2c/arduino_i2c_bus.h"
#include "i2c/ds2484_device.h"
#include "i2c/tca9548a_i2c_bus.h"
#include "logging/logger.h"
#include "one_wire/ds2484_one_wire_bus.h"
#include "one_wire/one_wire_address.h"
#include "one_wire/one_wire_device.h"

namespace owif {
namespace one_wire {

class OneWireSystem {
 public:
  using DeviceMap = std::map<OneWireAddress, std::shared_ptr<OneWireDevice>>;
  using DeviceAttributesList = std::vector<String>;

  OneWireSystem();

  // ---- Public APIs --------------------------------------------------------------------------------------------------

  auto Begin(bool run_initial_bus_scan = false) -> bool;
  auto Loop() -> void;

  auto Scan() -> bool;
  auto Scan(OneWireAddress const& address, bool& is_present, OneWireBus::BusId& bus_id) -> bool;
  auto Scan(OneWireAddress::FamilyCode family_code) -> bool;

  auto GetAvailableDevice(OneWireAddress const& address) -> std::shared_ptr<OneWireDevice>;
  auto GetAvailableDevices() -> DeviceMap&;
  auto GetAvailableDevices(OneWireAddress::FamilyCode family_code) -> DeviceMap;

  auto GetAvailableBuses() -> std::vector<std::reference_wrapper<OneWireBus>>;

  auto GetAttributes(OneWireAddress const& ow_address) -> DeviceAttributesList;

 private:
  // I2C constants
  static constexpr std::uint8_t kI2cBus{1};      // Use second I2C bus. First is used for ethernet PHY.
  static constexpr std::uint8_t kI2cSdaPin{4};   // SDA2, GPIO4
  static constexpr std::uint8_t kI2cSclPin{14};  // SCL2, GPIO14
  static constexpr std::uint32_t kI2cFrequency{400000};
  static constexpr std::uint32_t kI2cTimeout{0};
  static constexpr std::uint8_t kI2cMultiplexerResetPin{15};  // GPIO15 -> TCA TCA9548A 'RESET pin.

  static constexpr std::uint8_t kMuxedI2cBuses{2};  // Number of muxed I2C buses.

  static constexpr char const* kAttributePresence{"presence"};
  static constexpr char const* kAttributeTemperature{"temperature"};
  static constexpr char const* kAttributeVAD{"VAD"};
  static constexpr char const* kAttributeVDD{"VDD"};

  struct OwAddrBus {
    OneWireAddress addr;
    OneWireBus* bus;
  };

  auto CreateDevice(OneWireBus& bus, OneWireAddress const& address) -> std::shared_ptr<OneWireDevice>;

  logging::Logger& logger_{logging::logger_g};

  TwoWire i2c_twowire_{kI2cBus};
  i2c::ArduinoI2CBus i2c_bus_{i2c_twowire_, kI2cSdaPin, kI2cSclPin, kI2cFrequency, kI2cTimeout};
  i2c::Tca9548aDevice i2c_multiplexer_{i2c_bus_, i2c::Tca9548aDevice::kDefaultI2CAddress};

  std::array<i2c::Tca9548aI2CBus, kMuxedI2cBuses> i2c_muxed_buses_;
  std::array<i2c::Ds2484Device, kMuxedI2cBuses> ow_bus_masters_;
  std::array<one_wire::Ds2484OneWireBus, kMuxedI2cBuses> ow_buses_;

  DeviceMap ow_available_devices_{};
};

/*!
 * \brief Declaration of global OneWireSystem instance.
 */
extern OneWireSystem one_wire_system_g;

}  // namespace one_wire
}  // namespace owif

#endif  // OWIF_ONE_WIRE_ONE_WIRE_SUBSYSTEM_H

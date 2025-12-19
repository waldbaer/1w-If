#ifndef OWIF_ONE_WIRE_ONE_WIRE_SUBSYSTEM_H
#define OWIF_ONE_WIRE_ONE_WIRE_SUBSYSTEM_H

// ---- Includes ----
#include <map>
#include <memory>
#include <vector>

#include "config/onewire_config.h"
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

  auto Begin(config::OneWireConfig const& config) -> bool;
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

  static constexpr std::uint8_t kOneWireChannels{
      config::OneWireConfig::kOneWireChannels};  // Number of muxed I2C buses.

  // GPIO pins to control DS2484 SLPZ: active-low to enable low-power sleep mode
  static constexpr std::array<std::uint8_t, kOneWireChannels> kDs2484SlpzPins{
      32,  // Channe1: GPIO32
      33,  // Channe2: GPIO33
      5,   // Channe3: GPIO5
      17   // Channe4: GPIO17
  };

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

  config::OneWireConfig config_{};

  TwoWire i2c_twowire_{kI2cBus};
  i2c::ArduinoI2CBus i2c_bus_{i2c_twowire_, kI2cSdaPin, kI2cSclPin, kI2cFrequency, kI2cTimeout};
  i2c::Tca9548aDevice i2c_multiplexer_{i2c_bus_, i2c::Tca9548aDevice::kDefaultI2CAddress};

  std::vector<i2c::Tca9548aI2CBus> i2c_muxed_buses_{};
  std::vector<i2c::Ds2484Device> ow_bus_masters_;
  std::vector<one_wire::Ds2484OneWireBus> ow_buses_;

  DeviceMap ow_available_devices_{};
};

/*!
 * \brief Declaration of global OneWireSystem instance.
 */
extern OneWireSystem one_wire_system_g;

}  // namespace one_wire
}  // namespace owif

#endif  // OWIF_ONE_WIRE_ONE_WIRE_SUBSYSTEM_H

#include "one_wire/one_wire_subsystem.h"

#include <Arduino.h>

#include <algorithm>
#include <vector>

#include "i2c/arduino_i2c_bus.h"
#include "i2c/ds2484_device.h"
#include "i2c/tca9548a_device.h"
#include "i2c/tca9548a_i2c_bus.h"
#include "one_wire/ds18b20.h"
#include "one_wire/ds2411.h"
#include "one_wire/ds2438.h"
#include "one_wire/ds2484_one_wire_bus.h"
#include "one_wire/one_wire_device.h"
#include "util/language.h"

namespace owif {
namespace one_wire {

OneWireSystem::OneWireSystem() {
  i2c_muxed_buses_.reserve(kOneWireChannels);
  ow_bus_masters_.reserve(kOneWireChannels);
  ow_buses_.reserve(kOneWireChannels);
}

// ---- Public APIs ----------------------------------------------------------------------------------------------------

auto OneWireSystem::Begin(config::OneWireConfig const& config) -> bool {
  bool result{true};

  config_ = config;

  logger_.Debug(F("[OneWireSystem] Setup I2C bus & multiplexer..."));
  i2c_bus_.Begin();
  i2c_multiplexer_.Begin();

  logger_.Debug(F("[OneWireSystem] Setup 1-wire bus channels..."));

  for (std::uint8_t channel_id_zero_based{0}; channel_id_zero_based < kOneWireChannels; channel_id_zero_based++) {
    config::OneWireChannelConfig const& ow_channel_config{config.GetChannelConfig(channel_id_zero_based)};

    // zero-based bus IDs are equal for I2C and One-Wire
    i2c::Tca9548aI2CBus::Id const i2c_muxed_bus_id{static_cast<i2c::Tca9548aI2CBus::Id>(channel_id_zero_based)};

    // BusId is the final user-friendly channel number (one based)
    OneWireBus::BusId const channel_id{static_cast<OneWireBus::BusId>(channel_id_zero_based + 1)};

    // ---- Enable 1-Wire bus master DS2484 via SLPZ pin (active-low low power sleep) ----
    std::uint8_t const ds2484_slpz_pin{kDs2484SlpzPins[channel_id_zero_based]};
    pinMode(ds2484_slpz_pin, OUTPUT);
    digitalWrite(ds2484_slpz_pin, ow_channel_config.GetEnabled() ? HIGH : LOW);  // active-low sleep

    if (ow_channel_config.GetEnabled()) {
      // ---- Initialize muxed I2C bus (via TCA9548A) ----
      i2c_muxed_buses_.emplace_back(i2c_multiplexer_, i2c_muxed_bus_id);

      // ---- Initialize 1-Wire bus master DS2484 (accessed via muxed I2C bus) ----
      ow_bus_masters_.emplace_back(/* i2c_bus: */ i2c_muxed_buses_.back(),
                                   // 1-wire bus timing parameters
                                   /* active_pullup: */ true,
                                   /* strong pullup: */ false);

      ow_buses_.emplace_back(channel_id, /*bus_master=*/ow_bus_masters_.back());

      // ---- Setup DS2484 bus-masters ----
      ow_bus_masters_.back().Begin();
    }
  }

  // ---- Initial bus scan ----
  if (config_.GetRunInitialScan()) {
    std::uint32_t const start_time{millis()};
    Scan();
    std::uint32_t const scan_time{millis() - start_time};
    logger_.Info(F("[OneWireSystem] Initial 1-wire bus scan: found %u devices in %u ms"), ow_available_devices_.size(),
                 scan_time);
    for (DeviceMap::value_type const& ow_device : ow_available_devices_) {
      logger_.Info(F("[OneWireSystem]   1-wire device: %s | channel: %d"),
                   ow_device.second->GetAddress().Format().c_str(), ow_device.second->GetBusId());
    }
  }

  return result;
}

auto OneWireSystem::Loop() -> void {  // Nothing to be done
}

auto OneWireSystem::Scan() -> bool {
  bool result{true};

  // ---- Search available devices on all 1-wire buses ----
  std::vector<OwAddrBus> available_addresses{0};
  available_addresses.reserve(ow_available_devices_.size());

  for (one_wire::Ds2484OneWireBus& ow_bus : ow_buses_) {
    result &= ow_bus.Search();

    if (result) {
      std::vector<OneWireAddress> const& bus_available_addresses{ow_bus.GetDevices()};
      for (OneWireAddress const& addr : bus_available_addresses) {
        OneWireBus* b{&ow_bus};
        available_addresses.push_back(OwAddrBus{addr, b});
      }
    }
  }

  // ---- Destroy / Instantiate devices ----

  // Remove missing devices
  for (DeviceMap::iterator available_device{ow_available_devices_.begin()};
       available_device != ow_available_devices_.end();) {
    if (std::find_if(available_addresses.begin(), available_addresses.end(),
                     [&available_device](OwAddrBus& available_addr) {
                       return available_addr.addr == available_device->first;
                     }) == available_addresses.end()) {
      available_device = ow_available_devices_.erase(available_device);
    } else {
      ++available_device;
    }
  }

  // Add new devices
  for (OwAddrBus const& addr_bus : available_addresses) {
    if (ow_available_devices_.find(addr_bus.addr) == ow_available_devices_.end()) {
      ow_available_devices_[addr_bus.addr] = CreateDevice(*(addr_bus.bus), addr_bus.addr);
    }
  }

  if (!result) {
    logger_.Error("[OneWireSystem] 1-wire bus search failed.");
  }

  return result;
}

auto OneWireSystem::Scan(OneWireAddress const& address, bool& is_present, OneWireBus::BusId& bus_id) -> bool {
  bool result{true};

  is_present = false;

  for (one_wire::Ds2484OneWireBus& ow_bus : ow_buses_) {
    result &= ow_bus.Search(address, is_present);

    if (is_present) {
      // Add device to list of known devices
      ow_available_devices_[address] = CreateDevice(ow_bus, address);
      bus_id = ow_bus.GetId();
      break;
    } else {
      // Remove device from list of known devices
      for (DeviceMap::iterator available_device{ow_available_devices_.begin()};
           available_device != ow_available_devices_.end();) {
        if (available_device->first == address) {
          available_device = ow_available_devices_.erase(available_device);
        } else {
          available_device++;
        }
      }
    }
  }

  return result;
}

auto OneWireSystem::Scan(OneWireAddress::FamilyCode const family_code) -> bool {
  bool result{true};

  // ---- Search available devices on all 1-wire buses ----
  std::vector<OwAddrBus> available_addresses{0};
  available_addresses.reserve(ow_available_devices_.size());

  for (one_wire::Ds2484OneWireBus& ow_bus : ow_buses_) {
    result &= ow_bus.Search(family_code);

    if (result) {
      std::vector<OneWireAddress> const& bus_available_addresses{ow_bus.GetDevices()};
      for (OneWireAddress const& addr : bus_available_addresses) {
        available_addresses.push_back(OwAddrBus{addr, &ow_bus});
      }
    }
  }

  // ---- Destroy / Instantiate devices ----

  // Remove missing devices
  for (DeviceMap::iterator available_device{ow_available_devices_.begin()};
       available_device != ow_available_devices_.end();) {
    if (
        // only cleanup device of the searched family
        (available_device->first.GetFamilyCode() == family_code) &&
        // known device missing on the bus?
        (std::find_if(available_addresses.begin(), available_addresses.end(),
                      [&available_device](OwAddrBus& available_addr) {
                        return available_addr.addr == available_device->first;
                      }) == available_addresses.end())) {
      available_device = ow_available_devices_.erase(available_device);
    } else {
      available_device++;
    }
  }

  // Add new devices
  for (OwAddrBus const& addr_bus : available_addresses) {
    if (ow_available_devices_.find(addr_bus.addr) == ow_available_devices_.end()) {
      ow_available_devices_[addr_bus.addr] = CreateDevice(*(addr_bus.bus), addr_bus.addr);
    }
  }

  if (!result) {
    logger_.Error("[OneWireSystem] 1-wire bus search failed.");
  }

  return result;
}

auto OneWireSystem::GetAvailableDevices() -> DeviceMap& { return ow_available_devices_; }

auto OneWireSystem::GetAvailableDevices(OneWireAddress::FamilyCode family_code) -> DeviceMap {
  DeviceMap result{};

  for (DeviceMap::value_type const& available_device : ow_available_devices_) {
    if (available_device.first.GetFamilyCode() == family_code) {
      result[available_device.first] = available_device.second;
    }
  }

  return result;
}

auto OneWireSystem::GetAvailableDevice(OneWireAddress const& address) -> std::shared_ptr<OneWireDevice> {
  std::shared_ptr<OneWireDevice> result{};
  DeviceMap::iterator const found_device{ow_available_devices_.find(address)};
  if (found_device != ow_available_devices_.end()) {
    result = found_device->second;
  }
  return result;
}

auto OneWireSystem::GetAttributes(OneWireAddress const& ow_address) -> DeviceAttributesList {
  DeviceAttributesList result{};

  switch (ow_address.GetFamilyCode()) {
    case Ds2411::kFamilyCode:
      result.push_back(kAttributePresence);
      break;
    case Ds18b20::kFamilyCode:
      result.push_back(kAttributePresence);
      result.push_back(kAttributeTemperature);
      break;
    case Ds2438::kFamilyCode:
      result.push_back(kAttributePresence);
      result.push_back(kAttributeTemperature);
      result.push_back(kAttributeVAD);
      result.push_back(kAttributeVDD);
      break;
    default:
      // Unknown device. Don't add more attributes.
      break;
  }

  return result;
}

auto OneWireSystem::GetAvailableBuses() -> std::vector<std::reference_wrapper<OneWireBus>> {
  std::vector<std::reference_wrapper<OneWireBus>> result;
  result.reserve(ow_buses_.size());

  for (one_wire::Ds2484OneWireBus& ow_bus : ow_buses_) {
    result.push_back(ow_bus);
  }
  return result;
}

// ---- Private APIs ---------------------------------------------------------------------------------------------------

auto OneWireSystem::CreateDevice(OneWireBus& bus, OneWireAddress const& address) -> std::shared_ptr<OneWireDevice> {
  std::shared_ptr<OneWireDevice> result{nullptr};

  switch (address.GetFamilyCode()) {
    case Ds18b20::kFamilyCode:
      result = std::make_shared<Ds18b20>(bus, address, Ds18b20::Resolution::Res12Bit);
      break;
    case Ds2438::kFamilyCode:
      result = std::make_shared<Ds2438>(bus, address);
      break;
    case Ds2411::kFamilyCode:
    default:
      result = std::make_shared<Ds2411>(bus, address);
      break;
  }

  if (result) {
    bool setup_result{result->Begin()};
    if (!setup_result) {
      result = nullptr;
      logger_.Error(F("[OneWireSystem] Failed to setup 1-wire device with address '%s'"), address.Format().c_str());
    }
  }

  return result;
}

/*!
 * \brief Declaration of kDs2484SlpzPins constexpr
 */
constexpr std::array<std::uint8_t, OneWireSystem::kOneWireChannels> OneWireSystem::kDs2484SlpzPins;

// ---- Global WebServer Instance ----

/*!
 * \brief Definition of global OneWireSystem instance.
 */
OneWireSystem one_wire_system_g{};

}  // namespace one_wire
}  // namespace owif

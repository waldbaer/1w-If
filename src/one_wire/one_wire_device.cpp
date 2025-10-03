#include "one_wire/one_wire_device.h"

#include <Arduino.h>

#include <algorithm>

#include "util/crc.h"
#include "util/language.h"

namespace owif {
namespace one_wire {

OneWireDevice::OneWireDevice(OneWireBus& bus, OneWireAddress const& address) : bus_{bus}, address_{address} {}

// ---- Public APIS -------------------------------------------------------------------------------------------------

auto OneWireDevice::GetAddress() const -> OneWireAddress { return address_; }

auto OneWireDevice::GetFamilyCode() const -> OneWireAddress::FamilyCode { return address_.GetFamilyCode(); }

// ---- Protected APIS -------------------------------------------------------------------------------------------------

auto OneWireDevice::CheckAddress() -> bool {
  if (address_ != 0) {
    return true;
  }

  std::vector<OneWireAddress> const& devices{bus_.GetDevices()};
  std::vector<OneWireAddress>::const_iterator found_in_devices{std::find(devices.cbegin(), devices.cend(), address_)};
  if (found_in_devices == devices.end()) {
    logger_.Error(F("[OneWireDevice] 1-Wire device with address '%s' not found on bus"), address_.Format().c_str());
    return false;
  }

  return true;
}

auto OneWireDevice::SendCommand(std::uint8_t cmd, bool skip_rom_select) -> bool {
  bool result{true};
  if (skip_rom_select) {
    result &= bus_.Skip();
  } else {
    result &= bus_.Select(address_);
  }

  if (result) {
    result &= bus_.Write8(cmd);
  } else {
    logger_.Error(F("[OneWireDevice] SendCommand error"));
  }
  return result;
}

}  // namespace one_wire
}  // namespace owif

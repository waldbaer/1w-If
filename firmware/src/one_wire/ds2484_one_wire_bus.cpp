#include "one_wire/ds2484_one_wire_bus.h"

#include <Arduino.h>

namespace owif {
namespace one_wire {

// ---- Public APIs ----------------------------------------------------------------------------------------------------
Ds2484OneWireBus::Ds2484OneWireBus(OneWireBus::BusId bus_id, i2c::Ds2484Device& bus_master)
    : OneWireBus{bus_id}, bus_master_{bus_master} {}

auto Ds2484OneWireBus::Read8(std::uint8_t& value) -> bool { return bus_master_.Read8(value); }

auto Ds2484OneWireBus::Read64(std::uint64_t& value) -> bool { return bus_master_.Read64(value); }

auto Ds2484OneWireBus::Write8(std::uint8_t value) -> bool { return bus_master_.Write8(value); }

auto Ds2484OneWireBus::Write64(std::uint64_t value) -> bool { return bus_master_.Write64(value); }

// ---- Protected APIs -------------------------------------------------------------------------------------------------
auto Ds2484OneWireBus::ResetBus() -> bool { return bus_master_.ResetOneWire(); }

auto Ds2484OneWireBus::ResetSearch(std::uint64_t start_address, std::uint8_t last_discrepancy) -> void {
  address_ = start_address;
  last_discrepancy_ = last_discrepancy;
  last_device_flag_ = false;
}

/*!
 * based on https://www.analog.com/en/resources/app-notes/1wire-search-algorithm.html
 * mappings:
 * - id_bit_number -> bit_mask
 */
auto Ds2484OneWireBus::SearchNextDevice() -> std::uint64_t {
  if (last_device_flag_) {
    return 0;
  }

  std::uint64_t bit_mask{1};
  std::uint8_t last_zero{0};
  std::uint64_t address{address_};

  // Initiate search
  for (std::uint8_t bit_number{1}; bit_number <= 64; bit_number++, bit_mask <<= 1) {
    bool branch;

    // compute branch value for the case when there is a discrepancy
    // (there are devices with both 0s and 1s at this bit)
    if (bit_number < last_discrepancy_) {
      branch = (address & bit_mask) > 0;
    } else {
      branch = bit_number == last_discrepancy_;
    }

    bool id_bit;
    bool cmp_id_bit;
    bool branch_before{branch};
    if (!bus_master_.OneWireTriple(&branch, &id_bit, &cmp_id_bit)) {
      logger_.Warn(F("[DS484] OneWireTriple error. Quitting. bit_number:%d"), bit_number);
      return 0;
    }

    if (id_bit && cmp_id_bit) {
      logger_.Warn(F("[DS2484] no devices on the bus. Quitting. bit_number=%d"), bit_number);
      // No devices participating in search
      return 0;
    }

    if (!id_bit && !cmp_id_bit && !branch) {
      last_zero = bit_number;
    }

    // logger_.Verbose("[DS2484]: %d %d branch: %d %d\n", id_bit, cmp_id_bit, branch_before, branch);

    if (branch) {
      address |= bit_mask;
    } else {
      address &= ~bit_mask;
    }
  }

  last_discrepancy_ = last_zero;
  // logger_.Verbose("[DS2484]: last_discrepancy: %d  address: %llx\n", last_discrepancy_, address);

  if (last_discrepancy_ == 0) {
    // we're at root and have no choices left, so this was the last one.
    last_device_flag_ = true;
  }

  address_ = address;
  return address;
}

}  // namespace one_wire
}  // namespace owif

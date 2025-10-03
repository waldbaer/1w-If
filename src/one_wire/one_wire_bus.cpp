#include "one_wire/one_wire_bus.h"

#include <Arduino.h>

#include "one_wire/one_wire_address.h"
#include "util/crc.h"
#include "util/language.h"

namespace owif {
namespace one_wire {

OneWireBus::OneWireBus(BusId bus_id) : bus_id_{bus_id} {};

auto OneWireBus::GetId() const -> BusId { return bus_id_; }

auto OneWireBus::Search() -> bool {
  bool result{true};

  devices_.clear();

  ResetSearch(0, 0);
  std::uint64_t address{0};

  while (result) {
    result &= ResetBus();
    if (result) {
      result &= Write8(ToUnderlying(Commands::SearchRom));
      if (result) {
        address = SearchNextDevice();
        if (address == 0) {
          break;
        }
        std::uint8_t *address8{reinterpret_cast<std::uint8_t *>(&address)};
        if (util::crc8(address8, 7) != address8[7]) {
          logger_.Warn(F("[OneWireBus] Bus device has invalid CRC"));
        } else {
          devices_.emplace_back(address);
        }
      } else {
        logger_.Error(F("[OneWireBus] Search: Write8 failed"));
      }
    } else {
      logger_.Error(F("[OneWireBus] Search: Bus reset failed"));
    }
  }

  return result;
}

auto OneWireBus::Search(OneWireAddress::FamilyCode family_code) -> bool {
  bool result{true};

  devices_.clear();

  ResetSearch(/* address */ family_code, /* last discrepancy */ 64);
  std::uint64_t address{0};
  while (result) {
    result &= ResetBus();

    if (result) {
      result &= Write8(ToUnderlying(Commands::SearchRom));
      if (result) {
        address = SearchNextDevice();
        if (address == 0) {
          break;
        }

        if ((address & 0xFF) != family_code) {
          break;
        }

        // Check CRC
        std::uint8_t *address8{reinterpret_cast<std::uint8_t *>(&address)};
        if (util::crc8(address8, 7) != address8[7]) {
          logger_.Warn(F("[OneWireBus] Bus device has invalid CRC"));
        } else {
          devices_.emplace_back(address);
        }
      }
    }
  }

  return result;
}

auto OneWireBus::Search(OneWireAddress const address, bool &is_present) -> bool {
  is_present = false;

  ResetSearch(/* address */ address.GetFullAddress(), /* last discrepancy */ 64);
  bool result{ResetBus()};

  if (result) {
    result &= Write8(ToUnderlying(Commands::SearchRom));

    if (result) {
      std::uint64_t const found_address{SearchNextDevice()};
      is_present = (address == found_address);
    } else {
      logger_.Error(F("[OneWireBus] Failed to write8 in Search(addr) addr: %s"), address.Format().c_str());
    }
  }

  if (!result) {
    logger_.Warn(F("[OneWireBus] Single address search failed for address '%s'"), address.Format().c_str());
  }

  return result;
}

auto OneWireBus::GetDevices() -> std::vector<OneWireAddress> const & { return devices_; }

auto IRAM_ATTR OneWireBus::Select(OneWireAddress address) -> bool {
  bool result{ResetBus()};
  if (result) {
    result &= Write8(ToUnderlying(Commands::MatchRom));
    result &= Write64(address.GetFullAddress());
  }
  return result;
}

auto OneWireBus::Skip() -> bool {
  bool result{ResetBus()};
  if (result) {
    return Write8(ToUnderlying(Commands::SkipRom));
  }
  return result;
}

}  // namespace one_wire
}  // namespace owif

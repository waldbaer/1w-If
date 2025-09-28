#include "one_wire/ds18b20.h"

#include <Arduino.h>

#include "util/crc.h"
#include "util/language.h"

namespace owif {
namespace one_wire {

auto Ds18b20::MatchesFamily(OneWireDevice const& ow_device) -> bool { return ow_device.GetFamilyCode() == kFamilyCode; }

auto Ds18b20::FromDevice(OneWireDevice& device) -> Ds18b20* {
  Ds18b20* result{nullptr};

  if (MatchesFamily(device)) {
    result = reinterpret_cast<Ds18b20*>(&device);
  }

  return result;
}

Ds18b20::Ds18b20(OneWireBus& bus, OneWireAddress const& address, Resolution resolution)
    : OneWireDevice{bus, address}, resolution_{resolution}, sampling_time_{ToSamplingTime(resolution_)} {}

auto Ds18b20::Begin() -> bool {
  bool result{ReadScratchpad()};
  if (result) {
    result &= CheckScratchpad();
  }

  if (result) {
    std::uint8_t res;
    switch (this->resolution_) {
      case Resolution::Res12Bit:
        res = ToUnderlying(ConfigRegister::Resolution12Bit);
        break;
      case Resolution::Res11Bit:
        res = ToUnderlying(ConfigRegister::Resolution11Bit);
        break;
      case Resolution::Res10Bit:
        res = ToUnderlying(ConfigRegister::Resolution10Bit);
        break;
      case Resolution::Res9Bit:
      default:
        res = ToUnderlying(ConfigRegister::Resolution9Bit);
        break;
    }

    if (scratch_pad_[4] != res) {
      scratch_pad_[4] = res;

      result &= SendCommand(ToUnderlying(Command::WriteScratchpad));
      if (result) {
        bus_.Write8(scratch_pad_[2]);  // high alarm temp
        bus_.Write8(scratch_pad_[3]);  // low alarm temp
        bus_.Write8(scratch_pad_[4]);  // resolution
      }

      // write value to EEPROM
      result &= SendCommand(ToUnderlying(Command::CopyScratchpad));
    }
  }

  return result;
}

auto Ds18b20::SampleTemperature(bool skip_rom_select) -> bool {
  bool const result{SendCommand(ToUnderlying(Command::ConvertTemperature), skip_rom_select)};
  if (!result) {
    logger_.Error(F("[DS1820B] Start temperature conversion failed"));
  }
  return result;
}

auto Ds18b20::GetTemperature(float& temperature) -> bool {
  bool result{ReadScratchpad()};
  if (result) {
    result &= CheckScratchpad();
  }
  if (result) {
    std::int16_t temp{static_cast<std::int16_t>((scratch_pad_[1] << 8) | scratch_pad_[0])};

    switch (resolution_) {
      case Resolution::Res9Bit:
        temp &= 0xFFF8;  // For 9-bit resolution, bit 0..2 are undefined
        break;
      case Resolution::Res10Bit:
        temp &= 0xFFFC;  // For 10-bit resolution, bit 0 and 1 are undefined
        break;
      case Resolution::Res11Bit:
        temp &= 0xFFFE;  // For 11-bit resolution, bit 0 is undefined
        break;
      case Resolution::Res12Bit:
      default:
        break;
    }

    temperature = temp / 16.0f;
  }

  return result;
}

auto Ds18b20::GetSamplingTime() -> std::uint32_t { return sampling_time_; }

// ---- Private APIS ---------------------------------------------------------------------------------------------------

auto Ds18b20::ReadScratchpad() -> bool {
  bool result{SendCommand(ToUnderlying(Command::ReadScratchpad))};
  if (result) {
    for (std::uint8_t& byte : scratch_pad_) {
      result &= bus_.Read8(byte);
    }
  }

  if (!result) {
    logger_.Warn(F("[DS1820B] Read scratchpad failed"));
  }
  return result;
}

auto Ds18b20::CheckScratchpad() -> bool {
  bool const result{crc8(scratch_pad_, 8) == scratch_pad_[8]};

  if (!result) {
    logger_.Error(F("[DS1820B] scratchpad CRC error detected"));
  }
  return result;
}

auto Ds18b20::ToSamplingTime(Resolution resolution) -> std::size_t {
  std::size_t sampling_time{};
  switch (resolution) {
    case Resolution::Res9Bit:
      sampling_time = 94;  // ms
      break;
    case Resolution::Res10Bit:
      sampling_time = 188;  // ms
      break;
    case Resolution::Res11Bit:
      sampling_time = 375;  // ms
      break;
    case Resolution::Res12Bit:
    default:
      sampling_time = kWorstCaseSamplingTime;
  }
  return sampling_time;
}

}  // namespace one_wire
}  // namespace owif

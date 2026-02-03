#include "one_wire/ds2438.h"

#include <Arduino.h>

#include "one_wire/one_wire_address.h"
#include "util/crc.h"
#include "util/language.h"

namespace owif {
namespace one_wire {

auto Ds2438::MatchesFamily(OneWireDevice const& ow_device) -> bool { return ow_device.GetFamilyCode() == kFamilyCode; }

auto Ds2438::FromDevice(OneWireDevice& device) -> Ds2438* {
  Ds2438* result{nullptr};

  if (MatchesFamily(device)) {
    result = reinterpret_cast<Ds2438*>(&device);
  }

  return result;
}

Ds2438::Ds2438(OneWireBus& bus, OneWireAddress const& address) : OneWireDevice{bus, address} {}

auto Ds2438::Begin() -> bool {
  bool result{ReadScratchpad(Page::Page0)};
  if (result) {
    result &= CheckScratchpad();
  }

  return result;
}

auto Ds2438::SampleTemperature(bool skip_rom_select) -> bool {
  bool const result{SendCommand(ToUnderlying(Command::ConvertTemperature), skip_rom_select)};
  if (!result) {
    logger_.Error(F("[DS2438] Start temperature conversion failed"));
  }
  return result;
}

auto Ds2438::GetTemperature(float& temperature) -> bool {
  bool result{ReadScratchpad(Page::Page0)};
  if (result) {
    result &= CheckScratchpad();
  }
  if (result) {
    temperature = (int(scratch_pad_[2]) * 256 + scratch_pad_[1]) * 0.03125 * 0.125;
  }

  return result;
}

auto Ds2438::SampleVAD() -> bool {
  bool result{ClearConfigBit(ConfigBit::AD)};

  result &= SendCommand(ToUnderlying(Command::ConvertVoltage));
  if (!result) {
    logger_.Error(F("[DS2438] Start VAD conversion failed"));
  }
  return result;
}

auto Ds2438::GetVAD(float& vad) -> bool {
  bool result{ReadScratchpad(Page::Page0)};
  result &= CheckScratchpad();

  if (result) {
    vad = ((scratch_pad_[4] & 0x03) * 256 + scratch_pad_[3]) * 0.01;
  }

  return result;
}

auto Ds2438::SampleVDD() -> bool {
  bool result{SetConfigBit(ConfigBit::AD)};

  result &= SendCommand(ToUnderlying(Command::ConvertVoltage));
  if (!result) {
    logger_.Error(F("[DS2438] Start VDD conversion failed"));
  }
  return result;
}

auto Ds2438::GetVDD(float& vad) -> bool { return GetVAD(vad); }

// ---- Private APIS ---------------------------------------------------------------------------------------------------

auto Ds2438::ReadScratchpad(Page page) -> bool {
  // Recall EEPROM to scratchpad
  bool result{SendCommand(ToUnderlying(Command::RecallMemory))};
  result &= bus_.Write8(ToUnderlying(page));

  // Read the scratchpad memory
  result &= SendCommand(ToUnderlying(Command::ReadScratchpad));
  result &= bus_.Write8(ToUnderlying(page));
  for (std::uint8_t& byte : scratch_pad_) {
    result &= bus_.Read8(byte);
  }

  if (!result) {
    logger_.Warn(F("[DS2438] Read scratchpad failed"));
  }
  return result;
}

auto Ds2438::WriteScratchpad(Page page) -> bool {
  bool result{SendCommand(ToUnderlying(Command::WriteScratchpad))};
  result &= bus_.Write8(ToUnderlying(page));
  for (std::uint8_t& byte : scratch_pad_) {
    result &= bus_.Write8(byte);
  }

  // Read the scratchpad memory
  result &= SendCommand(ToUnderlying(Command::CopyScratchpad));
  result &= bus_.Write8(ToUnderlying(page));

  if (!result) {
    logger_.Warn(F("[DS2438] Write scratchpad failed"));
  }
  return result;
}

auto Ds2438::CheckScratchpad() -> bool {
  bool const result{util::crc8(scratch_pad_, 8) == scratch_pad_[8]};

  if (!result) {
    logger_.Error(F("[DS2438] scratchpad CRC error detected"));
  }
  return result;
}

auto Ds2438::SetConfigBit(ConfigBit config_bit) -> bool {
  bool result{ReadScratchpad(Page::Page0)};

  if (result) {
    std::uint8_t const mask{static_cast<std::uint8_t>(0x01 << ToUnderlying(config_bit))};
    if ((scratch_pad_[0] & mask) != mask) {
      scratch_pad_[0] |= mask;
      result &= WriteScratchpad(Page::Page0);
    }
  }
  return result;
}

auto Ds2438::ClearConfigBit(ConfigBit config_bit) -> bool {
  bool result{ReadScratchpad(Page::Page0)};

  if (result) {
    std::uint8_t const mask{static_cast<std::uint8_t>(0x01 << ToUnderlying(config_bit))};
    if ((scratch_pad_[0] & mask) != 0x00) {
      scratch_pad_[0] &= ~mask;
      result &= WriteScratchpad(Page::Page0);
    }
  }
  return result;
}

}  // namespace one_wire
}  // namespace owif

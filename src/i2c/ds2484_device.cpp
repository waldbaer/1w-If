#include "i2c/ds2484_device.h"

#include <Arduino.h>

#include "util/language.h"

namespace owif {
namespace i2c {

// ---- Public APIs ----------------------------------------------------------------------------------------------------
Ds2484Device::Ds2484Device(I2CBus &bus, bool active_pullup, bool strong_pullup, std::uint8_t tRSTL, std::uint8_t tMSP,
                           std::uint8_t tW0L, std::uint8_t tREC0, std::uint8_t RWPU)
    : I2CDevice{bus, kI2CAddress},
      active_pullup_{active_pullup},
      strong_pullup_{strong_pullup},
      tRSTL_{tRSTL},
      tMSP_{tMSP},
      tW0L_{tW0L},
      tREC0_{tREC0},
      RWPU_{RWPU} {}

auto Ds2484Device::Begin() -> bool {
  bool result{ResetDevice()};
  result &= ResetOneWire();

  return result;
}

auto Ds2484Device::Read8(std::uint8_t &value) -> bool {
  std::uint8_t read8_cmd{ToUnderlying(Command::OneWireReadByte)};

  bool result{write(&read8_cmd, 1) == i2c::ERROR_OK};

  if (result) {
    result &= WaitForCompletion();
  }

  if (result) {
    // Set the ReadPointer to the 'ReadDataRegister'
    std::uint8_t set_read_reg_cmd[2]{ToUnderlying(Command::SetReadPointer),
                                     // param: Sets the read pointer to the specified register
                                     ToUnderlying(ReadPointerCode::ReadDataRegister)};
    result &= write(set_read_reg_cmd, 2) == i2c::ERROR_OK;
  }

  if (result) {
    // Finally read the value of the 'ReadDataRegister'
    result &= read(&value, 1) == i2c::ERROR_OK;
  }

  if (!result) {
    logger_.Error(F("[DS2484] Failed to read a byte"));
  }

  return result;
}

auto Ds2484Device::Read64(std::uint64_t &value) -> bool {
  bool result{true};

  value = 0;
  for (std::uint8_t i = 0; i < 8; i++) {
    std::uint8_t read_byte;
    result &= Read8(read_byte);
    value |= (read_byte << (i * 8));
  }
  return result;
}

auto Ds2484Device::Write8(std::uint8_t value) -> bool {
  std::uint8_t const cmd[2]{ToUnderlying(Command::OneWireWriteByte), value};
  bool result{write(cmd, 2) == ErrorCode::ERROR_OK};
  result &= WaitForCompletion();

  return result;
};

auto Ds2484Device::Write64(std::uint64_t value) -> bool {
  bool result{true};
  for (std::uint8_t i{0}; i < 8; i++) {
    result &= Write8((value >> (i * 8)) & 0xFF);
  }
  return result;
}

auto Ds2484Device::ResetOneWire() -> bool {
  std::uint8_t reset_ow_cmd{ToUnderlying(Command::OneWireReset)};
  if (write(&reset_ow_cmd, 1) != i2c::ERROR_OK) {
    logger_.Error(F("[DS2484] Failed to write OneWireReset command"));
    return false;
  }
  if (!WaitForCompletion()) {
    logger_.Error(F("[DS2484] ResetOneWire: can't complete"));
    return false;
  }
  return true;
}

auto Ds2484Device::OneWireTriple(bool *branch, bool *id_bit, bool *cmp_id_bit) -> bool {
  std::uint8_t status;
  if (!ReadStatus(&status)) {
    logger_.Error(F("[DS2484] OneWireTriple: read status error"));
    return false;
  }

  std::uint8_t cmd_buffer[2]{
      ToUnderlying(Command::OneWireTriplet),
      // parameter: DirectionByte: DS2484 generates a write-one time slot if V = 1 and a write-zero time slot if V = 0
      (*branch ? ToUnderlying(DirectionByte::WriteOneTimeSlot) : ToUnderlying(DirectionByte::WriteZeroTimeSlot))};
  if (write(cmd_buffer, 2) != i2c::ERROR_OK) {
    logger_.Error(F("[DS2484] Failed to write OneWireTriple command"));
    return false;
  }

  if (!ReadStatus(&status)) {
    logger_.Error(F("[DS2484] OneWireTriple: read status error"));
    return false;
  }

  *id_bit = static_cast<bool>(status & ToUnderlying(StatusReg::SBR));
  *cmp_id_bit = static_cast<bool>(status & ToUnderlying(StatusReg::TSB));
  *branch = static_cast<bool>(status & ToUnderlying(StatusReg::DIR));

  return true;
}

// ---- Private APIs ---------------------------------------------------------------------------------------------------

auto Ds2484Device::ResetDevice() -> bool {
  bool result{true};

  std::uint8_t device_reset_cmd{ToUnderlying(Command::DeviceReset)};
  if (write(&device_reset_cmd, 1) != i2c::ERROR_OK) {
    logger_.Error(F("[DS2484] Failed to write ResetDevice command"));
    return false;
  }
  if (!WaitForCompletion()) {
    logger_.Error(F("[DS2484] ResetDevice: can't complete"));
    return false;
  }
  std::uint8_t config{static_cast<std::uint8_t>((active_pullup_ ? ToUnderlying(DeviceConfigReg::APU) : 0) |
                                                (strong_pullup_ ? ToUnderlying(DeviceConfigReg::SPU) : 0))};
  std::uint8_t write_config[2]{
      ToUnderlying(Command::WriteDeviceConfig),
      static_cast<std::uint8_t>(
          config |
          // When writing to the Device Configuration register, the new data is accepted only if the
          // upper nibble (bits 7 to 4) is the one’s complement of the lower nibble (bits 3 to 0).
          (~config << 4))};
  if (write(write_config, 2) != i2c::ERROR_OK) {
    logger_.Error(F("[DS2484] Failed to write config"));
    return false;
  }

  // After WriteDeviceConfig the read pointer is directly set to the config register.
  // Read it and compare with written config. Attention: When read, the upper nibble is always 0h.
  std::uint8_t response;
  if (read(&response, 1) != i2c::ERROR_OK) {
    logger_.Error(F("[DS2484] Failed to read config response"));
    return false;
  }

  if (response != (write_config[1] & 0x0F)) {
    logger_.Error(F("[DS2484] Config response does not match expected config"));
    return false;
  }

  // Check 1-wire port configuration registers
  std::uint8_t ow_port_config[8]{0};
  ReadOneWirePortConfig(ow_port_config);
  for (std::uint8_t byte : ow_port_config) {
    if (byte != kDefaultOneWirePortConfigValueCode) {
      logger_.Error(F("[DS2484] Unexpected 1-wire port configuration register value detected: %X"), byte);
    }
  }

  if (tRSTL_ != kDefaultOneWirePortConfigValueCode) {
    result &= WriteOneWirePortConfig(ControlByteParamSelection::tRSTL, tRSTL_);
  }
  if (tMSP_ != kDefaultOneWirePortConfigValueCode) {
    result &= WriteOneWirePortConfig(ControlByteParamSelection::tMSP, tMSP_);
  }
  if (tW0L_ != kDefaultOneWirePortConfigValueCode) {
    result &= WriteOneWirePortConfig(ControlByteParamSelection::tW0L, tW0L_);
  }
  if (tREC0_ != kDefaultOneWirePortConfigValueCode) {
    result &= WriteOneWirePortConfig(ControlByteParamSelection::tREC0, tREC0_);
  }
  if (RWPU_ != kDefaultOneWirePortConfigValueCode) {
    result &= WriteOneWirePortConfig(ControlByteParamSelection::RWPU, RWPU_);
  }

  // Check 1-wire port config after re-configuration
  ReadOneWirePortConfig(ow_port_config);

  result &= ow_port_config[0] == tRSTL_;
  result &= ow_port_config[2] == tMSP_;
  result &= ow_port_config[4] == tW0L_;
  result &= ow_port_config[6] == tREC0_;
  result &= ow_port_config[7] == RWPU_;

  if (!result) {
    logger_.Error(F("[DS2484] DeviceConfiguration failed."));
  }

  return result;
}

auto Ds2484Device::WaitForCompletion() -> bool {
  std::uint8_t status;
  return ReadStatus(&status);
}

auto Ds2484Device::ReadStatus(std::uint8_t *status) -> bool {
  for (std::uint8_t retry_nr{0}; retry_nr < 10; retry_nr++) {
    if (read(status, 1) != i2c::ERROR_OK) {
      logger_.Error(F("[DS2484] Unexpected status != OK: %X"), status);
      return false;
    }
    // Bit0: 1WB: 1-Wire line is busy
    if (!(*status & ToUnderlying(StatusReg::OW_1WB))) {
      return true;
    }
  }
  logger_.Error(F("[DS2484] ReadStatus too many retries"));
  return false;
}

auto Ds2484Device::ReadOneWirePortConfig(std::uint8_t *port_config_register) -> bool {
  // Set read pointer to the PortConfig register
  std::uint8_t set_read_reg_cmd[2]{ToUnderlying(Command::SetReadPointer),
                                   // param: Set the read pointer to the specified register
                                   ToUnderlying(ReadPointerCode::PortConfigRegister)};
  bool result{write(set_read_reg_cmd, 2) == i2c::ERROR_OK};

  if (result) {
    result &= read(port_config_register, 8) == i2c::ERROR_OK;
  }

  if (!result) {
    logger_.Error(F("[DS2484] Failed to read PortConfig register"));
  }
  return result;
}

auto Ds2484Device::WriteOneWirePortConfig(ControlByteParamSelection parameter_selection, std::uint8_t parameter_value,
                                          bool over_drive) -> bool {
  bool result{(ToUnderlying(parameter_selection) <= 0x07) &&  // 3 bit parameter selection
              (parameter_value <= 0x0F)};                     // 4 bit parameter value

  if (result) {
    std::uint8_t control_byte{};

    std::uint8_t adjust_port_config_cmd[2]{
        ToUnderlying(Command::AdjustOneWirePort),
        // param: ControlByte with single parameter value
        static_cast<std::uint8_t>(static_cast<std::uint8_t>((ToUnderlying(parameter_selection)) << 5) |  // Bits 5-7
                                  static_cast<std::uint8_t>((over_drive & 0x01) << 4) |                  // Bit 4
                                  static_cast<std::uint8_t>((parameter_value)))};                        // Bit 0-3

    result &= write(adjust_port_config_cmd, 2) == i2c::ERROR_OK;
  }

  if (!result) {
    logger_.Error(F("[DS2484] Failed to write 1-wire port config"));
  }

  return result;
}

constexpr std::uint8_t Ds2484Device::kI2CAddress;

}  // namespace i2c
}  // namespace owif

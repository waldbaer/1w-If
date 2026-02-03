#ifndef OWIF_I2C_DS2484_DEVICE_H
#define OWIF_I2C_DS2484_DEVICE_H

#include <cstdint>

#include "i2c/i2c_bus.h"
#include "i2c/i2c_device.h"
#include "logging/logger.h"
#include "util/language.h"

namespace owif {
namespace i2c {

/*!
 * \brief Representation of the DS2484 One-Wire Busmaster chip as I2C
 */
class Ds2484Device : public I2CDevice {
 public:
  /* Default value code for all 1-wire port settings: 0b0110*/
  static constexpr std::uint8_t kDefaultOneWirePortConfigValueCode{0x06};

  // Inherit I2CDevice constructor
  Ds2484Device(I2CBus& i2c_bus, bool active_pullup = false, bool strong_pullup = false,
               // Timing Config
               std::uint8_t tRSTL = kDefaultOneWirePortConfigValueCode,
               std::uint8_t tMSP = kDefaultOneWirePortConfigValueCode,
               std::uint8_t tW0L = kDefaultOneWirePortConfigValueCode,
               std::uint8_t tREC0 = kDefaultOneWirePortConfigValueCode,
               std::uint8_t RWPU = kDefaultOneWirePortConfigValueCode);

  auto Begin() -> bool;

  auto Read8(std::uint8_t& value) -> bool;
  auto Read64(std::uint64_t& value) -> bool;
  auto Write8(std::uint8_t value) -> bool;
  auto Write64(std::uint64_t value) -> bool;

  auto ResetOneWire() -> bool;
  auto ResetSearch() -> void;
  auto OneWireTriple(bool* branch, bool* id_bit, bool* cmp_id_bit) -> bool;

 private:
  // I2C address of the DS2484 cannot be modified.
  static constexpr std::uint8_t kI2CAddress{0x18};

  /*!
   * \brief Available I2C commands
   */
  enum class Command : std::uint8_t {
    DeviceReset = 0xF0,
    SetReadPointer = 0xE1,
    WriteDeviceConfig = 0xD2,
    AdjustOneWirePort = 0xC3,
    OneWireReset = 0xB4,
    OneWireSingleBit = 0x87,
    OneWireWriteByte = 0xA5,
    OneWireReadByte = 0x96,
    OneWireTriplet = 0x78
  };

  enum class DeviceConfigReg : std::uint8_t {
    APU = 0x01,     // Active Pullup
    PDN = 0x02,     // 1-Wire Power-Down
    SPU = 0x04,     // Strong Pullup
    OW_1WS = 0x08,  // 1-Wire Speed
  };

  enum class StatusReg : std::uint8_t {
    OW_1WB = 0x01,  // 1-Wire is busy
    PPD = 0x02,     // Presence-Pulse Detect
    SD = 0x04,      // Short Detected
    LL = 0x08,      // Logic Level
    RST = 0x10,     // Device Reset
    SBR = 0x20,     // Single Bit Result
    TSB = 0x40,     // Triplet Second Bit
    DIR = 0x80,     // Branch Direction Taken
  };

  // Table 5. Valid Read Pointer Codes
  enum class ReadPointerCode : std::uint8_t {
    DeviceConfigRegister = 0xC3,
    StatusRegister = 0xF0,
    ReadDataRegister = 0xE1,
    PortConfigRegister = 0xB4,
  };

  enum class DirectionByte : std::uint8_t {
    WriteZeroTimeSlot = 0x00,  // V: bit7 not set:
    WriteOneTimeSlot = 0x80    // V: bit7 set:
  };

  /* Table 6. Bit Allocation in the Control Byte
     BITS 7:5 of ControlByte */
  enum class ControlByteParamSelection : std::uint8_t {
    tRSTL = 0x00, /* 0b000 */
    tMSP = 0x01,  /* 0b001 */
    tW0L = 0x02,  /* 0b010 */
    tREC0 = 0x03, /* 0b011 */
    RWPU = 0x04   /* 0b100 */
  };

  /* Table 6. Bit Allocation in the Control Byte
     BITS 4: OD: Overdrive Control */
  enum class ControlByteOverDrive : std::uint8_t {
    Off = 0x00, /* OverDrive off */
    On = 0x10   /* OverDrive on */
  };

  auto ResetDevice() -> bool;

  auto WaitForCompletion() -> bool;
  auto ReadStatus(std::uint8_t* status) -> bool;

  auto ReadOneWirePortConfig(std::uint8_t* port_config_array) -> bool;
  auto WriteOneWirePortConfig(ControlByteParamSelection parameter_selection, std::uint8_t parameter_value,
                              bool over_drive = false) -> bool;

  logging::Logger& logger_{logging::logger_g};

  bool is_initialized_{false};
  // PullUp Config
  bool active_pullup_;
  bool strong_pullup_;

  // Timing Config

  std::uint8_t tRSTL_;
  std::uint8_t tMSP_;
  std::uint8_t tW0L_;
  std::uint8_t tREC0_;
  std::uint8_t RWPU_;
};

}  // namespace i2c
}  // namespace owif

#endif  // OWIF_I2C_DS2484_DEVICE_H

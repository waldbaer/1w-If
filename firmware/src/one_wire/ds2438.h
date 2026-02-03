#ifndef OWIF_ONE_WIRE_DS2438_H
#define OWIF_ONE_WIRE_DS2438_H

#include "logging/logger.h"
#include "one_wire/one_wire_address.h"
#include "one_wire/one_wire_bus.h"
#include "one_wire/one_wire_device.h"

// ---- Constants ----

namespace owif {
namespace one_wire {

/*!
 * \brief 1-Wire DS2438 Smart Battery Monitor
 */
class Ds2438 : public OneWireDevice {
 public:
  static constexpr OneWireAddress::FamilyCode kFamilyCode{0x26};
  static constexpr std::uint32_t kSamplingTime{10};  // Temperature and ADC sampling / conversion time in milliseconds

  static auto MatchesFamily(OneWireDevice const& ow_device) -> bool;
  static auto FromDevice(OneWireDevice& device) -> Ds2438*;

  Ds2438(OneWireBus& bus, OneWireAddress const& address);

  auto Begin() -> bool;

  auto SampleTemperature(bool skip_rom_select = false) -> bool;
  auto GetTemperature(float& temperature) -> bool;

  auto SampleVAD() -> bool;
  auto GetVAD(float& vad) -> bool;

  auto SampleVDD() -> bool;
  auto GetVDD(float& vdd) -> bool;

 private:
  /*!
   * \brief Available I2C commands
   */
  enum class Command : std::uint8_t {
    ConvertTemperature = 0x44,
    ConvertVoltage = 0xB4,
    ReadScratchpad = 0xBE,
    WriteScratchpad = 0x4E,
    CopyScratchpad = 0x48,
    RecallMemory = 0xB8
  };

  enum class Page : std::uint8_t {
    Page0 = 0,
    Page1 = 1,
    Page2 = 2,
    Page3 = 3,
    Page4 = 4,
    Page5 = 5,
    Page6 = 6,
    Page7 = 7,
  };

  enum class ConfigBit : std::uint8_t {
    IAD = 0,  // Bit0: Current A/D Control Bit
    CA = 2,   // Bit1: Current Accumulator Configuration
    EE = 3,   // Bit2: Current Accumulator Shadow Selector bit
    AD = 3,   // Bit3: "1" battery input VSS selected for ADC. "0": general purpose A/D input (VAD) is selected for ADC
    TB = 4,   // Bit4: Temperature busy flag
    NVB = 5,  // Bit5: Non-Volatile busy flag
    ADB = 6   // Bit6: ADC busy flag
  };

  auto ReadScratchpad(Page page) -> bool;
  auto WriteScratchpad(Page page) -> bool;
  auto CheckScratchpad() -> bool;

  auto SetConfigBit(ConfigBit config_bit) -> bool;
  auto ClearConfigBit(ConfigBit config_bit) -> bool;

  logging::Logger& logger_{logging::logger_g};

  std::uint8_t scratch_pad_[9]{0};
};

}  // namespace one_wire
}  // namespace owif

#endif  // OWIF_ONE_WIRE_DS2438_H

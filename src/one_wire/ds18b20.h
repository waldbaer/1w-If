#ifndef OWIF_ONE_WIRE_DS18B20_H
#define OWIF_ONE_WIRE_DS18B20_H

#include "logging/logger.h"
#include "one_wire/one_wire_address.h"
#include "one_wire/one_wire_bus.h"
#include "one_wire/one_wire_device.h"

// ---- Constants ----

namespace owif {
namespace one_wire {

class Ds18b20 : public OneWireDevice {
 public:
  enum class Resolution : std::uint8_t {
    Res9Bit = 9,
    Res10Bit = 10,
    Res11Bit = 11,
    Res12Bit = 12,
  };

  static constexpr OneWireAddress::FamilyCode kFamilyCode{0x28};

  // Worst case sampling time independent of resolution config
  static constexpr std::uint32_t kWorstCaseSamplingTime{750};

  static auto MatchesFamily(OneWireDevice const& ow_device) -> bool;
  static auto FromDevice(OneWireDevice& device) -> Ds18b20*;

  Ds18b20(OneWireBus& bus, OneWireAddress const& address, Resolution resolution = Resolution::Res12Bit);

  // ---- Public APIs --------------------------------------------------------------------------------------------------
  auto Begin() -> bool override;

  auto SampleTemperature(bool skip_rom_select = false) -> bool;
  auto GetTemperature(float& temperature) -> bool;

  auto GetSamplingTime() -> std::uint32_t;

 private:
  /*!
   * \brief Available I2C commands
   */
  enum class Command : std::uint8_t {
    ConvertTemperature = 0x44,
    ReadScratchpad = 0xBE,
    WriteScratchpad = 0x4E,
    CopyScratchpad = 0x48
  };

  enum class ConfigRegister : std::uint8_t {
    Resolution9Bit = 0x1F,   // R1: 0 | R0: 0 // sampling time: 93.75ms
    Resolution10Bit = 0x3F,  // R1: 0 | R0: 1 // sampling time: 187.5ms
    Resolution11Bit = 0x5F,  // R1: 1 | R0: 0 // sampling time: 375ms
    Resolution12Bit = 0x7F,  // R1: 1 | R0: 1 // sampling time: 750ms
  };

  auto ReadScratchpad() -> bool;
  auto CheckScratchpad() -> bool;

  static auto ToSamplingTime(Resolution resolution) -> std::size_t;

  logging::Logger& logger_{logging::logger_g};

  Resolution resolution_;
  std::uint32_t sampling_time_;
  std::uint8_t scratch_pad_[9]{0};
};

}  // namespace one_wire
}  // namespace owif

#endif  // OWIF_ONE_WIRE_DS18B20_H

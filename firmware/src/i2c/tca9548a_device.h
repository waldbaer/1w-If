#ifndef OWIF_I2C_TCA9548A_DEVICE_H
#define OWIF_I2C_TCA9548A_DEVICE_H
#include <cstdint>

#include "i2c/i2c_bus.h"
#include "i2c/i2c_device.h"
#include "logging/logger.h"

namespace owif {
namespace i2c {

class Tca9548aI2CBus;  // forward declaration due to circular dependency

/*!
 * \brief Representation of the TCA9548A I2C multiplexer chip as I2C device
 */
class Tca9548aDevice : public I2CDevice {
 public:
  static constexpr std::uint8_t kDefaultI2CAddress{0x70};

  enum class ChannelId : std::uint8_t {
    Channel0 = 0,
    Channel1 = 1,
    Channel2 = 2,
    Channel3 = 3,
    Channel4 = 4,
    Channel5 = 5,
    Channel6 = 6,
    Channel7 = 7,
  };

  // Inherit I2CDevice constructor
  Tca9548aDevice(I2CBus& bus, std::uint8_t address = kDefaultI2CAddress);

  auto Begin() -> bool;

  auto switch_to_channel(ChannelId channel) -> ErrorCode;
  auto disable_all_channels() -> ErrorCode;

 protected:
  friend class Tca9548aI2CBus;

 private:
  static constexpr std::uint8_t kDisableChannelsCommands{0x00};
  static constexpr std::uint8_t kNoChannelSelected{0xFF};

  logging::Logger& logger_{logging::logger_g};

  bool is_initialized_{false};
  std::uint8_t current_channel_{kNoChannelSelected};
};

}  // namespace i2c
}  // namespace owif

#endif  // OWIF_I2C_TCA9548A_DEVICE_H

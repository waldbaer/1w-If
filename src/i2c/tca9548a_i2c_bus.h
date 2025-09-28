#ifndef OWIF_I2C_TCA9548A_I2C_BUS_H
#define OWIF_I2C_TCA9548A_I2C_BUS_H

#include <cstdint>
#include <vector>

#include "i2c/i2c_bus.h"
#include "i2c/i2c_device.h"
#include "i2c/tca9548a_device.h"
#include "logging/logger.h"

namespace owif {
namespace i2c {

/*!
 * \brief Representation of an I2C bus multiplexed by the TCA9548A I2C multiplexer chip.
 */
class Tca9548aI2CBus : public I2CBus {
 public:
  using Id = Tca9548aDevice::ChannelId;

  Tca9548aI2CBus(Tca9548aDevice &parent, Tca9548aDevice::ChannelId multiplex_channel_id);

  i2c::ErrorCode readv(uint8_t address, i2c::ReadBuffer *buffers, size_t cnt) override;
  i2c::ErrorCode writev(uint8_t address, i2c::WriteBuffer *buffers, size_t cnt, bool stop) override;

  auto Scan() -> std::vector<std::pair<uint8_t, bool>>;

 private:
  logging::Logger &logger_{logging::logger_g};

  Tca9548aDevice::ChannelId channel_;
  Tca9548aDevice &parent_;
};

}  // namespace i2c
}  // namespace owif

#endif  // OWIF_I2C_TCA9548A_I2C_BUS_H

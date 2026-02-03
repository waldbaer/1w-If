#include "i2c/tca9548a_i2c_bus.h"

#include <Arduino.h>

#include "i2c/tca9548a_device.h"

namespace owif {
namespace i2c {

Tca9548aI2CBus::Tca9548aI2CBus(Tca9548aDevice &parent, Tca9548aDevice::ChannelId multiplex_channel_id)
    : parent_{parent}, channel_{multiplex_channel_id} {}

ErrorCode Tca9548aI2CBus::readv(uint8_t address, ReadBuffer *buffers, size_t cnt) {
  ErrorCode err{parent_.switch_to_channel(channel_)};
  if (err != ERROR_OK) {
    logger_.Error(F("[TCA9548AI2CBus] readv channel switch failed"));
    return err;
  }

  err = parent_.bus_.readv(address, buffers, cnt);

  // parent_.disable_all_channels();
  return err;
}

ErrorCode Tca9548aI2CBus::writev(uint8_t address, WriteBuffer *buffers, size_t cnt, bool stop) {
  ErrorCode err{parent_.switch_to_channel(channel_)};
  if (err != ERROR_OK) {
    logger_.Error(F("[TCA9548AI2CBus] writev channel switch failed"));
    return err;
  }

  err = parent_.bus_.writev(address, buffers, cnt, stop);

  // parent_.disable_all_channels();
  return err;
}

auto Tca9548aI2CBus::Scan() -> std::vector<std::pair<uint8_t, bool>> {
  ErrorCode err{parent_.switch_to_channel(channel_)};
  if (err != ERROR_OK) {
    logger_.Error(F("[TCA9548AI2CBus] ScanBus channel switch failed"));
    return std::vector<std::pair<uint8_t, bool>>{};
  }

  std::vector<std::pair<uint8_t, bool>> result{I2CBus::Scan()};

  // parent_.disable_all_channels();

  return result;
}

}  // namespace i2c
}  // namespace owif

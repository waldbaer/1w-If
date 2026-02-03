#include "i2c/tca9548a_device.h"

#include <Arduino.h>

#include "util/language.h"

namespace owif {
namespace i2c {

Tca9548aDevice::Tca9548aDevice(I2CBus &bus, std::uint8_t address) : I2CDevice{bus, address} {}

auto Tca9548aDevice::Begin() -> bool {
  bool result{true};

  std::uint8_t status{0};
  if (read(&status, 1) != ERROR_OK) {
    logger_.Error(F("[TCA9548A] setup failed: any channel already open"));
    return false;
  }

  is_initialized_ = true;
  return true;
}

auto Tca9548aDevice::switch_to_channel(ChannelId channel) -> ErrorCode {
  ErrorCode result{ErrorCode::ERROR_NOT_INITIALIZED};

  if (is_initialized_) {
    if (ToUnderlying(channel) != current_channel_) {
      std::uint8_t const channel_val{static_cast<std::uint8_t>(1 << ToUnderlying(channel))};
      result = write(&channel_val, 1);
    }
  } else {
    logger_.Error(F("[TCA9548A] channel switch: device not initialized"));
  }
  return result;
}

auto Tca9548aDevice::disable_all_channels() -> ErrorCode {
  ErrorCode result{ErrorCode::ERROR_NOT_INITIALIZED};
  current_channel_ = kNoChannelSelected;

  if (is_initialized_) {
    result = write(&kDisableChannelsCommands, 1);
  } else {
    logger_.Error(F("[TCA9548A] disable all channels: device not initialized"));
  }

  if (result != ErrorCode::ERROR_OK) {
    logger_.Error(F("[TCA9548A] disable all channels failed"));
  }
  return result;
}

constexpr std::uint8_t Tca9548aDevice::kDisableChannelsCommands;

}  // namespace i2c
}  // namespace owif

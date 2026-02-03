#ifndef OWIF_I2C_I2C_DEVICE_H
#define OWIF_I2C_I2C_DEVICE_H

#include <Wire.h>

#include <cstdint>

#include "i2c_bus.h"

namespace owif {
namespace i2c {

class I2CDevice {
 public:
  I2CDevice(I2CBus &bus, std::uint8_t address);
  virtual ~I2CDevice() = default;

  /// @brief reads an array of bytes from the device using an I2CBus
  /// @param data pointer to an array to store the bytes
  /// @param len length of the buffer = number of bytes to read
  /// @return an i2c::ErrorCode
  ErrorCode read(std::uint8_t *data, size_t len);

  /// @brief writes an array of bytes to a device using an I2CBus
  /// @param data pointer to an array that contains the bytes to send
  /// @param len length of the buffer = number of bytes to write
  /// @param stop (true/false): True will send a stop message, releasing the bus after
  /// transmission. False will send a restart, keeping the connection active.
  /// @return an i2c::ErrorCode
  ErrorCode write(std::uint8_t const *data, size_t len, bool stop = true);

 protected:
  std::uint8_t address_;  // I2C device address
  I2CBus &bus_;
};

}  // namespace i2c
}  // namespace owif

#endif  // OWIF_I2C_I2C_BUS_H

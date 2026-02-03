
#include "i2c_device.h"

#include <cstdint>

#include "i2c_bus.h"

namespace owif {
namespace i2c {

I2CDevice::I2CDevice(I2CBus &bus, std::uint8_t address) : bus_{bus}, address_{address} {}

/// @brief reads an array of bytes from the device using an I2CBus
/// @param data pointer to an array to store the bytes
/// @param len length of the buffer = number of bytes to read
/// @return an i2c::ErrorCode
ErrorCode I2CDevice::read(uint8_t *data, size_t len) { return bus_.read(address_, data, len); }

/// @brief writes an array of bytes to a device using an I2CBus
/// @param data pointer to an array that contains the bytes to send
/// @param len length of the buffer = number of bytes to write
/// @param stop (true/false): True will send a stop message, releasing the bus after
/// transmission. False will send a restart, keeping the connection active.
/// @return an i2c::ErrorCode
ErrorCode I2CDevice::write(const uint8_t *data, size_t len, bool stop) { return bus_.write(address_, data, len, stop); }

}  // namespace i2c
}  // namespace owif

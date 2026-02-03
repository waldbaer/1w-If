#ifndef OWIF_I2C_I2C_BUS_H
#define OWIF_I2C_I2C_BUS_H

#include <Wire.h>

#include <cstdint>
#include <vector>

namespace owif {
namespace i2c {

/// @brief Error codes returned by I2CBus and I2CDevice methods
enum ErrorCode {
  NO_ERROR = 0,                ///< No error found during execution of method
  ERROR_OK = 0,                ///< No error found during execution of method
  ERROR_INVALID_ARGUMENT = 1,  ///< method called invalid argument(s)
  ERROR_NOT_ACKNOWLEDGED = 2,  ///< I2C bus acknowledgment not received
  ERROR_TIMEOUT = 3,           ///< timeout while waiting to receive bytes
  ERROR_NOT_INITIALIZED = 4,   ///< call method to a not initialized bus
  ERROR_TOO_LARGE = 5,         ///< requested a transfer larger than buffers can hold
  ERROR_UNKNOWN = 6,           ///< miscellaneous I2C error during execution
  ERROR_CRC = 7,               ///< bytes received with a CRC error
};

/// @brief the ReadBuffer structure stores a pointer to a read buffer and its length
struct ReadBuffer {
  uint8_t *data;  ///< pointer to the read buffer
  size_t len;     ///< length of the buffer
};

/// @brief the WriteBuffer structure stores a pointer to a write buffer and its length
struct WriteBuffer {
  const uint8_t *data;  ///< pointer to the write buffer
  size_t len;           ///< length of the buffer
};

class I2CBus {
 public:
  virtual ~I2CBus() = default;

  virtual ErrorCode read(uint8_t address, uint8_t *buffer, size_t len) {
    ReadBuffer buf;
    buf.data = buffer;
    buf.len = len;
    return readv(address, &buf, 1);
  }

  virtual ErrorCode readv(uint8_t address, ReadBuffer *buffers, size_t count) = 0;

  virtual ErrorCode write(uint8_t address, uint8_t const *buffer, size_t len) {
    return write(address, buffer, len, true);
  }
  virtual ErrorCode write(uint8_t address, uint8_t const *buffer, size_t len, bool stop) {
    WriteBuffer buf;
    buf.data = buffer;
    buf.len = len;
    return writev(address, &buf, 1, stop);
  }

  virtual ErrorCode writev(uint8_t address, WriteBuffer *buffers, size_t cnt) {
    return writev(address, buffers, cnt, true);
  }

  virtual ErrorCode writev(uint8_t address, WriteBuffer *buffers, size_t count, bool stop) = 0;

 protected:
  virtual auto Scan() -> std::vector<std::pair<uint8_t, bool>> {
    std::vector<std::pair<uint8_t, bool>> scan_results{};
    scan_results.clear();

    for (std::uint8_t address = 8; address < 120; address++) {
      ErrorCode const err{writev(address, nullptr, 0)};
      scan_results.emplace_back(address, err == ERROR_OK);
    }

    return scan_results;
  }
};

}  // namespace i2c
}  // namespace owif

#endif  // OWIF_I2C_I2C_BUS_H

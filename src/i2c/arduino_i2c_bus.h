#ifndef OWIF_I2C_ARDUINO_I2C_BUS_H
#define OWIF_I2C_ARDUINO_I2C_BUS_H

#include <Wire.h>

#include <cstdint>

#include "i2c/i2c_bus.h"
#include "logging/logger.h"

namespace owif {
namespace i2c {

enum RecoveryCode {
  RECOVERY_FAILED_SCL_LOW,
  RECOVERY_FAILED_SDA_LOW,
  RECOVERY_COMPLETED,
};

class ArduinoI2CBus : public I2CBus {
 public:
  ArduinoI2CBus(TwoWire &two_wire, std::uint8_t sda_pin, std::uint8_t scl_pin, std::uint32_t frequency,
                std::uint32_t timeout = 0);

  auto Begin() -> bool;

  auto readv(uint8_t address, ReadBuffer *buffers, size_t cnt) -> ErrorCode override;
  auto writev(uint8_t address, WriteBuffer *buffers, size_t cnt, bool stop) -> ErrorCode override;

 protected:
  TwoWire &two_wire_;
  std::uint8_t sda_pin_;
  std::uint8_t scl_pin_;
  std::uint32_t frequency_;
  std::int8_t port_{-1};
  std::uint32_t timeout_{0};  // disabled by default
  bool is_initialized_{false};

 private:
  auto Recover() -> void;

  logging::Logger &logger_{logging::logger_g};

  RecoveryCode recovery_result_;
};

}  // namespace i2c
}  // namespace owif

#endif  // OWIF_I2C_ARDUINO_I2C_BUS_H

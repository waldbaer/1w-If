#include "i2c/arduino_i2c_bus.h"

#include <Arduino.h>
#include <Wire.h>

#include <cstdint>

namespace owif {
namespace i2c {

ArduinoI2CBus::ArduinoI2CBus(TwoWire &two_wire, std::uint8_t sda_pin, std::uint8_t scl_pin, std::uint32_t frequency,
                             std::uint32_t timeout)
    : two_wire_{two_wire}, sda_pin_{sda_pin}, scl_pin_{scl_pin}, frequency_{frequency}, timeout_{timeout} {}

auto ArduinoI2CBus::Begin() -> bool {
  Recover();
  bool result{recovery_result_ == RECOVERY_COMPLETED};

  if (result) {
    result &= two_wire_.begin(sda_pin_, scl_pin_);

    if (timeout_ > 0) {
      two_wire_.setTimeOut(timeout_ / 1000);  // unit: ms
    }
    result &= two_wire_.setClock(frequency_);

    is_initialized_ = result;
  }

  if (!result) {
    logger_.Error(F("[I2C] Arduino I2C bus setup failed"));
  }
  return result;
}

ErrorCode ArduinoI2CBus::readv(uint8_t address, ReadBuffer *buffers, size_t cnt) {
  // logging is only enabled with vv level, if warnings are shown the caller
  // should log them
  if (!is_initialized_) {
    logger_.Error(F("[I2C] bus not initialized"));
    return ERROR_NOT_INITIALIZED;
  }
  size_t to_request = 0;
  for (size_t i = 0; i < cnt; i++) to_request += buffers[i].len;
  size_t ret = two_wire_.requestFrom(address, to_request, true);
  if (ret != to_request) {
    logger_.Error(F("[I2C] Rx failed"));
    return ERROR_TIMEOUT;
  }

  for (size_t i = 0; i < cnt; i++) {
    const auto &buf = buffers[i];
    for (size_t j = 0; j < buf.len; j++) buf.data[j] = two_wire_.read();
  }

  return ERROR_OK;
}
ErrorCode ArduinoI2CBus::writev(uint8_t address, WriteBuffer *buffers, size_t cnt, bool stop) {
  // logging is only enabled with vv level, if warnings are shown the caller
  // should log them
  if (!is_initialized_) {
    logger_.Error(F("[I2C] bus not initialized"));
    return ERROR_NOT_INITIALIZED;
  }

  two_wire_.beginTransmission(address);
  size_t written = 0;
  for (size_t i = 0; i < cnt; i++) {
    const auto &buf = buffers[i];
    if (buf.len == 0)
      continue;
    size_t ret = two_wire_.write(buf.data, buf.len);
    written += ret;
    if (ret != buf.len) {
      logger_.Error(F("[I2C] Tx failed"));
      return ERROR_UNKNOWN;
    }
  }
  uint8_t status = two_wire_.endTransmission(stop);
  switch (status) {
    case 0:
      return ERROR_OK;
    case 1:
      // transmit buffer not large enough
      logger_.Error(F("[I2C] Tx failed: buffer not large enough"));
      return ERROR_UNKNOWN;
    case 2:
    case 3:
      logger_.Error(F("[I2C] Tx failed: not acknowledged"));
      return ERROR_NOT_ACKNOWLEDGED;
    case 5:
      logger_.Error(F("[I2C] Tx failed: timeout"));
      return ERROR_UNKNOWN;
    case 4:
    default:
      logger_.Error(F("[I2C] Tx failed: unknown"));
      return ERROR_UNKNOWN;
  }
}

// ---- Private APIs ---------------------------------------------------------------------------------------------------

auto ArduinoI2CBus::Recover() -> void {
  // For the upcoming operations, target for a 100kHz toggle frequency.
  // This is the maximum frequency for I2C running in standard-mode.
  // The actual frequency will be lower, because of the additional
  // function calls that are done, but that is no problem.
  std::uint32_t const half_period_usec{1000000 / 100000 / 2};

  // Activate input and pull up resistor for the SCL pin.
  pinMode(scl_pin_, INPUT_PULLUP);  // NOLINT

  // This should make the signal on the line HIGH. If SCL is pulled low
  // on the I2C bus however, then some device is interfering with the SCL
  // line. In that case, the I2C bus cannot be recovered.
  delayMicroseconds(half_period_usec);
  if (digitalRead(scl_pin_) == LOW) {  // NOLINT
    recovery_result_ = RECOVERY_FAILED_SCL_LOW;
    return;
  }

  // From the specification:
  // "If the data line (SDA) is stuck LOW, send nine clock pulses. The
  //  device that held the bus LOW should release it sometime within
  //  those nine clocks."
  // We don't really have to detect if SDA is stuck low. We'll simply send
  // nine clock pulses here, just in case SDA is stuck. Actual checks on
  // the SDA line status will be done after the clock pulses.

  // Make sure that switching to output mode will make SCL low, just in
  // case other code has setup the pin for a HIGH signal.
  digitalWrite(scl_pin_, LOW);  // NOLINT

  delayMicroseconds(half_period_usec);
  for (auto i = 0; i < 9; i++) {
    // Release pull up resistor and switch to output to make the signal LOW.
    pinMode(scl_pin_, INPUT);   // NOLINT
    pinMode(scl_pin_, OUTPUT);  // NOLINT
    delayMicroseconds(half_period_usec);

    // Release output and activate pull up resistor to make the signal HIGH.
    pinMode(scl_pin_, INPUT);         // NOLINT
    pinMode(scl_pin_, INPUT_PULLUP);  // NOLINT
    delayMicroseconds(half_period_usec);

    // When SCL is kept LOW at this point, we might be looking at a device
    // that applies clock stretching. Wait for the release of the SCL line,
    // but not forever. There is no specification for the maximum allowed
    // time. We yield and reset the WDT, so as to avoid triggering reset.
    // No point in trying to recover the bus by forcing a uC reset. Bus
    // should recover in a few ms or less else not likely to recovery at
    // all.
    auto wait = 250;
    while (wait-- && digitalRead(scl_pin_) == LOW) {  // NOLINT
      // App.feed_wdt();
      delayMicroseconds(half_period_usec * 2);
    }
    if (digitalRead(scl_pin_) == LOW) {  // NOLINT
      recovery_result_ = RECOVERY_FAILED_SCL_LOW;
      return;
    }
  }

  // Activate input and pull resistor for the SDA pin, so we can verify
  // that SDA is pulled HIGH in the following step.
  pinMode(sda_pin_, INPUT_PULLUP);  // NOLINT
  digitalWrite(sda_pin_, LOW);      // NOLINT

  // By now, any stuck device ought to have sent all remaining bits of its
  // transaction, meaning that it should have freed up the SDA line, resulting
  // in SDA being pulled up.
  if (digitalRead(sda_pin_) == LOW) {  // NOLINT
    recovery_result_ = RECOVERY_FAILED_SDA_LOW;
    return;
  }

  // From the specification:
  // "I2C-bus compatible devices must reset their bus logic on receipt of
  //  a START or repeated START condition such that they all anticipate
  //  the sending of a target address, even if these START conditions are
  //  not positioned according to the proper format."
  // While the 9 clock pulses from above might have drained all bits of a
  // single byte within a transaction, a device might have more bytes to
  // transmit. So here we'll generate a START condition to snap the device
  // out of this state.
  // SCL and SDA are already high at this point, so we can generate a START
  // condition by making the SDA signal LOW.
  delayMicroseconds(half_period_usec);
  pinMode(sda_pin_, INPUT);   // NOLINT
  pinMode(sda_pin_, OUTPUT);  // NOLINT

  // From the specification:
  // "A START condition immediately followed by a STOP condition (void
  //  message) is an illegal format. Many devices however are designed to
  //  operate properly under this condition."
  // Finally, we'll bring the I2C bus into a starting state by generating
  // a STOP condition.
  delayMicroseconds(half_period_usec);
  pinMode(sda_pin_, INPUT);         // NOLINT
  pinMode(sda_pin_, INPUT_PULLUP);  // NOLINT

  recovery_result_ = RECOVERY_COMPLETED;
}

}  // namespace i2c
}  // namespace owif

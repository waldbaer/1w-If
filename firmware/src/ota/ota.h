#ifndef OWIF_OTA_OTA_H
#define OWIF_OTA_OTA_H

#include <ArduinoOTA.h>

#include "ethernet/ethernet.h"
#include "logging/logger.h"
#include "logging/status_led.h"

namespace owif {
namespace ota {

class OtaSystem {
 public:
  OtaSystem() = default;

  auto Begin() -> bool;
  auto Loop() -> void;

 private:
  static constexpr char const* kHostname{"1w-If"};

  auto SetupOta(ethernet::ConnectionState connection_state) -> void;

  logging::StatusLed& status_led_{logging::status_led_g};
  logging::Logger& logger_{logging::logger_g};
};

/*!
 * \brief Declaration of global Ethernet instance.
 */
extern OtaSystem ota_system_g;

}  // namespace ota
}  // namespace owif

#endif  // OWIF_OTA_OTA_H

#ifndef OWIF_OTA_OTA_H
#define OWIF_OTA_OTA_H

#include <ArduinoOTA.h>

#include "ethernet/ethernet.h"
#include "logging/logger.h"

namespace owif {
namespace ota {

class OtaSystem {
 public:
  OtaSystem() = default;

  auto Begin() -> bool;
  auto Loop() -> void;

 private:
  static constexpr char const* kHostname{"1w-If"};
  static constexpr char const* kPassword{"1w-If"};
  static constexpr std::uint16_t kPort{3232};

  auto SetupOta(ethernet::ConnectionState connection_state) -> void;

  logging::Logger& logger_{logging::logger_g};
};

/*!
 * \brief Declaration of global Ethernet instance.
 */
extern OtaSystem ota_system_g;

}  // namespace ota
}  // namespace owif

#endif  // OWIF_OTA_OTA_H

#include "ota/ota.h"

#include "ethernet/ethernet.h"

namespace owif {
namespace ota {

auto OtaSystem::Begin() -> bool {
  bool result{true};
  logger_.Debug(F("[OTA] Setup..."));

  ethernet::ethernet_g.OnConnectionStateChange(
      [this](ethernet::ConnectionState connection_state) { SetupOta(connection_state); });

  return result;
}

auto OtaSystem::Loop() -> void { ArduinoOTA.handle(); }

// ---- Private APIs ---------------------------------------------------------------------------------------------------

auto OtaSystem::SetupOta(ethernet::ConnectionState connection_state) -> void {
  if (connection_state == ethernet::ConnectionState::kConnected) {
    logger_.Info(F("[OTA] Ethernet connected. Setup OTA...  Port: %u"), kPort);

    // OTA Configuration
    ArduinoOTA.setHostname(kHostname);
    ArduinoOTA.setPassword(kPassword);
    ArduinoOTA.setPort(kPort);

    ArduinoOTA.onStart([this]() { logger_.Info(F("[OTA] Starting update...")); });

    ArduinoOTA.setRebootOnSuccess(false);  // Handled by onEnd()
    ArduinoOTA.onEnd([this]() {
      logger_.Info(F("[OTA] Update finished. Restarting ..."));
      logger_.Info(F("[OTA] >> RESTART Hardware <<"));
      ESP.restart();
    });

    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
      static std::uint8_t progress_percent{0};  // static status for compare

      std::uint8_t const new_progress_percent{static_cast<std::uint8_t>((progress * 100 + total / 2) / total)};
      if (new_progress_percent != progress_percent) {
        progress_percent = new_progress_percent;
        logger_.InfoNoLn(F("[OTA] Update in progress: %u%%\n"), progress_percent);
      }
    });

    ArduinoOTA.onError([this](ota_error_t error) { logger_.ErrorNoLn(F("[OTA] Error occurred: %X"), error); });

    ArduinoOTA.begin();

  } else if (connection_state == ethernet::ConnectionState::kDisconnected) {
    logger_.Info(F("[OTA] Ethernet disconnected. Stopping OTA..."));
    ArduinoOTA.end();
  }
}

/*!
 * \brief Definition of global Ethernet instance.
 */
OtaSystem ota_system_g{};

}  // namespace ota
}  // namespace owif

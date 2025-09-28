/*! --------------------------------------------------------------------------------------------------------------------
 * 1w-If (1-Wire Interface)
 * An ESP32-based 1-Wire interface supporting access with physical ethernet and MQTT protocol.
 *
 * Copyright (c) 2025 Sebastian Waldvogel
 * ------------------------------------------------------------------------------------------------------------------ */

#include <Arduino.h>

#include "cmd/command_handler.h"
#include "config/persistency.h"
#include "ethernet/ethernet.h"
#include "logging/logger.h"
#include "mqtt/mqtt_client.h"
#include "mqtt/mqtt_message_handler.h"
#include "one_wire/ds18b20.h"
#include "one_wire/one_wire_subsystem.h"
#include "web_server/web_server.h"

namespace owif {

static char const kOwIfVersion[] PROGMEM = "0.1.0";
static constexpr std::uint32_t kSerialBaudRate{SERIAL_BAUD};  // SERIAL_BAUD defined by build process
static constexpr logging::LogLevel kDefaultLogLevel{logging::LogLevel::Verbose};

std::size_t publish_counter{0};

auto owif_setup() -> void {
  bool setup_result{true};

  setup_result &= logging::logger_g.Begin(kSerialBaudRate, kDefaultLogLevel);

  logging::logger_g.Info(F("-- 1-Wire Interface --------------------"));
  logging::logger_g.Info(F("Version: %s"), kOwIfVersion);
  logging::logger_g.Info(F("----------------------------------------"));

  setup_result &= one_wire::one_wire_system_g.Begin(/* run_initial_scan= */ true);

  setup_result &= cmd::command_handler_g.Begin(&one_wire::one_wire_system_g);

  // Setup WebServer & MqttClient before Ethernet to allow registration of ConnectionStateChangeHandlers
  setup_result &= web_server::web_server_g.Begin();
  setup_result &= mqtt::mqtt_client_g.Begin();
  setup_result &= mqtt::mqtt_msg_handler_g.Begin(&mqtt::mqtt_client_g, &cmd::command_handler_g);

  setup_result &= ethernet::ethernet_g.Begin();

  if (setup_result) {
    logging::logger_g.Info(F("Initialization finished. All sub-subsystems are initialized."));
  } else {
    logging::logger_g.Error(F("Initialization failed. Please check previous outputs. Restarting in 5sec..."));
    delay(5000);
    ESP.restart();
  }
}

auto owif_loop() -> void {
  one_wire::one_wire_system_g.Loop();
  web_server::web_server_g.Loop();
  mqtt::mqtt_client_g.Loop();
  mqtt::mqtt_msg_handler_g.Loop();
  cmd::command_handler_g.Loop();
}

}  // namespace owif

// ---- Arduino Main Entrypoint Functions ------------------------------------------------------------------------------
auto setup() -> void { owif::owif_setup(); }
auto loop() -> void { owif::owif_loop(); }
// ---------------------------------------------------------------------------------------------------------------------

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
#include "logging/multi_logger.h"
#include "logging/web_socket_logger.h"
#include "mqtt/mqtt_client.h"
#include "mqtt/mqtt_message_handler.h"
#include "one_wire/ds18b20.h"
#include "one_wire/one_wire_subsystem.h"
#include "ota/ota.h"
#include "util/abort_handler.h"
#include "web_server/web_server.h"

namespace owif {

static char const kOwIfVersion[] PROGMEM = "0.1.0";
static constexpr std::uint32_t kSerialBaudRate{SERIAL_BAUD};  // SERIAL_BAUD defined by build process

std::size_t publish_counter{0};

auto owif_setup() -> void {
  bool setup_result{true};

  // Setup Logging
  config::LoggingConfig const logging_config{config::persistency_g.LoadLoggingConfig()};
  if (logging_config.GetSerialLogEnabled()) {
    Serial.begin(kSerialBaudRate);
    setup_result &= logging::multi_logger_g.RegisterLogSink(Serial);
  }
  setup_result &= logging::logger_g.Begin(logging::multi_logger_g, logging_config.GetLogLevel());

  // Terminate Handler
  SetupTerminateHandler();

  logging::logger_g.Info(F("-- 1-Wire Interface --------------------"));
  logging::logger_g.Info(F("| Version: %s                        |"), kOwIfVersion);
  logging::logger_g.Info(F("----------------------------------------"));

  setup_result &= one_wire::one_wire_system_g.Begin(/* run_initial_scan= */ true);

  setup_result &= cmd::command_handler_g.Begin(&one_wire::one_wire_system_g);

  // Setup OTA / WebServer / MqttClient before Ethernet to allow registration of ConnectionStateChangeHandlers
  setup_result &= ota::ota_system_g.Begin();
  setup_result &= web_server::web_server_g.Begin(one_wire::one_wire_system_g);
  setup_result &= mqtt::mqtt_client_g.Begin();
  setup_result &= mqtt::mqtt_msg_handler_g.Begin(&mqtt::mqtt_client_g, &cmd::command_handler_g);

  setup_result &= ethernet::ethernet_g.Begin();

  if (logging_config.GetWebLogEnabled()) {
    setup_result &= logging::web_socket_logger_g.Begin(web_server::web_server_g.GetWebSocket());
    setup_result &= logging::multi_logger_g.RegisterLogSink(logging::web_socket_logger_g);
  }

  if (setup_result) {
    logging::logger_g.Info(F("[main] Initialization finished. All sub-subsystems are initialized."));
  } else {
    logging::logger_g.Abort(F("[main] Initialization failed. Please check previous outputs."));
  }
}

auto owif_loop() -> void {
  ota::ota_system_g.Loop();
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

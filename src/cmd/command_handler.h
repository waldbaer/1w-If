#ifndef OWIF_CMD_COMMAND_HANDLER_H
#define OWIF_CMD_COMMAND_HANDLER_H

// ---- Includes ----

#include <Arduino.h>
#include <ArduinoJson.h>

#include "cmd/command.h"
#include "cmd/ds18b20_command_handler.h"
#include "cmd/ds2438_command_handler.h"
#include "cmd/subscriptions_manager.h"
#include "logging/logger.h"
#include "one_wire/one_wire_subsystem.h"

namespace owif {
namespace cmd {

class CommandHandler {
 public:
  CommandHandler() = default;

  CommandHandler(CommandHandler const&) = delete;
  auto operator=(CommandHandler const&) -> CommandHandler& = delete;
  CommandHandler(CommandHandler&&) = delete;
  auto operator=(CommandHandler&&) -> CommandHandler& = delete;

  // ---- Public APIs --------------------------------------------------------------------------------------------------

  auto Begin(one_wire::OneWireSystem* one_wire_system, std::uint32_t command_queue_size = kDefaultCommandQueueSize)
      -> bool;
  auto Loop() -> void;

  auto EnqueueCommand(Command const& cmd) -> bool;

  auto SendCommandResponse(Command const& cmd, JsonDocument const& json) -> void;
  auto SendErrorResponse(Command const& cmd, char const* error_message, char const* request_json = "") -> void;

 private:
  using DeviceMap = one_wire::OneWireSystem::DeviceMap;
  static constexpr std::uint32_t kDefaultCommandQueueSize{100};

  auto ProcessCommandQueue() -> void;

  auto ProcessActionScan(Command& cmd) -> void;
  auto ProcessActionRead(Command& cmd) -> void;
  auto ProcessActionSubscribe(Command& cmd) -> void;
  auto ProcessActionUnsubscribe(Command& cmd) -> void;

  logging::Logger& logger_{logging::logger_g};

  one_wire::OneWireSystem* one_wire_system_;
  QueueHandle_t command_queue_{};

  SubscriptionsManager subscriptions_manager_{nullptr};
  Ds18b20CommandHandler ds18b20_command_handler_{nullptr, nullptr};  // valid init in Begin()
  Ds2438CommandHandler ds2438_command_handler_{nullptr, nullptr};    // valid init in Begin()
};

extern CommandHandler command_handler_g;

}  // namespace cmd
}  // namespace owif

#endif  // OWIF_CMD_COMMAND_HANDLER_H

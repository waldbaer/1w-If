#ifndef OWIF_CMD_PRESENCE_COMMAND_HANDLER_H
#define OWIF_CMD_PRESENCE_COMMAND_HANDLER_H

// ---- Includes ----

#include "cmd/command.h"
#include "logging/logger.h"
#include "one_wire/one_wire_subsystem.h"

namespace owif {
namespace cmd {

class CommandHandler;  // forward declaration due to circular dependency

class PresenceCommandHandler final {
 public:
  PresenceCommandHandler(CommandHandler* command_handler, one_wire::OneWireSystem* one_wire_system);

  PresenceCommandHandler(PresenceCommandHandler const&) = default;
  auto operator=(PresenceCommandHandler const&) -> PresenceCommandHandler& = default;
  PresenceCommandHandler(PresenceCommandHandler&&) = default;
  auto operator=(PresenceCommandHandler&&) -> PresenceCommandHandler& = default;

  ~PresenceCommandHandler() = default;

  // ---- Public APIs --------------------------------------------------------------------------------------------------

  auto ProcessPresenceSingleDevice(Command& cmd, char const* action, bool add_device_attributes) -> void;
  auto ProcessPresenceDeviceFamily(Command& cmd, char const* action, bool add_device_attributes) -> void;
  auto ProcessPresenceScanAll(Command& cmd) -> void;

 private:
  using DeviceMap = one_wire::OneWireSystem::DeviceMap;

  logging::Logger logger_{logging::logger_g};

  CommandHandler* command_handler_;
  one_wire::OneWireSystem* one_wire_system_;
};

}  // namespace cmd
}  // namespace owif

#endif  // OWIF_CMD_PRESENCE_COMMAND_HANDLER_H

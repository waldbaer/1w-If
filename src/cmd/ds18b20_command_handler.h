#ifndef OWIF_CMD_DS18B20_COMMAND_HANDLER_H
#define OWIF_CMD_DS18B20_COMMAND_HANDLER_H

// ---- Includes ----

#include "cmd/command.h"
#include "logging/logger.h"
#include "one_wire/ds18b20.h"
#include "one_wire/one_wire_subsystem.h"

namespace owif {
namespace cmd {

class CommandHandler;  // forward declaration due to circular dependency

class Ds18b20CommandHandler final {
 public:
  Ds18b20CommandHandler(CommandHandler* command_handler, one_wire::OneWireSystem* one_wire_system);

  Ds18b20CommandHandler(Ds18b20CommandHandler const&) = default;
  auto operator=(Ds18b20CommandHandler const&) -> Ds18b20CommandHandler& = default;
  Ds18b20CommandHandler(Ds18b20CommandHandler&&) = default;
  auto operator=(Ds18b20CommandHandler&&) -> Ds18b20CommandHandler& = default;

  // ---- Public APIs --------------------------------------------------------------------------------------------------
  auto ProcessReadSingleDevice(Command& cmd) -> void;

  auto ProcessReadDeviceFamily(Command& cmd) -> void;

 private:
  using DeviceMap = one_wire::OneWireSystem::DeviceMap;

  logging::Logger logger_{logging::logger_g};

  CommandHandler* command_handler_;
  one_wire::OneWireSystem* one_wire_system_;
};

}  // namespace cmd
}  // namespace owif

#endif  // OWIF_CMD_DS18B20_COMMAND_HANDLER_H

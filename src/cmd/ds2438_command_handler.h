#ifndef OWIF_CMD_DS2438_COMMAND_HANDLER_H
#define OWIF_CMD_DS2438_COMMAND_HANDLER_H

// ---- Includes ----

#include "cmd/command.h"
#include "logging/logger.h"
#include "one_wire/ds2438.h"
#include "one_wire/one_wire_subsystem.h"

namespace owif {
namespace cmd {

class CommandHandler;  // forward declaration due to circular dependency

class Ds2438CommandHandler final {
 public:
  Ds2438CommandHandler(CommandHandler* command_handler, one_wire::OneWireSystem* one_wire_system);

  Ds2438CommandHandler(Ds2438CommandHandler const&) = default;
  auto operator=(Ds2438CommandHandler const&) -> Ds2438CommandHandler& = default;
  Ds2438CommandHandler(Ds2438CommandHandler&&) = default;
  auto operator=(Ds2438CommandHandler&&) -> Ds2438CommandHandler& = default;

  ~Ds2438CommandHandler() = default;

  // ---- Public APIs --------------------------------------------------------------------------------------------------
  auto ProcessReadSingleDevice(Command& cmd) -> void;

  auto ProcessReadDeviceFamily(Command& cmd) -> void;

 private:
  using DeviceMap = one_wire::OneWireSystem::DeviceMap;

  auto ProcessDeviceTemperature(Command& cmd, one_wire::Ds2438& ds2438) -> void;
  auto ProcessFamilyTemperature(Command& cmd, one_wire::OneWireAddress::FamilyCode family_code,
                                DeviceMap const& ow_devices) -> void;

  auto ProcessDeviceVAD(Command& cmd, one_wire::Ds2438& ds2438) -> void;
  auto ProcessFamilyVAD(Command& cmd, one_wire::OneWireAddress::FamilyCode family_code, DeviceMap const& ow_devices)
      -> void;

  auto ProcessDeviceVDD(Command& cmd, one_wire::Ds2438& ds2438) -> void;
  auto ProcessFamilyVDD(Command& cmd, one_wire::OneWireAddress::FamilyCode family_code, DeviceMap const& ow_devices)
      -> void;

  auto AddJsonDeviceWithAttribute(JsonDocument& parent, one_wire::OneWireDevice const& device,
                                  char const* attribute_name, float const& attribute_value) -> void;

  auto AddJsonDeviceWithAttribute(JsonArray& parent, one_wire::OneWireDevice const& device, char const* attribute_name,
                                  float const& attribute_value) -> void;

  logging::Logger logger_{logging::logger_g};

  CommandHandler* command_handler_;
  one_wire::OneWireSystem* one_wire_system_;
};

}  // namespace cmd
}  // namespace owif

#endif  // OWIF_CMD_DS2438_COMMAND_HANDLER_H

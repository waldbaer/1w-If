// ---- Includes ----

#include "cmd/presence_command_handler.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#include "cmd/command.h"
#include "cmd/command_handler.h"
#include "cmd/json_builder.h"
#include "cmd/json_constants.h"
#include "one_wire/one_wire_subsystem.h"

namespace owif {
namespace cmd {

PresenceCommandHandler::PresenceCommandHandler(CommandHandler* command_handler,
                                               one_wire::OneWireSystem* one_wire_system)
    : command_handler_{command_handler}, one_wire_system_{one_wire_system} {}

// ---- Public APIs --------------------------------------------------------------------------------------------------
auto PresenceCommandHandler::ProcessPresenceSingleDevice(Command& cmd, char const* action, bool add_device_attributes)
    -> void {
  // ---- Scan for specific device ----
  one_wire::OneWireAddress const& searched_device{cmd.param1.param_value.device_id};

  logger_.Debug(F("[CmdHandler] Processing command 'Scan' [device_id=%s]"), searched_device.Format().c_str());

  bool is_present{false};
  bool const scan_result{one_wire_system_->Scan(searched_device, is_present)};
  if (scan_result) {
    // Add values in the document
    JsonDocument response_json{};
    response_json[json::kRootAction] = action;
    JsonObject json_device{response_json[json::kDevice].to<JsonObject>()};
    json_device[json::kDeviceId] = searched_device.Format().c_str();
    json_device[json::kAttributePresence] = is_present;
    if (add_device_attributes) {
      json::JsonBuilder::AddDeviceAttributes(one_wire_system_, json_device, searched_device);
    }
    command_handler_->SendCommandResponse(cmd, response_json);
  } else {
    command_handler_->SendErrorResponse(cmd, "Failed to scan 1-wire device availability.");
  }
}

auto PresenceCommandHandler::ProcessPresenceDeviceFamily(Command& cmd, char const* action, bool add_device_attributes)
    -> void {
  // ---- Scan for specific device family ----
  one_wire::OneWireAddress::FamilyCode const& searched_family_code{cmd.param2.param_value.family_code};
  logger_.Debug(F("[CmdHandler] Processing command 'Scan' [family_code=%X]"), searched_family_code);

  // Scan bus for specific device family
  bool const scan_result{one_wire_system_->Scan(searched_family_code)};
  if (scan_result) {
    DeviceMap const ow_devices{one_wire_system_->GetAvailableDevices(searched_family_code)};

    JsonDocument response_json{};
    response_json[json::kRootAction] = action;
    response_json[json::kFamilyCode] = searched_family_code;
    // Add values in the document
    JsonArray json_devices{response_json[json::kDevices].to<JsonArray>()};
    for (DeviceMap::value_type const& ow_device : ow_devices) {
      JsonObject json_device{json_devices.add<JsonObject>()};
      json_device[json::kDeviceId] = ow_device.first.Format().c_str();
      json_device[json::kAttributePresence] = true;
      if (add_device_attributes) {
        json::JsonBuilder::AddDeviceAttributes(one_wire_system_, json_device, ow_device.first);
      }
    }
    command_handler_->SendCommandResponse(cmd, response_json);
  } else {
    command_handler_->SendErrorResponse(cmd, "Failed to scan 1-wire device family availability.");
  }
}

auto PresenceCommandHandler::ProcessPresenceScanAll(Command& cmd) -> void {
  // ---- Scan for all available devices ----
  logger_.Debug(F("[CmdHandler] Processing command 'Scan' [all]"));

  bool const scan_result{one_wire_system_->Scan()};  // Scan bus for all available devices
  if (scan_result) {
    DeviceMap const& ow_devices{one_wire_system_->GetAvailableDevices()};

    // Add values in the document
    JsonDocument response_json{};
    response_json[json::kRootAction] = json::kActionScan;
    JsonArray json_devices{response_json[json::kDevices].to<JsonArray>()};
    for (DeviceMap::value_type const& ow_device : ow_devices) {
      JsonObject json_device{json_devices.add<JsonObject>()};
      json_device[json::kAttributePresence] = true;
      json::JsonBuilder::AddDeviceAttributes(one_wire_system_, json_device, ow_device.first);
    }
    command_handler_->SendCommandResponse(cmd, response_json);
  } else {
    command_handler_->SendErrorResponse(cmd, "Failed to scan 1-wire devices availability.");
  }
}
}  // namespace cmd
}  // namespace owif

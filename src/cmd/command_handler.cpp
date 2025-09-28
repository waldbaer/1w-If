#include "cmd/command_handler.h"

#include <ArduinoJson.h>

#include <type_traits>

#include "cmd/ds18b20_command_handler.h"
#include "cmd/ds2438_command_handler.h"
#include "cmd/json_constants.h"

namespace owif {
namespace cmd {

// ---- Public APIs ----------------------------------------------------------------------------------------------------

auto CommandHandler::Begin(one_wire::OneWireSystem* one_wire_system, std::uint32_t command_queue_size) -> bool {
  bool result{true};

  logger_.Debug(F("[CmdHandler] Setup..."));
  one_wire_system_ = one_wire_system;
  command_queue_ = xQueueCreate(command_queue_size, sizeof(cmd::Command));

  subscriptions_manager_ = SubscriptionsManager{this};
  ds18b20_command_handler_ = Ds18b20CommandHandler{this, one_wire_system_};
  ds2438_command_handler_ = Ds2438CommandHandler{this, one_wire_system_};

  return result;
}

auto CommandHandler::Loop() -> void {
  ProcessCommandQueue();
  subscriptions_manager_.Loop();
}

auto CommandHandler::EnqueueCommand(Command const& cmd) -> bool {
  bool result{false};

  BaseType_t const queue_send_result{xQueueSend(command_queue_, &cmd, /* xTicksToWait= */ 0)};
  if (queue_send_result == pdPASS) {
    result = true;
  } else {
    logger_.Error(F("[CmdHandler] Write to command queue failed!"));
  }

  return result;
}

auto CommandHandler::ProcessCommandQueue() -> void {
  cmd::Command cmd{};

  BaseType_t const queue_receive_result{xQueueReceive(command_queue_, &cmd, /* xTicksToWait= */ 0)};

  if (queue_receive_result == pdTRUE) {
    if (cmd.timer.IsExpired()) {
      switch (cmd.action) {
        case cmd::Action::Scan:
          ProcessActionScan(cmd);
          break;
        case cmd::Action::Read:
          ProcessActionRead(cmd);
          break;
        case cmd::Action::Subscribe:
          ProcessActionSubscribe(cmd);
          break;
        case cmd::Action::Unsubscribe:
          ProcessActionUnsubscribe(cmd);
          break;
        default:
          logger_.Error(F("[CmdHandler] Unknown/Unsupport command action type: %d"), cmd.action);
          SendErrorResponse(cmd, "Unknown/Unsupport command action type");
      }
    } else {
      EnqueueCommand(cmd);
    }
  }
}

auto CommandHandler::SendCommandResponse(Command const& cmd, JsonDocument const& json) -> void {
  if (cmd.result_callback.func != nullptr && cmd.result_callback.ctx != nullptr) {
    cmd.result_callback.func(cmd.result_callback.ctx, json);
  } else {
    logger_.Error(F("[CmdHandler] Invalid command result callback / ctx provided"));
  }
}

auto CommandHandler::SendErrorResponse(Command const& cmd, char const* error_message, char const* request_json)
    -> void {
  if (cmd.error_result_callback.func != nullptr && cmd.error_result_callback.ctx != nullptr) {
    cmd.error_result_callback.func(cmd.error_result_callback.ctx, error_message, request_json);
  } else {
    logger_.Error(F("[CmdHandler] Invalid command result callback / ctx provided"));
  }
}

// ---- Private APIs ---------------------------------------------------------------------------------------------------

/*
  param1: [Optional] device_id
  param2: [Optional] family_code
 */
auto CommandHandler::ProcessActionScan(Command& cmd) -> void {
  JsonDocument json{};
  json[json::kRootAction] = json::kActionScan;

  if (cmd.param1.param_available) {
    // ---- Scan for specific device ----
    one_wire::OneWireAddress const& searched_device{cmd.param1.param_value.device_id};

    logger_.Debug(F("[CmdHandler] Processing command 'Scan' [device_id=%s]"), searched_device.Format().c_str());

    bool is_present{false};
    bool const scan_result{one_wire_system_->Scan(searched_device, is_present)};
    if (scan_result) {
      // Add values in the document
      JsonObject json_device{json[json::kDevice].to<JsonObject>()};
      AddDeviceAttributes(json_device, searched_device);
      json_device[json::kActionScanIsPresent] = is_present;
    } else {
      SendErrorResponse(cmd, "Failed to scan 1-wire device availability.");
    }
  } else if (cmd.param2.param_available) {
    // ---- Scan for specific device family ----
    one_wire::OneWireAddress::FamilyCode const& searched_family_code{cmd.param2.param_value.family_code};
    logger_.Debug(F("[CmdHandler] Processing command 'Scan' [family_code=%X]"), searched_family_code);

    // Scan bus for specific device family
    bool const scan_result{one_wire_system_->Scan(searched_family_code)};
    if (scan_result) {
      DeviceMap const ow_devices{one_wire_system_->GetAvailableDevices(searched_family_code)};

      json[json::kFamilyCode] = searched_family_code;
      // Add values in the document
      JsonArray json_devices{json[json::kDevices].to<JsonArray>()};
      for (DeviceMap::value_type const& ow_device : ow_devices) {
        JsonObject json_device{json_devices.add<JsonObject>()};
        AddDeviceAttributes(json_device, ow_device.first);
      }
    } else {
      SendErrorResponse(cmd, "Failed to scan 1-wire device family availability.");
    }
  } else {
    // ---- Scan for all available devices ----
    logger_.Debug(F("[CmdHandler] Processing command 'Scan' [all]"));

    bool const scan_result{one_wire_system_->Scan()};  // Scan bus for all available devices
    if (scan_result) {
      DeviceMap const& ow_devices{one_wire_system_->GetAvailableDevices()};

      // Add values in the document
      JsonArray json_devices{json[json::kDevices].to<JsonArray>()};
      for (DeviceMap::value_type const& ow_device : ow_devices) {
        JsonObject json_device{json_devices.add<JsonObject>()};
        AddDeviceAttributes(json_device, ow_device.first);
      }
    } else {
      SendErrorResponse(cmd, "Failed to scan 1-wire devices availability.");
    }
  }

  SendCommandResponse(cmd, json);
}

/*
  param1: [Optional] device_id
  param2: [Optional] family_code
  param3: device_attribute
 */
auto CommandHandler::ProcessActionRead(Command& cmd) -> void {
  if (cmd.param3.param_available) {
    if (cmd.param1.param_available) {
      // ---- Read a specific device ----
      one_wire::OneWireAddress const& device_addr{cmd.param1.param_value.device_id};
      one_wire::OneWireAddress::FamilyCode const family_code{device_addr.GetFamilyCode()};
      switch (family_code) {
        case one_wire::Ds2438::kFamilyCode:
          ds2438_command_handler_.ProcessReadSingleDevice(cmd);
          break;
        case one_wire::Ds18b20::kFamilyCode:
          ds18b20_command_handler_.ProcessReadSingleDevice(cmd);
          break;
        default:
          SendErrorResponse(cmd, "Action 'read' for device family not supported");
          break;
      }
    } else if (cmd.param2.param_available) {
      // ---- Read a specific device family ----
      one_wire::OneWireAddress::FamilyCode const& family_code{cmd.param2.param_value.family_code};
      switch (family_code) {
        case one_wire::Ds2438::kFamilyCode:
          ds2438_command_handler_.ProcessReadDeviceFamily(cmd);
          break;
        case one_wire::Ds18b20::kFamilyCode:
          ds18b20_command_handler_.ProcessReadDeviceFamily(cmd);
          break;
        default:
          SendErrorResponse(cmd, "Action 'read' for device family not supported");
          break;
      }
    }
  } else {
    SendErrorResponse(cmd, "Missing device_attribute parameter");
  }
}

/*
  param1: [Optional] device_id
  param2: [Optional] family_code
  param3: device_attribute
  param4: interval
 */
auto CommandHandler::ProcessActionSubscribe(Command& cmd) -> void {
  logger_.Debug(F("[CmdHandler] Processing command 'subscribe'"));
  if (cmd.param3.param_available && cmd.param4.param_available) {
    subscriptions_manager_.ProcessActionSubscribe(cmd);
  } else {
    SendErrorResponse(cmd, "Missing device_attribute or interval parameter");
  }
}

/*
  param1: [Optional] device_id
  param2: [Optional] family_code
  param3: device_attribute
 */
auto CommandHandler::ProcessActionUnsubscribe(Command& cmd) -> void {
  logger_.Debug(F("[CmdHandler] Processing command 'unsubscribe'"));
  if (cmd.param3.param_available) {
    subscriptions_manager_.ProcessActionUnsubscribe(cmd);
  } else {
    SendErrorResponse(cmd, "Missing device_attribute parameter");
  }
}

auto CommandHandler::AddDeviceAttributes(JsonObject& json, one_wire::OneWireAddress const& ow_address) -> void {
  json[json::kDeviceId] = ow_address.Format().c_str();

  one_wire::OneWireSystem::DeviceAttributesList devices_attributes{one_wire_system_->GetAttributes(ow_address)};
  if (not devices_attributes.empty()) {
    JsonArray json_device_attributes{json[json::kAttributes].to<JsonArray>()};
    for (String const& attrib : devices_attributes) {
      json_device_attributes.add(attrib);
    }
  }
}

// ---- Global CommandHandler Instance ----
CommandHandler command_handler_g{};

}  // namespace cmd
}  // namespace owif

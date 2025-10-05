#include "cmd/command_handler.h"

#include <ArduinoJson.h>

#include <type_traits>

#include "cmd/ds18b20_command_handler.h"
#include "cmd/ds2438_command_handler.h"
#include "cmd/json_builder.h"
#include "cmd/json_constants.h"

namespace owif {
namespace cmd {

// ---- Public APIs ----------------------------------------------------------------------------------------------------

auto CommandHandler::Begin(one_wire::OneWireSystem* one_wire_system, std::uint32_t command_queue_size) -> bool {
  bool result{true};

  logger_.Debug(F("[CmdHandler] Setup..."));
  one_wire_system_ = one_wire_system;
  command_queue_ = xQueueCreate(command_queue_size, sizeof(cmd::Command));

  presence_command_handler_ = PresenceCommandHandler{this, one_wire_system_};
  ds18b20_command_handler_ = Ds18b20CommandHandler{this, one_wire_system_};
  ds2438_command_handler_ = Ds2438CommandHandler{this, one_wire_system_};
  subscriptions_manager_ = SubscriptionsManager{this};

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
        case cmd::Action::Restart:
          ProcessActionRestart(cmd);
          break;
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
          logger_.Error(F("[CmdHandler] Unknown/Unsupport command action type: %u"), cmd.action);
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

/*!
 * no parameters
 */
auto CommandHandler::ProcessActionRestart(Command& cmd) -> void {
  JsonDocument response_json{};
  response_json[json::kRootAction] = json::kActionRestart;
  response_json[json::kActionRestartAcknowledge] = true;

  SendCommandResponse(cmd, response_json);

  logger_.Info(F("[CmdHandler] >> RESTART Hardware << (requested via remote command)"));
  ESP.restart();
}

/*!
 * param1: [Optional] device_id
 * param2: [Optional] family_code
 */
auto CommandHandler::ProcessActionScan(Command& cmd) -> void {
  if (cmd.param1.param_available) {
    presence_command_handler_.ProcessPresenceSingleDevice(cmd);
  } else if (cmd.param2.param_available) {
    presence_command_handler_.ProcessPresenceDeviceFamily(cmd);
  } else {
    presence_command_handler_.ProcessPresenceScanAll(cmd);
  }
}

/*!
 * param1: [Optional] device_id
 * param2: [Optional] family_code
 * param3: device_attribute
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

/*!
 * param1: [Optional] device_id
 * param2: [Optional] family_code
 * param3: device_attribute
 * param4: interval
 */
auto CommandHandler::ProcessActionSubscribe(Command& cmd) -> void {
  logger_.Debug(F("[CmdHandler] Processing command 'subscribe'"));
  if (cmd.param3.param_available && cmd.param4.param_available) {
    subscriptions_manager_.ProcessActionSubscribe(cmd);
  } else {
    SendErrorResponse(cmd, "Missing device_attribute or interval parameter");
  }
}

/*!
 * param1: [Optional] device_id
 * param2: [Optional] family_code
 * param3: device_attribute
 */
auto CommandHandler::ProcessActionUnsubscribe(Command& cmd) -> void {
  logger_.Debug(F("[CmdHandler] Processing command 'unsubscribe'"));
  if (cmd.param3.param_available) {
    subscriptions_manager_.ProcessActionUnsubscribe(cmd);
  } else {
    SendErrorResponse(cmd, "Missing device_attribute parameter");
  }
}

// ---- Global CommandHandler Instance ----
CommandHandler command_handler_g{};

}  // namespace cmd
}  // namespace owif

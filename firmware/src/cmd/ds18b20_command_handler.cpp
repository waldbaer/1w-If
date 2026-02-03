// ---- Includes ----

#include "cmd/ds18b20_command_handler.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#include "cmd/command.h"
#include "cmd/command_handler.h"
#include "cmd/json_constants.h"
#include "one_wire/ds18b20.h"
#include "one_wire/one_wire_subsystem.h"

namespace owif {
namespace cmd {

Ds18b20CommandHandler::Ds18b20CommandHandler(CommandHandler* command_handler, one_wire::OneWireSystem* one_wire_system)
    : command_handler_{command_handler}, one_wire_system_{one_wire_system} {}

// ---- Public APIs --------------------------------------------------------------------------------------------------
auto Ds18b20CommandHandler::ProcessReadSingleDevice(Command& cmd) -> void {
  one_wire::OneWireAddress const& device_addr{cmd.param1.param_value.device_id};

  if (cmd.param3.param_value.device_attribute == DeviceAttributeType::Presence) {
    JsonDocument response_json{};
    response_json[json::kRootAction] = json::kActionRead;
    JsonObject json_device{response_json[json::kDevice].to<JsonObject>()};

    bool is_present{false};
    one_wire::OneWireBus::BusId bus_id{0};
    bool const scan_result{one_wire_system_->Scan(device_addr, is_present, bus_id)};

    if (is_present) {
      json_device[json::kChannel] = bus_id;
    }
    json_device[json::kDeviceId] = device_addr.Format().c_str();
    json_device[json::kAttributePresence] = is_present;

    command_handler_->SendCommandResponse(cmd, response_json);
  } else if (cmd.param3.param_value.device_attribute == DeviceAttributeType::Temperature) {
    logger_.Debug(F("[DS18B20 CmdHandler] Processing command 'read' [sub_action=%u][device_id=%s]"), cmd.sub_action,
                  device_addr.Format().c_str());

    std::shared_ptr<one_wire::OneWireDevice> ow_device{one_wire_system_->GetAvailableDevice(device_addr)};
    if (ow_device) {
      if (one_wire::Ds18b20::MatchesFamily(*ow_device)) {
        one_wire::Ds18b20* ds18b20{one_wire::Ds18b20::FromDevice(*ow_device)};

        // Sub-Action Handling
        if (cmd.sub_action == SubAction::None || cmd.sub_action == SubAction::TriggerSampling) {
          bool const sample_result{ds18b20->SampleTemperature()};
          if (sample_result) {
            cmd.timer.Reset(ds18b20->GetSamplingTime());
            cmd.sub_action = SubAction::ReadResult;
            command_handler_->EnqueueCommand(cmd);
          } else {
            command_handler_->SendErrorResponse(cmd, "Failed to start DS18B20 temperature sampling.");
          }
        } else if (cmd.sub_action == SubAction::ReadResult) {
          float sampled_temperature{0};
          bool const get_temp_result{ds18b20->GetTemperature(sampled_temperature)};
          if (get_temp_result) {
            JsonDocument response_json{};
            response_json[json::kRootAction] = json::kActionRead;

            JsonObject json_device{response_json[json::kDevice].to<JsonObject>()};
            json_device[json::kChannel] = ow_device->GetBusId();
            json_device[json::kDeviceId] = ow_device->GetAddress().Format().c_str();
            json_device[json::kActionReadAttributeTemperature] = sampled_temperature;
            command_handler_->SendCommandResponse(cmd, response_json);
          } else {
            command_handler_->SendErrorResponse(cmd, "Failed to get DS18B20 temperature.");
          }
        } else {
          logger_.Error(F("[DS18B20 CmdHandler] Unknown sub-action state %u"), cmd.sub_action);
        }
      } else {
        command_handler_->SendErrorResponse(cmd, "Failed to access DS18B20 1-Wire device.");
      }
    } else {
      command_handler_->SendErrorResponse(
          cmd, "1-Wire device not found / available. Manual scan might be necessary before.");
    }
  } else {
    command_handler_->SendErrorResponse(cmd, "Unsupported device attribute for DS18B20.");
  }
}

auto Ds18b20CommandHandler::ProcessReadDeviceFamily(Command& cmd) -> void {
  if (cmd.param3.param_value.device_attribute == DeviceAttributeType::Temperature) {
    one_wire::OneWireAddress::FamilyCode const& family_code{cmd.param2.param_value.family_code};
    logger_.Debug(F("[DS18B20 CmdHandler] Processing command 'read' [sub_action=%u][family_code=%X]"), cmd.sub_action,
                  family_code);

    DeviceMap const ow_devices{one_wire_system_->GetAvailableDevices(family_code)};

    if (not ow_devices.empty()) {
      JsonDocument response_json{};
      response_json[json::kRootAction] = json::kActionRead;
      response_json[json::kFamilyCode] = family_code;
      JsonArray json_devices{response_json[json::kDevices].to<JsonArray>()};

      if (cmd.sub_action == SubAction::None || cmd.sub_action == SubAction::TriggerSampling) {
        bool sample_result{true};
        for (std::reference_wrapper<one_wire::OneWireBus> ow_bus : one_wire_system_->GetAvailableBuses()) {
          logger_.Verbose(F("[DS18B20 CmdHandler] Trigger temperature sampling on 1-wire bus %u"),
                          ow_bus.get().GetId());
          one_wire::Ds18b20 dummy_ds18b20{ow_bus.get(), one_wire::OneWireAddress{0}};
          sample_result &= dummy_ds18b20.SampleTemperature(/* skip_rom_select= */ true);
        }

        if (sample_result) {
          cmd.timer.Reset(one_wire::Ds18b20::kWorstCaseSamplingTime);
          cmd.sub_action = SubAction::ReadResult;
          command_handler_->EnqueueCommand(cmd);
        } else {
          command_handler_->SendErrorResponse(cmd, "Failed to start DS18B20 temperature sampling.");
        }
      } else if (cmd.sub_action == SubAction::ReadResult) {
        for (DeviceMap::value_type const& ow_device : ow_devices) {
          one_wire::Ds18b20* ds18b20{one_wire::Ds18b20::FromDevice(*ow_device.second)};

          float sampled_temperature{0};
          bool const get_temp_result{ds18b20->GetTemperature(sampled_temperature)};
          if (get_temp_result) {
            JsonObject json_device{json_devices.add<JsonObject>()};

            json_device[json::kChannel] = ds18b20->GetBusId();
            json_device[json::kDeviceId] = ds18b20->GetAddress().Format().c_str();
            json_device[json::kActionReadAttributeTemperature] = sampled_temperature;
          } else {
            command_handler_->SendErrorResponse(cmd, "Failed to get DS18B20 temperature.");
            return;
          }
        }
        command_handler_->SendCommandResponse(cmd, response_json);
      } else {
        logger_.Error(F("[DS18B20 CmdHandler] Unknown sub-action state"), cmd.sub_action);
      }
    }

  } else {
    command_handler_->SendErrorResponse(cmd, "Unsupported device attribute for DS18B20.");
  }
}

}  // namespace cmd
}  // namespace owif

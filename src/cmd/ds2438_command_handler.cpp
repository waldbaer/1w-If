// ---- Includes ----

#include "cmd/ds2438_command_handler.h"

#include <Arduino.h>
#include <ArduinoJson.h>

#include "cmd/command.h"
#include "cmd/command_handler.h"
#include "cmd/json_constants.h"
#include "one_wire/ds2438.h"
#include "one_wire/one_wire_address.h"
#include "one_wire/one_wire_subsystem.h"

namespace owif {
namespace cmd {

Ds2438CommandHandler::Ds2438CommandHandler(CommandHandler* command_handler, one_wire::OneWireSystem* one_wire_system)
    : command_handler_{command_handler}, one_wire_system_{one_wire_system} {}

// ---- Public APIs --------------------------------------------------------------------------------------------------
auto Ds2438CommandHandler::ProcessReadSingleDevice(Command& cmd) -> void {
  one_wire::OneWireAddress const& device_addr{cmd.param1.param_value.device_id};
  logger_.Debug(F("[DS2438 CmdHandler] Processing command 'read' [sub_action=%u][device_id=%s]"), cmd.sub_action,
                device_addr.Format().c_str());
  std::shared_ptr<one_wire::OneWireDevice> ow_device{one_wire_system_->GetAvailableDevice(device_addr)};

  if (ow_device) {
    if (one_wire::Ds2438::MatchesFamily(*ow_device)) {
      one_wire::Ds2438* ds2438{one_wire::Ds2438::FromDevice(*ow_device)};

      if (cmd.param3.param_value.device_attribute == DeviceAttributeType::Temperature) {
        ProcessDeviceTemperature(cmd, *ds2438);
      } else if (cmd.param3.param_value.device_attribute == DeviceAttributeType::VAD) {
        ProcessDeviceVAD(cmd, *ds2438);
      } else if (cmd.param3.param_value.device_attribute == DeviceAttributeType::VDD) {
        ProcessDeviceVDD(cmd, *ds2438);
      } else {
        command_handler_->SendErrorResponse(cmd, "Unsupported device attribute for DS2438.");
      }
    } else {
      command_handler_->SendErrorResponse(cmd, "Failed to access DS2438 1-Wire device.");
    }
  } else {
    command_handler_->SendErrorResponse(cmd,
                                        "1-Wire device not found / available. Manual scan might be necessary before.");
  }
}

auto Ds2438CommandHandler::ProcessReadDeviceFamily(Command& cmd) -> void {
  one_wire::OneWireAddress::FamilyCode const& family_code{cmd.param2.param_value.family_code};
  logger_.Debug(F("[DS2438 CmdHandler] Processing command 'read' [sub_action=%u][family_code=%X]"), cmd.sub_action,
                family_code);
  DeviceMap const ow_devices{one_wire_system_->GetAvailableDevices(family_code)};

  if (not ow_devices.empty()) {
    if (cmd.param3.param_value.device_attribute == DeviceAttributeType::Temperature) {
      ProcessFamilyTemperature(cmd, family_code, ow_devices);
    } else if (cmd.param3.param_value.device_attribute == DeviceAttributeType::VAD) {
      ProcessFamilyVAD(cmd, family_code, ow_devices);
    } else if (cmd.param3.param_value.device_attribute == DeviceAttributeType::VDD) {
      ProcessFamilyVDD(cmd, family_code, ow_devices);
    } else {
      command_handler_->SendErrorResponse(cmd, "Unsupported device attribute for DS2438.");
    }
  }
}

// ---- Private APIs ---------------------------------------------------------------------------------------------------

auto Ds2438CommandHandler::ProcessDeviceTemperature(Command& cmd, one_wire::Ds2438& ds2438) -> void {
  // Sub-Action Handling
  if (cmd.sub_action == SubAction::None || cmd.sub_action == SubAction::TriggerSampling) {
    bool const sample_result{ds2438.SampleTemperature()};
    if (sample_result) {
      cmd.timer.Reset(one_wire::Ds2438::kSamplingTime);
      cmd.sub_action = SubAction::ReadResult;
      command_handler_->EnqueueCommand(cmd);
    } else {
      command_handler_->SendErrorResponse(cmd, "Failed to start DS2438 temperature sampling.");
    }
  } else if (cmd.sub_action == SubAction::ReadResult) {
    float sampled_temperature{0};
    bool const get_temperature_result{ds2438.GetTemperature(sampled_temperature)};
    if (get_temperature_result) {
      JsonDocument response_json{};
      response_json[json::kRootAction] = json::kActionRead;
      AddJsonDeviceWithAttribute(response_json, ds2438.GetAddress(), json::kActionReadAttributeTemperature,
                                 sampled_temperature);
      command_handler_->SendCommandResponse(cmd, response_json);
    } else {
      command_handler_->SendErrorResponse(cmd, "Failed to get DS2438 temperature.");
    }
  } else {
    logger_.Error(F("[DS2438 CmdHandler] Unknown sub-action state %u"), cmd.sub_action);
  }
}

auto Ds2438CommandHandler::ProcessFamilyTemperature(Command& cmd, one_wire::OneWireAddress::FamilyCode family_code,
                                                    DeviceMap const& ow_devices) -> void {
  if (cmd.sub_action == SubAction::None || cmd.sub_action == SubAction::TriggerSampling) {
    bool sample_result{true};
    for (std::reference_wrapper<one_wire::OneWireBus> ow_bus : one_wire_system_->GetAvailableBuses()) {
      logger_.Verbose(F("[DS2438 CmdHandler] Trigger temperature sampling on 1-wire bus %u"), ow_bus.get().GetId());
      one_wire::Ds2438 dummy_ds2438{ow_bus.get(), one_wire::OneWireAddress{0}};
      sample_result &= dummy_ds2438.SampleTemperature(/* skip_rom_select= */ true);
    }

    if (sample_result) {
      cmd.timer.Reset(one_wire::Ds2438::kSamplingTime);
      cmd.sub_action = SubAction::ReadResult;
      command_handler_->EnqueueCommand(cmd);
    } else {
      command_handler_->SendErrorResponse(cmd, "Failed to start DS2438 temperature sampling.");
    }
  } else if (cmd.sub_action == SubAction::ReadResult) {
    JsonDocument response_json{};
    response_json[json::kRootAction] = json::kActionRead;
    response_json[json::kFamilyCode] = family_code;
    JsonArray json_devices{response_json[json::kDevices].to<JsonArray>()};

    for (DeviceMap::value_type const& ow_device : ow_devices) {
      one_wire::Ds2438* ds2438{one_wire::Ds2438::FromDevice(*ow_device.second)};

      float sampled_temperature{0};
      bool const get_temp_result{ds2438->GetTemperature(sampled_temperature)};
      if (get_temp_result) {
        AddJsonDeviceWithAttribute(json_devices, ds2438->GetAddress(), json::kActionReadAttributeTemperature,
                                   sampled_temperature);
      } else {
        command_handler_->SendErrorResponse(cmd, "Failed to get DS2438 temperature.");
        return;
      }
    }
    command_handler_->SendCommandResponse(cmd, response_json);
  } else {
    logger_.Error(F("[DS2438 CmdHandler] Unknown sub-action state"), cmd.sub_action);
  }
}

auto Ds2438CommandHandler::ProcessDeviceVAD(Command& cmd, one_wire::Ds2438& ds2438) -> void {
  // Sub-Action Handling
  if (cmd.sub_action == SubAction::None || cmd.sub_action == SubAction::TriggerSampling) {
    bool const sample_result{ds2438.SampleVAD()};
    if (sample_result) {
      cmd.timer.Reset(one_wire::Ds2438::kSamplingTime);
      cmd.sub_action = SubAction::ReadResult;
      command_handler_->EnqueueCommand(cmd);
    } else {
      command_handler_->SendErrorResponse(cmd, "Failed to start DS2438 VAD sampling.");
    }
  } else if (cmd.sub_action == SubAction::ReadResult) {
    float sampled_vad{0};
    bool const get_vad_result{ds2438.GetVAD(sampled_vad)};
    if (get_vad_result) {
      JsonDocument response_json{};
      response_json[json::kRootAction] = json::kActionRead;
      AddJsonDeviceWithAttribute(response_json, ds2438.GetAddress(), json::kActionReadAttributeVAD, sampled_vad);
      command_handler_->SendCommandResponse(cmd, response_json);
    } else {
      command_handler_->SendErrorResponse(cmd, "Failed to get DS2438 VAD.");
    }
  } else {
    logger_.Error(F("[DS2438 CmdHandler] Unknown sub-action state %u"), cmd.sub_action);
  }
}

auto Ds2438CommandHandler::ProcessFamilyVAD(Command& cmd, one_wire::OneWireAddress::FamilyCode family_code,
                                            DeviceMap const& ow_devices) -> void {
  if (cmd.sub_action == SubAction::None || cmd.sub_action == SubAction::TriggerSampling) {
    // No skip_rom possible as every DS2438 config bit selecting VAD or VDD sampling must be configured before
    // sampling is started. Instead trigger every known device individually.
    bool sample_result{true};
    for (DeviceMap::value_type const& ow_device : ow_devices) {
      one_wire::Ds2438* ds2438{one_wire::Ds2438::FromDevice(*ow_device.second)};
      sample_result &= ds2438->SampleVAD();
    }

    if (sample_result) {
      cmd.timer.Reset(one_wire::Ds2438::kSamplingTime);
      cmd.sub_action = SubAction::ReadResult;
      command_handler_->EnqueueCommand(cmd);
    } else {
      command_handler_->SendErrorResponse(cmd, "Failed to start DS2438 VAD sampling.");
    }
  } else if (cmd.sub_action == SubAction::ReadResult) {
    JsonDocument response_json{};
    response_json[json::kRootAction] = json::kActionRead;
    response_json[json::kFamilyCode] = family_code;
    JsonArray json_devices{response_json[json::kDevices].to<JsonArray>()};

    for (DeviceMap::value_type const& ow_device : ow_devices) {
      one_wire::Ds2438* ds2438{one_wire::Ds2438::FromDevice(*ow_device.second)};

      float sampled_vad{0};
      bool const get_vad_result{ds2438->GetVAD(sampled_vad)};
      if (get_vad_result) {
        AddJsonDeviceWithAttribute(json_devices, ds2438->GetAddress(), json::kActionReadAttributeVAD, sampled_vad);
      } else {
        command_handler_->SendErrorResponse(cmd, "Failed to get DS2438 VAD.");
        return;
      }
    }
    command_handler_->SendCommandResponse(cmd, response_json);
  } else {
    logger_.Error(F("[DS2438 CmdHandler] Unknown sub-action state"), cmd.sub_action);
  }
}

auto Ds2438CommandHandler::ProcessDeviceVDD(Command& cmd, one_wire::Ds2438& ds2438) -> void {
  // Sub-Action Handling
  if (cmd.sub_action == SubAction::None || cmd.sub_action == SubAction::TriggerSampling) {
    bool const sample_result{ds2438.SampleVDD()};
    if (sample_result) {
      cmd.timer.Reset(one_wire::Ds2438::kSamplingTime);
      cmd.sub_action = SubAction::ReadResult;
      command_handler_->EnqueueCommand(cmd);
    } else {
      command_handler_->SendErrorResponse(cmd, "Failed to start DS2438 VDD sampling.");
    }
  } else if (cmd.sub_action == SubAction::ReadResult) {
    float sampled_vdd{0};
    bool const get_vdd_result{ds2438.GetVDD(sampled_vdd)};
    if (get_vdd_result) {
      JsonDocument response_json{};
      response_json[json::kRootAction] = json::kActionRead;
      AddJsonDeviceWithAttribute(response_json, ds2438.GetAddress(), json::kActionReadAttributeVDD, sampled_vdd);
      command_handler_->SendCommandResponse(cmd, response_json);
    } else {
      command_handler_->SendErrorResponse(cmd, "Failed to get DS2438 VDD.");
    }
  } else {
    logger_.Error(F("[DS2438 CmdHandler] Unknown sub-action state"), cmd.sub_action);
  }
}

auto Ds2438CommandHandler::ProcessFamilyVDD(Command& cmd, one_wire::OneWireAddress::FamilyCode family_code,
                                            DeviceMap const& ow_devices) -> void {
  if (cmd.sub_action == SubAction::None || cmd.sub_action == SubAction::TriggerSampling) {
    // No skip_rom possible as every DS2438 config bit selecting VAD or VDD sampling must be configured before
    // sampling is started. Instead trigger every known device individually.
    bool sample_result{true};
    for (DeviceMap::value_type const& ow_device : ow_devices) {
      one_wire::Ds2438* ds2438{one_wire::Ds2438::FromDevice(*ow_device.second)};
      sample_result &= ds2438->SampleVDD();
    }

    if (sample_result) {
      cmd.timer.Reset(one_wire::Ds2438::kSamplingTime);
      cmd.sub_action = SubAction::ReadResult;
      command_handler_->EnqueueCommand(cmd);
    } else {
      command_handler_->SendErrorResponse(cmd, "Failed to start DS2438 VDD sampling.");
    }
  } else if (cmd.sub_action == SubAction::ReadResult) {
    JsonDocument response_json{};
    response_json[json::kRootAction] = json::kActionRead;
    response_json[json::kFamilyCode] = family_code;
    JsonArray json_devices{response_json[json::kDevices].to<JsonArray>()};

    for (DeviceMap::value_type const& ow_device : ow_devices) {
      one_wire::Ds2438* ds2438{one_wire::Ds2438::FromDevice(*ow_device.second)};

      float sampled_vdd{0};
      bool const get_temp_result{ds2438->GetVDD(sampled_vdd)};
      if (get_temp_result) {
        AddJsonDeviceWithAttribute(json_devices, ds2438->GetAddress(), json::kActionReadAttributeVDD, sampled_vdd);
      } else {
        command_handler_->SendErrorResponse(cmd, "Failed to get DS2438 VDD.");
        return;
      }
    }
    command_handler_->SendCommandResponse(cmd, response_json);
  } else {
    logger_.Error(F("[DS2438 CmdHandler] Unknown sub-action state"), cmd.sub_action);
  }
}

auto Ds2438CommandHandler::AddJsonDeviceWithAttribute(JsonDocument& parent,
                                                      one_wire::OneWireAddress const& device_address,
                                                      char const* attribute_name, float const& attribute_value)
    -> void {
  JsonObject json_device{parent[json::kDevice].to<JsonObject>()};
  json_device[json::kDeviceId] = device_address.Format().c_str();
  json_device[attribute_name] = attribute_value;
}

auto Ds2438CommandHandler::AddJsonDeviceWithAttribute(JsonArray& parent, one_wire::OneWireAddress const& device_address,
                                                      char const* attribute_name, float const& attribute_value)
    -> void {
  JsonObject json_device{parent.add<JsonObject>()};
  json_device[json::kDeviceId] = device_address.Format().c_str();
  json_device[attribute_name] = attribute_value;
}

}  // namespace cmd
}  // namespace owif

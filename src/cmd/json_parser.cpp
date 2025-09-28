#include "cmd/json_parser.h"

#include "cmd/json_constants.h"
#include "logging/logger.h"

namespace owif {
namespace cmd {
namespace json {

// ---- Public APIs ----------------------------------------------------------------------------------------------------

auto JsonParser::ParseAddressing(JsonDocument const& json, CommandParam& device_id_param,
                                 CommandParam& family_code_param, bool any_attribute_required) -> bool {
  bool result{false};
  logging::Logger& logger{logging::logger_g};

  bool const has_device_id_param{json[cmd::json::kDeviceId].is<String>()};
  bool const has_family_code_param{json[cmd::json::kFamilyCode].is<one_wire::OneWireAddress::FamilyCode>()};

  if (has_device_id_param || has_family_code_param) {
    // DeviceId or family_code must be exclusively set
    if (has_device_id_param ^ has_family_code_param) {
      if (has_device_id_param) {
        // Scan for availability of a specific device
        String device_id{json[cmd::json::kDeviceId].as<String>()};

        std::unique_ptr<one_wire::OneWireAddress> ow_address{one_wire::OneWireAddress::FromOwfsFormat(device_id)};
        if (ow_address != nullptr) {
          device_id_param.param_available = true;
          device_id_param.param_value = cmd::CommandParamValue{.device_id = *ow_address};

          result = true;
        } else {
          logger.Error(F("[JsonParser] Failed to parse device_id string: %s"), device_id);
        }

      } else if (has_family_code_param) {
        one_wire::OneWireAddress::FamilyCode const family_code{
            json[cmd::json::kFamilyCode].as<one_wire::OneWireAddress::FamilyCode>()};

        family_code_param.param_available = true;
        family_code_param.param_value = cmd::CommandParamValue{.family_code = family_code};

        result = true;
      } else {
        // not possible due to previous checks
      }
    } else {
      logger.Error(F("[JsonParser] device_id or family_code must be exclusively available"));
    }
  } else {
    if (not any_attribute_required) {
      result = true;
    } else {
      logger.Error(F("[JsonParser] device_id or family_code must be set"));
    }
  }

  return result;
}

auto JsonParser::ParseDeviceAttribute(JsonDocument const& json, CommandParam& cmd_param) -> bool {
  bool result{true};

  bool const has_attribute_param{json[cmd::json::kAttribute].is<String>()};

  if (has_attribute_param) {
    String const attribute_string{json[cmd::json::kAttribute].as<String>()};

    if (attribute_string == kActionReadAttributeTemperature) {
      cmd_param.param_available = true;
      cmd_param.param_value.device_attribute = DeviceAttributeType::Temperature;
    } else if (attribute_string == kActionReadAttributeVAD) {
      cmd_param.param_available = true;
      cmd_param.param_value.device_attribute = DeviceAttributeType::VAD;
    } else if (attribute_string == kActionReadAttributeVDD) {
      cmd_param.param_available = true;
      cmd_param.param_value.device_attribute = DeviceAttributeType::VDD;
    } else {
      result = false;
    }
  } else {
    result = false;
  }
  return result;
}

}  // namespace json
}  // namespace cmd
}  // namespace owif

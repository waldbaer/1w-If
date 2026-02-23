#include "cmd/json_builder.h"

#include <Arduino.h>

#include "cmd/json_constants.h"
#include "one_wire/one_wire_subsystem.h"
#include "time/date_time.h"
#include "time/time_util.h"

namespace owif {
namespace cmd {
namespace json {

// ---- Public APIs ----------------------------------------------------------------------------------------------------

auto JsonBuilder::AddTimestamp(JsonDocument& json) -> void {
  time::DateTime const now{time::TimeUtil::Now()};
  time::FormattedTimeString formatted_timestamp{};
  time::TimeUtil::Format(now, formatted_timestamp);

  json[cmd::json::kTime] = formatted_timestamp;
}

auto JsonBuilder::AddDeviceAttributes(one_wire::OneWireSystem* one_wire_system, JsonObject& json,
                                      one_wire::OneWireAddress const& ow_address) -> void {
  JsonArray json_device_attributes{json[json::kAttributes].to<JsonArray>()};

  one_wire::OneWireSystem::DeviceAttributesList const devices_attributes{one_wire_system->GetAttributes(ow_address)};
  for (String const& attrib : devices_attributes) {
    json_device_attributes.add(attrib);
  }
}

}  // namespace json
}  // namespace cmd
}  // namespace owif

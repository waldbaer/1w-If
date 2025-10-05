#include "cmd/json_builder.h"

#include <Arduino.h>

#include "cmd/json_constants.h"
#include "one_wire/one_wire_subsystem.h"

namespace owif {
namespace cmd {
namespace json {

// ---- Public APIs ----------------------------------------------------------------------------------------------------

auto JsonBuilder::AddDeviceAttributes(one_wire::OneWireSystem* one_wire_system, JsonObject& json,
                                      one_wire::OneWireAddress const& ow_address) -> void {
  one_wire::OneWireSystem::DeviceAttributesList devices_attributes{one_wire_system->GetAttributes(ow_address)};
  if (not devices_attributes.empty()) {
    JsonArray json_device_attributes{json[json::kAttributes].to<JsonArray>()};
    for (String const& attrib : devices_attributes) {
      json_device_attributes.add(attrib);
    }
  }
}

}  // namespace json
}  // namespace cmd
}  // namespace owif

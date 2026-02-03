#ifndef OWIF_CMD_JSON_BUILDER_H
#define OWIF_CMD_JSON_BUILDER_H

// ---- Includes ----

#include "ArduinoJson.h"
#include "one_wire/one_wire_address.h"
#include "one_wire/one_wire_subsystem.h"

namespace owif {
namespace cmd {
namespace json {

class JsonBuilder final {
 public:
  JsonBuilder() = delete;
  JsonBuilder(JsonBuilder const&) = delete;
  auto operator=(JsonBuilder const&) -> JsonBuilder& = delete;
  JsonBuilder(JsonBuilder&&) = delete;
  auto operator=(JsonBuilder&&) -> JsonBuilder& = delete;

  ~JsonBuilder() = delete;

  static auto AddDeviceAttributes(one_wire::OneWireSystem* one_wire_system, JsonObject& json,
                                  one_wire::OneWireAddress const& ow_address) -> void;
};

}  // namespace json
}  // namespace cmd
}  // namespace owif

#endif  // OWIF_CMD_JSON_BUILDER_H

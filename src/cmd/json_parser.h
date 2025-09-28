#ifndef OWIF_CMD_JSON_PARSER_H
#define OWIF_CMD_JSON_PARSER_H

// ---- Includes ----
#include <Arduino.h>

#include <cstdint>

#include "cmd/command.h"

namespace owif {
namespace cmd {
namespace json {

class JsonParser final {
 public:
  JsonParser() = delete;
  JsonParser(JsonParser const&) = delete;
  auto operator=(JsonParser const&) -> JsonParser& = delete;
  JsonParser(JsonParser&&) = delete;
  auto operator=(JsonParser&&) -> JsonParser& = delete;

  static auto ParseAddressing(JsonDocument const& json, CommandParam& device_id_param, CommandParam& family_code_param,
                              bool any_attribute_required) -> bool;

  static auto ParseDeviceAttribute(JsonDocument const& json, CommandParam& cmd_param) -> bool;
};

}  // namespace json
}  // namespace cmd
}  // namespace owif

#endif  // OWIF_CMD_JSON_PARSER_H

#ifndef OWIF_CMD_COMMAND_TYPES_H
#define OWIF_CMD_COMMAND_TYPES_H

// ---- Includes ----
#include <ArduinoJson.h>

#include <cstdint>

#include "cmd/timer.h"
#include "one_wire/one_wire_address.h"

namespace owif {
namespace cmd {

enum class Action : std::uint8_t {
  Restart = 0x00,
  Scan = 0x01,
  Read = 0x02,
  Subscribe = 0x03,
  Unsubscribe = 0x04,
};

enum class SubAction : std::uint8_t {
  None = 0x00,
  TriggerSampling = 0x01,  // e.g. DS18B20: Start temperature sampling
  ReadResult = 0x02,       // e.g. DS18B20: Read sampled temperature after sampling time
};

enum class DeviceAttributeType : std::uint8_t {
  Presence = 0x00,
  Temperature = 0x01,
  VAD = 0x02,
  VDD = 0x03,
};

struct TimeIntervalType {
  using type = std::uint32_t;
  type value;
};

union CommandParamValue {
  one_wire::OneWireAddress device_id;
  one_wire::OneWireAddress::FamilyCode family_code;
  DeviceAttributeType device_attribute;
  TimeIntervalType interval;
};

struct CommandParam {
  bool param_available;           // Flag indicating if the optional parameter is available or not
  CommandParamValue param_value;  // The parameter value
};

struct CommandResultCallback {
  void (*func)(void* ctx, JsonDocument const& command_result);
  void* ctx;
};
struct ErrorResultCallback {
  void (*func)(void* ctx, char const* error_message, char const* request_json);
  void* ctx;
};

struct Command {
  Timer timer;
  Action action;
  SubAction sub_action;
  CommandParam param1;
  CommandParam param2;
  CommandParam param3;
  CommandParam param4;
  CommandResultCallback result_callback;
  ErrorResultCallback error_result_callback;
};

// Check that commands are trivially copyable. Required for command queue.
static_assert(std::is_trivially_copyable<Command>::value);

}  // namespace cmd
}  // namespace owif

#endif  // OWIF_CMD_COMMAND_TYPES_H

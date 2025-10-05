#ifndef OWIF_CMD_JSON_CONSTANTS_H
#define OWIF_CMD_JSON_CONSTANTS_H

// ---- Includes ----
#include <ArduinoJson.h>

#include <cstdint>

#include "one_wire/one_wire_address.h"

namespace owif {
namespace cmd {
namespace json {

// Error Handling
static constexpr char const* kRootError{"error"};
static constexpr char const* kErrorMessage{"message"};
static constexpr char const* kErrorRequest{"request"};

// Actions
static constexpr char const* kRootAction{"action"};

static constexpr char const* kActionRestart{"restart"};
static constexpr char const* kActionRestartAcknowledge{"acknowledge"};

static constexpr char const* kActionScan{"scan"};
static constexpr char const* kAttributePresence{"presence"};

static constexpr char const* kActionRead{"read"};
static constexpr char const* kActionReadAttributeTemperature{"temperature"};
static constexpr char const* kActionReadAttributeVAD{"VAD"};
static constexpr char const* kActionReadAttributeVDD{"VDD"};

static constexpr char const* kActionSubscribe{"subscribe"};
static constexpr char const* kActionSubscribeInterval{"interval"};
static constexpr char const* kActionSubscribeAcknowledge{"acknowledge"};
static constexpr char const* kActionUnsubscribe{"unsubscribe"};

// General attributes
static constexpr char const* kDevice{"device"};
static constexpr char const* kDevices{"devices"};
static constexpr char const* kDeviceId{"device_id"};
static constexpr char const* kFamilyCode{"family_code"};
static constexpr char const* kAttribute{"attribute"};
static constexpr char const* kAttributes{"attributes"};

}  // namespace json
}  // namespace cmd
}  // namespace owif

#endif  // OWIF_CMD_JSON_CONSTANTS_H

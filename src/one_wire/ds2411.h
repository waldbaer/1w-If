#ifndef OWIF_ONE_WIRE_DS2411_H
#define OWIF_ONE_WIRE_DS2411_H

#include "one_wire/one_wire_address.h"
#include "one_wire/one_wire_bus.h"
#include "one_wire/one_wire_device.h"

// ---- Constants ----

namespace owif {
namespace one_wire {

class Ds2411 : public OneWireDevice {
 public:
  static constexpr OneWireAddress::FamilyCode kFamilyCode{0x01};

  using OneWireDevice::OneWireDevice;

  auto Begin() -> bool override;
};

}  // namespace one_wire
}  // namespace owif

#endif  // OWIF_ONE_WIRE_DS2411_H

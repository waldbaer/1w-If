#include "one_wire/ds2411.h"

namespace owif {
namespace one_wire {

auto Ds2411::Begin() -> bool {
  // ID only 1-wire device: Nothing to be done.
  return true;
}

}  // namespace one_wire
}  // namespace owif

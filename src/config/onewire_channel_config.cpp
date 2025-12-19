#include "config/onewire_channel_config.h"

namespace owif {
namespace config {

auto OneWireChannelConfig::GetEnabled() const -> bool { return enabled_; }
auto OneWireChannelConfig::SetEnabled(bool enabled) -> void { enabled_ = enabled; }

}  // namespace config
}  // namespace owif

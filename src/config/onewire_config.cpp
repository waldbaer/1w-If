#include "config/onewire_config.h"

namespace owif {
namespace config {

auto OneWireConfig::GetRunInitialScan() const -> bool { return run_initial_scan_; }

auto OneWireConfig::SetRunInitialScan(bool enable) -> void { run_initial_scan_ = enable; };

auto OneWireConfig::GetChannelConfig(std::uint8_t channel_id) -> OneWireChannelConfig& {
  return channel_configs.at(channel_id);
}

auto OneWireConfig::GetChannelConfig(std::uint8_t channel_id) const -> OneWireChannelConfig const& {
  return channel_configs.at(channel_id);
}

}  // namespace config
}  // namespace owif

#ifndef OWIF_CONFIG_ONEWIRE_CONFIG_H
#define OWIF_CONFIG_ONEWIRE_CONFIG_H

#include <array>

#include "config/onewire_channel_config.h"

namespace owif {
namespace config {

class OneWireConfig {
 public:
  static constexpr bool kDefaultRunInitialScan{true};

  // Total number of 1-Wire channels / buses
  static constexpr std::uint8_t kOneWireChannels{4};
  // zero-based OneWire channel IDs
  static constexpr std::uint8_t kOneWireChannel1{0};
  static constexpr std::uint8_t kOneWireChannel2{1};
  static constexpr std::uint8_t kOneWireChannel3{2};
  static constexpr std::uint8_t kOneWireChannel4{3};

  OneWireConfig() = default;
  OneWireConfig(OneWireConfig const&) = default;
  OneWireConfig(OneWireConfig&&) = default;
  auto operator=(OneWireConfig const&) -> OneWireConfig& = default;
  auto operator=(OneWireConfig&&) -> OneWireConfig& = default;

  ~OneWireConfig() = default;

  // ---- Public APIs ----
  auto GetRunInitialScan() const -> bool;
  auto SetRunInitialScan(bool enable) -> void;

  auto GetChannelConfig(std::uint8_t channel_id) -> OneWireChannelConfig&;
  auto GetChannelConfig(std::uint8_t channel_id) const -> OneWireChannelConfig const&;

 private:
  bool run_initial_scan_{kDefaultRunInitialScan};
  std::array<OneWireChannelConfig, kOneWireChannels> channel_configs;
};

}  // namespace config
}  // namespace owif

#endif  // OWIF_CONFIG_ONEWIRE_CONFIG_H

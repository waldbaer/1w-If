#ifndef OWIF_CONFIG_ONEWIRE_CHANNEL_CONFIG_H
#define OWIF_CONFIG_ONEWIRE_CHANNEL_CONFIG_H

namespace owif {
namespace config {

class OneWireChannelConfig {
 public:
  static constexpr bool kDefaultEnabled{true};

  OneWireChannelConfig() = default;
  OneWireChannelConfig(OneWireChannelConfig const&) = default;
  OneWireChannelConfig(OneWireChannelConfig&&) = default;
  auto operator=(OneWireChannelConfig const&) -> OneWireChannelConfig& = default;
  auto operator=(OneWireChannelConfig&&) -> OneWireChannelConfig& = default;

  ~OneWireChannelConfig() = default;

  // ---- Public APIs ----

  auto GetEnabled() const -> bool;
  auto SetEnabled(bool enabled) -> void;

 private:
  bool enabled_{kDefaultEnabled};
};

}  // namespace config
}  // namespace owif

#endif  // OWIF_CONFIG_ONEWIRE_CHANNEL_CONFIG_H

#ifndef OWIF_CMD_SUBSCRIPTIONS_MANAGER_H
#define OWIF_CMD_SUBSCRIPTIONS_MANAGER_H

// ---- Includes ----

#include <map>

#include "cmd/command.h"
#include "logging/logger.h"
#include "one_wire/one_wire_address.h"

namespace owif {
namespace cmd {

class CommandHandler;  // forward declaration due to circular dependency

class SubscriptionsManager final {
 public:
  SubscriptionsManager(CommandHandler* command_handler);

  SubscriptionsManager(SubscriptionsManager const&) = default;
  auto operator=(SubscriptionsManager const&) -> SubscriptionsManager& = default;
  SubscriptionsManager(SubscriptionsManager&&) = default;
  auto operator=(SubscriptionsManager&&) -> SubscriptionsManager& = default;

  // ---- Public APIs --------------------------------------------------------------------------------------------------
  auto Loop() -> void;

  auto ProcessActionSubscribe(Command& cmd) -> void;

  auto ProcessActionUnsubscribe(Command& cmd) -> void;

 private:
  auto ConvertSubscribeToReadCommand(Command& cmd) -> void;

  logging::Logger logger_{logging::logger_g};

  struct SubscriptionKeyDevice {
   public:
    one_wire::OneWireAddress address;
    DeviceAttributeType attribute;

    auto operator==(SubscriptionKeyDevice const& other) const -> bool;
    auto operator<(SubscriptionKeyDevice const& other) const -> bool;
  };

  struct SubscriptionKeyFamily {
   public:
    one_wire::OneWireAddress::FamilyCode family_code;
    DeviceAttributeType attribute;

    auto operator==(SubscriptionKeyFamily const& other) const -> bool;
    auto operator<(SubscriptionKeyFamily const& other) const -> bool;
  };

  struct SubscriptionInfo {
    Timer timer;
    Command command;
  };

  CommandHandler* command_handler_;

  using SubscriptionsMapDevice = std::map<SubscriptionKeyDevice, SubscriptionInfo>;
  SubscriptionsMapDevice subscriptions_device_{};

  using SubsciptionsMapFamily = std::map<SubscriptionKeyFamily, SubscriptionInfo>;
  SubsciptionsMapFamily subscriptions_family_{};
};

}  // namespace cmd
}  // namespace owif

#endif  // OWIF_CMD_SUBSCRIPTIONS_MANAGER_H

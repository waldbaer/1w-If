// ---- Includes ----

#include <Arduino.h>
#include <ArduinoJson.h>

#include <tuple>

#include "cmd/command.h"
#include "cmd/command_handler.h"
#include "cmd/json_constants.h"
#include "one_wire/one_wire_address.h"

namespace owif {
namespace cmd {

SubscriptionsManager::SubscriptionsManager(CommandHandler* command_handler) : command_handler_{command_handler} {}

// ---- Public APIs --------------------------------------------------------------------------------------------------
auto SubscriptionsManager::Loop() -> void {
  for (SubscriptionsMapDevice::value_type& device_subscription : subscriptions_device_) {
    if (device_subscription.second.timer.IsExpired()) {
      logger_.Verbose("[SubscriptionsManager] Trigger command [action=%u] after interval:%u ms",
                      device_subscription.second.command.action, device_subscription.second.timer.GetDelay());
      command_handler_->EnqueueCommand(device_subscription.second.command);
      device_subscription.second.timer.Reset();
    }
  }

  for (SubscriptionsMapFamily::value_type& family_subscription : subscriptions_family_) {
    if (family_subscription.second.timer.IsExpired()) {
      logger_.Verbose("[SubscriptionsManager] Trigger command [action=%u] after interval:%u ms\n",
                      family_subscription.second.command.action, family_subscription.second.timer.GetDelay());
      command_handler_->EnqueueCommand(family_subscription.second.command);
      family_subscription.second.timer.Reset();
    }
  }
}

auto SubscriptionsManager::ProcessActionSubscribe(Command& cmd) -> void {
  logger_.Verbose("[SubscriptionsManager] process 'subscribe'");
  DeviceAttributeType const device_attribute{cmd.param3.param_value.device_attribute};
  TimeIntervalType const subscription_interval{cmd.param4.param_value.interval};

  if (cmd.param1.param_available) {
    // ---- Subscribe to a specific device ----
    one_wire::OneWireAddress const& device_addr{cmd.param1.param_value.device_id};

    logger_.Debug(F("[SubscriptionsManager] Subscribing to device: %s, attribute: %u, interval: %u ms"),
                  device_addr.Format().c_str(), device_attribute, subscription_interval);

    ConvertSubscribeToReadCommand(cmd);

    std::pair<SubscriptionsMapDevice::iterator, bool> const emplace_result{
        subscriptions_device_.emplace(SubscriptionKeyDevice{device_addr, device_attribute},
                                      SubscriptionInfo{Timer{subscription_interval.value}, cmd})};
    if (emplace_result.second) {
      // Acknowledge subscription
      JsonDocument json{};
      json[json::kRootAction] = json::kActionSubscribe;
      json[json::kActionSubscribeAcknowledge] = true;
      JsonObject json_device{json[json::kDevice].to<JsonObject>()};
      json_device[json::kDeviceId] = device_addr.Format().c_str();
      command_handler_->SendCommandResponse(cmd, json);

      // New subscription: Trigger command immediately
      command_handler_->EnqueueCommand(cmd);
    } else {
      emplace_result.first->second.timer = Timer{subscription_interval.value};
      emplace_result.first->second.command = cmd;

      logger_.Warn(F("[SubscriptionsManager] Already subscribed to device: %s, attribute: %u. Updating subscription."),
                   device_addr.Format().c_str(), device_attribute);
      command_handler_->SendErrorResponse(cmd,
                                          "WARN: Already subscribed to device / attribute. Updating subscription.");
    }

  } else if (cmd.param2.param_available) {
    // ---- Subscribe to a specific device family ----
    one_wire::OneWireAddress::FamilyCode const& family_code{cmd.param2.param_value.family_code};

    logger_.Debug(F("[SubscriptionsManager] Subscribing to device family: 0x%X, attribute: %u, interval: %u ms"),
                  family_code, device_attribute, subscription_interval);

    ConvertSubscribeToReadCommand(cmd);

    std::pair<SubscriptionsMapFamily::iterator, bool> const emplace_result{
        subscriptions_family_.emplace(SubscriptionKeyFamily{family_code, device_attribute},
                                      SubscriptionInfo{Timer{subscription_interval.value}, cmd})};
    if (emplace_result.second) {
      // Acknowledge subscription
      JsonDocument json{};
      json[json::kRootAction] = json::kActionSubscribe;
      json[json::kFamilyCode] = family_code;
      json[json::kActionSubscribeAcknowledge] = true;
      command_handler_->SendCommandResponse(cmd, json);

      // New subscription: Trigger command immediately
      command_handler_->EnqueueCommand(cmd);
    } else {
      emplace_result.first->second.timer = Timer{subscription_interval.value};
      emplace_result.first->second.command = cmd;

      logger_.Warn(F("[SubscriptionsManager] Already subscribed to device family: 0x%X, attribute: %u. Updating "
                     "subscription."),
                   family_code, device_attribute);
      command_handler_->SendErrorResponse(
          cmd, "WARN: Already subscribed to device family / attribute. Updating subscription.");
    }
  }
}

auto SubscriptionsManager::ProcessActionUnsubscribe(Command& cmd) -> void {
  logger_.Verbose("[SubscriptionsManager] process 'unsubscribe'");

  DeviceAttributeType const& device_attribute{cmd.param3.param_value.device_attribute};

  if (cmd.param1.param_available) {
    // ---- Unsubscribe a specific device ----
    one_wire::OneWireAddress const& device_addr{cmd.param1.param_value.device_id};

    logger_.Debug(F("[SubscriptionsManager] Unsubscribing from device: %s, attribute: %u"),
                  device_addr.Format().c_str(), device_attribute);

    SubscriptionsMapDevice::iterator found_subscription(
        subscriptions_device_.find(SubscriptionKeyDevice{device_addr, device_attribute}));
    if (found_subscription != subscriptions_device_.end()) {
      subscriptions_device_.erase(found_subscription);

      // Acknowledge unsubscribe
      JsonDocument json{};
      json[json::kRootAction] = json::kActionUnsubscribe;
      json[json::kActionSubscribeAcknowledge] = true;
      JsonObject json_device{json[json::kDevice].to<JsonObject>()};
      json_device[json::kDeviceId] = device_addr.Format().c_str();
      command_handler_->SendCommandResponse(cmd, json);
    } else {
      logger_.Error(F("[SubscriptionsManager] No subscription found for device: %s, attribute: %u"),
                    device_addr.Format().c_str(), device_attribute);
      command_handler_->SendErrorResponse(cmd, "WARN: No subscription for requested device / attribute found.");
    }

  } else if (cmd.param2.param_available) {
    // ---- Unsubscribe a specific device family ----
    one_wire::OneWireAddress::FamilyCode const& family_code{cmd.param2.param_value.family_code};

    logger_.Debug(F("[SubscriptionsManager] Unsubscribing from device family: 0x%X, attribute: %u"), family_code,
                  device_attribute);

    SubscriptionsMapFamily::iterator found_subscription(
        subscriptions_family_.find(SubscriptionKeyFamily{family_code, device_attribute}));
    if (found_subscription != subscriptions_family_.end()) {
      subscriptions_family_.erase(found_subscription);

      // Acknowledge unsubscribe
      JsonDocument json{};
      json[json::kRootAction] = json::kActionUnsubscribe;
      json[json::kFamilyCode] = family_code;
      json[json::kActionSubscribeAcknowledge] = true;
      command_handler_->SendCommandResponse(cmd, json);
    } else {
      logger_.Error(F("[SubscriptionsManager] No subscription found for device family: 0x%X, attribute: %u"),
                    family_code, device_attribute);
      command_handler_->SendErrorResponse(cmd, "WARN: No subscription for requested device family found.");
    }
  }
}

// ---- Private APIs ---------------------------------------------------------------------------------------------------

auto SubscriptionsManager::ConvertSubscribeToReadCommand(Command& cmd) -> void {
  cmd.action = Action::Read;

  // Unset the 'interval' parameter
  cmd.param4.param_available = false;
  cmd.param4.param_value.interval = TimeIntervalType{0};
}

auto SubscriptionsManager::SubscriptionKeyDevice::operator==(SubscriptionKeyDevice const& other) const -> bool {
  return std::tie(address, attribute) == std::tie(other.address, other.attribute);
}

auto SubscriptionsManager::SubscriptionKeyDevice::operator<(SubscriptionKeyDevice const& other) const -> bool {
  return std::tie(address, attribute) < std::tie(other.address, other.attribute);
}

auto SubscriptionsManager::SubscriptionKeyFamily::operator==(SubscriptionKeyFamily const& other) const -> bool {
  return std::tie(family_code, attribute) == std::tie(other.family_code, other.attribute);
}

auto SubscriptionsManager::SubscriptionKeyFamily::operator<(SubscriptionKeyFamily const& other) const -> bool {
  return std::tie(family_code, attribute) < std::tie(other.family_code, other.attribute);
}

}  // namespace cmd
}  // namespace owif

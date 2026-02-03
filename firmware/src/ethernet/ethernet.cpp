#include "ethernet/ethernet.h"

#include <algorithm>

#include "config/ethernet_config.h"
#include "config/persistency.h"
#include "util/language.h"

namespace owif {
namespace ethernet {

auto Ethernet::Begin() -> bool {
  logger_.Debug(F("[Ethernet] Setup..."));

  ethernet_config_ = config::persistency_g.LoadEthernetConfig();

  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t event_info) { OnEthernetEvent(event, event_info); });
  bool const result{ETH.begin()};

  // Set static IP address
  // ETH.config(IPAddress{192,168,10,40}, IPAddress{192,168,10,1}, IPAddress{255,255,255,0});

  return result;
}

auto Ethernet::IsConnected() -> bool { return connection_state_ == ConnectionState::kConnected; }

auto Ethernet::OnConnectionStateChange(ConnectionStateChangeHandler handler) -> void {
  if (handler) {
    connection_state_change_handlers_.push_back(handler);
  } else {
    logger_.Error(F("[Ethernet] Invalid ethernet connection state change handler!"));
  }
}

auto Ethernet::GetEthernetClient() -> Client& { return ethernet_client_; }

// ---- Private APIs ----

auto Ethernet::OnEthernetEvent(WiFiEvent_t const event, WiFiEventInfo_t const event_info) -> void {
  switch (event) {
    case ARDUINO_EVENT_ETH_START: {
      logger_.Debug(F("[Ethernet] Started"));

      connection_state_ = ConnectionState::kConnecting;
      ETH.setHostname(ethernet_config_.GetHostname().c_str());
    } break;

    case ARDUINO_EVENT_ETH_CONNECTED: {
    } break;
    case ARDUINO_EVENT_ETH_GOT_IP: {
      logger_.Debug(F("[Ethernet] connected | IP: %s MAC: %s Speed: %u Mbit/s (%s) Hostname: %s"),
                    ETH.localIP().toString().c_str(), ETH.macAddress().c_str(), ETH.linkSpeed(),
                    ETH.fullDuplex() ? F("FULL_DUPLEX") : F("HALF_DUPLEX"), ETH.getHostname());

      connection_state_ = ConnectionState::kConnected;
      NotifyConnectionStateChangeHandlers();
    } break;
    case ARDUINO_EVENT_ETH_DISCONNECTED: {
      logger_.Debug(F("[Ethernet] Disconnected"));

      connection_state_ = ConnectionState::kDisconnected;
      NotifyConnectionStateChangeHandlers();
    } break;
    case ARDUINO_EVENT_ETH_STOP:
      logger_.Debug(F("[Ethernet] Stopped"));

      connection_state_ = ConnectionState::kDisconnected;
      break;
    default:
      logger_.Error(F("[Ethernet] Unknown event received: %u"), event);
      break;
  }
}

auto Ethernet::NotifyConnectionStateChangeHandlers() -> void {
  for (ConnectionStateChangeHandler& handler : connection_state_change_handlers_) {
    handler(connection_state_);
  }
}

/*!
 * \brief Definition of global Ethernet instance.
 */
Ethernet ethernet_g{};

}  // namespace ethernet
}  // namespace owif

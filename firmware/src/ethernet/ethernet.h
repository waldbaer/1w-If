#ifndef OWIF_ETHERNET_ETHERNET_H
#define OWIF_ETHERNET_ETHERNET_H

// Configure Ethernet PHY 'LAN8720' connected via first I2C channel to the WT32-ETH01.
// Important: Define BEFORE include of ETH.h

#ifndef ETH_PHY_TYPE
#define ETH_PHY_TYPE ETH_PHY_LAN8720
#endif

#ifndef ETH_PHY_ADDR
#define ETH_PHY_ADDR 1
#endif

// I2C clock GPIO
#ifndef ETH_PHY_MDC
#define ETH_PHY_MDC 23
#endif

// I2C Data GPIO
#ifndef ETH_PHY_MDIO
#define ETH_PHY_MDIO 18
#endif

#ifndef ETH_PHY_POWER
#define ETH_PHY_POWER 16
#endif

#ifndef ETH_CLK_MODE
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN
#endif

#include <ETH.h>

#include <memory>
#include <vector>

#include "Client.h"
#include "config/ethernet_config.h"
#include "logging/logger.h"

namespace owif {
namespace ethernet {

enum class ConnectionState : std::uint8_t { kDisconnected = 0, kConnecting = 1, kConnected = 2 };

using ConnectionStateChangeHandler = std::function<void(ConnectionState connection_state)>;

class Ethernet {
 public:
  Ethernet() = default;
  Ethernet(Ethernet const&) = delete;
  Ethernet(Ethernet&&) = delete;
  auto operator=(Ethernet const&) -> Ethernet& = delete;
  auto operator=(Ethernet&&) -> Ethernet& = delete;

  ~Ethernet() = default;

  auto Begin() -> bool;

  auto IsConnected() -> bool;

  auto OnConnectionStateChange(ConnectionStateChangeHandler handler) -> void;

  auto GetEthernetClient() -> Client&;

 private:
  auto OnEthernetEvent(WiFiEvent_t event, WiFiEventInfo_t event_info) -> void;
  auto NotifyConnectionStateChangeHandlers() -> void;

  logging::Logger& logger_{logging::logger_g};
  config::EthernetConfig ethernet_config_{};

  WiFiClient ethernet_client_{};
  ConnectionState connection_state_{ConnectionState::kDisconnected};

  std::vector<ConnectionStateChangeHandler> connection_state_change_handlers_{};
};

/*!
 * \brief Declaration of global Ethernet instance.
 */
extern Ethernet ethernet_g;

}  // namespace ethernet
}  // namespace owif

#endif  // OWIF_ETHERNET_ETHERNET_H

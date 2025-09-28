#include "config/ethernet_config.h"

#include <string>

namespace owif {
namespace config {

auto EthernetConfig::GetHostname() const -> String const& { return hostname_; }
auto EthernetConfig::SetHostname(String hostname) -> void { hostname_ = std::move(hostname); }

}  // namespace config
}  // namespace owif

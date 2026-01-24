#ifndef OWIF_CONFIG_NTP_CONFIG_H
#define OWIF_CONFIG_NTP_CONFIG_H

#include <Preferences.h>

#include <string>

namespace owif {
namespace config {

class NtpConfig {
 public:
  static constexpr char const* kDefaultNtpServerAddr{"pool.ntp.org"};

  // POSIX Timezone string explanation for Germany/Berlin
  // "CET-1CEST,M3.5.0,M10.5.0/3"
  //
  // Format: STDoffsetDST,StartDST,EndDST
  //
  // CET-1          -> Standard time "CET" (UTC+1; POSIX offset sign is inverted)
  // CEST           -> Daylight/Summer time name
  // M3.5.0         -> DST starts: Month 3 (March), 5th week (last), day 0 (Sunday), at 02:00
  // M10.5.0/3      -> DST ends: Month 10 (October), 5th week (last), day 0 (Sunday), at 03:00
  //
  // Notes:
  // - Offsets are in hours relative to UTC; negative means east of UTC (+1h for CET)
  // - Month.week.day/time format for DST transitions
  // - After setting TZ, call tzset() to apply
  static constexpr char const* kDefaultTimezone{"CET-1CEST,M3.5.0/2,M10.5.0/3"};

  NtpConfig() = default;

  NtpConfig(NtpConfig const&) = default;
  NtpConfig(NtpConfig&&) = default;
  auto operator=(NtpConfig const&) -> NtpConfig& = default;
  auto operator=(NtpConfig&&) -> NtpConfig& = default;

  ~NtpConfig() = default;

  // ---- Public APIs ----

  auto GetServerAddr() const -> String const&;
  auto SetServerAddr(String server_addr) -> void;

  auto GetTimezone() const -> String const&;
  auto SetTimezone(String timezone) -> void;

 private:
  String server_addr_{kDefaultNtpServerAddr};
  String timezone_{kDefaultTimezone};
};

}  // namespace config
}  // namespace owif

#endif  // OWIF_CONFIG_NTP_CONFIG_H

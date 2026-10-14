// Whether Wi-Fi may use modem sleep, apart from ESP-IDF so it can be tested on the
// host (firmware/esp32/main/port_wifi.cpp). With modem sleep the access point holds
// downlink frames until the next DTIM wake-up, so audio arrives in bursts that leave
// the speaker short and then overflow it. Modem sleep stays on only on battery and
// outside a turn. The battery reading is refreshed every 5 s; a turn starting or
// ending applies at once, so the radio is fully awake before the reply arrives.
#pragma once
#include <cstdint>
#include <optional>

namespace hg {

class WifiPowerSave {
 public:
  static constexpr uint32_t kBatteryCheckMs = 5000;

  // The modem-sleep setting to switch to, or nothing when the applied one is right.
  template <class OnBattery>
  std::optional<bool> change(uint32_t now, OnBattery&& on_battery, bool in_turn) {
    if (!battery_checked_ || now - battery_checked_at_ >= kBatteryCheckMs) {
      on_battery_ = on_battery();
      battery_checked_ = true;
      battery_checked_at_ = now;
    }
    const bool want = on_battery_ && !in_turn;
    if (applied_ && *applied_ == want) return std::nullopt;
    return want;
  }
  // The caller switched modem sleep (only then: a failed switch is retried next time).
  void applied(bool modem_sleep) { applied_ = modem_sleep; }

 private:
  std::optional<bool> applied_;
  bool on_battery_ = false, battery_checked_ = false;
  uint32_t battery_checked_at_ = 0;
};

}  // namespace hg

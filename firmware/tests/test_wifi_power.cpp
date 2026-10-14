#include <cstdint>
#include <optional>

#include "check.hpp"
#include "wifi_power.hpp"

namespace {

// Applies whatever the rule asks for, like Wifi::apply_power_save, and returns it.
std::optional<bool> step(hg::WifiPowerSave& p, uint32_t now, bool on_battery, bool in_turn, int* reads = nullptr) {
  const auto want = p.change(now, [&] {
    if (reads) ++*reads;
    return on_battery;
  }, in_turn);
  if (want) p.applied(*want);
  return want;
}

}  // namespace

TEST("wifi power save: modem sleep only on battery, and full power when plugged in") {
  hg::WifiPowerSave plugged;
  CHECK(step(plugged, 0, false, false) == std::optional<bool>(false));  // the first call always applies
  CHECK(!step(plugged, 10, false, false));                            // then nothing until it changes
  hg::WifiPowerSave battery;
  CHECK(step(battery, 0, true, false) == std::optional<bool>(true));
  CHECK(!step(battery, 10, true, false));
}

TEST("wifi power save: a turn wakes the radio at once and lets it sleep again when it ends") {
  hg::WifiPowerSave p;
  CHECK(step(p, 0, true, false) == std::optional<bool>(true));
  // Well inside the 5 s battery check: the turn still applies immediately.
  CHECK(step(p, 100, true, true) == std::optional<bool>(false));
  CHECK(!step(p, 200, true, true));
  CHECK(step(p, 300, true, false) == std::optional<bool>(true));
}

TEST("wifi power save: the battery is read every 5 s, and unplugging applies at the next read") {
  hg::WifiPowerSave p;
  int reads = 0;
  step(p, 0, true, false, &reads);
  for (uint32_t t = 10; t < hg::WifiPowerSave::kBatteryCheckMs; t += 10) step(p, t, true, t % 20 == 0, &reads);
  CHECK_EQ(reads, 1);
  // Plugged in now: the reading only changes at the next check.
  CHECK(!step(p, hg::WifiPowerSave::kBatteryCheckMs - 10, false, false, &reads));
  CHECK(step(p, hg::WifiPowerSave::kBatteryCheckMs, false, false, &reads) == std::optional<bool>(false));
  CHECK_EQ(reads, 2);
}

TEST("wifi power save: a switch that failed is asked for again") {
  hg::WifiPowerSave p;
  CHECK(p.change(0, [] { return true; }, false) == std::optional<bool>(true));  // not applied
  CHECK(p.change(10, [] { return true; }, false) == std::optional<bool>(true));
  p.applied(true);
  CHECK(!p.change(20, [] { return true; }, false));
}

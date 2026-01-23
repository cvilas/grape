//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ego_clock.h"

#include <chrono>
#include <thread>

#include "clock_data_receiver.h"
#include "grape/log/syslog.h"

namespace grape {

//-------------------------------------------------------------------------------------------------
EgoClock::EgoClock(const std::string& system_name)
  : rx_(std::make_unique<ego_clock::ClockDataReceiver>(system_name)) {
}

//-------------------------------------------------------------------------------------------------
EgoClock::~EgoClock() = default;

//-------------------------------------------------------------------------------------------------
EgoClock::EgoClock(EgoClock&&) noexcept = default;

//-------------------------------------------------------------------------------------------------
auto EgoClock::create(const std::string& clock_name, const std::chrono::milliseconds& timeout)
    -> std::optional<EgoClock> {
  const auto until = WallClock::now() + timeout;
  static constexpr auto LOOP_WAIT = std::chrono::milliseconds(1);
  auto clock = EgoClock(clock_name);
  while (not clock.rx_->isInit() and (WallClock::now() < until)) {
    std::this_thread::sleep_for(LOOP_WAIT);
  }
  return clock.rx_->isInit() ? std::optional<EgoClock>(std::move(clock)) : std::nullopt;
}

//-------------------------------------------------------------------------------------------------
auto EgoClock::now() const noexcept -> EgoClock::TimePoint {
  return ego_clock::toEgoTime(rx_->transform(), WallClock::now());
}

//-------------------------------------------------------------------------------------------------
void EgoClock::sleepFor(const EgoClock::Duration& dt) const {
  std::this_thread::sleep_for(ego_clock::toWallDuration(rx_->transform(), dt));
}

//-------------------------------------------------------------------------------------------------
void EgoClock::sleepUntil(const EgoClock::TimePoint& tp) const {
  std::this_thread::sleep_until(ego_clock::toWallTime(rx_->transform(), tp));
}

}  // namespace grape

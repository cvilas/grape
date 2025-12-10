//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ego_clock.h"

#include <chrono>
#include <thread>

#include "clock_data_receiver.h"
#include "grape/log/syslog.h"
#include "grape/wall_clock.h"

namespace {

//-------------------------------------------------------------------------------------------------
constexpr auto toWallTime(const grape::ego_clock::ClockTransform& tf,
                          const grape::EgoClock::TimePoint& tp) -> grape::WallClock::TimePoint {
  const auto ns = (tf.scale * static_cast<double>(grape::EgoClock::toNanos(tp))) + tf.offset;
  return grape::WallClock::fromNanos(static_cast<std::int64_t>(ns));
}

//-------------------------------------------------------------------------------------------------
constexpr auto toEgoTime(const grape::ego_clock::ClockTransform& tf,
                         const grape::WallClock::TimePoint& tp) -> grape::EgoClock::TimePoint {
  const auto ns = (static_cast<double>(grape::WallClock::toNanos(tp)) - tf.offset) / tf.scale;
  return grape::EgoClock::fromNanos(static_cast<std::int64_t>(ns));
}

//-------------------------------------------------------------------------------------------------
constexpr auto toWallDuration(const grape::ego_clock::ClockTransform& tf,
                              const grape::EgoClock::Duration& dur) -> grape::WallClock::Duration {
  return std::chrono::duration_cast<grape::WallClock::Duration>(dur * tf.scale);
}

}  // namespace

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
  return toEgoTime(rx_->transform(), WallClock::now());
}

//-------------------------------------------------------------------------------------------------
void EgoClock::sleepFor(const EgoClock::Duration& dt) const {
  std::this_thread::sleep_for(toWallDuration(rx_->transform(), dt));
}

//-------------------------------------------------------------------------------------------------
void EgoClock::sleepUntil(const EgoClock::TimePoint& tp) const {
  std::this_thread::sleep_until(toWallTime(rx_->transform(), tp));
}

}  // namespace grape

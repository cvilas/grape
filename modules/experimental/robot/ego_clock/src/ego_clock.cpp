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

//=================================================================================================
class EgoClock::Impl {
public:
  explicit Impl(const std::string& clock_name) : receiver_(clock_name) {
  }
  auto receiver() -> grape::ego_clock::ClockDataReceiver& {
    return receiver_;
  }

private:
  grape::ego_clock::ClockDataReceiver receiver_;
};

//-------------------------------------------------------------------------------------------------
EgoClock::EgoClock(const std::string& system_name) : impl_(std::make_unique<Impl>(system_name)) {
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
  while (not clock.impl_->receiver().transform() and (WallClock::now() < until)) {
    std::this_thread::sleep_for(LOOP_WAIT);
  }
  return clock.impl_->receiver().transform().has_value() ?
             std::optional<EgoClock>(std::move(clock)) :
             std::nullopt;
}

//-------------------------------------------------------------------------------------------------
auto EgoClock::now() -> EgoClock::TimePoint {
  const auto tf = impl_->receiver().transform();
  if (tf) {
    return toEgoTime(*tf, WallClock::now());
  }
  syslog::Error("No master clock (unexpected)");
  return EgoClock::TimePoint{};
}

//-------------------------------------------------------------------------------------------------
void EgoClock::sleepFor(const EgoClock::Duration& dt) {
  const auto tf = impl_->receiver().transform();
  if (tf) {
    std::this_thread::sleep_for(toWallDuration(*tf, dt));
  } else {
    syslog::Error("No master clock (unexpected)");
  }
}

//-------------------------------------------------------------------------------------------------
void EgoClock::sleepUntil(const EgoClock::TimePoint& tp) {
  const auto tf = impl_->receiver().transform();
  if (tf) {
    std::this_thread::sleep_until(toWallTime(*tf, tp));
  } else {
    syslog::Error("No master clock (unexpected)");
  }
}

}  // namespace grape

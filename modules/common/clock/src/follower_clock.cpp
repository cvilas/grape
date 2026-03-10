//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/clock/follower_clock.h"

#include "tick.h"

namespace grape::clock {

//-------------------------------------------------------------------------------------------------
struct FollowerClock::Impl : ShmTick {
  using ShmTick::ShmTick;
};

//-------------------------------------------------------------------------------------------------
FollowerClock::~FollowerClock() = default;

//-------------------------------------------------------------------------------------------------
FollowerClock::FollowerClock(const std::string& source_name)
  : impl_(std::make_unique<Impl>(
        ShmTick::init(source_name, realtime::SharedMemory::Access::ReadOnly))) {
}

//-------------------------------------------------------------------------------------------------
auto FollowerClock::waitForNextTick(std::chrono::milliseconds timeout) const -> bool {
  auto& tick = impl_->tick();
  const auto result = tick.wait(tick.get(), timeout);
  return result.has_value() && result.value();
}

//-------------------------------------------------------------------------------------------------
auto FollowerClock::now() const noexcept -> FollowerClock::TimePoint {
  return FollowerClock::fromNanos(impl_->tick().get());
}

//-------------------------------------------------------------------------------------------------
void FollowerClock::sleepUntil(const FollowerClock::TimePoint& tp) const {
  auto& tick = impl_->tick();

  static constexpr auto TICK_WAIT_TIMEOUT = std::chrono::milliseconds(500);
  auto current_nanos = tick.get();
  while (FollowerClock::fromNanos(current_nanos) < tp) {
    const auto result = tick.wait(current_nanos, TICK_WAIT_TIMEOUT);
    if (not result) {
      break;
    }
    current_nanos = tick.get();
  }
}

//-------------------------------------------------------------------------------------------------
void FollowerClock::sleepFor(const FollowerClock::Duration& dt) const {
  sleepUntil(now() + dt);
}

}  // namespace grape::clock

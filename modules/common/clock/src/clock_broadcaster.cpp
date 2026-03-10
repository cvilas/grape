//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "grape/clock/clock_broadcaster.h"

#include "tick.h"

namespace grape::clock {

//-------------------------------------------------------------------------------------------------
struct ClockBroadcaster::Impl : ShmTick {
  using ShmTick::ShmTick;
};

//-------------------------------------------------------------------------------------------------
ClockBroadcaster::ClockBroadcaster(Config config)
  : config_(std::move(config))
  , impl_(std::make_unique<Impl>(
        ShmTick::init(config_.name, realtime::SharedMemory::Access::ReadWrite))) {
}

//-------------------------------------------------------------------------------------------------
ClockBroadcaster::~ClockBroadcaster() {
  ShmTick::cleanup(config_.name);
}

//-------------------------------------------------------------------------------------------------
void ClockBroadcaster::post(const FollowerClock::TimePoint& tp) {
  impl_->tick().post(FollowerClock::toNanos(tp));
}

}  // namespace grape::clock

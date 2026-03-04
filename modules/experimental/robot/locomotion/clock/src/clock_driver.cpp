//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/clock_driver.h"

#include <stdexcept>

#include "tick.h"

namespace grape {

//-------------------------------------------------------------------------------------------------
struct ClockDriver::Impl : realtime::SharedMemory {};

//-------------------------------------------------------------------------------------------------
ClockDriver::ClockDriver(const Config& config) {
  auto maybe_shm =
      clock::initShm(config.clock_name, grape::realtime::SharedMemory::Access::ReadWrite);
  if (maybe_shm) {
    impl_ = std::make_unique<Impl>(std::move(maybe_shm.value()));
  }
}

//-------------------------------------------------------------------------------------------------
ClockDriver::~ClockDriver() = default;

//-------------------------------------------------------------------------------------------------
void ClockDriver::tick(const FollowerClock::TimePoint& tp) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* tick = reinterpret_cast<clock::Tick*>(impl_->data().data());
  tick->post(FollowerClock::toNanos(tp));
}

}  // namespace grape

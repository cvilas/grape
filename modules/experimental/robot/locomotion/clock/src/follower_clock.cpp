//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/follower_clock.h"

#include <thread>

#include "tick.h"

namespace grape {

//-------------------------------------------------------------------------------------------------
struct FollowerClock::Impl : realtime::SharedMemory {};

//-------------------------------------------------------------------------------------------------
FollowerClock::~FollowerClock() = default;

//-------------------------------------------------------------------------------------------------
FollowerClock::FollowerClock(FollowerClock&&) noexcept = default;

//-------------------------------------------------------------------------------------------------
FollowerClock::FollowerClock(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {
}

//-------------------------------------------------------------------------------------------------
auto FollowerClock::create(const std::string& clock_name, const std::chrono::milliseconds& timeout)
    -> std::optional<FollowerClock> {
  const auto end = std::chrono::steady_clock::now() + timeout;
  static constexpr auto POLL_INTERVAL = std::chrono::milliseconds(1);

  auto maybe_shm = clock::initShm(clock_name, grape::realtime::SharedMemory::Access::ReadOnly);
  while (not maybe_shm) {
    if (std::chrono::steady_clock::now() >= end) {
      return std::nullopt;
    }
    std::this_thread::sleep_for(POLL_INTERVAL);
    maybe_shm = clock::initShm(clock_name, grape::realtime::SharedMemory::Access::ReadOnly);
  }
  auto impl = std::make_unique<Impl>(std::move(maybe_shm.value()));

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* tick = reinterpret_cast<clock::Tick*>(impl->data().data());

  // Wait for the first tick (nanos > 0)
  while (tick->get() == 0) {
    const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - std::chrono::steady_clock::now());
    if (remaining.count() <= 0) {
      return std::nullopt;
    }
    std::ignore = tick->wait(0, remaining);
  }

  return FollowerClock{ std::move(impl) };
}

//-------------------------------------------------------------------------------------------------
auto FollowerClock::now() const noexcept -> FollowerClock::TimePoint {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* tick = reinterpret_cast<clock::Tick*>(impl_->data().data());
  return FollowerClock::fromNanos(tick->get());
}

//-------------------------------------------------------------------------------------------------
void FollowerClock::sleepUntil(const FollowerClock::TimePoint& tp) const {
  // Block on futex ticks until the ego clock reaches the target time.
  // TICK_WAIT_TIMEOUT guards against indefinite blocking if the driver stops posting ticks.

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* tick = reinterpret_cast<clock::Tick*>(impl_->data().data());

  static constexpr auto TICK_WAIT_TIMEOUT = std::chrono::milliseconds(100);
  auto current_nanos = tick->get();
  while (FollowerClock::fromNanos(current_nanos) < tp) {
    std::ignore = tick->wait(current_nanos, TICK_WAIT_TIMEOUT);
    current_nanos = tick->get();
  }
}

//-------------------------------------------------------------------------------------------------
void FollowerClock::sleepFor(const FollowerClock::Duration& dt) const {
  sleepUntil(now() + dt);
}

}  // namespace grape

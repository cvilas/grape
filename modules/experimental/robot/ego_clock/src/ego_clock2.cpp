//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ego_clock2.h"

#include <thread>

#include "ego_clock2_signal.h"
#include "grape/realtime/shared_memory.h"

namespace grape {

//-------------------------------------------------------------------------------------------------
struct EgoClock2::Impl {
  explicit Impl(realtime::SharedMemory&& shm_in);
  realtime::SharedMemory shm;
  const ego_clock::EgoClock2Signal* signal{ nullptr };
};

//-------------------------------------------------------------------------------------------------
EgoClock2::Impl::Impl(realtime::SharedMemory&& shm_in) : shm(std::move(shm_in)) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  signal = reinterpret_cast<const ego_clock::EgoClock2Signal*>(shm.data().data());
}

//-------------------------------------------------------------------------------------------------
EgoClock2::EgoClock2(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {
}

//-------------------------------------------------------------------------------------------------
EgoClock2::~EgoClock2() = default;

//-------------------------------------------------------------------------------------------------
EgoClock2::EgoClock2(EgoClock2&&) noexcept = default;

//-------------------------------------------------------------------------------------------------
auto EgoClock2::create(const std::string& clock_name,
                       const std::chrono::milliseconds& timeout) -> std::optional<EgoClock2> {
  using Shm = realtime::SharedMemory;
  const auto end = std::chrono::steady_clock::now() + timeout;
  static constexpr auto POLL_INTERVAL = std::chrono::milliseconds(1);

  // Wait for the shared memory to be created by the driver
  auto maybe_shm = Shm::open(clock_name, Shm::Access::ReadOnly);
  while (not maybe_shm) {
    if (std::chrono::steady_clock::now() >= end) {
      return std::nullopt;
    }
    std::this_thread::sleep_for(POLL_INTERVAL);
    maybe_shm = Shm::open(clock_name, Shm::Access::ReadOnly);
  }

  auto impl = std::make_unique<Impl>(std::move(maybe_shm.value()));

  // Wait for the first tick (nanos > 0)
  while (impl->signal->get() == 0) {
    const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - std::chrono::steady_clock::now());
    if (remaining.count() <= 0) {
      return std::nullopt;
    }
    std::ignore = impl->signal->wait(0, remaining);
  }

  return EgoClock2(std::move(impl));
}

//-------------------------------------------------------------------------------------------------
auto EgoClock2::now() const noexcept -> EgoClock2::TimePoint {
  return EgoClock2::fromNanos(impl_->signal->get());
}

//-------------------------------------------------------------------------------------------------
void EgoClock2::sleepUntil(const EgoClock2::TimePoint& tp) const {
  // Block on futex ticks until the ego clock reaches the target time.
  // TICK_WAIT_TIMEOUT guards against indefinite blocking if the driver stops posting ticks.
  static constexpr auto TICK_WAIT_TIMEOUT = std::chrono::milliseconds(100);
  auto current_nanos = impl_->signal->get();
  while (EgoClock2::fromNanos(current_nanos) < tp) {
    std::ignore = impl_->signal->wait(current_nanos, TICK_WAIT_TIMEOUT);
    current_nanos = impl_->signal->get();
  }
}

//-------------------------------------------------------------------------------------------------
void EgoClock2::sleepFor(const EgoClock2::Duration& dt) const {
  sleepUntil(now() + dt);
}

}  // namespace grape

//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ego_clock2_driver.h"

#include <stdexcept>

#include "ego_clock2_signal.h"
#include "grape/realtime/shared_memory.h"

namespace grape {

//-------------------------------------------------------------------------------------------------
struct EgoClock2Driver::Impl {
  Impl(const Config& config, realtime::SharedMemory&& shm_in);
  ~Impl();
  realtime::SharedMemory shm;
  ego_clock::EgoClock2Signal* signal{ nullptr };
  std::string clock_name;
};

//-------------------------------------------------------------------------------------------------
EgoClock2Driver::Impl::Impl(const Config& config, realtime::SharedMemory&& shm_in)
  : shm(std::move(shm_in)), clock_name(config.clock_name) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  signal = reinterpret_cast<ego_clock::EgoClock2Signal*>(shm.data().data());
  // Initialise the signal in the shared memory region
  new (signal) ego_clock::EgoClock2Signal{};
}

//-------------------------------------------------------------------------------------------------
EgoClock2Driver::Impl::~Impl() {
  shm.close();
  std::ignore = realtime::SharedMemory::remove(clock_name);
}

//-------------------------------------------------------------------------------------------------
EgoClock2Driver::EgoClock2Driver(const Config& config) {
  using Shm = realtime::SharedMemory;

  // Remove any stale shared memory from a previous session before creating a new one
  std::ignore = Shm::remove(config.clock_name);

  auto maybe_shm =
      Shm::create(config.clock_name, sizeof(ego_clock::EgoClock2Signal), Shm::Access::ReadWrite);
  if (not maybe_shm) {
    throw std::runtime_error("EgoClock2Driver: Failed to create shared memory '" +
                             config.clock_name + "': " + std::string(maybe_shm.error().message()));
  }
  impl_ = std::make_unique<Impl>(config, std::move(maybe_shm.value()));
}

//-------------------------------------------------------------------------------------------------
EgoClock2Driver::~EgoClock2Driver() = default;

//-------------------------------------------------------------------------------------------------
void EgoClock2Driver::tick(const EgoClock2::TimePoint& ego_time) {
  impl_->signal->post(EgoClock2::toNanos(ego_time));
}

}  // namespace grape

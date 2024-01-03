//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstring>
#include <format>

#include <pthread.h>

#include "grape/exception.h"

namespace grape::realtime {

//=================================================================================================
/// Priority inheriting mutex
///
/// Locks should generally be avoided in time-critical paths. But if you have to use one, use this
/// instead of std::mutex to avoid potential for priority inversion.
///
/// Implements Lockable (https://en.cppreference.com/w/cpp/named_req/Lockable), necessitating
/// divergence from project coding style.
class Mutex {
public:
  // NOLINTBEGIN(readability-identifier-naming)
  using native_handle_type = pthread_mutex_t*;
  Mutex();
  ~Mutex();
  Mutex(const Mutex&) = delete;
  Mutex(Mutex&&) = delete;
  auto operator=(const Mutex&) -> Mutex& = delete;
  auto operator=(const Mutex&&) -> Mutex& = delete;
  void lock();
  void unlock() noexcept;
  [[nodiscard]] auto try_lock() noexcept -> bool;
  [[nodiscard]] auto native_handle() -> native_handle_type;
  // NOLINTEND(readability-identifier-naming)
private:
  pthread_mutex_t mutex_{};
};

//-------------------------------------------------------------------------------------------------
inline Mutex::Mutex() {
  pthread_mutexattr_t attr;

  {
    const auto result = pthread_mutexattr_init(&attr);
    if (result != 0) {
      panic<Exception>(std::format("pthread_mutexattr_init: {}", std::strerror(result)));
    }
  }

  {
    const auto result = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    if (result != 0) {
      panic<Exception>(std::format("pthread_mutexattr_setprotocol: {}", std::strerror(result)));
    }
  }

  {
    const auto result = pthread_mutex_init(&mutex_, &attr);
    if (result != 0) {
      panic<Exception>(std::format("pthread_mutex_init: {}", std::strerror(result)));
    }
  }
}

//-------------------------------------------------------------------------------------------------
inline Mutex::~Mutex() {
  pthread_mutex_destroy(&mutex_);
}

//-------------------------------------------------------------------------------------------------
inline void Mutex::lock() {
  const auto result = pthread_mutex_lock(&mutex_);
  if (result != 0) {
    panic<Exception>(std::strerror(result));
  }
}

//-------------------------------------------------------------------------------------------------
inline void Mutex::unlock() noexcept {
  std::ignore = pthread_mutex_unlock(&mutex_);
}

//-------------------------------------------------------------------------------------------------
inline auto Mutex::try_lock() noexcept -> bool {
  return (pthread_mutex_trylock(&mutex_) == 0);
}

//-------------------------------------------------------------------------------------------------
inline auto Mutex::native_handle() -> native_handle_type {
  return &mutex_;
}

}  // namespace grape::realtime

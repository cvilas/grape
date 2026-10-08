//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <stop_token>
#include <thread>
#include <vector>

#include "grape/splat/math.h"
#include "grape/splat/splat.h"

namespace grape::splat {

//=================================================================================================
/// Runs sortBackToFront() on a worker thread.
///
/// Sorting a million splats takes longer than a frame. Doing it on the render thread would make
/// interaction stutter, so the renderer instead asks for a sort whenever the camera moves and keeps
/// drawing with the most recent completed order until a new one arrives. Requests that arrive
/// while a sort is in progress replace each other: only the latest view matters.
class AsyncSorter {
public:
  explicit AsyncSorter(std::shared_ptr<const std::vector<Splat>> scene);

  /// Request sorting for the given view. Supersedes any pending request.
  /// @param view World-to-camera transform
  void request(const Mat4& view);

  /// @return Splat indices in back-to-front order, if a new result is available since last call
  [[nodiscard]] auto take() -> std::optional<std::vector<std::uint32_t>>;

  ~AsyncSorter() = default;
  AsyncSorter(const AsyncSorter&) = delete;
  AsyncSorter(AsyncSorter&&) = delete;
  auto operator=(const AsyncSorter&) = delete;
  auto operator=(AsyncSorter&&) = delete;

private:
  void run(const std::stop_token& stop);

  /// State shared between the render and sort threads
  struct Shared {
    std::mutex mutex;
    std::condition_variable_any cond;
    std::optional<Mat4> pending;
    std::optional<std::vector<std::uint32_t>> result;
  };

  std::shared_ptr<const std::vector<Splat>> scene_;
  std::unique_ptr<Shared> shared_{ std::make_unique<Shared>() };
  std::jthread worker_;  // NOTE: Declared last so it is stopped before other members are destroyed
};

}  // namespace grape::splat

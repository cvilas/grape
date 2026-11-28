//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include "async_sorter.h"

#include <utility>

#include "grape/splat/depth_sort.h"

namespace grape::splat {

//-------------------------------------------------------------------------------------------------
AsyncSorter::AsyncSorter(std::shared_ptr<const std::vector<Splat>> scene)
  : scene_(std::move(scene)), worker_([this](const std::stop_token& st) { run(st); }) {
}

//-------------------------------------------------------------------------------------------------
void AsyncSorter::request(const Mat4& view) {
  {
    const auto lock = std::scoped_lock(shared_->mutex);
    shared_->pending = view;
  }
  shared_->cond.notify_one();
}

//-------------------------------------------------------------------------------------------------
auto AsyncSorter::take() -> std::optional<std::vector<std::uint32_t>> {
  const auto lock = std::scoped_lock(shared_->mutex);
  return std::exchange(shared_->result, std::nullopt);
}

//-------------------------------------------------------------------------------------------------
void AsyncSorter::run(const std::stop_token& stop) {
  while (not stop.stop_requested()) {
    auto view = Mat4{};
    {
      auto lock = std::unique_lock(shared_->mutex);
      if (not shared_->cond.wait(lock, stop, [this] { return shared_->pending.has_value(); })) {
        return;
      }
      view = std::exchange(shared_->pending, std::nullopt).value_or(Mat4{});
    }
    auto order = sortBackToFront(*scene_, view);
    {
      const auto lock = std::scoped_lock(shared_->mutex);
      shared_->result = std::move(order);
    }
  }
}

}  // namespace grape::splat

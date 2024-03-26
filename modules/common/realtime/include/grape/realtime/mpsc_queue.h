//=================================================================================================
// Copyright (C) 2018 GRAPE Contributors
//=================================================================================================

#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <optional>
#include <vector>

namespace grape::realtime {

//=================================================================================================
/// Lock-free, non-blocking, multi-producer-single-consumer queue
///
template <typename T>
class MPSCQueue {
public:
  /// Create a queue
  /// @param capacity Maximum limit for number of items allowed in the queue.
  explicit MPSCQueue(std::size_t capacity);

  /// Attempt to enqueue without blocking.
  /// @note Can be called concurrently from multiple threads.
  /// @return true on success, false if queue is full.
  [[nodiscard]] auto tryPush(T&& obj) -> bool;

  /// Attempt to dequeue without blocking.
  /// @note Should not be called concurrently from multiple threads without mutual exclusion.
  /// @return An item from queue if the operation won't block, else nothing
  [[nodiscard]] auto tryPop() -> std::optional<T>;

  /// @return The number of items in queue.
  [[nodiscard]] auto count() const noexcept -> std::size_t;

private:
  static_assert(std::atomic_size_t::is_always_lock_free);
  std::atomic_size_t count_{ 0 };
  std::atomic_size_t head_{ 0 };
  std::size_t tail_{ 0 };
  struct Item {
    std::atomic_flag is_readable{ false };
    T value{};
  };
  std::vector<Item> items_;
};

//-------------------------------------------------------------------------------------------------
template <typename T>
MPSCQueue<T>::MPSCQueue(std::size_t capacity) : items_((capacity > 0 ? capacity : 1u)) {
}

//-------------------------------------------------------------------------------------------------
template <typename T>
auto MPSCQueue<T>::tryPush(T&& obj) -> bool {
  const auto count = count_.fetch_add(1, std::memory_order_acquire);
  const auto capacity = items_.size();
  if (count >= capacity) {
    // back off, queue is full
    count_.fetch_sub(1, std::memory_order_release);
    return false;
  }

  // increment head, giving 'exclusive' access to that element until readability flag is set
  const auto head = head_.fetch_add(1, std::memory_order_acquire) % capacity;
  auto& item = items_.at(head);
  assert(not item.is_readable.test(std::memory_order_acquire));
  item.value = std::move(obj);
  item.is_readable.test_and_set(std::memory_order_release);
  return true;
}

//-------------------------------------------------------------------------------------------------
template <typename T>
inline auto MPSCQueue<T>::tryPop() -> std::optional<T> {
  auto& item = items_.at(tail_);
  if (not item.is_readable.test(std::memory_order_acquire)) {
    // A thread could still be writing to this location
    return {};
  }

  item.is_readable.clear(std::memory_order_release);
  std::optional<T> ret;
  ret.emplace(std::move(item.value));

  if (++tail_ >= items_.size()) {
    tail_ = 0;
  }

  [[maybe_unused]] const auto count = count_.fetch_sub(1, std::memory_order_release);
  assert(count > 0);

  return ret;
}

//-------------------------------------------------------------------------------------------------
template <typename T>
inline auto MPSCQueue<T>::count() const noexcept -> std::size_t {
  return count_.load(std::memory_order_relaxed);
}

}  // namespace grape::realtime

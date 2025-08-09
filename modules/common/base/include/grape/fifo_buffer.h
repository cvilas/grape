//=================================================================================================
// Copyright (C) 2018 GRAPE Contributors
//=================================================================================================

#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <functional>
#include <span>
#include <vector>

namespace grape {

//=================================================================================================
/// Lock-free, non-blocking, multi-producer-single-consumer in-place modifiable raw FIFO buffer
///
class FIFOBuffer {
public:
  /// Buffer configuration options
  struct Config {
    std::size_t frame_length;  //!< Length of a single frame in bytes
    std::size_t num_frames;    //!< Number of frames in the buffer
  };

  /// Function signature to write a frame in-place into the buffer
  using WriterFunc = std::function<void(std::span<std::byte>)>;

  /// Function signature to read a frame in-place from the buffer
  using ReaderFunc = std::function<void(std::span<const std::byte>)>;

  /// Construct a buffer with the specified options
  explicit constexpr FIFOBuffer(const Config& options);

  /// Attempt to write a frame in-place without blocking.
  /// @param func Writing function
  /// @note Can be called concurrently from multiple threads.
  /// @return false if buffer is full and has no more space to write, else true.
  [[nodiscard]] auto visitToWrite(const WriterFunc& func) -> bool;

  /// Attempt to read a frame in-place without blocking.
  /// @param func Reading function
  /// @note Should not be called concurrently from multiple threads without mutual exclusion.
  /// @return false if buffer has no more items to read, else true
  [[nodiscard]] auto visitToRead(const ReaderFunc& func) -> bool;

  /// @return The number of items in queue.
  [[nodiscard]] auto count() const noexcept -> std::size_t;

private:
  static_assert(std::atomic_size_t::is_always_lock_free);
  Config config_;
  std::atomic_size_t count_{ 0 };
  std::atomic_size_t head_{ 0 };
  std::size_t tail_{ 0 };
  std::vector<std::atomic_flag> is_readable_;
  std::vector<std::byte> buffer_;
};

//-------------------------------------------------------------------------------------------------
constexpr FIFOBuffer::FIFOBuffer(const Config& options)
  : config_(options)
  , is_readable_(config_.num_frames)
  , buffer_(config_.num_frames * config_.frame_length) {
}

//-------------------------------------------------------------------------------------------------
inline auto FIFOBuffer::visitToWrite(const WriterFunc& func) -> bool {
  const auto count = count_.fetch_add(1, std::memory_order_acquire);
  if (count >= config_.num_frames) {
    // back off, queue is full
    count_.fetch_sub(1, std::memory_order_release);
    return false;
  }

  // increment head, giving 'exclusive' access to that element until readability flag is set
  const auto head = head_.fetch_add(1, std::memory_order_acquire) % config_.num_frames;
  auto& readability_flag = is_readable_.at(head);
  assert(not readability_flag.test(std::memory_order_acquire));

  // write frame
  const auto frame_offset = head * config_.frame_length;
  const auto frame_start = std::next(std::begin(buffer_), static_cast<std::int64_t>(frame_offset));
  func(std::span{ frame_start, config_.frame_length });
  readability_flag.test_and_set(std::memory_order_release);

  return true;
}

//-------------------------------------------------------------------------------------------------
inline auto FIFOBuffer::visitToRead(const ReaderFunc& func) -> bool {
  auto& readability_flag = is_readable_.at(tail_);
  if (not readability_flag.test(std::memory_order_acquire)) {
    // A thread could still be writing to this location
    return false;
  }

  // read frame
  readability_flag.clear(std::memory_order_release);
  const auto frame_offset = tail_ * config_.frame_length;
  const auto frame_start = std::next(std::begin(buffer_), static_cast<std::int64_t>(frame_offset));
  func(std::span{ frame_start, config_.frame_length });

  if (++tail_ >= config_.num_frames) {
    tail_ = 0;
  }

  [[maybe_unused]] const auto count = count_.fetch_sub(1, std::memory_order_release);
  assert(count > 0);

  return true;
}

//-------------------------------------------------------------------------------------------------
inline auto FIFOBuffer::count() const noexcept -> std::size_t {
  return count_.load(std::memory_order_relaxed);
}

}  // namespace grape

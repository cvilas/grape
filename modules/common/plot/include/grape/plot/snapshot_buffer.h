//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cassert>
#include <span>
#include <stdexcept>
#include <vector>

#include "grape/plot/style.h"

namespace grape::plot {

//=================================================================================================
/// Fixed-capacity FIFO buffer that retains the most recent N samples with the following
/// attributes:
/// - O(1) push, no heap allocation after construction.
/// - cache-friendly two-span view (oldest → newest).
///
class SnapshotBuffer {
public:
  static constexpr auto MIN_CAPACITY = 2UZ;

  constexpr explicit SnapshotBuffer(std::size_t cap) : data_(cap) {
    if (cap < MIN_CAPACITY) {
      throw std::invalid_argument("SnapshotBuffer: capacity must be >= 2");
    }
  }

  constexpr void pushBack(const Sample& sample) noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    data_[wrap(head_ + size_, capacity())] = sample;  // always write to logical tail
    if (size_ < capacity()) {
      ++size_;  // still filling: grow
    } else {
      head_ = wrap(head_ + 1, capacity());  // full: slide window forward
    }
  }

  [[nodiscard]] constexpr auto empty() const noexcept -> bool {
    return size_ == 0;
  }
  [[nodiscard]] constexpr auto size() const noexcept -> std::size_t {
    return size_;
  }
  [[nodiscard]] constexpr auto capacity() const noexcept -> std::size_t {
    return data_.size();
  }

  [[nodiscard]] constexpr auto operator[](std::size_t idx) const noexcept -> const Sample& {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    return data_[wrap(head_ + idx, capacity())];
  }

  [[nodiscard]] constexpr auto at(std::size_t idx) const -> const Sample& {
    if (idx >= size_) {
      throw std::out_of_range("SnapshotBuffer::at: index out of range");
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    return (*this)[idx];
  }

  /// @pre !empty()
  [[nodiscard]] constexpr auto front() const noexcept -> const Sample& {
    assert(!empty());  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    return data_[head_];
  }

  /// @pre !empty()
  [[nodiscard]] constexpr auto back() const noexcept -> const Sample& {
    assert(!empty());  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    return data_[wrap(head_ + size_ - 1, capacity())];
  }

  constexpr void clear() noexcept {
    head_ = 0;
    size_ = 0;
  }

  /// Returns two contiguous spans in chronological order (oldest → newest). When the buffer
  /// hasn't wrapped, the second span is empty. Traverse [s1, s2] in order to visit all elements.
  [[nodiscard]] constexpr auto view() const noexcept
      -> std::pair<std::span<const Sample>, std::span<const Sample>> {
    if (size_ == 0) {
      return { {}, {} };
    }
    const auto all = std::span<const Sample>{ data_ };
    const auto tail = head_ + size_;  // logical tail; may exceed capacity()
    if (tail <= capacity()) {
      return { all.subspan(head_, size_), {} };
    }
    return { all.subspan(head_, capacity() - head_), all.subspan(0, tail - capacity()) };
  }

private:
  [[nodiscard]] static constexpr auto wrap(std::size_t idx, std::size_t cap) noexcept
      -> std::size_t {
    return idx < cap ? idx : idx - cap;
  }

  std::vector<Sample> data_;
  std::size_t head_ = 0;
  std::size_t size_ = 0;
};

}  // namespace grape::plot

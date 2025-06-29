//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <array>
#include <concepts>

namespace grape::statistics {

//=================================================================================================
// Computes mean and variance over a sliding window of N values
//
// Reference:
// - Knuth TAOCP vol 2, 3rd edition, page 232
// - Welford's online algorithm: https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
//
template <std::floating_point T, std::size_t N>
class SlidingMean {
public:
  /// Holds computed statistics
  struct Stats {
    T mean;
    T variance;
  };

  /// Accumulates data
  /// @param value A data point to add to statistics
  /// @return Updated stats
  [[nodiscard]] constexpr auto append(const T& value) -> Stats {
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
    if (count_ < N) {
      buffer_[head_] = value;
      ++count_;

      // mean: m(k) = m(k-1) + { x(k) - m(k-1) } / k
      // variance: s(k) = s(k-1) + { x(k) - m(k-1) } * { x(k) - m(k) }

      const auto delta = value - mean_;
      mean_ += delta / static_cast<T>(count_);
      scaled_variance_ += delta * (value - mean_);
    } else {
      const auto stale_value = buffer_[head_];
      buffer_[head_] = value;

      // mean: m(k) = m(k-1) + { x(k) - x(0) } / k
      // variance: s(k) = s(k-1) + { x(k) - x(0) } * { x(k) - m(k) + x(0) - m(k-1) }

      const auto delta = value - stale_value;
      const auto stale_mean = mean_;
      mean_ += delta / static_cast<T>(N);
      scaled_variance_ += delta * (value - mean_ + stale_value - stale_mean);
    }
    // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
    head_ += 1U;
    if (head_ == N) {
      head_ = 0U;
    }
    const auto variance = (count_ > 1U) ? (scaled_variance_ / static_cast<T>(count_ - 1)) : T{};
    return { .mean = mean_, .variance = variance };
  }

  /// resets accumulator
  constexpr void reset() {
    buffer_.fill({});
    head_ = {};
    count_ = {};
    mean_ = {};
    scaled_variance_ = {};
  }

private:
  static_assert(N > 1, "Window size N must be > 1");

  std::array<T, N> buffer_{};
  std::size_t head_{};
  std::size_t count_{};
  T mean_{};
  T scaled_variance_{};
};
}  // namespace grape::statistics

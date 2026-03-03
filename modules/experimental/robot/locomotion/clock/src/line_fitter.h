//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cinttypes>
#include <cmath>
#include <numeric>
#include <optional>
#include <vector>

#include "grape/exception.h"

namespace grape::ego_clock {

//=================================================================================================
/// Fit a line to a set of (x,y) data points such that y = slope.x + intercept
///
class LineFitter {
public:
  /// A single data point
  struct DataPoint {
    double x{};
    double y{};
  };

  /// Line fit parameters such that [y = slope * x + intercept]
  struct FitParams {
    double slope{};
    double intercept{};
    double mse{};  //!< Mean Squared Error - measure of fit quality (lower is better)
  };

  /// Constructor
  /// @param window_size Number of samples in the sliding window (must be > 1)
  explicit LineFitter(std::size_t window_size);

  /// Add a data point to the sample set
  void add(const DataPoint& data);

  /// Process the current sample set and compute the fit parameters
  /// @return Fit parameters if enough samples are available
  [[nodiscard]] auto fit() const -> std::optional<FitParams>;

private:
  std::vector<DataPoint> samples_;
  std::size_t write_index_{ 0U };
};

//-------------------------------------------------------------------------------------------------
inline LineFitter::LineFitter(std::size_t window_size) : samples_(window_size) {
  if (window_size <= 1) {
    panic("LineFitter window_size must be greater than 1");
  }
}

//-------------------------------------------------------------------------------------------------
inline void LineFitter::add(const DataPoint& data) {
  samples_.at(write_index_ % samples_.size()) = data;
  write_index_++;
}

#define USE_TRANSFORM_REDUCE
#ifdef USE_TRANSFORM_REDUCE

//-------------------------------------------------------------------------------------------------
inline auto LineFitter::fit() const -> std::optional<FitParams> {
  if (write_index_ < 2U) {
    return std::nullopt;
  }
  const auto buf_size = samples_.size();
  const auto num_samples =
      (write_index_ < buf_size) ? static_cast<double>(write_index_) : static_cast<double>(buf_size);

  // Fit y = slope.x + intercept, such that the squared error is minimized
  // - slope = (n.∑(xᵢ.yᵢ) − ∑xᵢ.∑yᵢ) / (n.∑(xᵢ²) − (∑xᵢ)²),
  // - intercept = (∑yᵢ − slope.∑xᵢ) / n,
  // - mse = ∑(yᵢ - ŷᵢ)² / n, where ŷᵢ = slope.xᵢ + intercept

  const auto componentise = [](const auto& sample) {
    return std::tuple{ sample.x, sample.y, sample.x * sample.x, sample.x * sample.y };
  };
  const auto sum = [](const auto& aa, const auto& bb) {
    return std::tuple{ std::get<0>(aa) + std::get<0>(bb), std::get<1>(aa) + std::get<1>(bb),
                       std::get<2>(aa) + std::get<2>(bb), std::get<3>(aa) + std::get<3>(bb) };
  };
  const auto samples_end =
      (write_index_ < buf_size) ?
          std::next(samples_.begin(), static_cast<std::int64_t>(write_index_)) :
          samples_.end();
  const auto [sum_x, sum_y, sum_xx, sum_xy] = std::transform_reduce(
      samples_.begin(), samples_end, std::tuple{ 0.0, 0.0, 0.0, 0.0 }, sum, componentise);
  const auto denom = std::fma(sum_x, -sum_x, num_samples * sum_xx);
  const auto slope = std::fma(sum_x, -sum_y, num_samples * sum_xy) / denom;
  const auto intercept = std::fma(slope, -sum_x, sum_y) / num_samples;

  // Compute mean-squared-error
  const auto error_squared = [slope, intercept](const auto& sample) {
    const auto predicted_y = std::fma(slope, sample.x, intercept);
    const auto error = sample.y - predicted_y;
    return error * error;
  };
  const auto sum_error_squared =
      std::transform_reduce(samples_.begin(), samples_end, 0.0, std::plus<>{}, error_squared);
  const auto mse = sum_error_squared / num_samples;

  return FitParams{ .slope = slope, .intercept = intercept, .mse = mse };
}

#else

//-------------------------------------------------------------------------------------------------
/// Implementation using standard for loops instead of transform_reduce
[[nodiscard]] inline auto LineFitter::fit() const -> std::optional<FitParams> {
  if (write_index_ < 2U) {
    return std::nullopt;
  }

  // Fit y = slope.x + intercept, such that the squared error is minimized
  // - slope = (n.∑(xᵢ.yᵢ) − ∑xᵢ.∑yᵢ) / (n.∑(xᵢ²) − (∑xᵢ)²),
  // - intercept = (∑yᵢ − slope.∑xᵢ) / n,
  // - mse = ∑(yᵢ - ŷᵢ)² / n, where ŷᵢ = slope.xᵢ + intercept

  // Compute sums using standard for loops
  double sum_x = 0.0;
  double sum_y = 0.0;
  double sum_xx = 0.0;
  double sum_xy = 0.0;

  const auto buf_size = samples_.size();

  const auto samples_end =
      (write_index_ < buf_size) ?
          std::next(samples_.begin(), static_cast<std::int64_t>(write_index_)) :
          samples_.end();

  for (const auto& sample : std::span{ samples_.begin(), samples_end }) {
    sum_x += sample.x;
    sum_y += sample.y;
    sum_xx += sample.x * sample.x;
    sum_xy += sample.x * sample.y;
  }

  const auto num_samples = static_cast<double>((write_index_ < buf_size) ? write_index_ : buf_size);
  const auto denom = std::fma(sum_x, -sum_x, num_samples * sum_xx);
  const auto slope = std::fma(sum_x, -sum_y, num_samples * sum_xy) / denom;
  const auto intercept = std::fma(slope, -sum_x, sum_y) / num_samples;

  // Compute mean-squared-error using standard for loop
  double sum_error_squared = 0.0;
  for (const auto& sample : std::span{ samples_.begin(), samples_end }) {
    const auto predicted_y = std::fma(slope, sample.x, intercept);
    const auto error = sample.y - predicted_y;
    sum_error_squared += error * error;
  }
  const auto mse = sum_error_squared / num_samples;

  return FitParams{ .slope = slope, .intercept = intercept, .mse = mse };
}
#endif
}  // namespace grape::ego_clock

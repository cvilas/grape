//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <print>
#include <random>

#include "grape/statistics/sliding_mean.h"

//=================================================================================================
// Simulates temperature readings sampled every second with 1-minute sliding average
//
auto main() -> int {  // NOLINT(bugprone-exception-escape)
  // 60 seconds of sampled data at 1 sample per second
  static constexpr auto WINDOW_SIZE_SECONDS = 60UL;
  auto filter = grape::statistics::SlidingMean<double, WINDOW_SIZE_SECONDS>{};

  // Simulation parameters
  static constexpr auto AMBIENT_TEMP = 20.0;    // 20°C ambient temperature
  static constexpr auto NOISE_AMPLITUDE = 2.0;  // ±2°C sensor noise

  // Random number generation for simulating sensor
  std::random_device rd;
  std::mt19937 gen(rd());
  auto noise = std::uniform_real_distribution<double>(-NOISE_AMPLITUDE, NOISE_AMPLITUDE);

  std::println("Time(s)  Raw Temp(°C)  Filtered(°C)  Std Dev(°C)");
  std::println("-------  ------------  ------------  -----------");

  // Simulate 3 minutes of temperature readings
  static constexpr auto MAX_SECONDS = 120U;
  static constexpr auto REPORT_INTERVAL = 10U;
  for (auto second = 0U; second <= MAX_SECONDS; ++second) {
    const double slow_variation = std::sin(second * 0.01);
    const double sensor_noise = noise(gen);
    const double measured_temperature = AMBIENT_TEMP + slow_variation + sensor_noise;

    // Update the sliding window filter
    const auto stats = filter.append(measured_temperature);
    const double std_dev = std::sqrt(stats.variance);

    if (second % REPORT_INTERVAL == 0) {
      std::println("{:>7}  {:>12.2f}  {:>12.2f}  {:>11.2f}", second, measured_temperature,
                   stats.mean, std_dev);
    }
  }
  return EXIT_SUCCESS;
}

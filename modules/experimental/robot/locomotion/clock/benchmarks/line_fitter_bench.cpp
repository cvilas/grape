//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <array>
#include <random>

#include <benchmark/benchmark.h>

#include "../src/line_fitter.h"

namespace {

using LineFitter = grape::ego_clock::LineFitter;
using DataPoint = LineFitter::DataPoint;

//-------------------------------------------------------------------------------------------------
template <std::size_t NumPoints>
auto createSampleData() -> std::array<DataPoint, NumPoints> {
  constexpr double SLOPE = 2.0;
  constexpr double INTERCEPT = 1.0;
  constexpr double NOISE_LEVEL = 0.1;

  auto data = std::array<DataPoint, NumPoints>{};

  auto rd = std::random_device{};
  auto gen = std::mt19937{ rd() };
  auto noise_dist = std::normal_distribution<>{ 0.0, NOISE_LEVEL };

  for (auto i = 0UZ; i < NumPoints; ++i) {
    const auto x_value = static_cast<double>(i);
    const auto y_value = (SLOPE * x_value) + INTERCEPT + noise_dist(gen);
    data.at(i) = DataPoint{ .x = x_value, .y = y_value };
  }

  return data;
}

//-------------------------------------------------------------------------------------------------
// Benchmark for LineFitter::fit() with different sizes of data
template <std::size_t NumSamples>
void bmLineFitterFit(benchmark::State& state) {
  auto fitter = LineFitter(NumSamples);
  const auto sample_data = createSampleData<NumSamples>();

  for (const auto& point : sample_data) {
    fitter.add(point);
  }

  for (auto unused : state) {
    (void)unused;
    auto result = fitter.fit();
    benchmark::DoNotOptimize(result);
    benchmark::ClobberMemory();
  }
}

BENCHMARK_TEMPLATE(bmLineFitterFit, 16);
BENCHMARK_TEMPLATE(bmLineFitterFit, 32);
BENCHMARK_TEMPLATE(bmLineFitterFit, 64);
BENCHMARK_TEMPLATE(bmLineFitterFit, 128);
BENCHMARK_TEMPLATE(bmLineFitterFit, 256);
BENCHMARK_TEMPLATE(bmLineFitterFit, 512);
BENCHMARK_TEMPLATE(bmLineFitterFit, 1024);

}  // namespace

BENCHMARK_MAIN();

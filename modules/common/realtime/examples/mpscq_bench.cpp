//=================================================================================================
// Copyright (C) 2018 GRAPE Contributors
//=================================================================================================

#include <array>
#include <stdexcept>

#include <benchmark/benchmark.h>

#include "grape/realtime/mpsc_queue.h"

namespace {
struct Item {
  static constexpr auto DATA_SIZE = 1024U;
  std::array<std::byte, DATA_SIZE> data;
};

//-------------------------------------------------------------------------------------------------
void bmMpscqPush(benchmark::State& state) {
  const auto capacity = static_cast<std::size_t>(state.max_iterations);
  auto queue = grape::realtime::MPSCQueue<Item>(capacity);

  for (auto st : state) {
    (void)st;
    if (not queue.tryPush(Item{})) {
      throw std::runtime_error("tryPush failed");
    }
    benchmark::ClobberMemory();
  }
}

//-------------------------------------------------------------------------------------------------
void bmMpscqPop(benchmark::State& state) {
  const auto capacity = static_cast<std::size_t>(state.max_iterations);
  auto queue = grape::realtime::MPSCQueue<Item>(capacity);

  // fill the queue
  for (auto i = 0UZ; i < capacity; ++i) {
    if (not queue.tryPush(Item{})) {
      throw std::runtime_error("tryPush failed");
    }
  }

  for (auto st : state) {
    (void)st;
    auto item = queue.tryPop();
    benchmark::DoNotOptimize(item);
    if (not item.has_value()) {
      throw std::runtime_error("tryPop failed");
    }
    benchmark::ClobberMemory();
  }
}

constexpr auto MAX_ITERATIONS = 1000000U;

BENCHMARK(bmMpscqPush)->Iterations(MAX_ITERATIONS);
BENCHMARK(bmMpscqPop)->Iterations(MAX_ITERATIONS);
}  // namespace

BENCHMARK_MAIN();

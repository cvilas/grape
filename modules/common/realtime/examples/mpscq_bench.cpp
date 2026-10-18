//=================================================================================================
// Copyright (C) 2018 GRAPE Contributors
//=================================================================================================

#include <array>
#include <cstdint>
#include <latch>
#include <stdexcept>
#include <thread>
#include <vector>

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

  for (const auto st : state) {
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

  for (const auto st : state) {
    (void)st;
    auto item = queue.tryPop();
    benchmark::DoNotOptimize(item);
    if (not item.has_value()) {
      throw std::runtime_error("tryPop failed");
    }
    benchmark::ClobberMemory();
  }
}

//-------------------------------------------------------------------------------------------------
void bmMpscqConcurrent(benchmark::State& state) {
  static constexpr auto CAPACITY = 1024UZ;
  static constexpr auto ITEMS_PER_PRODUCER = 10'000UZ;
  const auto num_producers = static_cast<std::size_t>(state.range(0));
  for (const auto st : state) {
    (void)st;
    state.PauseTiming();
    auto queue = grape::realtime::MPSCQueue<std::uint64_t>(CAPACITY);
    std::latch ready{ static_cast<std::ptrdiff_t>(num_producers + 1U) };
    std::latch start{ 1 };

    std::thread consumer([&] {
      ready.count_down();
      start.wait();
      const auto total_items = num_producers * ITEMS_PER_PRODUCER;
      for (auto consumed = 0UZ; consumed < total_items;) {
        if (auto item = queue.tryPop()) {
          benchmark::DoNotOptimize(item);
          ++consumed;
        } else {
          std::this_thread::yield();
        }
      }
    });

    std::vector<std::thread> producers;
    producers.reserve(num_producers);
    for (std::size_t producer = 0; producer < num_producers; ++producer) {
      producers.emplace_back([&] {
        ready.count_down();
        start.wait();
        for (auto produced = 0UZ; produced < ITEMS_PER_PRODUCER;) {
          if (queue.tryPush(std::uint64_t{ 1U })) {
            ++produced;
          } else {
            std::this_thread::yield();
          }
        }
      });
    }

    ready.wait();
    state.ResumeTiming();
    start.count_down();
    for (auto& producer : producers) {
      producer.join();
    }
    consumer.join();
    state.PauseTiming();
  }
  state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(num_producers) *
                          static_cast<std::int64_t>(ITEMS_PER_PRODUCER));
}

constexpr auto MAX_ITERATIONS = 1000000U;

BENCHMARK(bmMpscqPush)->Iterations(MAX_ITERATIONS);
BENCHMARK(bmMpscqPop)->Iterations(MAX_ITERATIONS);
BENCHMARK(bmMpscqConcurrent)->Arg(1)->Arg(2)->Arg(4)->Iterations(10)->UseRealTime();
}  // namespace

BENCHMARK_MAIN();

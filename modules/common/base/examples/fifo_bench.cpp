//=================================================================================================
// Copyright (C) 2018 GRAPE Contributors
//=================================================================================================

#include <cstring>
#include <latch>
#include <stdexcept>
#include <thread>
#include <vector>

#include <benchmark/benchmark.h>

#include "grape/fifo_buffer.h"

namespace {

//-------------------------------------------------------------------------------------------------
void bmFifoWrite(benchmark::State& state) {
  using Fifo = grape::FIFOBuffer;
  const auto data_size = static_cast<std::size_t>(state.range(0));
  const auto config = Fifo::Config{
    .frame_length = data_size,
    .num_frames = static_cast<std::size_t>(state.max_iterations),
  };
  auto fifo = grape::FIFOBuffer(config);

  std::vector<std::byte> data(data_size);
  const auto writer = [&data](std::span<std::byte> buffer) -> void {
    std::memcpy(buffer.data(), data.data(), data.size());
  };
  bool succeeded = false;
  for (const auto st : state) {
    (void)st;
    benchmark::DoNotOptimize(succeeded = fifo.visitToWrite(writer));
    if (not succeeded) {
      throw std::runtime_error("write failed");
    }
    benchmark::ClobberMemory();
  }
}

//-------------------------------------------------------------------------------------------------
void bmFifoRead(benchmark::State& state) {
  using Fifo = grape::FIFOBuffer;
  const auto data_size = static_cast<std::size_t>(state.range(0));
  const auto config = Fifo::Config{
    .frame_length = data_size,
    .num_frames = static_cast<std::size_t>(state.max_iterations),
  };
  auto fifo = grape::FIFOBuffer(config);

  std::vector<std::byte> data(data_size);
  const auto writer = [&data](std::span<std::byte> buffer) -> void {
    std::memcpy(buffer.data(), data.data(), data.size());
  };

  // fill the queue
  for (auto fr = 0UZ; fr < config.num_frames; ++fr) {
    if (not fifo.visitToWrite(writer)) {
      throw std::runtime_error("write failed");
    }
  }

  const auto reader = [&data](std::span<const std::byte> buffer) -> void {
    std::memcpy(data.data(), buffer.data(), buffer.size_bytes());
  };

  bool succeeded = false;
  for (const auto st : state) {
    (void)st;
    benchmark::DoNotOptimize(succeeded = fifo.visitToRead(reader));
    if (not succeeded) {
      throw std::runtime_error("read failed");
    }
    benchmark::ClobberMemory();
  }
}

//-------------------------------------------------------------------------------------------------
void bmFifoConcurrent(benchmark::State& state) {
  static constexpr auto CAPACITY = 1024UZ;
  static constexpr auto ITEMS_PER_PRODUCER = 10'000UZ;
  const auto num_producers = static_cast<std::size_t>(state.range(0));
  for (const auto st : state) {
    (void)st;
    state.PauseTiming();
    auto fifo =
        grape::FIFOBuffer({ .frame_length = sizeof(std::uint64_t), .num_frames = CAPACITY });
    std::latch ready{ static_cast<std::ptrdiff_t>(num_producers + 1U) };
    std::latch start{ 1 };

    std::thread consumer([&] {
      ready.count_down();
      start.wait();
      const auto total_items = num_producers * ITEMS_PER_PRODUCER;
      for (auto consumed = 0UZ; consumed < total_items;) {
        const auto read = fifo.visitToRead([](std::span<const std::byte> frame) {
          std::uint64_t value{};
          std::memcpy(&value, frame.data(), sizeof(value));
          benchmark::DoNotOptimize(value);
        });
        if (read) {
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
        const std::uint64_t value = 1U;
        const auto writer = [value](std::span<std::byte> frame) {
          std::memcpy(frame.data(), &value, sizeof(value));
        };
        for (auto produced = 0UZ; produced < ITEMS_PER_PRODUCER;) {
          if (fifo.visitToWrite(writer)) {
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
constexpr auto DATA_SIZE_MULT = 2U;
constexpr auto DATA_SIZE_MIN = 8;
constexpr auto DATA_SIZE_MAX = 2048;

BENCHMARK(bmFifoWrite)
    ->RangeMultiplier(DATA_SIZE_MULT)
    ->Range(DATA_SIZE_MIN, DATA_SIZE_MAX)
    ->Iterations(MAX_ITERATIONS);

BENCHMARK(bmFifoRead)
    ->RangeMultiplier(DATA_SIZE_MULT)
    ->Range(DATA_SIZE_MIN, DATA_SIZE_MAX)
    ->Iterations(MAX_ITERATIONS);

BENCHMARK(bmFifoConcurrent)->Arg(1)->Arg(2)->Arg(4)->Iterations(10)->UseRealTime();

}  // namespace

BENCHMARK_MAIN();

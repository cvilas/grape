//=================================================================================================
// Copyright (C) 2018 GRAPE Contributors
//=================================================================================================

#include <cstring>
#include <stdexcept>

#include <benchmark/benchmark.h>

#include "grape/realtime/fifo_buffer.h"

namespace {

//-------------------------------------------------------------------------------------------------
void bmFifoWrite(benchmark::State& state) {
  using Fifo = grape::realtime::FIFOBuffer;
  const auto data_size = static_cast<std::size_t>(state.range(0));
  const auto config = Fifo::Config{ .frame_length = data_size,
                                    .num_frames = static_cast<std::size_t>(state.max_iterations) };
  auto fifo = grape::realtime::FIFOBuffer(config);

  std::vector<std::byte> data(data_size);
  const auto writer = [&data](std::span<std::byte> buffer) {
    std::memcpy(buffer.data(), data.data(), data.size());
  };
  bool succeeded = false;
  for (auto s : state) {
    (void)s;
    benchmark::DoNotOptimize(succeeded = fifo.visitToWrite(writer));
    if (not succeeded) {
      throw std::runtime_error("write failed");
    }
    benchmark::ClobberMemory();
  }
}

//-------------------------------------------------------------------------------------------------
void bmFifoRead(benchmark::State& state) {
  using Fifo = grape::realtime::FIFOBuffer;
  const auto data_size = static_cast<std::size_t>(state.range(0));
  const auto config = Fifo::Config{ .frame_length = data_size,
                                    .num_frames = static_cast<std::size_t>(state.max_iterations) };
  auto fifo = grape::realtime::FIFOBuffer(config);

  std::vector<std::byte> data(data_size);
  const auto writer = [&data](std::span<std::byte> buffer) {
    std::memcpy(buffer.data(), data.data(), data.size());
  };

  // fill the queue
  for (auto i = 0UZ; i < config.num_frames; ++i) {
    if (not fifo.visitToWrite(writer)) {
      throw std::runtime_error("write failed");
    }
  }

  const auto reader = [&data](std::span<const std::byte> buffer) {
    std::memcpy(data.data(), buffer.data(), buffer.size_bytes());
  };

  bool succeeded = false;
  for (auto s : state) {
    (void)s;
    benchmark::DoNotOptimize(succeeded = fifo.visitToRead(reader));
    if (not succeeded) {
      throw std::runtime_error("read failed");
    }
    benchmark::ClobberMemory();
  }
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

}  // namespace

BENCHMARK_MAIN();

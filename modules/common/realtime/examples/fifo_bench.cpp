//=================================================================================================
// Copyright (C) 2018 GRAPE Contributors
//=================================================================================================

#include <benchmark/benchmark.h>

#include "grape/realtime/fifo_buffer.h"

namespace {

//-------------------------------------------------------------------------------------------------
void bmFifoWrite(benchmark::State& state) {
  using Fifo = grape::realtime::FIFOBuffer;
  const auto data_size = static_cast<std::size_t>(state.range(0));
  const auto config = Fifo::Options{ .frame_length = data_size,
                                     .num_frames = static_cast<std::size_t>(state.max_iterations) };
  auto fifo = grape::realtime::FIFOBuffer(config);

  std::vector<std::byte> data(data_size);
  const auto writer = [&data](std::span<std::byte> buffer) {
    std::memcpy(buffer.data(), data.data(), data.size());
  };
  for (auto s : state) {
    (void)s;
    if (not fifo.visitToWrite(writer)) {
      throw std::runtime_error("write failed");
    }
  }
}

//-------------------------------------------------------------------------------------------------
void bmFifoRead(benchmark::State& state) {
  using Fifo = grape::realtime::FIFOBuffer;
  const auto data_size = static_cast<std::size_t>(state.range(0));
  const auto config = Fifo::Options{ .frame_length = data_size,
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

  for (auto s : state) {
    (void)s;
    if (not fifo.visitToRead(reader)) {
      throw std::runtime_error("read failed");
    }
  }
}
}  // namespace

static constexpr auto MAX_ITERATIONS = 1000000U;
static constexpr auto DATA_SIZE_MULT = 2U;
static constexpr auto DATA_SIZE_MIN = 8;
static constexpr auto DATA_SIZE_MAX = 4096;

// NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables)
BENCHMARK(bmFifoWrite)
    ->RangeMultiplier(DATA_SIZE_MULT)
    ->Range(DATA_SIZE_MIN, DATA_SIZE_MAX)
    ->Iterations(MAX_ITERATIONS);

// NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables)
BENCHMARK(bmFifoRead)
    ->RangeMultiplier(DATA_SIZE_MULT)
    ->Range(DATA_SIZE_MIN, DATA_SIZE_MAX)
    ->Iterations(MAX_ITERATIONS);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,cppcoreguidelines-pro-bounds-array-to-pointer-decay,modernize-use-trailing-return-type)
BENCHMARK_MAIN();

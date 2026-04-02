//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <atomic>
#include <cstring>
#include <format>
#include <stdexcept>
#include <vector>

#include <benchmark/benchmark.h>

#include "grape/realtime/spmc_ring_buffer.h"

namespace {

using Config = grape::spmc_ring_buffer::Config;
using Reader = grape::spmc_ring_buffer::Reader;
using Writer = grape::spmc_ring_buffer::Writer;

//-------------------------------------------------------------------------------------------------
auto uniqueShmName() -> std::string {
  static std::atomic<std::uint32_t> counter{ 0 };
  return std::format("spmcq_bench_{}", counter.fetch_add(1, std::memory_order_relaxed));
}

//-------------------------------------------------------------------------------------------------
void bmSpmcqWrite(benchmark::State& state) {
  const auto data_size = static_cast<std::size_t>(state.range(0));
  // Fixed capacity: ring semantics mean writes always succeed regardless of reader pace.
  static constexpr auto CAPACITY = 1024UZ;
  const auto config = Config{ .frame_length = data_size, .num_frames = CAPACITY };
  auto maybe_writer = Writer::create(uniqueShmName(), config);
  if (not maybe_writer) {
    throw std::runtime_error(std::string{ maybe_writer.error().message() });
  }
  auto& writer = maybe_writer.value();

  std::vector<std::byte> data(data_size);
  const auto writer_fn = [&data](std::span<std::byte> buffer) -> bool {
    std::memcpy(buffer.data(), data.data(), data.size());
    return true;
  };

  for (auto st : state) {
    (void)st;
    writer.visit(writer_fn);
    benchmark::ClobberMemory();
  }
}

//-------------------------------------------------------------------------------------------------
void bmSpmcqRead(benchmark::State& state) {
  const auto data_size = static_cast<std::size_t>(state.range(0));
  const auto num_frames = static_cast<std::size_t>(state.max_iterations);
  const auto shm_name = uniqueShmName();
  // capacity is num_frames + 1 so that after pre-filling num_frames writes,
  // read_lag = num_frames < capacity holds (read_lag == capacity is treated as Dropped).
  const auto config = Config{ .frame_length = data_size, .num_frames = num_frames + 1UZ };

  auto maybe_writer = Writer::create(shm_name, config);
  if (not maybe_writer) {
    throw std::runtime_error(std::string{ maybe_writer.error().message() });
  }
  auto maybe_reader = Reader::connect(shm_name);
  if (not maybe_reader) {
    throw std::runtime_error(std::string{ maybe_reader.error().message() });
  }
  auto& writer = maybe_writer.value();
  auto& reader = maybe_reader.value();

  std::vector<std::byte> data(data_size);
  const auto writer_fn = [&data](std::span<std::byte> buffer) -> bool {
    std::memcpy(buffer.data(), data.data(), data.size());
    return true;
  };

  // Pre-fill the ring buffer so the reader always has frames available.
  for (auto i = 0UZ; i < num_frames; ++i) {
    writer.visit(writer_fn);
  }

  const auto reader_fn = [&data](std::span<const std::byte> buffer) -> bool {
    std::memcpy(data.data(), buffer.data(), buffer.size_bytes());
    return true;
  };

  for (auto st : state) {
    (void)st;
    auto status = reader.visit(reader_fn);
    benchmark::DoNotOptimize(status);
    if (status != Reader::Status::Ok) {
      throw std::runtime_error("read failed");
    }
    benchmark::ClobberMemory();
  }
}

constexpr auto MAX_ITERATIONS = 1000000U;
constexpr auto DATA_SIZE_MULT = 2U;
constexpr auto DATA_SIZE_MIN = 8;
constexpr auto DATA_SIZE_MAX = 2048;

BENCHMARK(bmSpmcqWrite)
    ->RangeMultiplier(DATA_SIZE_MULT)
    ->Range(DATA_SIZE_MIN, DATA_SIZE_MAX)
    ->Iterations(MAX_ITERATIONS);

BENCHMARK(bmSpmcqRead)
    ->RangeMultiplier(DATA_SIZE_MULT)
    ->Range(DATA_SIZE_MIN, DATA_SIZE_MAX)
    ->Iterations(MAX_ITERATIONS);

}  // namespace

BENCHMARK_MAIN();

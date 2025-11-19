//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <array>
#include <cstring>
#include <random>
#include <stdexcept>

#include <benchmark/benchmark.h>

#include "grape/realtime/shared_memory.h"

namespace {

using ShMem = grape::realtime::SharedMemory;

constexpr auto SMALL_SIZE = 1024U;     // 1KB
constexpr auto MEDIUM_SIZE = 65536U;   // 64KB
constexpr auto LARGE_SIZE = 1048576U;  // 1MB

struct TestData {
  static constexpr auto DATA_SIZE = SMALL_SIZE;
  std::array<std::byte, DATA_SIZE> data;
};

//-------------------------------------------------------------------------------------------------
auto createUniqueShmName() -> std::string {
  static auto counter = std::atomic_int{ 0 };
  return "/grape_bench_shm_" + std::to_string(counter++);
}

//-------------------------------------------------------------------------------------------------
auto createRandomData(std::size_t size) -> std::vector<std::byte> {
  std::random_device rd;
  std::mt19937 gen(rd());
  constexpr auto MAX_BYTE_VALUE = 0xFF;
  std::uniform_int_distribution<int> dis(0, MAX_BYTE_VALUE);

  std::vector<std::byte> data(size);
  for (auto& byte : data) {
    byte = static_cast<std::byte>(dis(gen));
  }
  return data;
}

//-------------------------------------------------------------------------------------------------
void bmShmCreate(benchmark::State& state) {
  const auto size = static_cast<std::size_t>(state.range(0));

  for (auto st : state) {
    (void)st;
    const auto shm_name = createUniqueShmName();
    std::ignore = ShMem::remove(shm_name);

    auto shm_result = ShMem::create(shm_name, size, ShMem::Access::ReadWrite);
    if (not shm_result.has_value()) {
      throw std::runtime_error(std::string{ shm_result.error().message() });
    }

    benchmark::DoNotOptimize(shm_result);

    shm_result.value().close();
    std::ignore = ShMem::remove(shm_name);
  }
}

//-------------------------------------------------------------------------------------------------
void bmShmOpen(benchmark::State& state) {
  const auto size = static_cast<std::size_t>(state.range(0));
  const auto shm_name = createUniqueShmName();
  std::ignore = ShMem::remove(shm_name);

  // Create the shared memory once for all iterations
  auto shm_create_result = ShMem::create(shm_name, size, ShMem::Access::ReadWrite);
  if (not shm_create_result.has_value()) {
    throw std::runtime_error(std::string{ shm_create_result.error().message() });
  }
  auto& created_shm = shm_create_result.value();

  for (auto st : state) {
    (void)st;

    auto shm_result = ShMem::open(shm_name, ShMem::Access::ReadOnly);
    if (not shm_result.has_value()) {
      throw std::runtime_error(std::string{ shm_result.error().message() });
    }

    benchmark::DoNotOptimize(shm_result);

    auto& shm = shm_result.value();
    shm.close();
  }

  created_shm.close();
  std::ignore = ShMem::remove(shm_name);
}

//-------------------------------------------------------------------------------------------------
void bmShmWrite(benchmark::State& state) {
  const auto size = static_cast<std::size_t>(state.range(0));
  const auto shm_name = createUniqueShmName();
  const auto test_data = createRandomData(size);
  std::ignore = ShMem::remove(shm_name);

  auto shm_result = ShMem::create(shm_name, size, ShMem::Access::ReadWrite);
  if (not shm_result.has_value()) {
    throw std::runtime_error(std::string{ shm_result.error().message() });
  }
  auto& shm = shm_result.value();

  for (auto st : state) {
    (void)st;

    std::memcpy(shm.data().data(), test_data.data(), size);
    benchmark::ClobberMemory();
  }

  shm.close();
  std::ignore = ShMem::remove(shm_name);

  state.SetBytesProcessed(
      static_cast<int64_t>(static_cast<std::size_t>(state.iterations()) * size));
}

//-------------------------------------------------------------------------------------------------
void bmShmRead(benchmark::State& state) {
  const auto size = static_cast<std::size_t>(state.range(0));
  const auto shm_name = createUniqueShmName();
  const auto test_data = createRandomData(size);
  std::ignore = ShMem::remove(shm_name);

  auto shm_result = ShMem::create(shm_name, size, ShMem::Access::ReadWrite);
  if (not shm_result.has_value()) {
    throw std::runtime_error(std::string{ shm_result.error().message() });
  }
  auto& shm = shm_result.value();

  // Write test data once
  std::memcpy(shm.data().data(), test_data.data(), size);

  std::vector<std::byte> read_buffer(size);

  for (auto st : state) {
    (void)st;

    std::memcpy(read_buffer.data(), shm.data().data(), size);
    benchmark::DoNotOptimize(read_buffer);
  }

  shm.close();
  std::ignore = ShMem::remove(shm_name);

  state.SetBytesProcessed(
      static_cast<int64_t>(static_cast<std::size_t>(state.iterations()) * size));
}

//-------------------------------------------------------------------------------------------------
void bmShmCopy(benchmark::State& state) {
  const auto size = static_cast<std::size_t>(state.range(0));
  const auto src_name = createUniqueShmName();
  const auto dst_name = createUniqueShmName();
  const auto test_data = createRandomData(size);
  std::ignore = ShMem::remove(src_name);
  std::ignore = ShMem::remove(dst_name);

  auto src_result = ShMem::create(src_name, size, ShMem::Access::ReadWrite);
  if (not src_result.has_value()) {
    throw std::runtime_error(std::string{ src_result.error().message() });
  }
  auto dst_result = ShMem::create(dst_name, size, ShMem::Access::ReadWrite);
  if (not dst_result.has_value()) {
    throw std::runtime_error(std::string{ dst_result.error().message() });
  }

  auto& src_shm = src_result.value();
  auto& dst_shm = dst_result.value();

  // Write test data to source
  std::memcpy(src_shm.data().data(), test_data.data(), size);

  for (auto st : state) {
    (void)st;

    std::memcpy(dst_shm.data().data(), src_shm.data().data(), size);
    benchmark::ClobberMemory();
  }

  src_shm.close();
  dst_shm.close();
  std::ignore = ShMem::remove(src_name);
  std::ignore = ShMem::remove(dst_name);

  state.SetBytesProcessed(
      static_cast<int64_t>(static_cast<std::size_t>(state.iterations()) * size));
}

constexpr auto MIN_ITERATIONS = 1000U;
constexpr auto MAX_ITERATIONS = 10000U;

// Test different sizes: 1KB, 64KB, 1MB
BENCHMARK(bmShmCreate)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE)
    ->Arg(LARGE_SIZE)
    ->Iterations(MIN_ITERATIONS);
BENCHMARK(bmShmOpen)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE)
    ->Arg(LARGE_SIZE)
    ->Iterations(MIN_ITERATIONS);
BENCHMARK(bmShmWrite)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE)
    ->Arg(LARGE_SIZE)
    ->Iterations(MAX_ITERATIONS);
BENCHMARK(bmShmRead)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE)
    ->Arg(LARGE_SIZE)
    ->Iterations(MAX_ITERATIONS);
BENCHMARK(bmShmCopy)
    ->Arg(SMALL_SIZE)
    ->Arg(MEDIUM_SIZE)
    ->Arg(LARGE_SIZE)
    ->Iterations(MAX_ITERATIONS);

}  // namespace

BENCHMARK_MAIN();

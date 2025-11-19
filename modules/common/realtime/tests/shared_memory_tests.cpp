//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <algorithm>
#include <chrono>
#include <cstring>
#include <random>
#include <thread>

#include "catch2/catch_test_macros.hpp"
#include "grape/realtime/shared_memory.h"

namespace {

using ShMem = grape::realtime::SharedMemory;

constexpr auto TEST_SHM_SIZE = 1024U;

//-------------------------------------------------------------------------------------------------
auto createUniqueShmName() -> std::string {
  static auto counter = std::atomic_int{ 0 };
  return "/test_shm_" + std::to_string(counter++);
}

//-------------------------------------------------------------------------------------------------
[[maybe_unused]] auto createRandomData() -> std::array<std::byte, TEST_SHM_SIZE> {
  std::random_device rd;
  std::mt19937 gen(rd());
  const auto range = std::pair<int, int>{ 0, 0xFF };
  std::uniform_int_distribution<int> dist(range.first, range.second);

  std::array<std::byte, TEST_SHM_SIZE> data{};
  std::ranges::generate(data, [&dist, &gen]() noexcept { return std::byte(dist(gen)); });
  return data;
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Basic creation and existence checks", "[SharedMemory]") {
  // Creating an shm with invalid name fails
  const auto* const invalid_shm_name = "/invalid/name/format";
  REQUIRE_FALSE(ShMem::create(invalid_shm_name, TEST_SHM_SIZE, ShMem::Access::ReadWrite));

  const auto test_shm_name = createUniqueShmName();

  // shouldn't exist yet
  REQUIRE_FALSE(ShMem::exists(test_shm_name));
  REQUIRE_FALSE(ShMem::remove(test_shm_name));

  // opening a non-existent shared memory fails
  REQUIRE_FALSE(ShMem::open(test_shm_name, ShMem::Access::ReadOnly));

  // create, then check
  auto maybe_shm = ShMem::create(test_shm_name, TEST_SHM_SIZE, ShMem::Access::ReadWrite);
  REQUIRE(ShMem::exists(test_shm_name));

  // Check requested data size
  REQUIRE(maybe_shm.has_value());
  REQUIRE(maybe_shm->data().size() == TEST_SHM_SIZE);

  // Creating shared memory that already exists should fail
  REQUIRE_FALSE(ShMem::create(test_shm_name, TEST_SHM_SIZE, ShMem::Access::ReadWrite));

  // Opening an existing shared memory object should succeed
  auto maybe_shm_open = ShMem::open(test_shm_name, ShMem::Access::ReadOnly);
  REQUIRE(maybe_shm_open.has_value());

  // Opened shared memory segment should have the expected size
  REQUIRE(maybe_shm_open->data().size() == TEST_SHM_SIZE);

  REQUIRE(ShMem::remove(test_shm_name));
  REQUIRE_FALSE(ShMem::exists(test_shm_name));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Basic write and read", "[SharedMemory]") {
  const auto test_shm_name = createUniqueShmName();
  const auto test_data = createRandomData();

  {  // create shm and write test data
    const auto shm_size = test_data.size();
    auto shm_opt = ShMem::create(test_shm_name, shm_size, ShMem::Access::ReadWrite);
    REQUIRE(shm_opt.has_value());
    auto& shm = shm_opt.value();
    std::ranges::copy(test_data, std::begin(shm.data()));
    shm.close();
  }
  {  // open the shared memory, read it and confirm data is as expected
    auto shm_opt = ShMem::open(test_shm_name, ShMem::Access::ReadOnly);
    REQUIRE(shm_opt.has_value());
    auto& shm = shm_opt.value();
    REQUIRE(std::ranges::equal(shm.data(), test_data));
    shm.close();

    // Data span should be empty after close
    REQUIRE(shm.data().empty());
    REQUIRE(shm.data().data() == nullptr);
  }

  std::ignore = ShMem::remove(test_shm_name);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("SharedMemory move semantics", "[SharedMemory]") {
  SECTION("Move constructor works correctly") {
    const auto test_shm_name = createUniqueShmName();
    auto maybe_shm = ShMem::create(test_shm_name, TEST_SHM_SIZE, ShMem::Access::ReadWrite);
    REQUIRE(maybe_shm.has_value());

    const auto original_data = maybe_shm->data();
    REQUIRE_FALSE(original_data.empty());

    // Move construct
    auto moved_shm = std::move(maybe_shm.value());

    // Original should be empty, moved should have the data
    REQUIRE(maybe_shm->data().empty());
    REQUIRE(moved_shm.data().data() == original_data.data());
    REQUIRE(moved_shm.data().size() == original_data.size());

    moved_shm.close();
    std::ignore = ShMem::remove(test_shm_name);
  }

  SECTION("Move assignment works correctly") {
    const auto test_shm_name = createUniqueShmName();
    const auto test_shm_name2 = createUniqueShmName();
    auto result1 = ShMem::create(test_shm_name, TEST_SHM_SIZE, ShMem::Access::ReadWrite);
    auto result2 = ShMem::create(test_shm_name2, TEST_SHM_SIZE * 2UZ, ShMem::Access::ReadWrite);
    REQUIRE(result1.has_value());
    REQUIRE(result2.has_value());

    [[maybe_unused]] auto original_data1 = result1->data();
    auto original_data2 = result2->data();

    // Move assign
    result1.value() = std::move(result2.value());

    // result1 should now have result2's data, result2 should be empty
    REQUIRE(result2->data().empty());
    REQUIRE(result1->data().data() == original_data2.data());
    REQUIRE(result1->data().size() == original_data2.size());

    result1->close();
    std::ignore = ShMem::remove(test_shm_name);
    std::ignore = ShMem::remove(test_shm_name2);
  }
}

}  // namespace

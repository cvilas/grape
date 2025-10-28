//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include <algorithm>
#include <chrono>
#include <cstring>
#include <thread>

#include "catch2/catch_test_macros.hpp"
#include "grape/realtime/shared_memory.h"

namespace {

using grape::realtime::SharedMemory;

constexpr const char* TEST_SHM_NAME = "/test_shared_memory";
constexpr const char* TEST_SHM_NAME_2 = "/test_shared_memory_2";
constexpr std::size_t TEST_SIZE = 1024;

// Helper to clean up any existing shared memory from previous tests
void cleanup_test_shm() {
  std::ignore = SharedMemory::remove(TEST_SHM_NAME);
  std::ignore = SharedMemory::remove(TEST_SHM_NAME_2);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("SharedMemory static methods", "[SharedMemory]") {
  cleanup_test_shm();

  SECTION("exists() returns false for non-existent shared memory") {
    REQUIRE_FALSE(SharedMemory::exists(TEST_SHM_NAME));
  }

  SECTION("remove() fails for non-existent shared memory") {
    auto result = SharedMemory::remove(TEST_SHM_NAME);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().message().find("No such file or directory") != std::string::npos);
  }

  SECTION("create() and exists() work together") {
    auto result = SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadWrite);
    REQUIRE(result.has_value());

    REQUIRE(SharedMemory::exists(TEST_SHM_NAME));

    result->close();
    cleanup_test_shm();
  }

  SECTION("remove() succeeds for existing shared memory") {
    auto create_result =
        SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadWrite);
    REQUIRE(create_result.has_value());
    create_result->close();

    auto remove_result = SharedMemory::remove(TEST_SHM_NAME);
    REQUIRE(remove_result.has_value());

    REQUIRE_FALSE(SharedMemory::exists(TEST_SHM_NAME));
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("SharedMemory create()", "[SharedMemory]") {
  cleanup_test_shm();

  SECTION("create() with ReadWrite access succeeds") {
    auto result = SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadWrite);
    REQUIRE(result.has_value());

    auto& shm = result.value();
    REQUIRE(shm.data().size() >= TEST_SIZE);  // May be page-aligned, so >= actual size
    REQUIRE(shm.data().data() != nullptr);

    shm.close();
    cleanup_test_shm();
  }

  SECTION("create() with ReadOnly access succeeds") {
    auto result = SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadOnly);
    REQUIRE(result.has_value());

    auto& shm = result.value();
    REQUIRE(shm.data().size() >= TEST_SIZE);
    REQUIRE(shm.data().data() != nullptr);

    shm.close();
    cleanup_test_shm();
  }

  SECTION("create() fails when shared memory already exists") {
    auto result1 = SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadWrite);
    REQUIRE(result1.has_value());

    auto result2 = SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadWrite);
    REQUIRE_FALSE(result2.has_value());
    REQUIRE(result2.error().message().find("File exists") != std::string::npos);

    result1->close();
    cleanup_test_shm();
  }

  SECTION("create() with different sizes") {
    constexpr std::size_t small_size = 256;
    constexpr std::size_t large_size = 4096;

    auto result1 = SharedMemory::create(TEST_SHM_NAME, small_size, SharedMemory::Access::ReadWrite);
    REQUIRE(result1.has_value());
    REQUIRE(result1->data().size() >= small_size);
    result1->close();

    std::ignore = SharedMemory::remove(TEST_SHM_NAME);

    auto result2 = SharedMemory::create(TEST_SHM_NAME, large_size, SharedMemory::Access::ReadWrite);
    REQUIRE(result2.has_value());
    REQUIRE(result2->data().size() >= large_size);
    result2->close();

    cleanup_test_shm();
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("SharedMemory open()", "[SharedMemory]") {
  cleanup_test_shm();

  SECTION("open() fails for non-existent shared memory") {
    auto result = SharedMemory::open(TEST_SHM_NAME, SharedMemory::Access::ReadWrite);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().message().find("No such file or directory") != std::string::npos);
  }

  SECTION("open() succeeds after create()") {
    auto create_result =
        SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadWrite);
    REQUIRE(create_result.has_value());
    create_result->close();

    auto open_result = SharedMemory::open(TEST_SHM_NAME, SharedMemory::Access::ReadWrite);
    REQUIRE(open_result.has_value());

    auto& shm = open_result.value();
    REQUIRE(shm.data().size() >= TEST_SIZE);
    REQUIRE(shm.data().data() != nullptr);

    shm.close();
    cleanup_test_shm();
  }

  SECTION("open() with ReadOnly access") {
    auto create_result =
        SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadWrite);
    REQUIRE(create_result.has_value());
    create_result->close();

    auto open_result = SharedMemory::open(TEST_SHM_NAME, SharedMemory::Access::ReadOnly);
    REQUIRE(open_result.has_value());
    REQUIRE(open_result->data().size() >= TEST_SIZE);

    open_result->close();
    cleanup_test_shm();
  }

  SECTION("Multiple processes can open same shared memory") {
    auto create_result =
        SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadWrite);
    REQUIRE(create_result.has_value());
    create_result->close();

    auto open_result1 = SharedMemory::open(TEST_SHM_NAME, SharedMemory::Access::ReadWrite);
    auto open_result2 = SharedMemory::open(TEST_SHM_NAME, SharedMemory::Access::ReadOnly);

    REQUIRE(open_result1.has_value());
    REQUIRE(open_result2.has_value());

    open_result1->close();
    open_result2->close();
    cleanup_test_shm();
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("SharedMemory data access", "[SharedMemory]") {
  cleanup_test_shm();

  SECTION("data() returns valid span") {
    auto result = SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadWrite);
    REQUIRE(result.has_value());

    auto& shm = result.value();
    auto data_span = shm.data();

    REQUIRE(data_span.size() >= TEST_SIZE);
    REQUIRE(data_span.data() != nullptr);

    shm.close();
    cleanup_test_shm();
  }

  SECTION("Writing and reading data") {
    auto result = SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadWrite);
    REQUIRE(result.has_value());

    auto& shm = result.value();
    auto data_span = shm.data();

    // Write test pattern
    const std::string test_data = "Hello, Shared Memory!";
    std::memcpy(data_span.data(), test_data.c_str(), test_data.size());

    // Verify we can read it back
    std::string read_back(reinterpret_cast<const char*>(data_span.data()), test_data.size());
    REQUIRE(read_back == test_data);

    shm.close();
    cleanup_test_shm();
  }

  SECTION("Data persists across open/close cycles") {
    const std::string test_data = "Persistent data test";

    // Write data
    {
      auto result = SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadWrite);
      REQUIRE(result.has_value());

      auto data_span = result->data();
      std::memcpy(data_span.data(), test_data.c_str(), test_data.size());
      result->close();
    }

    // Read data back
    {
      auto result = SharedMemory::open(TEST_SHM_NAME, SharedMemory::Access::ReadOnly);
      REQUIRE(result.has_value());

      auto data_span = result->data();
      std::string read_back(reinterpret_cast<const char*>(data_span.data()), test_data.size());
      REQUIRE(read_back == test_data);
      result->close();
    }

    cleanup_test_shm();
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("SharedMemory close()", "[SharedMemory]") {
  cleanup_test_shm();

  SECTION("close() can be called multiple times safely") {
    auto result = SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadWrite);
    REQUIRE(result.has_value());

    auto& shm = result.value();
    REQUIRE_FALSE(shm.data().empty());

    shm.close();
    REQUIRE(shm.data().empty());

    // Second close should be safe
    REQUIRE_NOTHROW(shm.close());
    REQUIRE(shm.data().empty());

    cleanup_test_shm();
  }

  SECTION("data() returns empty span after close()") {
    auto result = SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadWrite);
    REQUIRE(result.has_value());

    auto& shm = result.value();
    REQUIRE_FALSE(shm.data().empty());

    shm.close();
    REQUIRE(shm.data().empty());
    REQUIRE(shm.data().data() == nullptr);

    cleanup_test_shm();
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("SharedMemory move semantics", "[SharedMemory]") {
  cleanup_test_shm();

  SECTION("Move constructor works correctly") {
    auto result = SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadWrite);
    REQUIRE(result.has_value());

    auto original_data = result->data();
    REQUIRE_FALSE(original_data.empty());

    // Move construct
    auto moved_shm = std::move(result.value());

    // Original should be empty, moved should have the data
    REQUIRE(result->data().empty());
    REQUIRE(moved_shm.data().data() == original_data.data());
    REQUIRE(moved_shm.data().size() == original_data.size());

    moved_shm.close();
    cleanup_test_shm();
  }

  SECTION("Move assignment works correctly") {
    auto result1 = SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadWrite);
    auto result2 =
        SharedMemory::create(TEST_SHM_NAME_2, TEST_SIZE * 2, SharedMemory::Access::ReadWrite);
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
    cleanup_test_shm();
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("SharedMemory RAII behavior", "[SharedMemory]") {
  cleanup_test_shm();

  SECTION("Destructor automatically closes shared memory") {
    {
      auto result = SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadWrite);
      REQUIRE(result.has_value());
      // SharedMemory destructor should be called here automatically
    }

    // Shared memory should still exist (destructor only unmaps, doesn't remove)
    REQUIRE(SharedMemory::exists(TEST_SHM_NAME));

    cleanup_test_shm();
  }

  SECTION("Manual close before destructor") {
    {
      auto result = SharedMemory::create(TEST_SHM_NAME, TEST_SIZE, SharedMemory::Access::ReadWrite);
      REQUIRE(result.has_value());
      result->close();
      // Destructor should handle already-closed state gracefully
    }

    REQUIRE(SharedMemory::exists(TEST_SHM_NAME));
    cleanup_test_shm();
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("SharedMemory error handling", "[SharedMemory]") {
  cleanup_test_shm();

  SECTION("Invalid shared memory name") {
    // Names without leading slash are invalid
    auto result = SharedMemory::create("invalid_name", TEST_SIZE, SharedMemory::Access::ReadWrite);
    REQUIRE_FALSE(result.has_value());
  }

  SECTION("Zero size shared memory") {
    auto result = SharedMemory::create(TEST_SHM_NAME, 0, SharedMemory::Access::ReadWrite);
    // This should still succeed but with minimal size
    REQUIRE(result.has_value());
    result->close();
    cleanup_test_shm();
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("SharedMemory multi-process simulation", "[SharedMemory]") {
  cleanup_test_shm();

  SECTION("Producer-consumer pattern simulation") {
    constexpr int MAGIC_NUMBER = 0x12345678;

    // Producer: create and write
    {
      auto result =
          SharedMemory::create(TEST_SHM_NAME, sizeof(int), SharedMemory::Access::ReadWrite);
      REQUIRE(result.has_value());

      auto data_span = result->data();
      *reinterpret_cast<int*>(data_span.data()) = MAGIC_NUMBER;
      result->close();
    }

    // Consumer: open and read
    {
      auto result = SharedMemory::open(TEST_SHM_NAME, SharedMemory::Access::ReadOnly);
      REQUIRE(result.has_value());

      auto data_span = result->data();
      int read_value = *reinterpret_cast<const int*>(data_span.data());
      REQUIRE(read_value == MAGIC_NUMBER);
      result->close();
    }

    cleanup_test_shm();
  }
}

}  // namespace
//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "catch2/catch_test_macros.hpp"
#include "grape/realtime/mutex.h"

namespace {

// NOLINTBEGIN(cert-err58-cpp)

TEST_CASE("Mutex functionality", "[Mutex]") {
  grape::realtime::Mutex mutex;

  SECTION("Lock and Unlock") {
    mutex.lock();
    REQUIRE_NOTHROW(mutex.unlock());
  }

  SECTION("Try Lock") {
    REQUIRE(mutex.try_lock());
    REQUIRE_FALSE(mutex.try_lock());  // Second try should fail
    mutex.unlock();                   // Release the lock for future tests
  }

  SECTION("Native Handle") {
    grape::realtime::Mutex::native_handle_type handle = mutex.native_handle();
    REQUIRE(handle != nullptr);
  }

  SECTION("Multiple Lock/Unlock") {
    mutex.lock();
    REQUIRE_FALSE(mutex.try_lock());  // Try lock while already locked should fail
    mutex.unlock();
    REQUIRE_NOTHROW(mutex.lock());
    REQUIRE_FALSE(mutex.try_lock());  // Try lock while already locked should fail
    REQUIRE_NOTHROW(mutex.unlock());
  }
}

// NOLINTEND(cert-err58-cpp)

}  // namespace

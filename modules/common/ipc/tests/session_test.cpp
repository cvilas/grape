//=================================================================================================
// Copyright (C) 2025 GRAPE Contributors
//=================================================================================================

#include "grape/ipc/session.h"

#include "catch2/catch_test_macros.hpp"

namespace {

// NOLINTBEGIN(cert-err58-cpp)

//=================================================================================================
TEST_CASE("Session initialisation tests", "[ipc]") {
  REQUIRE_FALSE(grape::ipc::ok());  // Not ok until init
  grape::ipc::init(grape::ipc::Config{});
  REQUIRE_THROWS(grape::ipc::init(grape::ipc::Config{}));  // multiple init not allowed
}

// NOLINTEND(cert-err58-cpp)

}  // namespace

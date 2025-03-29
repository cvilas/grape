//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "catch2/catch_test_macros.hpp"
#include "grape/app/app.h"

namespace {

//-------------------------------------------------------------------------------------------------
TEST_CASE("Must initialise once", "[app]") {
  REQUIRE_THROWS(grape::app::ok());
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Initialising with default configuration works", "[app]") {
  constexpr auto CONFIG_FILE = "config/app.lua";

  // Must initialise without exceptions
  REQUIRE_NOTHROW(grape::app::init(CONFIG_FILE));

  // But cannot initialise twice
  REQUIRE_THROWS(grape::app::init(CONFIG_FILE));
}

}  // namespace

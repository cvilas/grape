//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

#include <array>
#include <concepts>
#include <exception>
#include <source_location>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#include "catch2/catch_test_macros.hpp"
#include "grape/exception.h"

namespace {

static_assert(std::derived_from<grape::Exception, std::runtime_error>);
static_assert(std::derived_from<grape::Exception, std::exception>);
static_assert(std::is_nothrow_move_constructible_v<grape::Exception>);
static_assert(std::is_nothrow_move_assignable_v<grape::Exception>);

//-------------------------------------------------------------------------------------------------
TEST_CASE("Exceptions are catchable through standard exception types", "[Exception]") {
  CHECK_THROWS_AS(grape::panic("test error"), std::runtime_error);
  CHECK_THROWS_AS(grape::panic("test error"), std::exception);

  try {
    grape::panic("test error");
  } catch (const std::exception& ex) {
    CHECK(std::string_view{ ex.what() } == "test error");
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Exceptions retain their diagnostics when copied and moved", "[Exception]") {
  const auto location = std::source_location::current();
  const grape::Exception original{ "test error", location, {} };
  auto copy = original;
  const auto moved = std::move(copy);
  grape::Exception assigned{ "other error", std::source_location::current(), {} };
  assigned = original;
  grape::Exception move_assigned{ "other error", std::source_location::current(), {} };
  move_assigned = std::move(assigned);

  for (const auto* ex :
       std::array<const grape::Exception*, 3>{ &original, &moved, &move_assigned }) {
    CHECK(std::string_view{ ex->what() } == "test error");
    CHECK(ex->location().line() == location.line());
    CHECK(std::string_view{ ex->location().file_name() } == location.file_name());
    CHECK(ex->trace().trace().empty());
  }
}

}  // namespace

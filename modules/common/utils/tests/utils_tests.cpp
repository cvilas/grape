//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "catch2/catch_test_macros.hpp"
#include "grape/utils/utils.h"

namespace {

//-------------------------------------------------------------------------------------------------
TEST_CASE("string trimming") {
  constexpr auto STR = "/path/to/some/file.txt";
  constexpr auto START_TOKEN = "to";
  constexpr auto END_TOKEN = ".txt";
  constexpr auto TRUNCATED = grape::utils::truncate(STR, START_TOKEN, END_TOKEN);
  constexpr auto EXPECTED = "to/some/file";
  CHECK(TRUNCATED == EXPECTED);
}
}  // namespace

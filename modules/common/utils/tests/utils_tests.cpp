//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "catch2/catch_test_macros.hpp"
#include "grape/utils/utils.h"

namespace my_ns {
template <typename T>
struct CustomType {};
}  // namespace my_ns

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

//-------------------------------------------------------------------------------------------------
TEST_CASE("get type name") {
  CHECK("int" == grape::utils::getTypeName<int>());
  CHECK("my_ns::CustomType<int>" == grape::utils::getTypeName<my_ns::CustomType<int>>());
}

}  // namespace

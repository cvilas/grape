//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <filesystem>
#include <fstream>

#include "catch2/catch_test_macros.hpp"
#include "grape/utils/file_system.h"

namespace {

// NOLINTBEGIN(clang-analyzer-optin.core.EnumCastOutOfRange)

//-------------------------------------------------------------------------------------------------
TEST_CASE("Resolves file path in the right order", "[file_system]") {
  const auto user_data_path = grape::utils::getUserHomePath() / ".grape";
  const auto* const file_to_search = "test_file";

  // create user data directory if it doesn't exist
  if (not std::filesystem::exists(user_data_path)) {
    REQUIRE(std::filesystem::create_directory(user_data_path));
  }

  // create the test file in current path
  const auto local_file_path = std::filesystem::current_path() / file_to_search;
  auto local_file_stream = std::ofstream(local_file_path);
  local_file_stream.close();
  REQUIRE(std::filesystem::exists(local_file_path));

  // create the test file in user home path
  const auto user_file_path = (user_data_path / file_to_search);
  auto user_file_stream = std::ofstream(user_file_path);
  user_file_stream.close();
  REQUIRE(std::filesystem::exists(user_file_path));

  // Try to open test file. It should be found in the current path
  const auto found_file = grape::utils::resolveFilePath(file_to_search);
  REQUIRE(found_file.has_value());
  REQUIRE(*found_file == local_file_path);  // NOLINT(bugprone-unchecked-optional-access)

  // Remove the local file
  REQUIRE(std::filesystem::remove(local_file_path));

  // Try to open test file again. This time it should be found in the user data path
  const auto found_file2 = grape::utils::resolveFilePath(file_to_search);
  REQUIRE(found_file2.has_value());
  REQUIRE(*found_file2 == user_file_path);  // NOLINT(bugprone-unchecked-optional-access)

  // Remove the user data file
  REQUIRE(std::filesystem::remove(user_file_path));

  // Try to open test file again. It should not be found
  const auto found_file3 = grape::utils::resolveFilePath(file_to_search);
  REQUIRE_FALSE(found_file3.has_value());
}

// NOLINTEND(clang-analyzer-optin.core.EnumCastOutOfRange)

}  // namespace

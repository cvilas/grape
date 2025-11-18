//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <sys/wait.h>
#include <unistd.h>

#include "catch2/catch_test_macros.hpp"
#include "grape/utils/file_system.h"

namespace {

//-------------------------------------------------------------------------------------------------
// Helper function: executes program with optional environment variables and captures output
auto executeAndCapture(const std::filesystem::path& program,
                       const std::vector<std::pair<std::string, std::string>>& env_vars = {})
    -> std::string {
  constexpr int READ_END = 0;
  constexpr int WRITE_END = 1;
  auto pipe_fd = std::array<int, 2>{};

  if (pipe(pipe_fd.data()) == -1) {
    return {};
  }

  const auto pid = fork();
  if (pid == -1) {
    close(pipe_fd[READ_END]);
    close(pipe_fd[WRITE_END]);
    return {};
  }

  if (pid == 0) {  // Child process
    close(pipe_fd[READ_END]);
    dup2(pipe_fd[WRITE_END], STDOUT_FILENO);
    close(pipe_fd[WRITE_END]);

    // Set environment variables
    for (const auto& [key, value] : env_vars) {
      setenv(key.c_str(), value.c_str(), 1);  // NOLINT(concurrency-mt-unsafe)
    }

    // Execute the program
    execl(program.c_str(), program.c_str(), nullptr);  // NOLINT(cppcoreguidelines-pro-type-vararg)
    _exit(EXIT_FAILURE);                               // execl failed
  }

  // Parent process
  close(pipe_fd[WRITE_END]);

  static constexpr auto RESULT_BUF_LENGTH = 256UL;
  auto result = std::string{};
  auto buffer = std::array<char, RESULT_BUF_LENGTH>{};
  ssize_t bytes_read = 0;
  while ((bytes_read = read(pipe_fd[READ_END], buffer.data(), buffer.size())) > 0) {
    result.append(buffer.data(), static_cast<size_t>(bytes_read));
  }

  close(pipe_fd[READ_END]);
  waitpid(pid, nullptr, 0);

  if (!result.empty() && result.back() == '\n') {
    result.pop_back();
  }

  return result;
}

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

//-------------------------------------------------------------------------------------------------
TEST_CASE("Resolves system name in the right order", "[file_system]") {
  const auto helper_path = grape::utils::getProgramPath().parent_path() / "grape_show_system_name";
  REQUIRE(std::filesystem::exists(helper_path));

  SECTION("Falls back to hostname when no env SYSTEM_NAME or file system_name exists") {
    const auto result = executeAndCapture(helper_path);
    const auto host_name = grape::utils::getHostName();
    REQUIRE(result == host_name);
  }

  SECTION("Reads from SYSTEM_NAME environment variable") {
    const std::string test_system_name = "test_system_name_from_env";
    const auto result = executeAndCapture(helper_path, { { "SYSTEM_NAME", test_system_name } });
    REQUIRE(result == test_system_name);
  }

  SECTION("Reads from system_name file when SYSTEM_NAME not set") {
    const auto data_path = grape::utils::getUserHomePath() / ".grape_show_system_name";
    const auto system_name_file = data_path / "system_name";
    if (not std::filesystem::exists(data_path)) {
      REQUIRE(std::filesystem::create_directory(data_path));
    }
    const std::string test_system_name = "test_system_name_from_file";
    {
      auto file_stream = std::ofstream(system_name_file);
      file_stream << test_system_name << "\n";
    }
    REQUIRE(std::filesystem::exists(system_name_file));

    const auto result = executeAndCapture(helper_path);
    CHECK(result == test_system_name);

    // cleanup
    CHECK(std::filesystem::remove(system_name_file));
    CHECK(std::filesystem::remove(data_path));
  }
}

// NOLINTEND(clang-analyzer-optin.core.EnumCastOutOfRange)

}  // namespace

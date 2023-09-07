//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include "catch2/catch_test_macros.hpp"
#include "grape/utils/command_line_args.h"
#include "grape/utils/utils.h"

namespace grape::utils::tests {

// NOLINTBEGIN(cert-err58-cpp)

//-------------------------------------------------------------------------------------------------
TEST_CASE("string trimming") {
  const auto* const input = "  Hello, World!  \t\n";
  const auto* const expected_output = "Hello, World!";
  const auto trimmed = grape::utils::trim(input);
  CHECK(trimmed == expected_output);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("command line args parsing") {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
  const char* argv[] = { "--key1=value", "--key2" };
  const auto argc = 2;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  const auto parser = grape::utils::CommandLineArgs(argc, argv);

  SECTION("Extracts key-value pairs") {
    CHECK(parser.getOption<std::string>("key1") == "value");
  }

  SECTION("Extracts flags without value") {
    CHECK(parser.hasOption("key2"));
  }

  SECTION("Detects unspecified option") {
    CHECK_FALSE(parser.hasOption("key3"));
  }

  SECTION("Reading unspecified option triggers error") {
    CHECK(parser.getOption<std::string>("key3").error() ==
          grape::utils::CommandLineArgs::Error::NotFound);
  }

  SECTION("Parsing value into incompatible type triggers error") {
    CHECK(parser.getOption<int>("key1").error() ==
          grape::utils::CommandLineArgs::Error::Unparsable);
  }
}

// NOLINTEND(cert-err58-cpp)
}  // namespace grape::utils::tests

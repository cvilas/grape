//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "catch2/catch_test_macros.hpp"
#include "grape/conio/program_options.h"

namespace {

using ProgramOptions = grape::conio::ProgramOptions;
using ProgramDescription = grape::conio::ProgramDescription;

//-------------------------------------------------------------------------------------------------
TEST_CASE("Returns error on duplicate declaration of options", "[program_options]") {
  static constexpr auto KEY = "duplicated_key";
  REQUIRE_THROWS(ProgramDescription("duplicate option declarations test")
                     .declareOption<int>(KEY, "a key", 1)
                     .declareOption<std::string>(KEY, "duplicate", "")
                     .parse(0, nullptr));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Returns error on undeclared options specified on the command line",
          "[program_options]") {
  // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  const char* argv[] = { "--test_key=10" };
  const auto argc = 1;
  const auto args = ProgramDescription("undeclared option test").parse(argc, argv);
  // NOLINTEND(cppcoreguidelines-avoid-c-arrays,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  REQUIRE_THROWS(args.getOption<std::string>("undeclared_key"));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Returns error if required option is not specified on the command line",
          "[program_options]") {
  REQUIRE_THROWS(ProgramDescription("missing option test")
                     .declareOption<std::string>("required_key", "A required key")
                     .parse(0, nullptr));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Returns error on type mismatch between declaration and parsing", "[program_options]") {
  const auto args = ProgramDescription("declaration and parsed type mismatch test")
                        .declareOption<int>("int_key", "An integer key", 1)
                        .parse(0, nullptr);
  REQUIRE_THROWS(args.getOption<std::string>("int_key"));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Returns error if value specified at runtime is unparsable as defined type",
          "[program_options]") {
  // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  const char* argv[] = { "--int_key=\"a string value\"" };
  const auto argc = 1;
  const auto args = ProgramDescription("unparsable option test")
                        .declareOption<int>("int_key", "An integer key")
                        .parse(argc, argv);
  // NOLINTEND(cppcoreguidelines-avoid-c-arrays,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  REQUIRE_THROWS(args.getOption<int>("int_key"));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Reads required option if specified on the command line", "[program_options]") {
  // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  const char* argv[] = { "--required_key=16" };
  const auto argc = 1;
  const auto args = ProgramDescription("required option test")
                        .declareOption<int>("required_key", "A required key")
                        .parse(argc, argv);
  // NOLINTEND(cppcoreguidelines-avoid-c-arrays,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  REQUIRE(args.getOption<int>("required_key") == 16);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Default value is used for optional argument if unspecified", "[program_options]") {
  static constexpr auto INT_KEY_DEFAULT_VALUE = 20;
  const auto args =
      ProgramDescription("optional args default value test")
          .declareOption<int>("int_key", "optional integer key", INT_KEY_DEFAULT_VALUE)
          .parse(0, nullptr);
  REQUIRE(args.getOption<int>("int_key") == INT_KEY_DEFAULT_VALUE);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Default value is overridden when optional argument is specified", "[program_options]") {
  static constexpr auto INT_KEY_DEFAULT_VALUE = 20;
  // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  const char* argv[] = { "--int_key=10" };
  const auto argc = 1;
  const auto args =
      ProgramDescription("optional arg override test")
          .declareOption<int>("int_key", "optional integer key", INT_KEY_DEFAULT_VALUE)
          .parse(argc, argv);
  // NOLINTEND(cppcoreguidelines-avoid-c-arrays,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  REQUIRE(args.getOption<int>("int_key") == 10);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Detects unspecified option", "[program_options]") {
  const auto args = ProgramDescription("unspecified option test").parse(0, nullptr);
  REQUIRE_FALSE(args.hasOption("key3"));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Ensures 'help' is always available", "[program_options]") {
  const auto args = ProgramDescription("check help option test").parse(0, nullptr);
  REQUIRE(args.hasOption("help"));
}

}  // namespace

//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include "catch2/catch_test_macros.hpp"
#include "grape/conio/program_options.h"

namespace grape::utils::tests {

// NOLINTBEGIN(cert-err58-cpp)

//-------------------------------------------------------------------------------------------------
TEST_CASE("Throws on attempted duplicate definition of option", "[program_args]") {
  auto desc = grape::conio::ProgramDescription("duplication test");
  CHECK_THROWS_AS(
      desc.defineOption<int>("key1", "a key").defineOption<int>("key1", "duplicate of key1"),
      grape::InvalidOperationException);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Throws if undefined options are specified on the command line", "[program_args]") {
  auto desc = grape::conio::ProgramDescription("undefined option test");
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
  const char* argv[] = { "--test_key=10" };
  const auto argc = 1;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  CHECK_THROWS_AS(std::move(desc).parse(argc, argv), grape::InvalidParameterException);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Throws if required option is not specified on the command line", "[program_args]") {
  auto desc = grape::conio::ProgramDescription("missing option test");
  desc.defineOption<std::string>("required_key", "A required key");
  CHECK_THROWS_AS(std::move(desc).parse(0, nullptr), grape::InvalidConfigurationException);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Reads required option if specified on the command line", "[program_args]") {
  auto desc = grape::conio::ProgramDescription("required option test");
  desc.defineOption<int>("required_key", "A required key");
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
  const char* argv[] = { "--required_key=16" };
  const auto argc = 1;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  const auto args = std::move(desc).parse(argc, argv);
  CHECK(args.getOption<int>("required_key") == 16);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Default value is used for optional argument if unspecified", "[program_args]") {
  static constexpr auto INT_KEY_DEFAULT_VALUE = 20;
  auto desc = grape::conio::ProgramDescription("optional args default value test");
  desc.defineOption<int>("int_key", "optional integer key", INT_KEY_DEFAULT_VALUE);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  const auto args = std::move(desc).parse(0, nullptr);
  CHECK(args.getOption<int>("int_key") == INT_KEY_DEFAULT_VALUE);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Default value is overridden when optional argument is unspecified", "[program_args]") {
  static constexpr auto INT_KEY_DEFAULT_VALUE = 20;
  auto desc = grape::conio::ProgramDescription("optional arg override test");
  desc.defineOption<int>("int_key", "optional integer key", INT_KEY_DEFAULT_VALUE);
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
  const char* argv[] = { "--int_key=10" };
  const auto argc = 1;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  const auto args = std::move(desc).parse(argc, argv);
  CHECK(args.getOption<int>("int_key") == 10);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Detects unspecified option", "[program_args]") {
  auto desc = grape::conio::ProgramDescription("unspecified option test");
  const auto args = std::move(desc).parse(0, nullptr);
  CHECK_FALSE(args.hasOption("key3"));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Ensures 'help' is always available", "[program_args]") {
  auto desc = grape::conio::ProgramDescription("check help option test");
  const auto args = std::move(desc).parse(0, nullptr);
  CHECK(args.hasOption("help"));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Throws on type mismatch between declaration and parsing", "[program_args]") {
  auto desc = grape::conio::ProgramDescription("declaration and parsed type mismatch test");
  desc.defineOption<int>("int_key", "An integer key", 1);
  const auto args = std::move(desc).parse(0, nullptr);
  CHECK_THROWS_AS(args.getOption<std::string>("int_key"), grape::TypeMismatchException);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Throws if value specified at runtime is unparsable as defined type", "[program_args]") {
  auto desc = grape::conio::ProgramDescription("unparsable option test");
  desc.defineOption<int>("int_key", "An integer key");

  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
  const char* argv[] = { "--int_key=\"a string value\"" };
  const auto argc = 1;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  const auto args = std::move(desc).parse(argc, argv);

  CHECK_THROWS_AS(args.getOption<int>("int_key"), grape::TypeMismatchException);
}

// NOLINTEND(cert-err58-cpp)
}  // namespace grape::utils::tests

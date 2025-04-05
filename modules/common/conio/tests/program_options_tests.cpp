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
  const auto args = ProgramDescription("duplicate option declarations test")
                        .declareOption<int>(KEY, "a key", 1)
                        .declareOption<std::string>(KEY, "duplicate", "")
                        .parse(0, nullptr);
  REQUIRE(not args.has_value());
  REQUIRE(args.error().code == ProgramOptions::Error::Code::Redeclared);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Returns error on undeclared options specified on the command line",
          "[program_options]") {
  // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  const char* argv[] = { "--test_key=10" };
  const auto argc = 1;
  const auto args = ProgramDescription("undeclared option test").parse(argc, argv);
  // NOLINTEND(cppcoreguidelines-avoid-c-arrays,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
  const auto undeclared_option = args.value().getOption<std::string>("undeclared_key");
  REQUIRE(not undeclared_option.has_value());
  REQUIRE(undeclared_option.error().code == ProgramOptions::Error::Code::Undeclared);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Returns error if required option is not specified on the command line",
          "[program_options]") {
  const auto args = ProgramDescription("missing option test")
                        .declareOption<std::string>("required_key", "A required key")
                        .parse(0, nullptr);
  REQUIRE(not args.has_value());
  REQUIRE(args.error().code == ProgramOptions::Error::Code::Undefined);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Returns error on type mismatch between declaration and parsing", "[program_options]") {
  const auto args = ProgramDescription("declaration and parsed type mismatch test")
                        .declareOption<int>("int_key", "An integer key", 1)
                        .parse(0, nullptr);
  REQUIRE(args.has_value());
  const auto opt = args.value().getOption<std::string>("int_key");
  REQUIRE(not opt.has_value());
  REQUIRE(opt.error().code == ProgramOptions::Error::Code::TypeMismatch);
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
  REQUIRE(args.has_value());
  const auto opt = args.value().getOption<int>("int_key");
  REQUIRE(not opt.has_value());
  REQUIRE(opt.error().code == ProgramOptions::Error::Code::Unparsable);
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
  REQUIRE(args.has_value());
  REQUIRE(args.value().getOption<int>("required_key") == 16);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Default value is used for optional argument if unspecified", "[program_options]") {
  static constexpr auto INT_KEY_DEFAULT_VALUE = 20;
  const auto args =
      ProgramDescription("optional args default value test")
          .declareOption<int>("int_key", "optional integer key", INT_KEY_DEFAULT_VALUE)
          .parse(0, nullptr);
  REQUIRE(args.has_value());
  REQUIRE(args.value().getOption<int>("int_key") == INT_KEY_DEFAULT_VALUE);
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
  REQUIRE(args.has_value());
  REQUIRE(args.value().getOption<int>("int_key") == 10);
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Detects unspecified option", "[program_options]") {
  const auto args = ProgramDescription("unspecified option test").parse(0, nullptr);
  REQUIRE(args.has_value() == true);
  REQUIRE_FALSE(args.value().hasOption("key3"));
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("Ensures 'help' is always available", "[program_options]") {
  const auto args = ProgramDescription("check help option test").parse(0, nullptr);
  REQUIRE(args.has_value());
  REQUIRE(args.value().hasOption("help"));
}

}  // namespace

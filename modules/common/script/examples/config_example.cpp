//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/script/script.h"

//=================================================================================================
// Example shows how to configure fields in a custom data structure from a script
//=================================================================================================

// An example data structure that is configurable
struct PersonnelRecord {
  void configure(const grape::script::ConfigTable& table);
  unsigned int age{ 0 };
  std::string name;
};

// example configuration script for PersonnelRecord (standard Lua script)
static constexpr std::string_view CONFIG = R"(
  name="Jane Smith"
  age=65
)";

using ConfigTableException = grape::Exception<grape::script::ConfigTable::Error>;

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;
  try {
    const auto script = grape::script::ConfigScript(std::string(CONFIG));
    const auto table = script.table();
    PersonnelRecord record;
    record.configure(table);
    std::println("From configuration, name='{}', age={}.", record.name, record.age);
    return EXIT_SUCCESS;
  } catch (const ConfigTableException& ex) {
    const auto err_str = toString(ex.data());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = fprintf(stderr, "[%.*s]\n", static_cast<int>(err_str.length()), err_str.data());
    ConfigTableException::consume();
    return EXIT_FAILURE;
  } catch (const grape::script::ConfigScriptException& ex) {
    const auto err_str = toString(ex.data());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = fprintf(stderr, "[%.*s]\n", static_cast<int>(err_str.length()), err_str.data());
    grape::script::ConfigScriptException::consume();
    return EXIT_FAILURE;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}

//-------------------------------------------------------------------------------------------------
void PersonnelRecord::configure(const grape::script::ConfigTable& table) {
  const auto age_result = table.read<int>("age");
  if (age_result.has_value()) {
    age = static_cast<unsigned int>(age_result.value());
  } else {
    grape::panic<ConfigTableException>("Error reading age", age_result.error());
  }

  const auto name_result = table.read<std::string>("name");
  if (name_result.has_value()) {
    name = name_result.value();
  } else {
    grape::panic<ConfigTableException>("Error reading name", name_result.error());
  }
}

//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/script/script.h"

//=================================================================================================
// Example shows how to configure fields in a custom data structure from a script
//=================================================================================================

// An exception class for configuration errors
class ConfigException : public grape::Exception {
public:
  ConfigException(const std::string& msg, std::source_location loc) : Exception(msg, loc) {
  }
};

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
  } catch (const std::exception& ex) {
    std::println(stderr, "Exception: {}", ex.what());
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
void PersonnelRecord::configure(const grape::script::ConfigTable& table) {
  const auto age_result = table.read<int>("age");
  if (age_result.has_value()) {
    age = static_cast<unsigned int>(age_result.value());
  } else {
    grape::panic<ConfigException>(
        std::format("Error reading age: {}\n", toString(age_result.error())));
  }

  const auto name_result = table.read<std::string>("name");
  if (name_result.has_value()) {
    name = name_result.value();
  } else {
    grape::panic<ConfigException>(
        std::format("Error reading name: {}\n", toString(name_result.error())));
  }
}

//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/exception.h"
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
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}

//-------------------------------------------------------------------------------------------------
void PersonnelRecord::configure(const grape::script::ConfigTable& table) {
  const auto age_result = table.read<int>("age");
  if (not age_result.has_value()) {
    grape::panic<grape::Exception>(
        std::format("Error reading age: {}", toString(age_result.error())));
  }
  age = static_cast<unsigned int>(age_result.value());

  const auto name_result = table.read<std::string>("name");
  if (not name_result.has_value()) {
    grape::panic<grape::Exception>(
        std::format("Error reading name: {}", toString(name_result.error())));
  }
  name = name_result.value();
}

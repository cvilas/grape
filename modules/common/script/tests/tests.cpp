//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <cmath>

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "grape/exception.h"
#include "grape/script/script.h"

namespace grape::script::tests {

// NOLINTBEGIN(cert-err58-cpp,cppcoreguidelines-avoid-magic-numbers)

// test configuration script
static constexpr std::string_view TEST_CONFIG = R"(
name = "Jane Smith"
age = 65
is_qualified = true

nested_table = {
  nested_nested_table = {
    radius = 2.156,
    circumference = 2 * math.pi * 2.156,
  }
}

test_array = {90, 28, 16}

)";

//-------------------------------------------------------------------------------------------------
TEST_CASE("[configuration script tests]", "[script]") {
  auto script = grape::script::ConfigScript(std::string(TEST_CONFIG));
  const auto root_table = script.table();

  using ConfigTable = grape::script::ConfigTable;

  SECTION("can parse all supported types") {
    // string
    const auto name = root_table.read<std::string>("name");
    CHECK(name.has_value());
    CHECK(name.value() == "Jane Smith");

    // bool
    const auto is_qualified = root_table.read<bool>("is_qualified");
    CHECK(is_qualified.has_value());
    CHECK(is_qualified.value() == true);

    // int
    const auto age = root_table.read<int>("age");
    CHECK(age.has_value());
    CHECK(age.value() == 65);

    // table
    const auto table = root_table.read<ConfigTable>("nested_table.nested_nested_table");
    CHECK(table.has_value());

    // float. Also reads from a nested table
    static constexpr auto EPSILON = 0.0001;
    static constexpr auto EXPECTED_RADIUS = 2.156;
    const auto radius = table.value().read<float>("radius");
    CHECK(radius.has_value());
    CHECK_THAT(static_cast<double>(radius.value()),
               Catch::Matchers::WithinRel(EXPECTED_RADIUS, EPSILON));

    // reads math expressions
    const auto circ = table.value().read<float>("circumference");
    static constexpr auto EXPECTED_CIRC = 2. * M_PI * EXPECTED_RADIUS;
    CHECK(circ.has_value());

    CHECK_THAT(static_cast<double>(circ.value()),
               Catch::Matchers::WithinRel(EXPECTED_CIRC, EPSILON));

    // array of values
    const auto array_result = root_table.read<ConfigTable>("test_array");
    CHECK(array_result.has_value());
    const auto& array = array_result.value();
    const auto sz = array.size();
    CHECK(sz == 3);
    const auto expected = std::array<int, 3>{ 90, 28, 16 };
    for (size_t i = 0; i < sz; ++i) {
      const auto elem_result = array.read<int>(i);
      CHECK(elem_result.has_value());
      const auto& elem = elem_result.value();
      CHECK(expected.at(i) == elem);
    }

    // invalid array index returns error
    const auto elem_result = array.read<int>(sz);
    CHECK(not elem_result.has_value());
    CHECK(elem_result.error() == ConfigTable::Error::NotFound);
  }

  SECTION("trying to parse data as incomptible type returns error") {
    // bool from string
    const auto bool_from_string = root_table.read<bool>("name");
    CHECK(not bool_from_string.has_value());
    CHECK(bool_from_string.error() == ConfigTable::Error::Unparsable);

    // table from bool
    const auto table_from_bool = root_table.read<ConfigTable>("is_qualified");
    CHECK(not table_from_bool.has_value());
    CHECK(table_from_bool.error() == ConfigTable::Error::Unparsable);

    // string from float
    const auto table = root_table.read<ConfigTable>("nested_table.nested_nested_table");
    CHECK(table.has_value());
    const auto string_from_float = table.value().read<std::string>("radius");
    CHECK(not string_from_float.has_value());
    CHECK(string_from_float.error() == ConfigTable::Error::Unparsable);
  }

  SECTION("trying to parse non-existant key returns error") {
    // test invalid key
    const auto entry1 = root_table.read<bool>("non-existant-key");
    CHECK(entry1.error() == ConfigTable::Error::NotFound);

    // test dereferencing a valid key as a sub-table
    const auto entry2 = root_table.read<bool>("name.non-existant-key");
    CHECK(entry2.error() == ConfigTable::Error::Unparsable);
  }
}

//-------------------------------------------------------------------------------------------------
TEST_CASE("errroneous script throws exception", "[script]") {
  static constexpr std::string_view ERROR_SCRIPT = R"(
    name = "Jane Smith", -- syntax error: ',' character
  )";

  CHECK_THROWS_AS(grape::script::ConfigScript(std::string(ERROR_SCRIPT)), grape::Exception);
}

// NOLINTEND(cert-err58-cpp,cppcoreguidelines-avoid-magic-numbers)

}  // namespace grape::script::tests

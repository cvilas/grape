//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <flat_map>
#include <format>
#include <functional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "grape/conio/string_streamable.h"
#include "grape/exception.h"
#include "grape/utils/enums.h"
#include "grape/utils/utils.h"

namespace grape::conio {

//=================================================================================================
/// Container for command line arguments of an application. Created by ProgramDescription.
class ProgramOptions {
public:
  /// Holds program options and their details
  struct Option {
    std::string key;
    std::string brief;
    std::string value;
    std::string_view type;
  };

  /// Check whether an option exists
  /// @param key The option (without the '--').
  /// @return true if option is found
  [[nodiscard]] constexpr auto exists(std::string_view key) const -> bool;

  /// Get the value for an option. Throws if the option does not exist
  /// @param key The option (without the '--').
  /// @return  The value of the specified option.
  template <StringStreamable T>
  [[nodiscard]] constexpr auto get(std::string_view key) const -> T;

private:
  friend class ProgramDescription;

  /// @brief Constructor. See ProgramDescription::parse
  /// @param options List of supported program options
  explicit constexpr ProgramOptions(std::flat_map<std::string, Option, std::less<>>&& options);
  std::flat_map<std::string, Option, std::less<>> options_;
};

//=================================================================================================
/// Define online help and supported command line arguments for an application.
///
/// Features:
/// - Enforces that every supported command line option is declared and described exactly once
/// - Notifies an error if required options are not specified on the command line
/// - Notifies an error if value types are mismatched between where they are defined and where they
/// are used
/// - Ensures 'help' is always available
/// See also ProgramOptions
/// See example program for usage
class ProgramDescription {
public:
  /// @brief Creates object
  /// @param brief A brief text describing the application
  constexpr explicit ProgramDescription(std::string_view brief);

  /// @brief Declare a required command line option (--key=value)
  /// @tparam T Value type
  /// @param key Key of the key-value pair
  /// @param brief A brief text describing the option
  /// @return Reference to self. Enables daisy-chained calls
  template <StringStreamable T>
  constexpr auto declareOption(this auto&& self, std::string_view key, std::string_view brief)
      -> decltype(auto);

  /// @brief Declare an optional command line option (--key=value)
  /// @tparam T Value type
  /// @param key Key of the key-value pair
  /// @param brief A brief text describing the option
  /// @param default_value Default value to use if the option is not specified on the command line
  /// @return Reference to self. Enables daisy-chained calls
  template <StringStreamable T>
  constexpr auto declareOption(this auto&& self, std::string_view key, std::string_view brief,
                               const T& default_value) -> decltype(auto);

  /// @brief Parses and returns command line options at runtime.
  /// @param argc Number of arguments on the command line
  /// @param argv array of C-style strings
  /// @return Object containing command line options
  [[nodiscard]] auto parse(int argc, const char** argv) && -> ProgramOptions;

private:
  static constexpr auto HELP_KEY = "help";
  struct CheckedOption {
    ProgramOptions::Option option;
    bool is_required{ false };
    bool is_specified{ false };
  };
  std::flat_map<std::string, CheckedOption, std::less<>> options_;
  std::string help_text_;
};

//-------------------------------------------------------------------------------------------------
constexpr ProgramDescription::ProgramDescription(std::string_view brief) {
  help_text_ = std::string{ "Help requested:\n" } + std::string{ brief } + "\nOptions:\n";
  help_text_ += std::format("--{} [optional]: {}\n", HELP_KEY, "This text!");
}

//-------------------------------------------------------------------------------------------------
template <StringStreamable T>
constexpr auto ProgramDescription::declareOption(this auto&& self, std::string_view key,
                                                 std::string_view brief) -> decltype(auto) {
  if (key == HELP_KEY) {
    panic(std::format("'{}' is a reserved option key", key));
  }
  constexpr auto TYPE_NAME = utils::getTypeName<T>();
  const auto insert_result = self.options_.try_emplace(
      std::string(key), CheckedOption{
               .option = {
                   .key = std::string(key),
                   .brief = std::string(brief),
                   .value = "",
                   .type = TYPE_NAME,
               },
               .is_required = true,
               .is_specified = false,
           });
  if (not insert_result.second) {
    panic(std::format("Redeclared option: {}", key));
  }
  self.help_text_ += std::format("--{} [required]: {}. [type: {}]\n", key, brief, TYPE_NAME);
  return std::forward<decltype(self)>(self);
}

//-------------------------------------------------------------------------------------------------
template <StringStreamable T>
constexpr auto ProgramDescription::declareOption(this auto&& self, std::string_view key,
                                                 std::string_view brief, const T& default_value)
    -> decltype(auto) {
  if (key == HELP_KEY) {
    panic(std::format("'{}' is a reserved option key", key));
  }
  const auto to_string = [](const auto& val) {
    if constexpr (std::is_enum_v<std::decay_t<decltype(val)>>) {
      return std::string{ grape::enums::name(val) };
    } else {
      return std::format("{}", val);
    }
  };
  constexpr auto TYPE_NAME = utils::getTypeName<T>();
  const auto default_value_str = to_string(default_value);
  const auto insert_result = self.options_.try_emplace(
      std::string(key), CheckedOption{
               .option = {
                   .key = std::string(key),
                   .brief = std::string(brief),
                   .value = default_value_str,
                   .type = TYPE_NAME,
               },
               .is_required = false,
               .is_specified = false,
           });
  if (not insert_result.second) {
    panic(std::format("Redeclared option: {}", key));
  }
  self.help_text_ += std::format("--{} [optional]: {}; (default: {}) [type: {}]\n", key, brief,
                                 default_value_str, TYPE_NAME);
  return std::forward<decltype(self)>(self);
}

//-------------------------------------------------------------------------------------------------
constexpr ProgramOptions::ProgramOptions(
    std::flat_map<std::string, ProgramOptions::Option, std::less<>>&& options)
  : options_(std::move(options)) {
}

//-------------------------------------------------------------------------------------------------
constexpr auto ProgramOptions::exists(std::string_view key) const -> bool {
  return options_.contains(key);
}

//-------------------------------------------------------------------------------------------------
template <StringStreamable T>
constexpr auto ProgramOptions::get(std::string_view key) const -> T {
  const auto it = options_.find(key);
  if (it == options_.end()) {
    panic(std::format("Undeclared option: {}", key));
  }

  if (it->second.type != utils::getTypeName<T>()) {
    panic(std::format("Type mismatch for option: {}", key));
  }

  if constexpr (std::is_same_v<T, std::string>) {
    // note: since std::istringstream extracts only up to whitespace, this special case is
    // neccessary for parsing strings containing multiple words
    return it->second.value;
  } else if constexpr (std::is_enum_v<T>) {
    const auto opt = grape::enums::cast<T>(it->second.value);
    if (not opt.has_value()) {
      panic(std::format("Unparsable option: {}={}", key, it->second.value));
    }
    return *opt;
  } else {
    T value;
    std::istringstream stream(it->second.value);
    if (not(stream >> value) or not stream.eof()) {
      panic(std::format("Unparsable option: {}={}", key, it->second.value));
    }
    return value;
  }
}

}  // namespace grape::conio

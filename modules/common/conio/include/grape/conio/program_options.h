//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <algorithm>
#include <format>
#include <print>
#include <sstream>
#include <string>
#include <vector>

#include "grape/exception.h"
#include "grape/utils/utils.h"

namespace grape::conio {

//=================================================================================================
/// Types that are convertable to and from a string
template <typename T>
concept StringStreamable = requires(std::string str, T value) {
  std::istringstream{ str } >> value;
  std::ostringstream{ str } << value;
};

//=================================================================================================
/// Container for command line options for a program. To be used with ProgramDescription class.
class ProgramOptions {
public:
  /// @brief  Holds program options and their details
  struct Option {
    std::string key;
    std::string description;
    std::string value_type;
    std::string value;
    bool is_required{ false };
    bool is_specified{ false };
  };

public:
  /// Check whether an option was specified on the command line
  /// @param option  The command line option (without the '--').
  /// @return true if option is found
  [[nodiscard]] auto hasOption(const std::string& option) const -> bool;

  /// Get the value specified for a command line option
  /// @param option  The command line option (without the '--').
  /// @return  The value of the specified option.
  template <StringStreamable T>
  [[nodiscard]] auto getOption(const std::string& option) const -> T;

private:
  friend class ProgramDescription;

  /// @brief Constructor. See ProgramDescription::parse
  /// @param options List of supported program options
  explicit ProgramOptions(std::vector<Option>&& options);

  std::vector<Option> options_;
};

//=================================================================================================
/// Program description and command line parsing utility. Works with ProgramOptions class.
///
/// Features:
/// - Enforces that every supported command line option is described exactly once
/// - Throws if unsupported options are specified on the command line
/// - Throws if required options are not specified on the command line
/// - Throws if value types are mismatched between where they are defined and where they are used
/// - Ensures 'help' is always available
/// See also ProgramOptions
class ProgramDescription {
public:
  /// @brief Creates object
  /// @param brief A brief text describing the program
  constexpr explicit ProgramDescription(const std::string& brief);

  /// @brief Defines a required option (--key=value) on the command line
  /// @tparam T Value type
  /// @param key Key of the key-value pair
  /// @param description A brief text describing the option
  /// @return Reference to the object. Enables daisy-chained calls
  template <StringStreamable T>
  constexpr auto defineOption(const std::string& key,
                              const std::string& description) -> ProgramDescription&;

  /// @brief Defines a command line option (--key=value) that is optional
  /// @tparam T Value type
  /// @param key Key of the key-value pair
  /// @param description A brief text describing the option
  /// @param default_value Default value to use if the option is not specified on the command line
  /// @return Reference to the object. Enables daisy-chained calls
  template <StringStreamable T>
  constexpr auto defineOption(const std::string& key, const std::string& description,
                              const T& default_value) -> ProgramDescription&;

  /// @brief Builds the container to parse command line options.
  /// @note: The resources in this object is moved into the returned object, making this object
  /// unvalid.
  /// @param argc Number of arguments on the command line
  /// @param argv array of C-style strings
  /// @return Object containing command line options
  auto parse(int argc, const char** argv) && -> ProgramOptions;

private:
  static constexpr auto HELP_KEY = "help";

  auto insertHelp() -> std::vector<ProgramOptions::Option>::const_iterator;
  std::vector<ProgramOptions::Option> options_;
};

//-------------------------------------------------------------------------------------------------
inline constexpr ProgramDescription::ProgramDescription(const std::string& brief) {
  options_.emplace_back(HELP_KEY, "", utils::getTypeName<std::string>(), brief, false, false);
}

//-------------------------------------------------------------------------------------------------
template <StringStreamable T>
inline constexpr auto ProgramDescription::defineOption(
    const std::string& key, const std::string& description) -> ProgramDescription& {
  const auto it = std::find_if(options_.begin(), options_.end(),
                               [&key](const auto& opt) { return key == opt.key; });
  if (it != options_.end()) {
    panic<InvalidOperationException>(std::format("Attempted redefinition of option '{}'", key));
  }
  options_.emplace_back(key, description, utils::getTypeName<T>(), "", true, false);
  return *this;
}

//-------------------------------------------------------------------------------------------------
template <StringStreamable T>
inline constexpr auto
ProgramDescription::defineOption(const std::string& key, const std::string& description,
                                 const T& default_value) -> ProgramDescription& {
  const auto it = std::find_if(options_.begin(), options_.end(),
                               [&key](const auto& opt) { return key == opt.key; });
  if (it != options_.end()) {
    panic<InvalidOperationException>(std::format("Attempted redefinition of option '{}'", key));
  }
  options_.emplace_back(key, description, utils::getTypeName<T>(), std::format("{}", default_value),
                        false, false);
  return *this;
}

//-------------------------------------------------------------------------------------------------
template <StringStreamable T>
inline auto ProgramOptions::getOption(const std::string& option) const -> T {
  const auto it = std::find_if(options_.begin(), options_.end(),
                               [&option](const auto& opt) { return option == opt.key; });
  if (it == options_.end()) {
    panic<InvalidParameterException>(std::format("Undefined option '{}'", option));
  }

  const auto my_type = utils::getTypeName<T>();
  if (it->value_type != my_type) {
    panic<TypeMismatchException>(
        std::format("Tried to parse option '{}' as type {} but it's specified as type {}", option,
                    my_type, it->value_type));
  }
  if constexpr (std::is_same_v<T, std::string>) {
    // note: since std::istringstream extracts only up to whitespace, this special case is
    // neccessary for parsing strings containing multiple words
    return it->value;
  }
  T value;
  std::istringstream stream(it->value);
  if (not(stream >> value)) {
    panic<TypeMismatchException>(std::format(
        "Unable to parse value '{}' as type {} for option '{}'", it->value, my_type, option));
  }
  return value;
}
}  // namespace grape::conio
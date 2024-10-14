//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <concepts>
#include <cstdint>
#include <expected>
#include <format>
#include <sstream>
#include <string>
#include <vector>

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
/// Container for command line arguments of an application. Created by ProgramDescription.
class ProgramOptions {
public:
  /// Error description
  struct Error {
    enum class Code : std::uint8_t {
      Undeclared,   //!< accessed an undeclared option
      Redeclared,   //!< option declared multiple times
      Undefined,    //!< option declared as required but not defined
      Unparsable,   //!< option not parsable as requested type
      TypeMismatch  //!< option type at declaration does not match type at definition
    };
    Code code;
    std::string key;
  };

  /// Holds program options and their details
  struct Option {
    std::string key;
    std::string brief;
    std::string value;
    std::string_view type;
    bool is_required{ false };
    bool is_specified{ false };
  };

public:
  /// Check whether an option was specified on the command line
  /// @param key The command line option (without the '--').
  /// @return true if option is found
  [[nodiscard]] auto hasOption(const std::string& key) const -> bool;

  /// Get the value specified for a command line option
  /// @param key The command line option (without the '--').
  /// @return  The value of the specified option.
  template <StringStreamable T>
  [[nodiscard]] auto getOption(const std::string& key) const -> std::expected<T, Error>;

private:
  friend class ProgramDescription;

  /// @brief Constructor. See ProgramDescription::parse
  /// @param options List of supported program options
  explicit ProgramOptions(std::vector<Option>&& options);

  static constexpr auto HELP_KEY = "help";

  static void insertHelp(std::vector<Option>& options);

  std::vector<Option> options_;
};

//=================================================================================================
/// Define online help and supported command line arguments for an application.
///
/// Features:
/// - Enforces that every supported command line option is declared and described exactly once
/// - Notifies an error if unsupported options are specified on the command line
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
  constexpr explicit ProgramDescription(const std::string& brief);

  /// @brief Declare a required command line option (--key=value)
  /// @tparam T Value type
  /// @param key Key of the key-value pair
  /// @param brief A brief text describing the option
  /// @return Reference to self. Enables daisy-chained calls
  template <StringStreamable T>
  auto declareOption(const std::string& key, const std::string& brief) -> ProgramDescription&;

  /// @brief Declare an optional command line option (--key=value)
  /// @tparam T Value type
  /// @param key Key of the key-value pair
  /// @param brief A brief text describing the option
  /// @param default_value Default value to use if the option is not specified on the command line
  /// @return Reference to self. Enables daisy-chained calls
  template <StringStreamable T>
  auto declareOption(const std::string& key, const std::string& brief, const T& default_value)
      -> ProgramDescription&;

  /// @brief Parses and returns command line options at runtime.
  /// @param argc Number of arguments on the command line
  /// @param argv array of C-style strings
  /// @return Object containing command line options
  [[nodiscard]] auto parse(int argc, const char** argv) const
      -> std::expected<ProgramOptions, ProgramOptions::Error>;

private:
  std::vector<ProgramOptions::Option> options_;
};

//-------------------------------------------------------------------------------------------------
constexpr ProgramDescription::ProgramDescription(const std::string& brief) {
  options_.emplace_back(ProgramOptions::Option{ .key = ProgramOptions::HELP_KEY,
                                                .brief = brief,
                                                .value = "",
                                                .type = utils::getTypeName<std::string>(),
                                                .is_required = false,
                                                .is_specified = false });
}

//-------------------------------------------------------------------------------------------------
template <StringStreamable T>
auto ProgramDescription::declareOption(const std::string& key, const std::string& brief)
    -> ProgramDescription& {
  options_.emplace_back(ProgramOptions::Option{ .key = key,
                                                .brief = brief,
                                                .value = "",
                                                .type = utils::getTypeName<T>(),
                                                .is_required = true,
                                                .is_specified = false });
  return *this;
}

//-------------------------------------------------------------------------------------------------
template <StringStreamable T>
auto ProgramDescription::declareOption(const std::string& key, const std::string& brief,
                                       const T& default_value) -> ProgramDescription& {
  options_.emplace_back(ProgramOptions::Option{ .key = key,
                                                .brief = brief,
                                                .value = std::format("{}", default_value),
                                                .type = utils::getTypeName<T>(),
                                                .is_required = false,
                                                .is_specified = false });
  return *this;
}

//-------------------------------------------------------------------------------------------------
template <StringStreamable T>
auto ProgramOptions::getOption(const std::string& key) const -> std::expected<T, Error> {
  const auto it = std::find_if(options_.begin(), options_.end(),
                               [&key](const auto& opt) { return key == opt.key; });
  if (it == options_.end()) {
    return std::unexpected(Error{ .code = Error::Code::Undeclared, .key = key });
  }

  if (it->type != utils::getTypeName<T>()) {
    return std::unexpected(Error{ .code = Error::Code::TypeMismatch, .key = key });
  }

  if constexpr (std::is_same_v<T, std::string>) {
    // note: since std::istringstream extracts only up to whitespace, this special case is
    // neccessary for parsing strings containing multiple words
    return it->value;
  }

  T value;
  std::istringstream stream(it->value);
  if (not(stream >> value)) {
    return std::unexpected(Error{ .code = Error::Code::Unparsable, .key = key });
  }

  return value;
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] constexpr auto toString(const ProgramOptions::Error::Code& code) -> std::string_view {
  switch (code) {
    case ProgramOptions::Error::Code::Undeclared:
      return "Undeclared";
    case ProgramOptions::Error::Code::Redeclared:
      return "Redeclared";
    case ProgramOptions::Error::Code::Undefined:
      return "Undefined";
    case ProgramOptions::Error::Code::Unparsable:
      return "Unparsable";
    case ProgramOptions::Error::Code::TypeMismatch:
      return "TypeMismatch";
  };
  return {};
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] inline auto toString(const ProgramOptions::Error& ex) -> std::string {
  return std::format("Option '{}' {}", ex.key, toString(ex.code));
}
}  // namespace grape::conio
//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#pragma once

#include <cstdint>
#include <expected>
#include <unordered_map>

#include "grape/utils/utils.h"

namespace grape::utils {

/// Command line arguments processor
/// Processes command line options in the format `--key=value` or `--flag`.  See example programs
class CommandLineArgs {
public:
  enum class Error : std::uint8_t {
    NotFound,   //!< Option not found
    Unparsable  //!< Value of option could not be converted into user requested data type
  };

  /// @brief  Builds the container to parse command line options
  /// @param argc Number of arguments on the command line
  /// @param argv array of C-style strings
  CommandLineArgs(int argc, const char* argv[]);  // NOLINT(cppcoreguidelines-avoid-c-arrays)

  /// Check whether an option was specified on the command line
  /// @return true if option is found
  [[nodiscard]] auto hasOption(const std::string& option) const -> bool;

  /// Get the value specified for a command line option
  /// @param option  The command line option (without the '-'). (input)
  /// @return  The value of the specified option.
  template <istringstreamable T>
  [[nodiscard]] auto getOption(const std::string& option) const -> std::expected<T, Error>;

  /// Convenience method. Returns internal container of key-values.
  [[nodiscard]] auto options() const -> const std::unordered_map<std::string, std::string>&;

private:
  std::unordered_map<std::string, std::string> key_values_;
};

//-------------------------------------------------------------------------------------------------
inline constexpr auto toString(CommandLineArgs::Error e) -> std::string_view {
  switch (e) {
    case CommandLineArgs::Error::NotFound:
      return "NotFound";
    case CommandLineArgs::Error::Unparsable:
      return "Unparsable";
  };
}

//-------------------------------------------------------------------------------------------------
template <istringstreamable T>
inline auto CommandLineArgs::getOption(const std::string& option) const -> std::expected<T, Error> {
  const auto& it = key_values_.find(option);
  if (it == key_values_.end()) {
    return std::unexpected(Error::NotFound);
  }
  T value;
  std::istringstream stream(it->second);
  if (stream >> value) {
    return value;
  }
  return std::unexpected(Error::Unparsable);
}
}  // namespace grape::utils

//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include "grape/utils/command_line_args.h"

#include <vector>

namespace grape::utils {

//-------------------------------------------------------------------------------------------------
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
CommandLineArgs::CommandLineArgs(int argc, const char* argv[]) {
  const auto args = std::vector<std::string>(argv, argv + argc);
  for (const auto& a : args) {
    if (a.find("--") == 0) {               //!< '--' indicates key-value pair
      const std::string kv = a.substr(2);  //!< extract the key-value
      const size_t pos = kv.find('=');     //!< find position of delimiter for value
      key_values_.emplace(kv.substr(0, pos) /* key */,
                          ((pos == std::string::npos) ? "" : kv.substr(pos + 1)) /* value */);
    }
  }
}

//-------------------------------------------------------------------------------------------------
auto CommandLineArgs::hasOption(const std::string& option) const -> bool {
  return (key_values_.end() != key_values_.find(option));
}

//-------------------------------------------------------------------------------------------------
auto CommandLineArgs::options() const -> const std::unordered_map<std::string, std::string>& {
  return key_values_;
}

}  // namespace grape::utils

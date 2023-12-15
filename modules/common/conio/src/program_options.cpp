//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include "grape/conio/program_options.h"

namespace grape::conio {

//-------------------------------------------------------------------------------------------------
ProgramOptions::ProgramOptions(std::vector<Option>&& options) : options_(std::move(options)) {
}

//-------------------------------------------------------------------------------------------------
auto ProgramOptions::hasOption(const std::string& option) const -> bool {
  return (options_.end() != std::find_if(options_.begin(), options_.end(),
                                         [&option](const auto& opt) { return option == opt.key; }));
}

//-------------------------------------------------------------------------------------------------
auto ProgramDescription::parse(int argc, const char** argv) && -> ProgramOptions {
  const auto help_it = insertHelp();

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  const auto args_list = std::vector<std::string>(argv, argv + argc);
  for (const auto& arg : args_list) {
    if (arg.starts_with("--")) {
      const std::string kv = arg.substr(2);
      const size_t pos = kv.find('=');
      const auto key = kv.substr(0, pos);
      const auto it = std::find_if(options_.begin(), options_.end(),
                                   [&key](const auto& opt) { return key == opt.key; });

      if (it == options_.end()) {
        panic<InvalidParameterException>(std::format("Undefined option '{}'", key));
      }
      if (it != help_it) {
        it->value = ((pos == std::string::npos) ? "" : kv.substr(pos + 1));
      }
      it->is_specified = true;
    }
  }

  // print help
  if (help_it->is_specified) {
    std::println(stderr, "{}", help_it->value);
    exit(0);
  }

  // check all required arguments are specified
  for (const auto& entry : options_) {
    if (entry.is_required and not entry.is_specified) {
      panic<InvalidConfigurationException>(
          std::format("Required option '{}' not specified", entry.key));
    }
  }
  return ProgramOptions(std::move(options_));
}

//-------------------------------------------------------------------------------------------------
auto ProgramDescription::insertHelp() -> std::vector<ProgramOptions::Option>::const_iterator {
  const auto it = std::find_if(options_.begin(), options_.end(),
                               [](const auto& opt) { return HELP_KEY == opt.key; });
  std::stringstream help_stream;
  help_stream << it->value << "\nOptions:\n";
  for (const auto& entry : options_) {
    if (entry.key == HELP_KEY) {
      continue;
    }
    if (entry.is_required) {
      help_stream << std::format("--{} [required]: {}. [type: {}]\n", entry.key, entry.description,
                                 entry.value_type);
    } else {
      help_stream << std::format("--{} [optional]: {}; (default: {}) [type: {}]\n", entry.key,
                                 entry.description, entry.value, entry.value_type);
    }
  }
  help_stream << std::format("--{} [optional]: {}", HELP_KEY, "This text!");
  it->value = help_stream.str();
  return it;
}

}  // namespace grape::conio

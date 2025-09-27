//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/conio/program_options.h"

#include <algorithm>
#include <print>

namespace grape::conio {

//-------------------------------------------------------------------------------------------------
ProgramOptions::ProgramOptions(std::vector<ProgramOptions::Option>&& options)
  : options_(std::move(options)) {
}

//-------------------------------------------------------------------------------------------------
auto ProgramOptions::hasOption(const std::string& key) const -> bool {
  return (options_.end() !=
          std::ranges::find_if(
              options_, [&key](const auto& opt) noexcept -> bool { return key == opt.key; }));
}

//-------------------------------------------------------------------------------------------------
void ProgramOptions::insertHelp(std::vector<Option>& options) {
  std::string help_string;
  auto it_help = std::vector<Option>::iterator();
  for (auto it = options.begin(); it != options.end(); ++it) {
    if (it->key == HELP_KEY) {
      it_help = it;
      help_string += it->brief + "\nOptions:\n";
      continue;
    }
    if (it->is_required) {
      help_string += std::format("--{} [required]: {}. [type: {}]\n", it->key, it->brief, it->type);
    } else {
      help_string += std::format("--{} [optional]: {}; (default: {}) [type: {}]\n", it->key,
                                 it->brief, it->value, it->type);
    }
  }
  help_string += std::format("--{} [optional]: {}", HELP_KEY, "This text!");
  it_help->value = help_string;
}

//-------------------------------------------------------------------------------------------------
auto ProgramDescription::parse(int argc, const char** argv) const -> ProgramOptions {
  auto declared_options = options_;
  ProgramOptions::insertHelp(declared_options);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  const auto args_list = std::vector<std::string>(argv, argv + argc);
  for (const auto& arg : args_list) {
    if (arg.starts_with("--")) {
      const std::string kv = arg.substr(2);
      const size_t sep = kv.find('=');
      const auto key = kv.substr(0, sep);
      const auto it = std::ranges::find_if(
          declared_options, [&key](const auto& opt) noexcept -> bool { return key == opt.key; });

      // skip option that is not declared as supported
      if (it == declared_options.end()) {
        continue;
      }
      it->is_specified = true;

      if (key == ProgramOptions::HELP_KEY) {
        // print help and exit if it was specified
        std::println(stderr, "{}", it->value);
        std::exit(EXIT_SUCCESS);  // NOLINT(concurrency-mt-unsafe)
      } else {
        it->value = ((sep == std::string::npos) ? "" : kv.substr(sep + 1));
      }
    }
  }

  // check all required arguments are specified
  for (const auto& entry : declared_options) {
    if (entry.is_required and not entry.is_specified) {
      panic(std::format("Undefined option: {}", entry.key));
    }
  }

  // Check for duplicate declarations
  std::ranges::sort(declared_options, [](const auto& opt_a, const auto& opt_b) noexcept -> auto {
    return opt_a.key < opt_b.key;
  });
  const auto dup_it = std::ranges::adjacent_find(
      declared_options,
      [](const auto& opt_a, const auto& opt_b) noexcept -> auto { return opt_a.key == opt_b.key; });
  if (dup_it != declared_options.end()) {
    panic(std::format("Redeclared option: {}", dup_it->key));
  }

  return ProgramOptions(std::move(declared_options));
}

}  // namespace grape::conio

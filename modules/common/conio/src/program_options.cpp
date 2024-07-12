//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/conio/program_options.h"

#include <algorithm>
#include <print>

#include <unistd.h>  // _exit()

namespace grape::conio {

//-------------------------------------------------------------------------------------------------
ProgramOptions::ProgramOptions(std::vector<ProgramOptions::Option>&& options)
  : options_(std::move(options)) {
}

//-------------------------------------------------------------------------------------------------
auto ProgramOptions::hasOption(const std::string& key) const -> bool {
  return (options_.end() !=
          std::ranges::find_if(options_, [&key](const auto& opt) { return key == opt.key; }));
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
auto ProgramDescription::parse(int argc, const char** argv) const
    -> std::expected<ProgramOptions, ProgramOptions::Error> {
  auto tmp_options = options_;
  ProgramOptions::insertHelp(tmp_options);

  using Error = ProgramOptions::Error;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  const auto args_list = std::vector<std::string>(argv, argv + argc);
  for (const auto& arg : args_list) {
    if (arg.starts_with("--")) {
      const std::string kv = arg.substr(2);
      const size_t pos = kv.find('=');
      const auto key = kv.substr(0, pos);
      const auto it =
          std::ranges::find_if(tmp_options, [&key](const auto& opt) { return key == opt.key; });

      if (it == tmp_options.end()) {
        return std::unexpected(Error{ .code = Error::Code::Undeclared, .key = key });
      }
      it->is_specified = true;

      if (key == ProgramOptions::HELP_KEY) {
        // print help and exit if it was specified
        std::println(stderr, "{}", it->value);
        _exit(EXIT_SUCCESS);
      } else {
        it->value = ((pos == std::string::npos) ? "" : kv.substr(pos + 1));
      }
    }
  }

  // check all required arguments are specified
  for (const auto& entry : tmp_options) {
    if (entry.is_required and not entry.is_specified) {
      return std::unexpected(Error{ .code = Error::Code::Undefined, .key = entry.key });
    }
  }

  // Check for duplicate declarations
  std::ranges::sort(tmp_options, [](const auto& a, const auto& b) { return a.key < b.key; });
  const auto dup_it = std::ranges::adjacent_find(
      tmp_options, [](const auto& a, const auto& b) { return a.key == b.key; });
  if (dup_it != tmp_options.end()) {
    return std::unexpected(Error{ .code = Error::Code::Redeclared, .key = dup_it->key });
  }

  return ProgramOptions(std::move(tmp_options));
}

}  // namespace grape::conio

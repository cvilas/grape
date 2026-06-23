//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include "grape/conio/program_options.h"

#include <cstdio>
#include <cstdlib>
#include <print>
#include <span>
#include <utility>

namespace grape::conio {

//-------------------------------------------------------------------------------------------------
auto ProgramDescription::parse(int argc, const char** argv) && -> ProgramOptions {
  for (const std::string_view arg : std::span<const char* const>(argv, static_cast<size_t>(argc))) {
    if (arg.starts_with("--")) {
      const std::string_view kv = arg.substr(2);
      const size_t sep = kv.find('=');
      const auto key = kv.substr(0, sep);
      if (key == HELP_KEY) {
        std::println(stderr, "{}", help_text_);
        std::exit(EXIT_SUCCESS);  // NOLINT(concurrency-mt-unsafe)
      }

      const auto it = options_.find(key);
      if (it == options_.end()) {
        // undeclared and unsupported option. skip
        continue;
      }
      it->second.option.value = ((sep == std::string::npos) ? "" : kv.substr(sep + 1));
      it->second.is_specified = true;
    }
  }

  auto parsed_options = std::flat_map<std::string, ProgramOptions::Option, std::less<>>{};
  for (const auto [key, entry] : options_) {
    if (entry.is_required and not entry.is_specified) {
      panic(std::format("Undefined option: {}", key));
    }
    parsed_options.try_emplace(key, entry.option);
  }
  return ProgramOptions(std::move(parsed_options));
}

}  // namespace grape::conio

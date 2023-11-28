//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/utils/command_line_args.h"

//=================================================================================================
// Prints all command line options passed as '--key=value'
auto main(int argc, const char* argv[]) -> int {
  try {
    const auto parser = grape::utils::CommandLineArgs(argc, argv);
    const auto& options = parser.options();
    if (options.empty()) {
      std::println("No command line options specified");
      return EXIT_SUCCESS;
    }
    for (const auto& [key, value] : options) {
      std::println("{} = {}", key, value);
    }
    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
  }
}

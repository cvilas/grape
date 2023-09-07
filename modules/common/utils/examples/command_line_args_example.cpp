//=================================================================================================
// Copyright (C) 2018-2023 GRAPE Contributors
//=================================================================================================

#include <cstdlib>
#include <iostream>

#include "grape/utils/command_line_args.h"

//=================================================================================================
// Prints all command line options passed as '--key=value'
auto main(int argc, const char* argv[]) -> int {
  const auto parser = grape::utils::CommandLineArgs(argc, argv);
  const auto& options = parser.options();
  if (options.empty()) {
    std::cerr << "No command line options specified\n";
    return EXIT_SUCCESS;
  }
  for (const auto& [key, value] : options) {
    std::cout << key << " = " << value << '\n';
  }
  return EXIT_SUCCESS;
}

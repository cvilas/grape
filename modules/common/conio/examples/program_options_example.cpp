//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/conio/program_options.h"

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    // describe the program and all it's command line options, then parse the command line args
    const auto args_opt =
        grape::conio::ProgramDescription("A dummy service that does nothing")
            .declareOption<int>("port", "The port this service is available on")
            .declareOption<std::string>("address", "The IP address of this service", "[::]")
            .parse(argc, argv);

    if (not args_opt.has_value()) {
      throw grape::conio::ProgramOptions::Error{ args_opt.error() };
    }
    const auto& args = args_opt.value();

    const auto port_opt = args.getOption<int>("port");
    if (not port_opt.has_value()) {
      throw grape::conio::ProgramOptions::Error{ port_opt.error() };
    }

    const auto addr_opt = args.getOption<std::string>("address");
    if (not addr_opt.has_value()) {
      throw grape::conio::ProgramOptions::Error{ addr_opt.error() };
    }

    // help is always available. Specify '--help' on command line or get it directly as here.
    std::println("Help text:\n{}\n", args.getOption<std::string>("help").value());

    // print the arguments passed. At this point, they have all been validated.
    std::println("You specified port = {}", port_opt.value());
    std::println("The IP address in use is {}", addr_opt.value());

    return EXIT_SUCCESS;
  } catch (const grape::conio::ProgramOptions::Error& ex) {
    const auto code_str = toString(ex.code);
    /// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = fprintf(stderr, "Option '%s' %.*s", ex.key.c_str(),
                          static_cast<int>(code_str.length()), code_str.data());
    return EXIT_FAILURE;
  } catch (const std::exception& ex) {
    /// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = std::fprintf(stderr, "Exception %s\n", ex.what());
    return EXIT_FAILURE;
  }
}

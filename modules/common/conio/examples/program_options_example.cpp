//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/conio/program_options.h"
#include "grape/utils/format_ranges.h"

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    // describe the program and all it's command line options, then parse the command line args
    const auto args =
        grape::conio::ProgramDescription("A dummy service that does nothing")
            .declareOption<int>("port", "The port this service is available on")
            .declareOption<std::string>("address", "The IP address of this service", "[::]")
            .declareOption<std::vector<unsigned int>>("cpus", "CPU affinity list", { 1U, 2U })
            .parse(argc, argv);

    const auto port = args.getOption<int>("port");
    const auto addr = args.getOption<std::string>("address");
    const auto cpus = args.getOption<std::vector<unsigned int>>("cpus");

    // help is always available. Specify '--help' on command line or get it directly as here.
    std::println("Help text:\n{}\n", args.getOption<std::string>("help"));

    // print the arguments passed. At this point, they have all been validated.
    std::println("You specified port = {}", port);
    std::println("The IP address in use is {}", addr);
    std::println("CPU affinity list: {}", cpus);

    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}

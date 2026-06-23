//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <cstdlib>
#include <print>
#include <string>
#include <vector>

#include "grape/conio/program_options.h"
#include "grape/exception.h"

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

    const auto port = args.get<int>("port");
    const auto addr = args.get<std::string>("address");
    const auto cpus = args.get<std::vector<unsigned int>>("cpus");

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

//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>

#include "grape/conio/program_options.h"

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    // describe the program and all it's command line options
    auto desc = grape::conio::ProgramDescription("A dummy service that does nothing");
    desc.declareOption<int>("port", "The port this service is available on")
        .declareOption<std::string>("address", "The IP address of this service", "[::]");

    // parse the command line arguments
    const auto args = std::move(desc).parse(argc, argv);
    const auto port = args.getOption<int>("port");
    const auto address = args.getOption<std::string>("address");

    // help is always available. Specify '--help' on command line or get it directly as here.
    std::println("Help text:\n{}\n", args.getOption<std::string>("help"));

    // print the arguments passed
    std::println("You specified port = {}", port);
    std::println("The IP address in use is {}", address);

    return EXIT_SUCCESS;
  } catch (const grape::conio::ProgramOptionException& ex) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = fprintf(stderr, "[%s] ", toString(ex.data()).data());
    grape::conio::ProgramOptionException::consume();
    return EXIT_FAILURE;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}

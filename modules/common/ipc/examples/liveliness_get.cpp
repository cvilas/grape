//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>
#include <thread>

#include "examples_utils.h"
#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// Queries currently alive tokens that match a given key expression. To learn how tokens are
// declared, see liveliness_declare.
//
// Typical usage:
// ```bash
// liveliness_get [--key=my/key/expression/**]
// ```
//
// Paired with example: liveliness_declare
//
// Derived from:
// https://github.com/eclipse-zenoh/zenoh-cpp/blob/main/examples/zenohc/z_get_liveliness.cxx
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/**";

    const auto args_opt =
        grape::conio::ProgramDescription("Queries liveliness token")
            .declareOption<std::string>("key", "key expression to query liveliness of", DEFAULT_KEY)
            .parse(argc, argv);

    if (not args_opt.has_value()) {
      throw grape::conio::ProgramOptions::Error{ args_opt.error() };
    }
    const auto& args = args_opt.value();

    std::println("Opening session...");
    auto config = zenoh::Config::create_default();
    auto session = zenoh::Session::open(std::move(config));

    const auto key = grape::ipc::ex::getOptionOrThrow<std::string>(args, "key");
    std::println("Sending liveliness query for '{}'...", key);
    static constexpr auto FIFO_LENGTH = 16u;
    auto replies = session.liveliness_get(key, zenoh::channels::FifoChannel(FIFO_LENGTH));

    for (auto res = replies.recv(); std::holds_alternative<zenoh::Reply>(res);
         res = replies.recv()) {
      const auto& reply = std::get<zenoh::Reply>(res);
      if (reply.is_ok()) {
        const auto& sample = reply.get_ok();
        std::println(">> Alive token ('{}')", sample.get_keyexpr().as_string_view());
      } else {
        std::println("Received an error");
      }
    }

    return EXIT_SUCCESS;
  } catch (const grape::conio::ProgramOptions::Error& ex) {
    std::ignore = std::fputs(toString(ex).c_str(), stderr);
    return EXIT_FAILURE;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}

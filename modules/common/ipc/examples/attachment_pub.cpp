//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <map>
#include <print>
#include <thread>

#include "examples_utils.h"
#include "grape/conio/conio.h"
#include "grape/exception.h"
#include "grape/ipc/ipc.h"

//=================================================================================================
// The Attachment feature enables users to attach metadata to the payload. The data and metadata
// can come from different memory regions, avoiding the need to copy both into a new buffer and
// then serializing them as a single payload. In principle this is similar to vectored I/O:
// https://en.wikipedia.org/wiki/Vectored_I/O
//
// Typical usage:
// ```bash
// attachment_pub [--key=demo/example/test --value="Hello World"]
// ```
//
// Paired with example: attachment_sub.cpp
//
//=================================================================================================

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  try {
    static constexpr auto DEFAULT_VALUE = "Put from attachment publisher";
    static constexpr auto DEFAULT_KEY = "grape/ipc/example/zenoh/put";

    const auto args_opt =
        grape::conio::ProgramDescription("Attachment publisher example")
            .declareOption<std::string>("key", "Key expression", DEFAULT_KEY)
            .declareOption<std::string>("value", "Data to put on the key", DEFAULT_VALUE)
            .parse(argc, argv);

    if (not args_opt.has_value()) {
      throw grape::conio::ProgramOptions::Error{ args_opt.error() };
    }
    const auto& args = args_opt.value();
    const auto key = grape::ipc::ex::getOptionOrThrow<std::string>(args, "key");
    const auto value = grape::ipc::ex::getOptionOrThrow<std::string>(args, "value");

    auto config = zenoh::Config::create_default();

    std::println("Opening session...");
    auto session = zenoh::Session::open(std::move(config));

    std::println("Declaring Publisher on '{}'", key);
    auto pub = session.declare_publisher(key);

    // allocate and set attachment map
    auto attachment =
        std::map<std::string, std::string>{ { "lang", "C++" }, { "domain", "robotics" } };

    // periodically publish data with attachments
    static constexpr auto LOOP_WAIT = std::chrono::seconds(1);
    uint64_t idx = 0;
    while (true) {
      const auto msg = std::format("[{}] {}", idx++, value);
      std::println("Publishing Data ('{} : {})", key, msg);
      attachment["index"] = std::to_string(idx);
      pub.put(msg, { .encoding = zenoh::Encoding("text/plain"),
                     .attachment = zenoh::ext::serialize(attachment) });
      std::this_thread::sleep_for(LOOP_WAIT);
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

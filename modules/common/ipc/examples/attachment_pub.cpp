//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <map>
#include <thread>

#include "grape/conio/program_options.h"
#include "zenoh_utils.h"

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
      grape::panic<grape::Exception>(toString(args_opt.error()));
    }
    const auto& args = args_opt.value();
    const auto key = args.getOptionOrThrow<std::string>("key");
    const auto value = args.getOptionOrThrow<std::string>("value");

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
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}

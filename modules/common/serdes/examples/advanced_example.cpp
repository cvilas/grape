//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include "advanced_example.h"

#include <chrono>
#include <print>

#include "grape/serdes/stream.h"

namespace {

//-------------------------------------------------------------------------------------------------
auto nowNs() -> std::uint64_t {
  const auto dur = std::chrono::system_clock::now().time_since_epoch();
  const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(dur);
  return static_cast<uint64_t>(ns.count());
}

//-------------------------------------------------------------------------------------------------
auto nsToTp(uint64_t ns) -> std::chrono::time_point<std::chrono::system_clock> {
  const auto dur =
      std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::nanoseconds(ns));
  return std::chrono::system_clock::time_point(dur);
}

}  // namespace

//=================================================================================================
/// Demonstrates serialisation of a complex data structure
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;

  constexpr auto BUF_SIZE = 1024U;
  using OutStream = grape::serdes::OutStream<BUF_SIZE>;
  using InStream = grape::serdes::InStream;
  using Serialiser = grape::serdes::Serialiser<OutStream>;
  using Deserialiser = grape::serdes::Deserialiser<InStream>;

  try {
    const auto pose = PoseStamped{ .nanoseconds = nowNs(),
                                   .position = { .x = 0.01, .y = 2.0, .z = 10.0 },
                                   .orientation = { .x = 0.01, .y = 0.03, .z = 0.1, .w = 1 } };

    // serialise
    auto ostream = OutStream();
    auto serialiser = Serialiser(ostream);
    if (not serialiser.pack(pose)) {
      std::println("Serialisation error. Exiting");
      return EXIT_FAILURE;
    }
    std::println("Serialised into {} bytes", ostream.size());

    // deserialise
    auto istream = InStream(ostream.data());
    auto pose2 = PoseStamped{};
    auto deserialiser = Deserialiser(istream);
    if (not deserialiser.unpack(pose2)) {
      std::println("Deserialisation error. Exiting");
      return EXIT_FAILURE;
    }

    // compare
    std::println("Original pose: timestamp={}, position=[{}, {}, {}], orientation=[{}, {}, {}, {}]",
                 nsToTp(pose.nanoseconds), pose.position.x, pose.position.y, pose.position.z,
                 pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w);

    std::println(
        "Recovered pose: timestamp={}, position=[{}, {}, {}], orientation=[{}, {}, {}, {}]",
        nsToTp(pose.nanoseconds), pose2.position.x, pose2.position.y, pose2.position.z,
        pose2.orientation.x, pose2.orientation.y, pose2.orientation.z, pose2.orientation.w);

    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}

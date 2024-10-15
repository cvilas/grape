//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <chrono>
#include <print>

#include "grape/serdes/serdes.h"
#include "grape/serdes/stream.h"

namespace {

//-------------------------------------------------------------------------------------------------
struct Position {
  double x{};
  double y{};
  double z{};
};

//-------------------------------------------------------------------------------------------------
struct Quaternion {
  double x{};
  double y{};
  double z{};
  double w{};
};

//-------------------------------------------------------------------------------------------------
struct PoseStamped {
  std::chrono::time_point<std::chrono::system_clock> timestamp;
  Position position{};
  Quaternion orientation{};
};

constexpr auto BUF_SIZE = 1024u;
using OutStream = grape::serdes::OutStream<BUF_SIZE>;
using InStream = grape::serdes::InStream;
using Serialiser = grape::serdes::Serialiser<OutStream>;
using Deserialiser = grape::serdes::Deserialiser<InStream>;

//-------------------------------------------------------------------------------------------------
auto toNanosecs(const std::chrono::time_point<std::chrono::system_clock>& tp) -> std::uint64_t {
  const auto dur = tp.time_since_epoch();
  const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(dur);
  return static_cast<uint64_t>(ns.count());
}

//-------------------------------------------------------------------------------------------------
auto fromNanosecs(uint64_t ns) -> std::chrono::time_point<std::chrono::system_clock> {
  const auto dur =
      std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::nanoseconds(ns));
  return std::chrono::system_clock::time_point(dur);
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] auto pack(Serialiser& ser, const Position& p) -> bool {
  if (not ser.pack(p.x)) {
    return false;
  }
  if (not ser.pack(p.y)) {
    return false;
  }
  if (not ser.pack(p.z)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] auto unpack(Deserialiser& des, Position& p) -> bool {
  if (not des.unpack(p.x)) {
    return false;
  }
  if (not des.unpack(p.y)) {
    return false;
  }
  if (not des.unpack(p.z)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] auto pack(Serialiser& ser, const Quaternion& q) -> bool {
  if (not ser.pack(q.x)) {
    return false;
  }
  if (not ser.pack(q.y)) {
    return false;
  }
  if (not ser.pack(q.z)) {
    return false;
  }
  if (not ser.pack(q.w)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] auto unpack(Deserialiser& des, Quaternion& q) -> bool {
  if (not des.unpack(q.x)) {
    return false;
  }
  if (not des.unpack(q.y)) {
    return false;
  }
  if (not des.unpack(q.z)) {
    return false;
  }
  if (not des.unpack(q.w)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] auto pack(Serialiser& ser, const PoseStamped& p) -> bool {
  if (not ser.pack(toNanosecs(p.timestamp))) {
    return false;
  }
  if (not pack(ser, p.position)) {
    return false;
  }
  if (not pack(ser, p.orientation)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] auto unpack(Deserialiser& des, PoseStamped& p) -> bool {
  std::uint64_t ns{};
  if (not des.unpack(ns)) {
    return false;
  }
  p.timestamp = fromNanosecs(ns);
  if (not unpack(des, p.position)) {
    return false;
  }
  if (not unpack(des, p.orientation)) {
    return false;
  }
  return true;
}

}  // namespace

//=================================================================================================
/// Demonstrates serialisation of a complex data structure
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;
  try {
    const auto pose = PoseStamped{ .timestamp = std::chrono::system_clock::now(),
                                   .position = { .x = 0.01, .y = 2.0, .z = 10.0 },
                                   .orientation = { .x = 0.01, .y = 0.03, .z = 0.1, .w = 1 } };

    // serialise
    auto ostream = OutStream();
    auto serialiser = Serialiser(ostream);
    if (not pack(serialiser, pose)) {
      std::println("Serialisation error. Exiting");
      return EXIT_FAILURE;
    }
    std::println("Serialised into {} bytes", ostream.size());

    // deserialise
    auto istream = InStream({ ostream.data(), ostream.size() });
    auto pose2 = PoseStamped{};
    auto deserialiser = Deserialiser(istream);
    if (not unpack(deserialiser, pose2)) {
      std::println("Deserialisation error. Exiting");
      return EXIT_FAILURE;
    }

    // compare
    std::println("Original pose: timestamp={}, position=[{}, {}, {}], orientation=[{}, {}, {}, {}]",
                 pose.timestamp, pose.position.x, pose.position.y, pose.position.z,
                 pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w);

    std::println(
        "Recovered pose: timestamp={}, position=[{}, {}, {}], orientation=[{}, {}, {}, {}]",
        pose2.timestamp, pose2.position.x, pose2.position.y, pose2.position.z, pose2.orientation.x,
        pose2.orientation.y, pose2.orientation.z, pose2.orientation.w);

    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = std::fprintf(stderr, "%s\n", ex.what());
    return EXIT_FAILURE;
  }
}
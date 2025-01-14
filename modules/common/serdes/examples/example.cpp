//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <array>
#include <fstream>
#include <optional>
#include <print>
#include <span>
#include <string>
#include <vector>

#include "grape/serdes/serdes.h"
#include "grape/serdes/stream.h"

namespace {

//-------------------------------------------------------------------------------------------------
// Example/Test structure
struct State {
  std::string name;
  double timestamp{};
  std::array<double, 3> position{};
};

//-------------------------------------------------------------------------------------------------
// generic file writer
auto writeToFile(std::span<const char> data, const std::string& filename) -> bool {
  auto file = std::ofstream(filename, std::ios::binary);
  if (not file) {
    std::println(stderr, "Failed to open '{}' for writing", filename);
    return false;
  }
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  file.write(data.data(), static_cast<std::streamsize>(data.size()));
  return true;
}

//-------------------------------------------------------------------------------------------------
// generic file reader
auto readFromFile(const std::string& filename) -> std::optional<std::vector<char>> {
  auto file = std::ifstream(filename, std::ios::binary | std::ios::ate);
  if (not file) {
    std::println(stderr, "Failed to open '{}' for reading", filename);
    return {};
  }

  const auto size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(static_cast<std::size_t>(size));
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  file.read(buffer.data(), size);

  return buffer;
}

constexpr auto BUF_SIZE = 1024U;
using OutStream = grape::serdes::OutStream<BUF_SIZE>;
using InStream = grape::serdes::InStream;
using Serialiser = grape::serdes::Serialiser<OutStream>;
using Deserialiser = grape::serdes::Deserialiser<InStream>;

//-------------------------------------------------------------------------------------------------
[[nodiscard]] auto serialize(Serialiser& ser, const State& st) -> bool {
  if (not ser.pack(st.name)) {
    return false;
  }
  if (not ser.pack(st.timestamp)) {
    return false;
  }
  if (not ser.pack(st.position)) {
    return false;
  }
  return true;
}

//-------------------------------------------------------------------------------------------------
[[nodiscard]] auto deserialize(Deserialiser& des, State& st) -> bool {
  if (not des.unpack(st.name)) {
    return false;
  }
  if (not des.unpack(st.timestamp)) {
    return false;
  }
  if (not des.unpack(st.position)) {
    return false;
  }
  return true;
}

}  // namespace

//=================================================================================================
/// Demonstrates serialisation, storage, and deserialisation of a simple data structure
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;
  try {
    const auto state = State{ .name = "example_state",
                              .timestamp = 1234.567890,
                              .position = std::array<double, 3>{ 1.0, 2.0, 3.0 } };

    // serialise
    auto ostream = OutStream();
    auto serialiser = Serialiser(ostream);
    if (not serialize(serialiser, state)) {
      std::println("Serialisation error. Exiting");
      return EXIT_FAILURE;
    }

    // Write to file
    static constexpr auto FILENAME = "state.grp";
    if (not writeToFile({ ostream.data(), ostream.size() }, FILENAME)) {
      std::println(stderr, "Exiting");
      return EXIT_FAILURE;
    }
    std::println("Serialized state written to '{}'", FILENAME);

    // Read from file
    auto maybe_data = readFromFile(FILENAME);
    if (not maybe_data.has_value()) {
      std::println(stderr, "Exiting");
      return EXIT_FAILURE;
    }
    auto istream = InStream(maybe_data.value());
    std::println("Read {} bytes from '{}'", istream.size(), FILENAME);

    // deserialise
    auto state2 = State{};
    auto deserialiser = Deserialiser(istream);
    if (not deserialize(deserialiser, state2)) {
      std::println("Deserialisation error. Exiting");
      return EXIT_FAILURE;
    }

    // compare
    std::println("Original state: name={}, timestamp={}, position=[{}, {}, {}]", state.name,
                 state.timestamp, state.position[0], state.position[1], state.position[2]);

    std::println("Deserialized state: name={}, timestamp={}, position=[{}, {}, {}]", state2.name,
                 state2.timestamp, state2.position[0], state2.position[1], state2.position[2]);

    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    std::ignore = std::fputs(ex.what(), stderr);
    return EXIT_FAILURE;
  }
}

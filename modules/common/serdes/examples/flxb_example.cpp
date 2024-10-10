//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <flatbuffers/flexbuffers.h>

#include "examples_utils.h"
namespace {
//-------------------------------------------------------------------------------------------------
void serialize(flexbuffers::Builder& fbb, const grape::serdes::examples::State& state) {
  fbb.Vector([&]() {
    fbb.String(state.name);
    fbb.Double(state.timestamp);
    fbb.FixedTypedVector(state.position.data(), state.position.size());
  });
}

//-------------------------------------------------------------------------------------------------
auto deserialize(const std::vector<char>& buffer, grape::serdes::examples::State& state) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  const auto* const vbeg = reinterpret_cast<const uint8_t*>(buffer.data());
  const auto root = flexbuffers::GetRoot(vbeg, buffer.size()).AsVector();
  state.name = root[0].AsString().str();
  state.timestamp = root[1].AsDouble();
  const auto pos = root[2].AsFixedTypedVector();
  for (std::size_t i = 0u; i < 3u && i < pos.size(); ++i) {
    state.position.at(i) = pos[i].AsDouble();
  }
}

}  // namespace

//=================================================================================================
/// Demonstrates schemaless serialisation, storage, and deserialisation in C++ using FlexBuffers.
/// For deserialisation in Python, run flxb_deserialise_example.py on the generated data file
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;
  try {
    const auto state =
        grape::serdes::examples::State{ .name = "example_state",
                                        .timestamp = 1234.567890,
                                        .position = std::array<double, 3>{ 1.0, 2.0, 3.0 } };

    // serialise
    static constexpr auto INITIAL_SIZE = 1024u;
    auto fbb = flexbuffers::Builder(INITIAL_SIZE);
    serialize(fbb, state);
    fbb.Finish();

    // Write to file
    static constexpr auto FILENAME = "state.flxb";
    const auto& fbbuf = fbb.GetBuffer();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto ser_bytes = std::span{ reinterpret_cast<const char*>(fbbuf.data()), fbbuf.size() };

    if (not grape::serdes::examples::writeToFile(ser_bytes, FILENAME)) {
      std::println(stderr, "Exiting");
      return EXIT_FAILURE;
    }
    std::println("Serialized state written to '{}'", FILENAME);

    // Read from file
    const auto maybe_bytes = grape::serdes::examples::readFromFile(FILENAME);
    if (not maybe_bytes.has_value()) {
      std::println(stderr, "Exiting");
      return EXIT_FAILURE;
    }
    const auto& ser_bytes2 = maybe_bytes.value();
    std::println("Read {} bytes from '{}'", ser_bytes2.size(), FILENAME);

    // deserialise
    grape::serdes::examples::State deserialized_state;
    deserialize(ser_bytes2, deserialized_state);

    // compare
    std::println("Original state: name={}, timestamp={}, position=[{}, {}, {}]", state.name,
                 state.timestamp, state.position[0], state.position[1], state.position[2]);

    std::println("Deserialized state: name={}, timestamp={}, position=[{}, {}, {}]",
                 deserialized_state.name, deserialized_state.timestamp,
                 deserialized_state.position[0], deserialized_state.position[1],
                 deserialized_state.position[2]);

    return EXIT_SUCCESS;
  } catch (const std::exception& ex) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = std::fprintf(stderr, "%s\n", ex.what());
    return EXIT_FAILURE;
  }
}
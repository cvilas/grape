//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

// #define USE_FASTCDR

#ifdef USE_FASTCDR
#include <fastcdr/FastCdr.h>
#else
#include <fastcdr/Cdr.h>
#endif
#include <fastcdr/FastBuffer.h>

#include "examples_utils.h"

namespace {

#ifdef USE_FASTCDR
using Serializer = eprosima::fastcdr::FastCdr;
#else
using Serializer = eprosima::fastcdr::Cdr;
#endif

//-------------------------------------------------------------------------------------------------
void serialize(Serializer& cdr, const grape::serdes::examples::State& s) {
  cdr << s.name;
  cdr << s.timestamp;
  cdr << s.position;
}

//-------------------------------------------------------------------------------------------------
void deserialize(Serializer& cdr, grape::serdes::examples::State& s) {
  cdr >> s.name;
  cdr >> s.timestamp;
  cdr >> s.position;
}

}  // namespace

//=================================================================================================
/// Demonstrates schemaless serialisation, storage, and deserialisation in C++ using FastCDR.
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;
  try {
    const auto state =
        grape::serdes::examples::State{ .name = "example_state",
                                        .timestamp = 1234.567890,
                                        .position = std::array<double, 3>{ 1.0, 2.0, 3.0 } };

    // serialise
    eprosima::fastcdr::FastBuffer ser_buf;
    Serializer ser(ser_buf);

    serialize(ser, state);

    // Write to file
    static constexpr auto FILENAME = "state.cdr";
    const auto ser_bytes = std::span{ ser_buf.getBuffer(), ser.get_serialized_data_length() };
    if (not grape::serdes::examples::writeToFile(ser_bytes, FILENAME)) {
      std::println(stderr, "Exiting");
      return EXIT_FAILURE;
    }
    std::println("Serialized state written to '{}'", FILENAME);

    // Read from file
    auto maybe_data = grape::serdes::examples::readFromFile(FILENAME);
    if (not maybe_data.has_value()) {
      std::println(stderr, "Exiting");
      return EXIT_FAILURE;
    }
    auto& ser_data = maybe_data.value();
    std::println("Read {} bytes from '{}'", ser_data.size(), FILENAME);

    // deserialise
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    eprosima::fastcdr::FastBuffer deser_buf(ser_data.data(), ser_data.size());
    Serializer deser(deser_buf);
    grape::serdes::examples::State deserialized_state;
    deserialize(deser, deserialized_state);

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
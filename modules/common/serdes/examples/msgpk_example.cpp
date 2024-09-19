//=================================================================================================
// Copyright (C) 2024 GRAPE Contributors
//=================================================================================================

#include <type_traits>

#include <msgpack.hpp>

#include "examples_utils.h"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
  namespace adaptor {

  //-----------------------------------------------------------------------------------------------
  template <>
  struct pack<grape::serdes::examples::State> {
    template <typename Stream>
    auto operator()(msgpack::packer<Stream>& o,
                    grape::serdes::examples::State const& v) const -> msgpack::packer<Stream>& {
      o.pack_array(3);
      o.pack(v.name);
      o.pack(v.timestamp);
      o.pack(v.position);
      return o;
    }
  };

  //-----------------------------------------------------------------------------------------------
  template <>
  struct convert<grape::serdes::examples::State> {
    auto operator()(msgpack::object const& o,
                    grape::serdes::examples::State& v) const -> msgpack::object const& {
      if (o.type != msgpack::type::ARRAY) {
        throw msgpack::type_error();
      }
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
      const auto& arr = o.via.array;
      if (arr.size != 3) {
        throw msgpack::type_error();
      }
      // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      v.name = arr.ptr[0].as<std::string>();
      v.timestamp = arr.ptr[1].as<double>();
      v.position = arr.ptr[2].as<std::array<double, 3>>();
      // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      return o;
    }
  };

  }  // namespace adaptor
}  // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
}  // namespace msgpack

//=================================================================================================
/// Demonstrates schemaless serialisation, storage, and deserialisation in C++ using MessagePack.
/// See msgpack-cxx/example for more usage examples
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;
  try {
    std::println("Protocol version: {}", MSGPACK_DEFAULT_API_VERSION);

    const auto state =
        grape::serdes::examples::State{ .name = "example_state",
                                        .timestamp = 1234.567890,
                                        .position = std::array<double, 3>{ 1.0, 2.0, 3.0 } };

    // serialise
    msgpack::sbuffer ss;
    msgpack::pack(ss, state);

    // Write to file
    static constexpr auto FILENAME = "state.msgpk";
    if (not grape::serdes::examples::writeToFile({ ss.data(), ss.size() }, FILENAME)) {
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
    const auto& ser_bytes2 = maybe_data.value();
    std::println("Read {} bytes from '{}'", ser_bytes2.size(), FILENAME);

    // deserialise
    grape::serdes::examples::State deserialized_state;
    const auto oh = msgpack::unpack(ser_bytes2.data(), ser_bytes2.size());
    oh.get().convert(deserialized_state);

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
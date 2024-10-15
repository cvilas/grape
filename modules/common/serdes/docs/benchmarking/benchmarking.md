# Choosing a serialisation scheme

For POD types and data structures based on them, the following may be the simplest way to serialise and deserialise; just remap the data as a stream of characters:

```c++
struct MyCustomObject{/* fields */};

template<Stream S, DataType T>
void serialise(S& io, const T& obj) {
  io.write(reinterpret_cast<char* const>(&obj), sizeof(T));
}

template<Stream S, DataType T>
auto deserialise(S& io) -> T {
  T obj;
  io.read(reinterpret_cast<char*>(&obj), sizeof(T));
  return obj;
}
```

This is not portable across architectures and compilers due to potential differences in memory alignment (padding), endianness, etc. It is also not suitable for complex structures containing pointers or dynamic memory (eg: `std::vector`). Therefore, we want a scheme built on top of a _specification_ defining rules for encoding and decoding data-types. Interoperability is a side-effect of such specification; if it is simple and well documented, its easy to implement encoders and decoders in any programming language.

The requirements stated in the [README](../../README.md) narrowed down third-party options to the following:

- [FlexBuffers](https://flatbuffers.dev/flexbuffers.html) (Schemaless extension to FlatBuffers)
- [MessagePack](https://msgpack.org/)
- [FastCDR](https://github.com/eProsima/Fast-CDR)
- [zenoh::Bytes](https://zenoh-cpp.readthedocs.io/en/latest/serialization_deserialization.html)

The main advantage With `zenoh::Bytes` is convenience, as it comes with Zenoh IPC. However, Zettascale advises against using it in performance-critical use-cases (as of Oct 2024). Also, we would like something that is standalone, as serialisation is used not just with IPC. Therefore it was eliminated as an option.

For the rest of them, representative [benchmarks](./examples/bench.cpp) (on X86-64) are as follows: 

```c++
// benchmarked data structure
struct Person {
  std::string name;
  std::int32_t id{};
  std::string email;
  std::vector<float> scores;
};

// Source data to serialise
auto generatePerson() -> Person {
  return {
    .name = "John Doe", .id = 25, .email = "john.doe@example.com", .scores = { 2.1f, 3.2f, 4.5f }
  };
}

```

```text
------------------------------------------------------------------
Benchmark                     CPU time    Serialised Data Size
------------------------------------------------------------------
bmFastCDRSerialize            15.6 ns     58 bytes
bmFastCDRDeserialize          24.8 ns         
bmCDRSerialize                24.9 ns     64 bytes
bmCDRDeserialize              34.3 ns
bmMsgpackSerialize            17.3 ns     48 bytes
bmMsgpackDeserialize           114 ns         
bmFlexBufferSerialize          172 ns     63 bytes
bmFlexBufferDeserialize       31.3 ns         
bmGrapeSerialize              11.0 ns     56 bytes (This module's custom implementation)
bmGrapeDeserialize            26.1 ns         
```

Scheme          | C++/Python interop | encode speed | decode speed | API simplicity | Documentation
----------------|--------------------|--------------|--------------|----------------|---------------
Grape (custom)  | ===                | ========     | ====         | ====           | ====
FastCDR/FastCDR | -                  | =======      | ====         | ====           | =
FastCDR/CDR     | =                  | ======       | ===          | ====           | ==
Messagepack     | ====               | ===          | =            | ===            | ====
Flexbuffers     | ====               | =            | ===          | ==             | ===

FastCDR is simple and extremely fast. It modifies the existing CDR standard by essentially eliminating padding bytes added for alignment. There is no out-of-the-box Python support and documentation is scarce. The next obvious candidate is CDR itself (also provided by FastCDR library), which implements Extended CDR version 2 (see chapter 10 of the [DDS interoperability wire protocol](https://www.omg.org/spec/DDSI-RTPS/)).

Messagepack wins overall for painless interoperability, fairly simple API, good documentation and reasonable performance. It also produces the smallest serialised data size for high wire-level efficiency. The benchmarked data structure contained 44 bytes of data. MessagePack just adds 4 bytes in overhead.

With FlatBuffers, deserialisation speed was genuinely impressive. On the contrary, serialisation speed was genuinely unimpressive.

After reviewing third-party libraries, curiosity led me to implement a proof of concept custom serialiser that uses a simple byte-packing scheme. Turns out it performed better than FastCDR, the best performing amongst the third-party options in the above list. In retrospect, this is unsurprising - the two are conceptually equivalent after all. Additionally, the custom serialiser code is way shorter and simpler thanks to Modern C++ (`std::span`, `concept`).

## How to benchmark against third-party serialisation libraries

- Add the third-party libs to `external/CMakeLists.txt`

```cmake
# -------------------------------------------------------------------------------------------------
# FlatBuffers
if(flatbuffers IN_LIST EXTERNAL_PROJECTS_LIST)
  find_package(flatbuffers ${FLATBUFFERS_VERSION_REQUIRED} QUIET)
  if(flatbuffers_FOUND)
    message(STATUS "FlatBuffers: Using version ${flatbuffers_VERSION} from ${flatbuffers_DIR}")
    add_dummy_target(flatbuffers)
  else()
    message(STATUS "FlatBuffers: Building ${FLATBUFFERS_VERSION_REQUIRED} from source")
    set(FLATBUFFERS_CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS} 
        -DFLATBUFFERS_BUILD_FLATC=OFF
        -DFLATBUFFERS_BUILD_FLATLIB=ON 
        -DFLATBUFFERS_BUILD_TESTS=OFF)
    ExternalProject_Add(
      flatbuffers
      GIT_REPOSITORY "https://github.com/google/flatbuffers.git"
      GIT_TAG v${FLATBUFFERS_VERSION_REQUIRED}
      GIT_SHALLOW true
      CMAKE_ARGS ${FLATBUFFERS_CMAKE_ARGS})
  endif()
endif()

# -------------------------------------------------------------------------------------------------
# MessagePack
if(msgpack-cxx IN_LIST EXTERNAL_PROJECTS_LIST)
  find_package(msgpack-cxx ${MSGPACK_VERSION_REQUIRED} QUIET)
  if(msgpack-cxx_FOUND)
    message(STATUS "MesssagePack: Using version ${msgpack-cxx_VERSION} from ${msgpack-cxx_DIR}")
    add_dummy_target(msgpack-cxx)
  else()
    message(STATUS "MessagePack: Building ${MSGPACK_VERSION_REQUIRED} from source")
    set(MSGPACK_CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS} 
        -DMSGPACK_USE_BOOST=OFF 
        -DMSGPACK_BUILD_DOCS=OFF
        -DMSGPACK_BUILD_EXAMPLES=OFF 
        -DMSGPACK_BUILD_TESTS=OFF)
    ExternalProject_Add(
      msgpack-cxx
      GIT_REPOSITORY "https://github.com/msgpack/msgpack-c.git"
      GIT_TAG cpp-${MSGPACK_VERSION_REQUIRED}
      GIT_SHALLOW true
      CMAKE_ARGS ${MSGPACK_CMAKE_ARGS})
  endif()
endif()

# --------------------------------------------------------------------------------------------------
# Fast CDR
if(fastcdr IN_LIST EXTERNAL_PROJECTS_LIST)
  find_package(fastcdr ${FASTCDR_VERSION_REQUIRED} QUIET)
  if(fastcdr_FOUND)
    message(STATUS "Fast-CDR: Using version ${fastcdr_VERSION} from ${fastcdr_DIR}")
    add_dummy_target(fastcdr)
  else()
    message(STATUS "Fast-CDR: Building ${FASTCDR_VERSION_REQUIRED} from source")
    ExternalProject_Add(
      fastcdr
      GIT_REPOSITORY "https://github.com/eprosima/fastcdr.git"
      GIT_TAG v${FASTCDR_VERSION_REQUIRED}
      GIT_SHALLOW true
      CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS})
  endif()
endif()
```

- In `CMakeLists.txt` define the benchmark target as follows

```cmake
find_package(flatbuffers ${FLATBUFFERS_VERSION_REQUIRED} REQUIRED)
find_package(fastcdr ${FASTCDR_VERSION_REQUIRED} REQUIRED)
find_package(msgpack-cxx ${MSGPACK_VERSION_REQUIRED} REQUIRED)

define_module_example(
  NAME bench
  SOURCES bench.cpp
  PRIVATE_INCLUDE_PATHS ""
  PRIVATE_LINK_LIBS benchmark::benchmark flatbuffers::flatbuffers fastcdr msgpack-cxx
  PUBLIC_LINK_LIBS "")
```
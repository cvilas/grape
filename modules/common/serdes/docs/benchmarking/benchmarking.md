# Benchmarking third-party serialisation libraries

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
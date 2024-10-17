# =================================================================================================
# Copyright (C) 2024 GRAPE Contributors
# =================================================================================================

find_package(flatbuffers ${FLATBUFFERS_VERSION_REQUIRED} REQUIRED)
find_package(fastcdr ${FASTCDR_VERSION_REQUIRED} REQUIRED)
find_package(msgpack-cxx ${MSGPACK_VERSION_REQUIRED} REQUIRED)

define_module_example(
  NAME bench
  SOURCES bench.cpp
  PRIVATE_INCLUDE_PATHS ""
  PRIVATE_LINK_LIBS benchmark::benchmark flatbuffers::flatbuffers fastcdr msgpack-cxx
  PUBLIC_LINK_LIBS "")
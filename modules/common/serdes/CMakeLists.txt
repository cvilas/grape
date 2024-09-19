# =================================================================================================
# Copyright (C) 2024 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME serdes
  DEPENDS_ON_MODULES ""
  DEPENDS_ON_EXTERNAL_PROJECTS "flatbuffers;msgpack-cxx;fastcdr")

find_package(flatbuffers ${FLATBUFFERS_VERSION_REQUIRED} REQUIRED)
find_package(fastcdr ${FASTCDR_VERSION_REQUIRED} REQUIRED)
find_package(msgpack-cxx ${MSGPACK_VERSION_REQUIRED} REQUIRED)

# library sources
set(SOURCES src/serdes.cpp README.md include/grape/serdes/serdes.h)

# library target
define_module_library(
  NAME serdes
  PUBLIC_LINK_LIBS flatbuffers::flatbuffers fastcdr msgpack-cxx
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)

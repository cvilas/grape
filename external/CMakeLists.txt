# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

# =================================================================================================
# This is the configuration file for third party dependencies that must be built standalone and
# isolated from rest of the project. The dependencies declared in this file are downloaded,
# configured and built at configuration time.
# =================================================================================================

cmake_minimum_required(VERSION 3.28.0)
project(grape-external LANGUAGES C CXX)

find_package(Git REQUIRED)
include(ExternalProject)
include(${CMAKE_CURRENT_LIST_DIR}/third_party_versions.cmake)

# -------------------------------------------------------------------------------------------------
# Setup baseline build configuration
# -------------------------------------------------------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/")
set_directory_properties(PROPERTIES EP_UPDATE_DISCONNECTED 1) # skip update step

# Collect common arguments to configure external projects
set(EP_CMAKE_EXTRA_ARGS
    -DCMAKE_BUILD_TYPE=Release
    -DCMAKE_INSTALL_RPATH=${CMAKE_INSTALL_RPATH}
    -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_CXX_COMPILER_LAUNCHER=${CMAKE_CXX_COMPILER_LAUNCHER}
    -DCMAKE_C_COMPILER_LAUNCHER=${CMAKE_C_COMPILER_LAUNCHER}
    -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD})

# helper macro useful for dependency handling
macro(add_dummy_target)
  if(NOT TARGET ${ARGV0})
    add_custom_target(${ARGV0})
  endif()
endmacro()

# -------------------------------------------------------------------------------------------------
# External projects follow
# -------------------------------------------------------------------------------------------------

# -------------------------------------------------------------------------------------------------
# zenohcxx
if(zenohcxx IN_LIST EXTERNAL_PROJECTS_LIST)
  # install zenoh-c first
  find_package(zenohc ${ZENOHC_VERSION_REQUIRED} QUIET)
  if(zenohc_FOUND)
    message(STATUS "zenohc: Using version ${zenohc_VERSION} from ${zenohc_DIR}")
    add_dummy_target(zenohc)
  else()
    message(STATUS "zenohc: Building ${ZENOHC_VERSION_REQUIRED} from source")
    set(ZENOHC_CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS} 
        -DZENOHC_BUILD_WITH_UNSTABLE_API=ON
        -DZENOHC_BUILD_WITH_SHARED_MEMORY=ON # shm requires both
        -DCMAKE_VERBOSE_MAKEFILE=ON)
    ExternalProject_Add(
      zenohc
      GIT_REPOSITORY "https://github.com/eclipse-zenoh/zenoh-c.git"
      GIT_TAG ${ZENOHC_VERSION_REQUIRED}
      GIT_SHALLOW true
      CMAKE_ARGS ${ZENOHC_CMAKE_ARGS})
  endif()

  find_package(zenohcxx ${ZENOHCXX_VERSION_REQUIRED} QUIET)
  if(zenohcxx_FOUND)
    message(STATUS "zenohcxx: Using version ${zenohcxx_VERSION} from ${zenohcxx_DIR}")
    add_dummy_target(zenohcxx)
  else()
    message(STATUS "zenohcxx: Building ${ZENOHCXX_VERSION_REQUIRED} from source")
    set(ZENOHCXX_CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS} 
        -DZENOHCXX_ZENOHC=ON 
        -DZENOHCXX_ZENOHPICO=OFF
        -DZENOHCXX_EXAMPLES_PROTOBUF=OFF)
    ExternalProject_Add(
      zenohcxx
      GIT_REPOSITORY "https://github.com/eclipse-zenoh/zenoh-cpp.git"
      GIT_TAG ${ZENOHCXX_VERSION_REQUIRED}
      GIT_SHALLOW true
      DEPENDS zenohc
      CMAKE_ARGS ${ZENOHCXX_CMAKE_ARGS})
  endif()
endif()

# -------------------------------------------------------------------------------------------------
# GLFW3
if(glfw3 IN_LIST EXTERNAL_PROJECTS_LIST)
  find_package(glfw3 ${GLFW3_VERSION_REQUIRED} QUIET)
  if(glfw3_FOUND)
    message(STATUS "GLFW3: Using version ${glfw3_VERSION} from ${glfw3_DIR}")
    add_dummy_target(glfw3)
  else()
    message(STATUS "GLFW3: Building ${GLFW3_VERSION_REQUIRED} from source")
    set(GLFW3_CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS} 
        -DGLFW_BUILD_DOCS=OFF 
        -DGLFW_BUILD_EXAMPLES=OFF
        -DGLFW_BUILD_TESTS=OFF 
        -DGLFW_BUILD_X11=ON 
        -DGLFW_BUILD_WAYLAND=ON)
    ExternalProject_Add(
      glfw3
      GIT_REPOSITORY "https://github.com/glfw/glfw.git"
      GIT_TAG ${GLFW3_VERSION_REQUIRED}
      GIT_SHALLOW true
      CMAKE_ARGS ${GLFW3_CMAKE_ARGS})
  endif()
endif()

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

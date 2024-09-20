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
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/")
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

# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

# =================================================================================================
# This is the configuration file for third party dependencies that must be built standalone and
# isolated from rest of the project. The dependencies declared in this file are downloaded,
# configured and built at configuration time.
# =================================================================================================

cmake_minimum_required(VERSION 3.28.3)
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
# TODO: Pin to specific version number when released
if(zenohcxx IN_LIST EXTERNAL_PROJECTS_LIST)
  # install zenoh-c first 
  find_package(zenohc QUIET) # ${ZENOHC_VERSION_REQUIRED} QUIET)
  if(zenohc_FOUND)
    message(STATUS "zenohc: Using version ${zenohc_VERSION} from ${zenohc_DIR}")
    add_dummy_target(zenohc)
  else()
    message(STATUS "zenohc: Building ${ZENOHC_VERSION_REQUIRED} from source")
    ExternalProject_Add(
      zenohc
      GIT_REPOSITORY "https://github.com/eclipse-zenoh/zenoh-c.git"
      GIT_TAG main # TODO v${ZENOHC_VERSION_REQUIRED}
      GIT_SHALLOW true
      CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS} -DCMAKE_VERBOSE_MAKEFILE=ON)
  endif()

  find_package(zenohcxx QUIET) # TODO ${ZENOHCXX_VERSION_REQUIRED} QUIET)
  if(zenohcxx_FOUND)
    message(STATUS "zenohcxx: Using version ${zenohcxx_VERSION} from ${zenohcxx_DIR}")
    add_dummy_target(zenohcxx)
  else()
    message(STATUS "zenohcxx: Building ${ZENOHCXX_VERSION_REQUIRED} from source")
    ExternalProject_Add(
      zenohcxx
      GIT_REPOSITORY "https://github.com/eclipse-zenoh/zenoh-cpp.git"
      GIT_TAG main # TODO v${ZENOHCXX_VERSION_REQUIRED}
      GIT_SHALLOW true
      SOURCE_SUBDIR install # configure the 'install' sub-project to avoid building zenoh-c again
      DEPENDS zenohc
      CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS})
  endif()
endif()

# -------------------------------------------------------------------------------------------------
# SDL3
# TODO: Specify version number when available
if(SDL3 IN_LIST EXTERNAL_PROJECTS_LIST)
  set(SDL3_VERSION_REQUIRED 3.0.0)
  find_package(SDL3 QUIET) # ${SDL3_VERSION_REQUIRED} QUIET)
  if(SDL3_FOUND)
    message(STATUS "SDL3: Using version ${SDL3_VERSION} from ${SDL3_DIR}")
    add_dummy_target(SDL3)
  else()
    message(STATUS "SDL3: Building ${SDL3_VERSION_REQUIRED} from source")
    set(SDL_CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS} 
      -DSDL_RENDER=ON
      -DSDL_AUDIO=OFF
      -DSDL_VIDEO=ON
      -DSDL_CAMERA=OFF
      -DSDL_HAPTIC=OFF
      -DSDL_JOYSTICK=OFF
      -DSDL_HIDAPI=OFF
      -DSDL_POWER=OFF
      -DSDL_SENSOR=OFF
      -DSDL_DIALOG=OFF
      -DSDL_COCOA=OFF
      -DSDL_TEST_LIBRARY=OFF 
      -DSDL_TESTS=OFF 
      -DSDL_DISABLE_INSTALL_DOCS=ON 
      -DSDL_DISABLE_INSTALL_CPACK=ON)
    ExternalProject_Add(
      SDL3
      GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
      GIT_TAG main # v${SDL3_VERSION_REQUIRED}
      GIT_SHALLOW true
      CMAKE_ARGS ${SDL_CMAKE_ARGS})
  endif()
endif()

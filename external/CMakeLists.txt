# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

# =================================================================================================
# This is the configuration file for third party dependencies that must be built standalone and
# isolated from rest of the project. The dependencies declared in this file are downloaded,
# configured and built at configuration time.
# =================================================================================================

cmake_minimum_required(VERSION 3.31.6)
project(grape-external LANGUAGES C CXX)

find_package(Git REQUIRED)
include(ExternalProject)
include(${CMAKE_CURRENT_LIST_DIR}/third_party_versions.cmake)

# -------------------------------------------------------------------------------------------------
# Setup baseline build configuration
# -------------------------------------------------------------------------------------------------
set_directory_properties(PROPERTIES EP_UPDATE_DISCONNECTED 1) # skip update step
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/")
set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")

# Collect common arguments to configure external projects
set(EP_CMAKE_EXTRA_ARGS
    -DCMAKE_BUILD_TYPE=Release
    -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
    -DCMAKE_INSTALL_RPATH=${CMAKE_INSTALL_RPATH}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_CXX_COMPILER_LAUNCHER=${CMAKE_CXX_COMPILER_LAUNCHER}
    -DCMAKE_C_COMPILER_LAUNCHER=${CMAKE_C_COMPILER_LAUNCHER})

# helper macro useful for dependency handling
macro(add_dummy_target)
  if(NOT TARGET ${ARGV0})
    add_custom_target(${ARGV0})
  endif()
endmacro()

# -------------------------------------------------------------------------------------------------
# External projects follow
# -------------------------------------------------------------------------------------------------

if(eCAL IN_LIST EXTERNAL_PROJECTS_LIST)
    set(ECAL_DEPENDS yaml-cpp tcp_pubsub)
    list(APPEND EXTERNAL_PROJECTS_LIST ${ECAL_DEPENDS})
endif()

# -------------------------------------------------------------------------------------------------
# SDL3
if(SDL3 IN_LIST EXTERNAL_PROJECTS_LIST)
  find_package(SDL3 ${SDL3_VERSION_REQUIRED} QUIET)
  if(SDL3_FOUND)
    message(STATUS "SDL3: Using version ${SDL3_VERSION} from ${SDL3_DIR}")
    add_dummy_target(SDL3)
  else()
    message(STATUS "SDL3: Building ${SDL3_VERSION_REQUIRED} from source")
    set(SDL3_CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS} 
      -DSDL_AUDIO=ON
      -DSDL_VIDEO=ON
      -DSDL_GPU=ON
      -DSDL_RENDER=ON
      -DSDL_CAMERA=ON
      -DSDL_WAYLAND=ON
      -DSDL_TEST_LIBRARY=OFF
      -DSDL_TESTS=OFF
      -DSDL_EXAMPLES=OFF)
    ExternalProject_Add(
      SDL3
      GIT_REPOSITORY "https://github.com/libsdl-org/SDL"
      GIT_TAG "release-${SDL3_VERSION_REQUIRED}"
      GIT_SHALLOW true
      CMAKE_ARGS ${SDL3_CMAKE_ARGS})
  endif()
endif()

# -------------------------------------------------------------------------------------------------
# yaml-cpp (eCAL dependency)
if(yaml-cpp IN_LIST EXTERNAL_PROJECTS_LIST)
  find_package(yaml-cpp ${YAML_CPP_VERSION_REQUIRED} QUIET)
  if(yaml-cpp_FOUND)
    message(STATUS "yaml-cpp: Using version ${yaml-cpp_VERSION} from ${yaml-cpp_DIR}")
    add_dummy_target(yaml-cpp)
  else()
    message(STATUS "yaml-cpp: Building ${YAML_CPP_VERSION_REQUIRED} from source")
    set(YAML_CPP_CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS} 
      -DBUILD_TESTING=OFF
      -DYAML_CPP_BUILD_TOOLS=OFF
      -DYAML_CPP_BUILD_TESTS=OFF
      -DYAML_CPP_FORMAT_SOURCE=OFF
      -DYAML_BUILD_SHARED_LIBS=OFF
      -DYAML_CPP_INSTALL=ON
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5) 
    set(YAML_CPP_PATCH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/sources/yaml_cpp_stdint.patch)
    ExternalProject_Add(
      yaml-cpp
      GIT_REPOSITORY "https://github.com/jbeder/yaml-cpp.git"
      GIT_TAG ${YAML_CPP_VERSION_REQUIRED}
      GIT_SHALLOW true
      PATCH_COMMAND patch -p1 < ${YAML_CPP_PATCH_FILE}
      CMAKE_ARGS ${YAML_CPP_CMAKE_ARGS})
  endif()
endif()

# -------------------------------------------------------------------------------------------------
# tcp-pubsub (eCAL dependency)
if(tcp_pubsub IN_LIST EXTERNAL_PROJECTS_LIST)
  find_package(tcp_pubsub ${TCP_PUBSUB_VERSION_REQUIRED} QUIET)
  if(tcp_pubsub_FOUND)
    message(STATUS "tcp_pubsub: Using version ${tcp_pubsub_VERSION} from ${tcp_pubsub_DIR}")
    add_dummy_target(tcp_pubsub)
  else()
    message(STATUS "tcp_pubsub: Building ${TCP_PUBSUB_VERSION_REQUIRED} from source")
    set(TCP_PUBSUB_CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS} 
        -DTCP_PUBSUB_BUILD_SAMPLES=OFF
        -DTCP_PUBSUB_BUILD_ECAL_SAMPLES=OFF
        -DBUILD_SHARED_LIBS=OFF) # workaround for https://github.com/eclipse-ecal/ecal/issues/2014
    ExternalProject_Add(
      tcp_pubsub
      GIT_REPOSITORY "https://github.com/eclipse-ecal/tcp_pubsub.git"
      GIT_TAG v${TCP_PUBSUB_VERSION_REQUIRED}
      GIT_SHALLOW true
      CMAKE_ARGS ${TCP_PUBSUB_CMAKE_ARGS})
  endif()
endif()

# -------------------------------------------------------------------------------------------------
# eCAL
if(eCAL IN_LIST EXTERNAL_PROJECTS_LIST)
  find_package(eCAL ${ECAL_VERSION_REQUIRED} QUIET)
  if(eCAL_FOUND)
    message(STATUS "eCAL: Using version ${eCAL_VERSION} from ${eCAL_DIR}")
    add_dummy_target(eCAL)
  else()
    message(STATUS "eCAL: Building ${ECAL_VERSION_REQUIRED} from source")
    set(ECAL_CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS} 
        -DECAL_USE_HDF5=OFF
        -DECAL_USE_QT=OFF
        -DECAL_BUILD_APPS=OFF
        -DECAL_BUILD_SAMPLES=OFF
        -DECAL_THIRDPARTY_BUILD_TCP_PUBSUB=OFF
        -DECAL_THIRDPARTY_BUILD_YAML-CPP=OFF
        -DECAL_THIRDPARTY_BUILD_PROTOBUF=ON
        -DECAL_INSTALL_SAMPLE_SOURCES=OFF
        -DECAL_CPACK_PACK_WITH_INNOSETUP=OFF
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5
        -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=cmake/submodule_dependencies.cmake)
    ExternalProject_Add(
      eCAL
      GIT_REPOSITORY "https://github.com/eclipse-ecal/ecal.git"
      GIT_TAG v${ECAL_VERSION_REQUIRED}-rc.2 # TODO(vilas): Update on release v6
      GIT_SHALLOW true
      DEPENDS yaml-cpp tcp_pubsub
      CMAKE_ARGS ${ECAL_CMAKE_ARGS})
  endif()
endif()



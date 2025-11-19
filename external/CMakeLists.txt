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
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
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
  list(APPEND EXTERNAL_PROJECTS_LIST ftxui)
endif()


# -------------------------------------------------------------------------------------------------
# FTXUI
if(ftxui IN_LIST EXTERNAL_PROJECTS_LIST)
  find_package(ftxui ${FTXUI_VERSION_REQUIRED} QUIET)
  if(ftxui_FOUND)
    message(STATUS "ftxui: Using version ${ftxui_VERSION} from ${ftxui_DIR}")
    add_dummy_target(ftxui)
  else()
    message(STATUS "ftxui: Building ${FTXUI_VERSION_REQUIRED} from source")
    set(FTXUI_CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS}
    -DFTXUI_ENABLE_INSTALL=ON 
    -DFTXUI_BUILD_EXAMPLES=OFF
    -DFTXUI_QUIET=OFF
    -DFTXUI_BUILD_DOCS=OFF
    -DFTXUI_BUILD_TESTS=OFF
    -DFTXUI_BUILD_TESTS_FUZZER=OFF
    -DFTXUI_ENABLE_COVERAGE=OFF
    -DFTXUI_DEV_WARNINGS=OFF
    -DFTXUI_CLANG_TIDY=OFF)
    ExternalProject_Add(
      ftxui
      URL ${CMAKE_CURRENT_SOURCE_DIR}/sources/FTXUI-${FTXUI_VERSION_REQUIRED}.tar.gz
      CMAKE_ARGS ${FTXUI_CMAKE_ARGS})
  endif()
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
      URL ${CMAKE_CURRENT_SOURCE_DIR}/sources/SDL-release-${SDL3_VERSION_REQUIRED}.tar.gz
      CMAKE_ARGS ${SDL3_CMAKE_ARGS})
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
        -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=cmake/submodule_dependencies.cmake 
        -DECAL_USE_CURL=OFF 
        -DECAL_USE_FTXUI=ON 
        -DECAL_USE_HDF5=OFF 
        -DECAL_USE_QT=OFF 
        -DECAL_USE_CAPNPROTO=OFF 
        -DECAL_USE_FLATBUFFERS=OFF 
        -DECAL_USE_NPCAP=OFF 
        -DBUILD_TESTING=OFF
        -DECAL_BUILD_DOCS=OFF 
        -DECAL_BUILD_APPS=ON 
        -DECAL_BUILD_SAMPLES=OFF 
        -DECAL_BUILD_TESTS=OFF 
        -DECAL_BUILD_TIMEPLUGINS=ON
        -DECAL_BUILD_C_BINDING=OFF 
        -DECAL_BUILD_CSHARP_BINDING=OFF 
        -DECAL_BUILD_PY_BINDING=OFF 
        -DECAL_BUILD_APP_SDK=OFF 
        -DECAL_BUILD_C_BINDING=OFF 
        -DECAL_INSTALL_SAMPLE_SOURCES=OFF  
        -DECAL_DEPLOY_DEPENDENCIES=OFF 
        -DECAL_CPACK_PACK_WITH_INNOSETUP=OFF 
        -DECAL_THIRDPARTY_BUILD_YAML-CPP=ON 
        -DECAL_THIRDPARTY_BUILD_TCP_PUBSUB=ON 
        -DECAL_THIRDPARTY_BUILD_PROTOBUF=ON 
        -DECAL_THIRDPARTY_BUILD_ASIO=ON 
        -DECAL_THIRDPARTY_BUILD_CURL=OFF 
        -DECAL_THIRDPARTY_BUILD_FINEFTP=OFF 
        -DECAL_THIRDPARTY_BUILD_FTXUI=OFF
        -DECAL_THIRDPARTY_BUILD_RECYCLE=ON 
        -DECAL_THIRDPARTY_BUILD_SPDLOG=ON 
        -DECAL_THIRDPARTY_BUILD_TCLAP=ON 
        -DECAL_THIRDPARTY_BUILD_TINYXML2=ON 
        -DCMAKE_POLICY_VERSION_MINIMUM=3.5 
        -DCMAKE_EXPORT_COMPILE_COMMANDS=OFF)

    ExternalProject_Add(
      eCAL
      URL ${CMAKE_CURRENT_SOURCE_DIR}/sources/ecal-fat-source-${ECAL_VERSION_REQUIRED}.tar.gz
      CMAKE_ARGS ${ECAL_CMAKE_ARGS}
      DEPENDS ftxui)
  endif()
endif()

# -------------------------------------------------------------------------------------------------
# lz4
if(lz4 IN_LIST EXTERNAL_PROJECTS_LIST)
  find_package(lz4 ${LZ4_VERSION_REQUIRED} QUIET)
  if(lz4_FOUND)
    message(STATUS "lz4: Using version ${lz4_VERSION} from ${lz4_DIR}")
    add_dummy_target(lz4)
  else()
    message(STATUS "lz4: Building ${LZ4_VERSION_REQUIRED} from source")
    set(LZ4_CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS}
      -DLZ4_BUILD_CLI=OFF)
    ExternalProject_Add(
      lz4
      URL ${CMAKE_CURRENT_SOURCE_DIR}/sources/lz4-${LZ4_VERSION_REQUIRED}.tar.gz
      SOURCE_SUBDIR "build/cmake"
      CMAKE_ARGS ${LZ4_CMAKE_ARGS})
  endif()
endif()


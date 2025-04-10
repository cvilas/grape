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
set_directory_properties(PROPERTIES EP_UPDATE_DISCONNECTED 1) # skip update step
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/")
set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/../${CMAKE_INSTALL_LIBDIR}:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

# Collect common arguments to configure external projects
set(EP_CMAKE_EXTRA_ARGS
    -DCMAKE_BUILD_TYPE=Release
    -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_INSTALL_PREFIX=${EP_DEPLOY_DIR}
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
# eCAL
if(eCAL IN_LIST EXTERNAL_PROJECTS_LIST)
  find_package(eCAL ${ECAL_VERSION_REQUIRED} QUIET)
  if(eCAL_FOUND)
    message(STATUS "eCAL: Using version ${eCAL_VERSION} from ${eCAL_DIR}")
    add_dummy_target(eCAL)
  else()
    message(STATUS "eCAL: Building ${ECAL_VERSION_REQUIRED} from source")

    # Build tcp-pubsub separately with bug fixes we need
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
    
    # now build eCAL itself
    set(ECAL_CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS} 
        -DECAL_USE_HDF5=OFF
        -DECAL_USE_QT=OFF
        -DECAL_BUILD_APPS=OFF
        -DECAL_BUILD_SAMPLES=OFF
        -DECAL_THIRDPARTY_BUILD_TCP_PUBSUB=OFF
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
      DEPENDS tcp_pubsub
      CMAKE_ARGS ${ECAL_CMAKE_ARGS})
  endif()
endif()



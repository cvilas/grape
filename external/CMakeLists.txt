#=================================================================================================
# Copyright (C) 2018-2023 GRAPE Contributors
#=================================================================================================

#=================================================================================================
# This is the configuration file for third party dependencies that must be built standalone and 
# isolated from rest of the project. The dependencies declared in this file are downloaded, 
# configured and built at configuration time.
#=================================================================================================

cmake_minimum_required(VERSION 3.27.3)
project(grape-external LANGUAGES C CXX)

find_package(Git REQUIRED)
include(ExternalProject)
include(ProcessorCount)
ProcessorCount(processor_count)

#--------------------------------------------------------------------------------------------------
# Setup baseline build configuration
#--------------------------------------------------------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/")
set_directory_properties(PROPERTIES EP_UPDATE_DISCONNECTED 1)   # skip update step

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

#--------------------------------------------------------------------------------------------------
# External projects follow
#--------------------------------------------------------------------------------------------------

#--------------------------------------------------------------------------------------------------
# Abseil C++
if (absl IN_LIST EXTERNAL_PROJECTS_LIST)
    set(ABSEIL_VERSION_REQUIRED 20230802)
    set(ABSEIL_PATCH_REQUIRED 0)
    find_package(absl ${ABSEIL_VERSION_REQUIRED} QUIET)
    if (absl_FOUND)
        message(STATUS "Abseil: Using version ${absl_VERSION} from ${absl_DIR}")
        add_dummy_target(absl)
    else()
        message(STATUS "Abseil: Building ${ABSEIL_VERSION_REQUIRED}.${ABSEIL_PATCH_REQUIRED} from source")
        set(ABSEIL_CMAKE_ARGS
            -DABSL_BUILD_TESTING=OFF
            -DABSL_ENABLE_INSTALL=ON
            -DABSL_PROPAGATE_CXX_STD=ON
            -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD})
        ExternalProject_Add(
            absl
            URL https://github.com/abseil/abseil-cpp/archive/refs/tags/${ABSEIL_VERSION_REQUIRED}.${ABSEIL_PATCH_REQUIRED}.tar.gz
            URL_HASH MD5=f40605e07aa804aa82e7090f12db7e34
            #GIT_REPOSITORY "https://github.com/abseil/abseil-cpp.git"
            #GIT_TAG ${ABSEIL_VERSION_REQUIRED}.${ABSEIL_PATCH_REQUIRED}
            GIT_SHALLOW true
            CMAKE_ARGS ${EP_CMAKE_EXTRA_ARGS} ${ABSEIL_CMAKE_ARGS})
    endif()
endif()
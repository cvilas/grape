#=================================================================================================
# Copyright (C) 2018 GRAPE Contributors
#=================================================================================================

# Top level CMake file for the Grape Build System. See README.md

set(GBS_TEMPLATES_DIR ${CMAKE_CURRENT_LIST_DIR})

include(${GBS_TEMPLATES_DIR}/00_utilities.cmake)
include(${GBS_TEMPLATES_DIR}/01_version.cmake)
include(${GBS_TEMPLATES_DIR}/02_build_config.cmake)
include(${GBS_TEMPLATES_DIR}/03_compiler_config.cmake)
include(${GBS_TEMPLATES_DIR}/04_code_formatter.cmake)
include(${GBS_TEMPLATES_DIR}/05_modules.cmake)
include(${GBS_TEMPLATES_DIR}/06_packager_config.cmake)

#-------------------------------------------------------------------------------------------------
# Enumerate all modules and those selected for build (with -DBUILD_MODULES)
enumerate_modules(ROOT_PATH ${CMAKE_SOURCE_DIR}/modules)

#-------------------------------------------------------------------------------------------------
# Build external dependencies before configuring project

# define staging directories for external projects
set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/external)  # location of build tree for external projects
set(EP_DEPLOY_DIR ${EP_BINARY_DIR}/deploy)       # location of artifacts from external projects
file(MAKE_DIRECTORY ${EP_DEPLOY_DIR})
get_property(_external_projects_list GLOBAL PROPERTY EXTERNAL_PROJECTS)

# Because execute_process() cannot handle ';' character in a list when passed as argument, escape it
string(REPLACE ";" "\\;" formatted_external_projects_list "${_external_projects_list}")

# Configuration of all external dependencies are in external/CMakeLists.txt
# NOTE: external/CMakeLists.txt is processed as if it was a separate project. This means:
# - Variables set there are not shared by the rest of this project
# - CMake parameters must be explicitly passed as if cmake was called on it from the command line
message(STATUS "========= External dependencies: Configuring =========")

# Delete local deployment directories of external projects
execute_process(
  COMMAND bash -c "find ${EP_BINARY_DIR} -type d \\( -name \"deploy\" -o -name \"tmp\" \\) | xargs rm -r"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  RESULT_VARIABLE _result
  ERROR_VARIABLE _error
)
if(NOT _result EQUAL 0)
  message(WARNING "Clean up command failed: ${_error}")
else()
  message(STATUS "Cleaned up external build directories")
  file(MAKE_DIRECTORY ${EP_DEPLOY_DIR}) # make the deploy directory again
endif()

# Configure external projects
execute_process(
  COMMAND ${CMAKE_COMMAND}
  -G "Ninja" ${CMAKE_SOURCE_DIR}/external # Use 'Ninja' for parallel build
  -DEXTERNAL_PROJECTS_LIST=${formatted_external_projects_list}
  -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
  -DEP_DEPLOY_DIR=${EP_DEPLOY_DIR}
  -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
  -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
  -DCMAKE_CXX_COMPILER_LAUNCHER=${CMAKE_CXX_COMPILER_LAUNCHER}
  -DCMAKE_C_COMPILER_LAUNCHER=${CMAKE_C_COMPILER_LAUNCHER}
  -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
  WORKING_DIRECTORY ${EP_BINARY_DIR}
  RESULT_VARIABLE _result
  ERROR_VARIABLE _error
)

# if configuration succeeded, then build all external projects
if (${_result} EQUAL 0)
  message(STATUS "========= External dependencies: Building    =========")
  execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    WORKING_DIRECTORY ${EP_BINARY_DIR}
    RESULT_VARIABLE _result
    ERROR_VARIABLE _error)
endif ()

# If anything went wrong with external project build, stop and exit
if (NOT ${_result} EQUAL 0)
  message(FATAL_ERROR "Error processing ${CMAKE_SOURCE_DIR}/external/CMakeLists.txt: ${_error}")
endif ()

list(PREPEND CMAKE_PREFIX_PATH ${EP_DEPLOY_DIR}/)
install(DIRECTORY ${EP_DEPLOY_DIR}/ DESTINATION ${CMAKE_INSTALL_PREFIX} USE_SOURCE_PERMISSIONS)
if (CMAKE_CROSSCOMPILING)
  list(APPEND CMAKE_FIND_ROOT_PATH ${EP_DEPLOY_DIR})
endif ()

message(STATUS "======= External dependencies: Completed build =======")

#-------------------------------------------------------------------------------------------------
# Configure modules enabled with -DBUILD_MODULES="semi-colon;separated;list".
# - Special value "all" will build all modules found
if (NOT BUILD_MODULES)
  message(STATUS "BUILD_MODULES not specified. Only modules set to ALWAYS_BUILD will be enabled.")
else ()
  message(STATUS "Requested modules (BUILD_MODULES): ${BUILD_MODULES}")
endif ()
message(STATUS "----------------------------------------------------------------------")
message(STATUS "Configuring modules")
message(STATUS "----------------------------------------------------------------------")
configure_modules()

#print_cmake_variables()


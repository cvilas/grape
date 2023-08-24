#=================================================================================================
# Copyright (C) 2018-2023 GRAPE Contributors
#=================================================================================================

declare_module(NAME base ALWAYS_BUILD DEPENDS_ON_MODULES "" DEPENDS_ON_EXTERNAL_PROJECTS "")

# generate version header
configure_file(src/version.in ${CMAKE_CURRENT_SOURCE_DIR}/src/version_impl.h @ONLY)

set(SOURCES 
  include/grape/exception.h
  include/grape/utility.h
  include/grape/version.h 
  src/exception.cpp 
  src/utility.cpp
  src/version.cpp)

define_module_library(
    NAME base 
    SOURCES ${SOURCES}
    PUBLIC_INCLUDE_PATHS
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
)

add_subdirectory(examples)
add_subdirectory(tests)
add_subdirectory(apps)
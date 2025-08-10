# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME base
  DEPENDS_ON_MODULES "utils"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

set(SOURCES include/grape/error.h include/grape/exception.h include/grape/fifo_buffer.h
            include/grape/fixed_string.h src/exception.cpp)

define_module_library(
  NAME base
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PUBLIC_LINK_LIBS grape::utils)

add_subdirectory(examples)
add_subdirectory(tests)

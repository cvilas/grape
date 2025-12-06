# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(NAME plot)

set(SOURCES include/grape/plot/plottable.h)

define_module_library(
  NAME plot
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PUBLIC_LINK_LIBS "")

add_subdirectory(examples)
add_subdirectory(tests)

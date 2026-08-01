# =================================================================================================
# Copyright (C) 2026 GRAPE Contributors
# =================================================================================================

declare_module(NAME version ALWAYS_BUILD NO_MODULE_DEPENDENCIES)

# generate version header
configure_file(src/version_impl.h.in ${CMAKE_CURRENT_BINARY_DIR}/version_impl.h @ONLY)

# library sources
set(HEADERS include/grape/version.h)
set(SOURCES src/version.cpp src/version_impl.h.in)

# library target
define_module_library(
  NAME version
  PUBLIC_LINK_LIBS ""
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  PRIVATE_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(apps)

# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME script
  DEPENDS_ON_MODULES base
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(SOURCES src/script.cpp README.md include/grape/script/script.h)

# library target
define_module_library(
  NAME script
  PUBLIC_LINK_LIBS grape::base
  PRIVATE_LINK_LIBS $<BUILD_LOCAL_INTERFACE:lualib>
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lua-5.4.7/src/)

# Subprojects
add_subdirectory(third_party)
add_subdirectory(tests)
add_subdirectory(examples)

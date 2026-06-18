# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME script
  DEPENDS_ON_MODULES "base"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(HEADERS include/grape/script/script.h)
set(SOURCES src/script.cpp)

# library target
define_module_library(
  NAME script
  PUBLIC_LINK_LIBS "grape::base"
  PRIVATE_LINK_LIBS $<BUILD_LOCAL_INTERFACE:lualib>
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lua-5.5.0/src/)

# Subprojects
add_subdirectory(third_party)
add_subdirectory(tests)
add_subdirectory(examples)

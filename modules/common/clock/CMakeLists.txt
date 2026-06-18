# =================================================================================================
# Copyright (C) 2026 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME follower_clock
  DEPENDS_ON_MODULES "base"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  message(STATUS "${MODULE_NAME}: Only supported on Linux. Bypassing module.")
  return()
endif()

# library sources
set(HEADERS include/grape/clock/follower_clock.h include/grape/clock/clock_broadcaster.h)
set(SOURCES src/tick.h src/clock_broadcaster.cpp src/follower_clock.cpp)

# library target
define_module_library(
  NAME follower_clock
  PUBLIC_LINK_LIBS "grape::base"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)

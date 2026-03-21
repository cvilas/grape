# =================================================================================================
# Copyright (C) 2026 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME follower_clock
  DEPENDS_ON_MODULES "base"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(SOURCES src/tick.h src/clock_broadcaster.cpp src/follower_clock.cpp README.md
            include/grape/clock/follower_clock.h include/grape/clock/clock_broadcaster.h)

# library target
define_module_library(
  NAME follower_clock
  PUBLIC_LINK_LIBS "grape::base"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)

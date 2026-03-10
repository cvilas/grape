# =================================================================================================
# Copyright (C) 2026 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME clock
  DEPENDS_ON_MODULES "realtime"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(SOURCES src/tick.h src/clock_broadcaster.cpp src/follower_clock.cpp README.md
            include/grape/clock/follower_clock.h include/grape/clock/clock_broadcaster.h)

# library target
define_module_library(
  NAME ego_clock
  PUBLIC_LINK_LIBS "grape::realtime"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)

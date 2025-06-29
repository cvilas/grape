# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME robot_loco_arbiter
  DEPENDS_ON_MODULES "robot_loco_common;statistics;conio;ipc;log"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(SOURCES README.md src/arbiter.cpp include/grape/robot/loco/arbiter.h)

# library target
define_module_library(
  NAME robot_loco_arbiter
  PUBLIC_LINK_LIBS "grape::robot_loco_common;grape::statistics;grape::conio;grape::ipc;grape::log"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)

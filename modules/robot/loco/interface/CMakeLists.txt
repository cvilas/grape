# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME robot_loco_interface
  DEPENDS_ON_MODULES "serdes"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

set(SOURCES
    README.md include/grape/robot/loco/arbiter_status.h include/grape/robot/loco/command.h
    include/grape/robot/loco/keep_alive_cmd.h include/grape/robot/loco/move_3d_cmd.h
    include/grape/robot/loco/topics.h)

define_module_library(
  NAME robot_loco_interface
  PUBLIC_LINK_LIBS "grape::serdes"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

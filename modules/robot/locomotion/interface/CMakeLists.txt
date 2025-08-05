# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME locomotion_interface
  DEPENDS_ON_MODULES "serdes"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

set(SOURCES
    README.md include/grape/locomotion/arbiter_status.h include/grape/locomotion/command.h
    include/grape/locomotion/keep_alive_cmd.h include/grape/locomotion/move_3d_cmd.h
    include/grape/locomotion/topics.h)

define_module_library(
  NAME locomotion_interface
  PUBLIC_LINK_LIBS "grape::serdes"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

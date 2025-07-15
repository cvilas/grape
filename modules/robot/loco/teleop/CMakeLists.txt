# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME robot_loco_teleop
  DEPENDS_ON_MODULES "robot_loco_interface;conio;ipc;log"
  DEPENDS_ON_EXTERNAL_PROJECTS "ftxui")

# library sources
set(SOURCES README.md src/teleop_client.cpp include/grape/robot/loco/teleop_client.h)

# library target
define_module_library(
  NAME robot_loco_teleop_client
  PUBLIC_LINK_LIBS "grape::robot_loco_interface;grape::conio;grape::ipc;grape::log"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(examples)
add_subdirectory(apps)

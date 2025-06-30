# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME robot_teleop_client
  DEPENDS_ON_MODULES "robot_loco_common"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(SOURCES 
  README.md 
  src/client.cpp 
  include/grape/robot/teleop/topics.h)

# library target
define_module_library(
  NAME robot_teleop_client
  PUBLIC_LINK_LIBS "grape::robot_loco_common"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)

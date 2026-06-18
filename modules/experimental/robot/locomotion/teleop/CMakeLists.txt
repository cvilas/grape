# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME teleop
  DEPENDS_ON_MODULES "locomotion_interface;conio;ipc;log"
  DEPENDS_ON_EXTERNAL_PROJECTS "ftxui")

# library sources
set(HEADERS include/grape/locomotion/teleop_client.h)
set(SOURCES README.md src/teleop_client.cpp)

# library target
define_module_library(
  NAME teleop_client
  PUBLIC_LINK_LIBS "grape::locomotion_interface;grape::ipc;grape::log"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(examples)
add_subdirectory(apps)

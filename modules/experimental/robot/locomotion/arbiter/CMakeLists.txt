# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME locomotion_arbiter
  DEPENDS_ON_MODULES "locomotion_interface;statistics;conio;ipc;log"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(HEADERS include/grape/locomotion/arbiter.h)
set(SOURCES README.md src/arbiter.cpp)

# library target
define_module_library(
  NAME locomotion_arbiter
  PUBLIC_LINK_LIBS "grape::locomotion_interface;grape::statistics;grape::log;grape::ipc"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)

# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME ego_clock
  DEPENDS_ON_MODULES "ipc;log"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(SOURCES
    src/line_fitter.h
    src/clock_data.h
    src/clock_data_receiver.h
    src/clock_data_receiver.cpp
    src/ego_clock.cpp
    src/ego_clock_driver.cpp
    README.md
    include/grape/ego_clock_driver.h
    include/grape/ego_clock.h)

# library target
define_module_library(
  NAME ego_clock
  PUBLIC_LINK_LIBS "grape::ipc;grape::log"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)
add_subdirectory(benchmarks)

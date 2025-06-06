# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME realtime
  DEPENDS_ON_MODULES "base"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(SOURCES
    src/schedule.cpp
    README.md
    include/grape/realtime/error.h
    include/grape/realtime/fifo_buffer.h
    include/grape/realtime/fixed_string.h
    include/grape/realtime/mpsc_queue.h
    include/grape/realtime/mutex.h
    include/grape/realtime/schedule.h
    include/grape/realtime/thread.h)

# library target
define_module_library(
  NAME realtime
  PUBLIC_LINK_LIBS grape::base
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:include>
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)

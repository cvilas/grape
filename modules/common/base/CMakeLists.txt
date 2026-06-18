# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME base
  DEPENDS_ON_MODULES "utils"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

set(HEADERS
    include/grape/error.h
    include/grape/exception.h
    include/grape/fifo_buffer.h
    include/grape/fixed_string.h
    include/grape/shared_memory.h
    include/grape/wall_clock.h
    include/grape/wall_timer.h)

set(SOURCES src/exception.cpp src/shared_memory.cpp)

define_module_library(
  NAME base
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  PUBLIC_LINK_LIBS grape::utils)

add_subdirectory(examples)
add_subdirectory(tests)

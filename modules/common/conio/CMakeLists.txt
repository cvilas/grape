# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

declare_module(
  NAME conio
  ALWAYS_BUILD
  DEPENDS_ON_MODULES "base"
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(HEADERS include/grape/conio/string_streamable.h include/grape/conio/program_options.h
            include/grape/conio/conio.h)
set(SOURCES src/program_options.cpp src/conio.cpp)

# library target
define_module_library(
  NAME conio
  PUBLIC_LINK_LIBS "grape::base"
  PRIVATE_LINK_LIBS ""
  SOURCES ${SOURCES}
  PUBLIC_HEADERS ${HEADERS}
  PRIVATE_INCLUDE_PATHS ""
  SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)

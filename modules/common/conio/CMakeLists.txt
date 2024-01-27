#=================================================================================================
# Copyright (C) 2023 GRAPE Contributors
#=================================================================================================

declare_module(NAME conio 
  ALWAYS_BUILD 
  DEPENDS_ON_MODULES utils base
  DEPENDS_ON_EXTERNAL_PROJECTS "")

# library sources
set(SOURCES
        src/program_options.cpp
        src/conio.cpp
        README.md
        include/grape/conio/program_options.h
        include/grape/conio/conio.h)

# library target
define_module_library(
        NAME conio
        PUBLIC_LINK_LIBS grape::utils grape::base
        PRIVATE_LINK_LIBS ""
        SOURCES ${SOURCES}
        PUBLIC_INCLUDE_PATHS
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE_INCLUDE_PATHS ""
        SYSTEM_PRIVATE_INCLUDE_PATHS "")

# Subprojects
add_subdirectory(tests)
add_subdirectory(examples)

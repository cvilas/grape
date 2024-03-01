#=================================================================================================
# Copyright (C) 2023 GRAPE Contributors
#=================================================================================================

define_module_example(
        NAME program_options_example
        SOURCES program_options_example.cpp
        PUBLIC_INCLUDE_PATHS
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        PUBLIC_LINK_LIBS "")
        

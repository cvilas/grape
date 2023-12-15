#=================================================================================================
# Copyright (C) 2018-2023 GRAPE Contributors
#=================================================================================================

define_module_example(
        NAME program_args_example
        SOURCES program_args_example.cpp
        PUBLIC_INCLUDE_PATHS
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        PUBLIC_LINK_LIBS "")
        

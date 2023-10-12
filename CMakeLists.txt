#=================================================================================================
# Copyright (C) 2018-2023 GRAPE Contributors
#=================================================================================================

cmake_minimum_required(VERSION 3.26)
project(grape LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Required for shared libraries

# Integrate the Grape Build System to setup the project
include(${CMAKE_SOURCE_DIR}/gbs/gbs.cmake)



# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

cmake_minimum_required(VERSION 3.31.6)
project(grape LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) 
set(CMAKE_UNITY_BUILD ON)

# Integrate the Grape Build System to setup the project
include(${CMAKE_SOURCE_DIR}/external/third_party_versions.cmake)
include(${CMAKE_SOURCE_DIR}/gbs/gbs.cmake)

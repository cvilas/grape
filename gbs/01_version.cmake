# =================================================================================================
# Copyright (C) 2018 GRAPE Contributors
# =================================================================================================

find_package(Git REQUIRED)

# Extract current branch name
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE REPO_BRANCH
  RESULT_VARIABLE _result
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT _result EQUAL 0)
  set(REPO_BRANCH "unknown-branch")
  message(WARNING "Repo branch check failed. Will use \"${REPO_BRANCH}\"")
endif()

# Extract commit hash of the head
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE REPO_HASH
  RESULT_VARIABLE _result
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT _result EQUAL 0)
  set(REPO_HASH "ffffffff")
  message(WARNING "Repo commit hash check failed. Will use \"${REPO_HASH}\"")
endif()

# Extract version information
execute_process(
  COMMAND git describe --tags --abbrev=0
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE REPO_TAG
  RESULT_VARIABLE _result
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT _result EQUAL 0)
  set(REPO_TAG "v0.0.0")
  message(WARNING "Repo version check failed. Will use \"${REPO_TAG}\"")
endif()

# Extract major, minor and patch versions. NOTE: works only for repo tag in format vX.Y.Z
if(NOT ${REPO_TAG} MATCHES "^v([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
  message(FATAL_ERROR "Expected version tag in format v1.2.3, got ${REPO_TAG}")
endif()
string(REGEX MATCHALL "[0-9]+" VERSION_ELEMENTS "${REPO_TAG}")
list(GET VERSION_ELEMENTS 0 VERSION_MAJOR)
list(GET VERSION_ELEMENTS 1 VERSION_MINOR)
list(GET VERSION_ELEMENTS 2 VERSION_PATCH)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

# print all that
message(STATUS "Version: ${VERSION}, branch: \"${REPO_BRANCH}\", commit hash: ${REPO_HASH}")

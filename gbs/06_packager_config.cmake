# =================================================================================================
# Copyright (C) 2018 GRAPE Contributors
# =================================================================================================

# When ON, the 'pack' target produces a runtime-only package (shared libraries + executables).
# Headers, static libraries, and CMake config files are excluded.
# When OFF (default), all artifacts including the dev component are packaged.
option(PACK_RUNTIME_ONLY "Exclude headers and static libraries from package" OFF)

# Define packaging rules
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Vilas Kumar Chitrakaran")
if(DEFINED PACKAGE_SUFFIX)
  set(CPACK_PACKAGE_NAME "${CPACK_PACKAGE_NAME}-${PACKAGE_SUFFIX}")
endif()
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_DIRECTORY ${PROJECT_BINARY_DIR}/package)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

set(CPACK_SET_DESTDIR ON) # ON: Create tmp staging area for packaging. Don't modify host filesystem
set(CPACK_INSTALL_PREFIX "")

set(CPACK_GENERATOR "TBZ2")

# Component-based packaging: each component produces its own archive
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
if(PACK_RUNTIME_ONLY)
  set(CPACK_COMPONENTS_ALL runtime)
else()
  set(CPACK_COMPONENTS_ALL runtime dev)
endif()

include(CPack)

# ==================================================================================================
# Adds a custom target to build and package up all artifacts for deployment on `make pack`
# Note: build everything first (e.g. `make all && make pack`); `pack` does not auto-trigger a build
# because the `all` pseudo-target is not portable across CMake generators.
add_custom_target(
  pack
  COMMENT "Build and package all artifacts for deployment"
  COMMAND ${CMAKE_COMMAND} -E env cpack
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# =================================================================================================
# Copyright (C) 2026 GRAPE Contributors
# =================================================================================================

# Helpers shared by CMake wrappers around third-party sources that ship without a usable CMake
# package (see external/README.md).

include_guard(GLOBAL)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# -------------------------------------------------------------------------------------------------
# Installs targets, their HEADERS file sets, a license file and an exact-version CMake package for
# the current project. Expects ${PROJECT_NAME}-config.cmake.in next to the calling CMakeLists.txt.
#
# Headers are installed under include/${PROJECT_NAME}, preserving their layout relative to the file
# set BASE_DIRS. The exported file set puts include/${PROJECT_NAME} on the include path, as upstream
# headers include siblings as "imgui.h". include/ is added for consumers to use <imgui/imgui.h>.
#
# Parameters:
#  TARGETS (list): Library targets to install. Exported as ${PROJECT_NAME}::<EXPORT_NAME>
#  LICENSE (path): Upstream license file
function(install_wrapper_package)
  cmake_parse_arguments(ARG "" "LICENSE" "TARGETS" ${ARGN})
  if(NOT ARG_TARGETS OR NOT ARG_LICENSE)
    message(FATAL_ERROR "install_wrapper_package: TARGETS and LICENSE are required")
  endif()

  set(header_dir ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
  set(config_dir ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

  foreach(target IN LISTS ARG_TARGETS)
    # Upstream makes no ABI guarantees between releases, so every release gets its own SONAME
    set_target_properties(${target} PROPERTIES VERSION ${PROJECT_VERSION}
                                               SOVERSION ${PROJECT_VERSION})
    target_include_directories(${target} INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
  endforeach()

  install(
    TARGETS ${ARG_TARGETS}
    EXPORT ${PROJECT_NAME}-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT dev
    FILE_SET HEADERS DESTINATION ${header_dir} COMPONENT dev)
  install(
    EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}-targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${config_dir}
    COMPONENT dev)

  configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}-config.cmake.in
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION ${config_dir}
    NO_SET_AND_CHECK_MACRO)
  write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY ExactVersion)
  install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
                ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
          DESTINATION ${config_dir}
          COMPONENT dev)

  install(FILES ${ARG_LICENSE}
          DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/${PROJECT_NAME}
          COMPONENT runtime)
endfunction()

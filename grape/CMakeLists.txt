# generate version header
set(VERSION_IN ${CMAKE_CURRENT_SOURCE_DIR}/common/src/version.in)
set(VERSION_OUT ${CMAKE_CURRENT_SOURCE_DIR}/common/include/grape/common/version.h)
configure_file(${VERSION_IN} ${VERSION_OUT} @ONLY)

#################################################################
# NOTE: sudirectories are listed in the order of dependencies - the
# first project has no dependency on any other.
#################################################################

add_subdirectory(common)
add_subdirectory(plant)
add_subdirectory(supervisor)

#------------------------------------------------------------------------------
# compile objects into a library
#------------------------------------------------------------------------------

add_library(${PLANT_LIBRARY_TARGET}
    $<TARGET_OBJECTS:${PROJECT_NAME}_common>
    $<TARGET_OBJECTS:${PROJECT_NAME}_plantlib>)
set_target_properties(${PLANT_LIBRARY_TARGET} PROPERTIES VERSION ${VERSION} SOVERSION ${VERSION_MAJOR} )
if (DOXYGEN_FOUND)
add_dependencies(${PLANT_LIBRARY_TARGET} docs)
endif()

add_library(${SUPERVISOR_LIBRARY_TARGET}
    $<TARGET_OBJECTS:${PROJECT_NAME}_common>
    $<TARGET_OBJECTS:${PROJECT_NAME}_supervisorlib>)
set_target_properties(${SUPERVISOR_LIBRARY_TARGET} PROPERTIES VERSION ${VERSION} SOVERSION ${VERSION_MAJOR} )
if (DOXYGEN_FOUND)
add_dependencies(${SUPERVISOR_LIBRARY_TARGET} docs)
endif()

#------------------------------------------------------------------------------
# installation
#------------------------------------------------------------------------------

configure_file(${PROJECT_SOURCE_DIR}/cmake/config.cmake.in
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-config.cmake" @ONLY)

configure_file(${PROJECT_SOURCE_DIR}/cmake/config-version.cmake.in
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-config-version.cmake" @ONLY)

install(TARGETS ${PLANT_LIBRARY_TARGET} ${SUPERVISOR_LIBRARY_TARGET}
        EXPORT ${PROJECT_NAME}-targets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        COMPONENT libraries)

install(EXPORT ${PROJECT_NAME}-targets DESTINATION lib/${PROJECT_NAME}/cmake)

install(FILES
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-config.cmake"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-config-version.cmake"
  DESTINATION lib/${PROJECT_NAME}/cmake)

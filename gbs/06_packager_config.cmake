# =================================================================================================
# Copyright (C) 2018 GRAPE Contributors
# =================================================================================================

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

include(CPack)

# ==================================================================================================
# Adds a custom target to build and package up all artifacts for deployment on `make pack`
add_custom_target(
  pack 
  COMMENT "Build and package all artifacts for deployment"
  COMMAND ${CMAKE_COMMAND} -E env cpack
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DEPENDS all
)

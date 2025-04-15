# =================================================================================================
# Copyright (C) 2018 GRAPE Contributors
# =================================================================================================

# Define packaging rules
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
if(DEFINED PACKAGE_SUFFIX)
  set(CPACK_PACKAGE_NAME "${CPACK_PACKAGE_NAME}-${PACKAGE_SUFFIX}")
endif()
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_PACKAGE_VENDOR "Vilas Kumar Chitrakaran")
set(CPACK_PACKAGE_DIRECTORY ${PROJECT_BINARY_DIR}/package)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

set(CPACK_SET_DESTDIR ON)
set(CPACK_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

set(CPACK_GENERATOR "TBZ2")
#set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
#set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_VENDOR})
#set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)

# ==================================================================================================
# Adds a custom target to package up all artifacts for deployment on `make pack`
add_custom_target(
  pack 
  COMMENT "Package all artifacts for deployment"
  COMMAND ${CMAKE_COMMAND} -E env cpack
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

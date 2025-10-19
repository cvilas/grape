# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

define_module_app(
  NAME picam_pub
  SOURCES picam_pub.cpp
  PUBLIC_LINK_LIBS "grape::conio;grape::script")

set(CONF_DEST_DIR ${CMAKE_PROJECT_NAME}/picam_pub)
configure_file(config.lua ${CMAKE_BINARY_DIR}/share/${CONF_DEST_DIR}/config.lua)
install(FILES config.lua DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${CONF_DEST_DIR})

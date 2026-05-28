# =================================================================================================
# Copyright (C) 2026 GRAPE Contributors
# =================================================================================================

define_module_example(NAME example SOURCES example.cpp)

# QCustomPlot example — requires Qt6; QCustomPlot is vendored in qcustomplot/
find_package(Qt6 QUIET COMPONENTS Core Widgets PrintSupport OpenGL OpenGLWidgets)
if(Qt6_FOUND)
  message(STATUS "${MODULE_NAME}: QCustomPlot example enabled (Qt6 ${Qt6_VERSION})")

  set(QCP_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/qcustomplot)

  add_library(qcustomplotlib OBJECT ${QCP_SOURCES_DIR}/qcustomplot.cpp)
  set_target_properties(qcustomplotlib PROPERTIES AUTOMOC ON)
  set_target_properties(qcustomplotlib PROPERTIES CXX_CLANG_TIDY "")
  set_target_properties(qcustomplotlib PROPERTIES COMPILE_OPTIONS ${THIRD_PARTY_COMPILER_WARNINGS})
  set_target_properties(qcustomplotlib PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_include_directories(qcustomplotlib PRIVATE ${QCP_SOURCES_DIR})
  target_compile_definitions(qcustomplotlib PUBLIC QCUSTOMPLOT_USE_OPENGL)
  target_link_libraries(qcustomplotlib PUBLIC Qt6::Core Qt6::Widgets Qt6::PrintSupport
                                              Qt6::OpenGL Qt6::OpenGLWidgets)

  define_module_example(NAME qcp_example SOURCES qcp_example.cpp
  SYSTEM_PRIVATE_INCLUDE_PATHS ${QCP_SOURCES_DIR}
  PRIVATE_LINK_LIBS qcustomplotlib)
else()
  message(
    STATUS "${MODULE_NAME}: QCustomPlot example disabled (Qt6 not found; install qt6-base-dev)")
endif()

# =================================================================================================
# Copyright (C) 2025 grape contributors
# =================================================================================================

set(SOURCES hello.cpp)

define_module_example(
  NAME hello
  SOURCES ${SOURCES}
  PRIVATE_LINK_LIBS PkgConfig::LIBCAMERA)

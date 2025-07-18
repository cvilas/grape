# =================================================================================================
# Copyright (C) 2018 GRAPE Contributors
# =================================================================================================

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # required by source analysis tools

# Build shared libraries by default
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Baseline compiler warning settings for project and external targets
add_compile_options(-Wall -Wextra -Wpedantic -Werror)
set(THIRD_PARTY_COMPILER_WARNINGS -Wall -Wextra -Wpedantic)

# clang warnings
set(CLANG_WARNINGS -Weverything
  -Wno-c++23-compat
  -Wno-c++20-compat
  -Wno-pre-c++23-compat-pedantic
  -Wno-pre-c++20-compat-pedantic
  -Wno-pre-c++17-compat
  -Wno-c++98-compat
  -Wno-c++98-compat-pedantic
  -Wno-unsafe-buffer-usage
  -Wno-padded
  -Wno-switch-default
  -Wno-ctad-maybe-unsupported
  -Wno-global-constructors
  -Wno-weak-vtables
  -Wno-exit-time-destructors
  -Wno-documentation-unknown-command
  -Wno-reserved-macro-identifier
  -Wno-old-style-cast)

# GCC warnings
set(GCC_WARNINGS
  -Wshadow # warn the user if a variable declaration shadows one from a parent context
  -Wnon-virtual-dtor # warn if a class with virtual functions has a non-virtual destructor.
  -Wcast-align # warn for potential performance problem casts
  -Wunused # warn on anything being unused
  -Woverloaded-virtual # warn if you overload (not override) a virtual function
  -Wconversion # warn on type conversions that may lose data
  -Wsign-conversion # warn on sign conversions
  -Wnull-dereference # warn if a null dereference is detected
  -Wdouble-promotion # warn if float is implicit promoted to double
  -Wformat=2 # warn on security issues around functions that format output (ie printf)
  -Wimplicit-fallthrough # warn on statements that fallthrough without an explicit annotation
  -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
  -Wduplicated-cond # warn if if / else chain has duplicated conditions
  -Wduplicated-branches # warn if if / else branches have duplicated code
  -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
  -Wconversion
  -Wcast-qual
  -Wpointer-arith
)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  add_compile_options(-fcolor-diagnostics -fexperimental-library ${CLANG_WARNINGS})
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  add_compile_options(-fdiagnostics-color=always ${GCC_WARNINGS})
else()
  message(FATAL_ERROR "Unsupported compiler '${CMAKE_CXX_COMPILER_ID}'")
endif()

# enable sanitizer options
set(SANITIZERS "")

option(ENABLE_ASAN "Enable address sanitizer" FALSE)
if(ENABLE_ASAN)
  list(APPEND SANITIZERS "address")
endif()

option(ENABLE_LSAN "Enable run-time memory leak detector" FALSE)
if(${ENABLE_LSAN})
  list(APPEND SANITIZERS "leak")
endif()

option(ENABLE_UBSAN "Enable undefined behavior sanitizer" FALSE)
if(ENABLE_UBSAN)
  list(APPEND SANITIZERS "undefined")
endif()

option(ENABLE_TSAN "Enable thread sanitizer" FALSE)
if(${ENABLE_TSAN})
  if("address" IN_LIST SANITIZERS OR "leak" IN_LIST SANITIZERS)
    message(FATAL_ERROR "Thread sanitizer does not work with Address and Leak sanitizer enabled")
  else()
    list(APPEND SANITIZERS "thread")
  endif()
endif()

option(ENABLE_MSAN "Enable memory sanitizer" FALSE)
if(${ENABLE_MSAN})
  if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    message(WARNING "Memory sanitizer requires all code (including libc++) to be MSAN-instrumented to avoid false positives")
  endif()
  if("address" IN_LIST SANITIZERS
     OR "thread" IN_LIST SANITIZERS
     OR "leak" IN_LIST SANITIZERS)
    message(
      FATAL_ERROR "Memory sanitizer does not work with Address, Thread or Leak sanitizer enabled")
  else()
    list(APPEND SANITIZERS "memory")
  endif()
endif()

list(JOIN SANITIZERS "," LIST_OF_SANITIZERS)
if(LIST_OF_SANITIZERS)
  if(NOT "${LIST_OF_SANITIZERS}" STREQUAL "")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=${LIST_OF_SANITIZERS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=${LIST_OF_SANITIZERS}")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=${LIST_OF_SANITIZERS}")
  endif()
endif()

# Set whether to enable inter-procedural optimisation
option(ENABLE_IPO "Enable interprocedural optimization" ON)
if(ENABLE_IPO)
  include(CheckIPOSupported)
  check_ipo_supported(RESULT is_ipo_supported OUTPUT output)
  if(is_ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    cmake_policy(SET CMP0069 NEW) # For submodules (eg: googletest) that use CMake older than v3.8
    set(CMAKE_POLICY_DEFAULT_CMP0069 NEW) # About: Run "cmake --help-policy CMP0069"
  else()
    message(WARNING "Interprocedural optimisation is not supported: ${output}")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
  endif()
endif()

# Configure compiler cache
option(ENABLE_CACHE "Enable cache if available" ON)
if(ENABLE_CACHE)
  set(CACHE_PROGRAM_OPTIONS "ccache" "sccache")
  foreach(cache_option IN LISTS CACHE_PROGRAM_OPTIONS)
    find_program(CACHE_BIN ${cache_option})
    if(CACHE_BIN)
      set(CMAKE_CXX_COMPILER_LAUNCHER ${CACHE_BIN})
      set(CMAKE_C_COMPILER_LAUNCHER ${CACHE_BIN})
      break()
    endif()
  endforeach()
  if(NOT CACHE_BIN)
    message(WARNING "Compiler cache requested but none found")
  endif()
endif()

# Configure test coverage
option(ENABLE_COVERAGE "Collect coverage data" OFF)
if(ENABLE_COVERAGE)
  add_compile_options("--coverage")
  add_link_options("--coverage")
endif()

# Linter (clang-tidy)
option(ENABLE_LINTER "Enable static analysis" ON)
option(LINTER_WARNING_IS_ERROR "Treat linter warnings as errors" OFF)
if(ENABLE_LINTER)
  find_program(LINTER_BIN NAMES clang-tidy QUIET)
  if(LINTER_BIN)
    set(LINTER_ARGS
      -extra-arg=-Wno-ignored-optimization-argument
      -extra-arg=-Wno-unknown-warning-option)
    if(LINTER_WARNING_IS_ERROR)
      list(APPEND LINTER_ARGS -warnings-as-errors=*)
    endif()  
    # NOTE: To speed up linting, clang-tidy is invoked via clang-tidy-cache.
    # (https://github.com/matus-chochlik/ctcache) Cache location is set by environment variable
    # CTCACHE_DIR
    set(LINTER_INVOKE_COMMAND ${GBS_TEMPLATES_DIR}/clang-tidy-cache.py ${LINTER_BIN} -p ${CMAKE_BINARY_DIR} ${LINTER_ARGS})
    set(CMAKE_C_CLANG_TIDY ${LINTER_INVOKE_COMMAND})
    set(CMAKE_CXX_CLANG_TIDY ${LINTER_INVOKE_COMMAND})
  else()
    message(WARNING "Linter (clang-tidy) not found.")
  endif()
endif()

# print summary
message(STATUS "Compiler configuration:")
message(STATUS "\tCompiler          : ${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "\tBUILD_SHARED_LIBS : ${BUILD_SHARED_LIBS}")
message(STATUS "\tENABLE_CACHE      : ${ENABLE_CACHE} (${CACHE_BIN})")
message(STATUS "\tENABLE_IPO        : ${ENABLE_IPO} (supported: ${is_ipo_supported})")
message(STATUS "\tENABLE_ASAN       : ${ENABLE_ASAN}")
message(STATUS "\tENABLE_LSAN       : ${ENABLE_LSAN}")
message(STATUS "\tENABLE_MSAN       : ${ENABLE_MSAN}")
message(STATUS "\tENABLE_TSAN       : ${ENABLE_TSAN}")
message(STATUS "\tENABLE_UBSAN      : ${ENABLE_UBSAN}")
message(STATUS "\tENABLE_COVERAGE   : ${ENABLE_COVERAGE}")
message(STATUS "\tENABLE_LINTER           : ${ENABLE_LINTER} (${LINTER_BIN})")
message(STATUS "\tLINTER_WARNING_IS_ERROR : ${LINTER_WARNINGS_AS_ERRORS}")

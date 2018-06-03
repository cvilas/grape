######################################################
# NOTE: Do not add dependencies to this library. It should be independent of the rest of the project
######################################################

file(GLOB_RECURSE this_src ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp version.in)
file(GLOB_RECURSE this_hdr ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME}_common OBJECT ${this_src} ${this_hdr})
add_clang_format(${PROJECT_NAME}_common)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)

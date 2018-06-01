file(GLOB_RECURSE this_src ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp version.in)
file(GLOB_RECURSE this_hdr ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/grape/common/include)

add_library(${PROJECT_NAME}_plantlib OBJECT ${this_src} ${this_hdr})
add_dependencies(${PROJECT_NAME}_plantlib ${PROJECT_NAME}_common)
add_clang_format(${PROJECT_NAME}_plantlib)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)

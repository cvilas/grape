# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

define_module_app(
  NAME ipc_router
  SOURCES router.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME scout
  SOURCES scout.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME info
  SOURCES info.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME ping
  SOURCES ping.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME pong
  SOURCES pong.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME throughput_pub
  SOURCES throughput_pub.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME throughput_sub
  SOURCES throughput_sub.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME queryable
  SOURCES queryable.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME query_sub 
  SOURCES query_sub.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include> 
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME query_get
  SOURCES query_get.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME query_get_channel
  SOURCES query_get_channel.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME query_get_channel_non_blocking
  SOURCES query_get_channel_non_blocking.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME put
  SOURCES put.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME pub_example
  SOURCES pub_example.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME sub_example
  SOURCES sub_example.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME pub_delete
  SOURCES pub_delete.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME pub_shm
  SOURCES pub_shm.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME sub_shm
  SOURCES sub_shm.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME pub_cache
  SOURCES pub_cache.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

#define_module_example( 
#  NAME pull SOURCES pull.cpp zenoh_utils.h 
#  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include> 
#  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME pub_client
  SOURCES pub_client.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME sub_client
  SOURCES sub_client.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME liveliness_declare
  SOURCES liveliness_declare.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME liveliness_sub
  SOURCES liveliness_sub.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME liveliness_get
  SOURCES liveliness_get.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME attachment_pub
  SOURCES attachment_pub.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME attachment_sub
  SOURCES attachment_sub.cpp zenoh_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

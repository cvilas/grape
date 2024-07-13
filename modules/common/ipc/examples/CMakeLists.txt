# =================================================================================================
# Copyright (C) 2023 GRAPE Contributors
# =================================================================================================

define_module_example(
  NAME zenoh_scout
  SOURCES zenoh_scout.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_info
  SOURCES zenoh_info.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_ping
  SOURCES zenoh_ping.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_pong
  SOURCES zenoh_pong.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_throughput_pub
  SOURCES zenoh_throughput_pub.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_throughput_sub
  SOURCES zenoh_throughput_sub.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_queryable
  SOURCES zenoh_queryable.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_query_sub
  SOURCES zenoh_query_sub.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_query_get
  SOURCES zenoh_query_get.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_query_get_channel
  SOURCES zenoh_query_get_channel.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_query_get_channel_non_blocking
  SOURCES zenoh_query_get_channel_non_blocking.cpp
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_put
  SOURCES zenoh_put.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_pub
  SOURCES zenoh_pub.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_pub_delete
  SOURCES zenoh_pub_delete.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_pub_shm
  SOURCES zenoh_pub_shm.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_pub_cache
  SOURCES zenoh_pub_cache.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_sub
  SOURCES zenoh_sub.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_pull
  SOURCES zenoh_pull.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_pub_client
  SOURCES zenoh_pub_client.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_sub_client
  SOURCES zenoh_sub_client.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_liveliness_declare
  SOURCES zenoh_liveliness_declare.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_liveliness_sub
  SOURCES zenoh_liveliness_sub.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_liveliness_get
  SOURCES zenoh_liveliness_get.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_attachment_pub
  SOURCES zenoh_attachment_pub.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

define_module_example(
  NAME zenoh_attachment_sub
  SOURCES zenoh_attachment_sub.cpp examples_utils.h
  PUBLIC_INCLUDE_PATHS $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
  PUBLIC_LINK_LIBS grape::conio)

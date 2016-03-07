project(libc-os)

cmake_minimum_required(VERSION 2.4.0)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
set(os_sources assumes.c debug_private.c alloc_once_impl.c)

add_library(libc-os OBJECT ${os_sources})

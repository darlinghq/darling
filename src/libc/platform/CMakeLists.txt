project(libc-platform)

cmake_minimum_required(VERSION 2.4.0)

set(platform_sources OSAtomic.c)

add_library(libc-platform OBJECT ${platform_sources})



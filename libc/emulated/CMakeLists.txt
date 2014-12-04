project(libc-emulated)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c89")

set(emulated_sources brk.c bsd_signal.c lchflags.c lchmod.c lutimes.c statvfs.c tcgetsid.c)

add_library(libc-emulated STATIC ${emulated_sources})

project(libc-compat)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c89")

set(compat_sources setregid.c setreuid.c sigaltstk.c sigcompat.c)

set(compat_sources ${compat_sources}
	FreeBSD/creat.c
	FreeBSD/gethostid.c
	FreeBSD/getwd.c
	FreeBSD/killpg.c
	FreeBSD/sethostid.c
	FreeBSD/setpgrp.c
	FreeBSD/setrgid.c
	FreeBSD/setruid.c)

SET_SOURCE_FILES_PROPERTIES(FreeBSD/creat.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_CREAT")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/killpg.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_KILLPG")

add_library(libc-compat STATIC ${compat_sources})

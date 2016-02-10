project(libc-stdtime)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -include stdbool.h")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/../fbsdcompat/_fbsd_compat_.h")

set(stdtime_sources getdate.c timezone_unix03.c)

set(stdtime_sources ${stdtime_sources}
	FreeBSD/asctime.c
	FreeBSD/difftime.c
	FreeBSD/ftime.c
	FreeBSD/localtime.c
	FreeBSD/strftime.c
	FreeBSD/strptime.c
	FreeBSD/time32.c
	FreeBSD/timelocal.c
)

SET_SOURCE_FILES_PROPERTIES(FreeBSD/localtime.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_MKTIME")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/strftime.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_STRFTIME -DLIBC_ALIAS_STRFTIME_L")
SET_SOURCE_FILES_PROPERTIES(FreeBSD/strptime.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_STRPTIME -DLIBC_ALIAS_STRPTIME_L")

add_library(libc-stdtime OBJECT ${stdtime_sources})

# Legacy variant
set(stdtime-legacy_sources
	FreeBSD/localtime.c
	FreeBSD/strftime.c
	FreeBSD/strptime.c)
add_library(libc-stdtime_legacy OBJECT ${stdtime-legacy_sources})
SET_TARGET_PROPERTIES(libc-stdtime_legacy PROPERTIES COMPILE_DEFINITIONS "BUILDING_VARIANT;VARIANT_LEGACY")


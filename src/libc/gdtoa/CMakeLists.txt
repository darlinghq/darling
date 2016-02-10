project(libc-gdtoa)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

# set(gdtoa_sources _ldbl_util.c) # doesn't build

set(gdtoa_sources ${gdtoa_sources}
	FreeBSD/gdtoa-dmisc.c
	FreeBSD/gdtoa-dtoa.c
	FreeBSD/gdtoa-gdtoa.c
	FreeBSD/gdtoa-gethex.c
	FreeBSD/gdtoa-gmisc.c
	FreeBSD/gdtoa-hd_init.c
	FreeBSD/gdtoa-hexnan.c
	FreeBSD/gdtoa-misc.c
	FreeBSD/gdtoa-smisc.c
	FreeBSD/gdtoa-strtod.c
	FreeBSD/gdtoa-strtodg.c
	FreeBSD/gdtoa-strtof.c
	#FreeBSD/gdtoa-strtoIg.c
	FreeBSD/gdtoa-strtopdd.c
	FreeBSD/gdtoa-strtopx.c
	FreeBSD/gdtoa-strtord.c
	FreeBSD/gdtoa-sum.c
	FreeBSD/gdtoa-ulp.c
	FreeBSD/glue.c
	FreeBSD/_hdtoa.c
	FreeBSD/_ldtoa.c
	#FreeBSD/machdep_ldisd.c # these are not for x86
	#FreeBSD/machdep_ldisdd.c
	#FreeBSD/machdep_ldisQ.c
	FreeBSD/machdep_ldisx.c)

#SET_SOURCE_FILES_PROPERTIES(FreeBSD/gdtoa-strtod.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_STRTOD -DLIBC_ALIAS_STRTOD_L")
#SET_SOURCE_FILES_PROPERTIES(FreeBSD/gdtoa-strtof.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_STRTOF -DLIBC_ALIAS_STRTOF_L")

add_library(libc-gdtoa OBJECT ${gdtoa_sources})

# Legacy variant
set(gdtoa-legacy_sources
	FreeBSD/gdtoa-strtod.c
	FreeBSD/gdtoa-strtodg.c
	FreeBSD/gdtoa-strtof.c)
add_library(libc-gdtoa_legacy OBJECT ${gdtoa-legacy_sources})
SET_TARGET_PROPERTIES(libc-gdtoa_legacy PROPERTIES COMPILE_FLAGS
	"-DBUILDING_VARIANT -DVARIANT_LEGACY -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0")


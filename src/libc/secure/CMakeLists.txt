project(libc-secure)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -D_C99_SOURCE -D_FORTIFY_SOURCE=0 -D__va_list=__builtin_va_list")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../gdtoa/FreeBSD")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../gdtoa")

set(secure_sources
	chk_fail.c
	memcpy_chk.c
	memmove_chk.c
	memset_chk.c
	snprintf_chk.c
	sprintf_chk.c
	stpcpy_chk.c
	stpncpy_chk.c
	strcat_chk.c
	strcpy_chk.c
	strlcpy_chk.c
	memccpy_chk.c
	secure.c
	strlcat_chk.c
	strncat_chk.c
	strncpy_chk.c
	vsnprintf_chk.c
	vsprintf_chk.c
)

#SET_SOURCE_FILES_PROPERTIES(FreeBSD/snprintf.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS}")

add_library(libc-secure OBJECT ${secure_sources})

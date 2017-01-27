project(libc-string)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -D_SECURE__STRING_H_")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/../fbsdcompat/_fbsd_compat_.h")

add_definitions(-D__STDC_WANT_LIB_EXT1__=1)

set(string_sources
	bcopy.c
	stpcpy.c
	stpncpy.c
	strcat.c
	#strcpy.c
	strncat.c
	#strncpy.c
)

# x86_64 only, i386 has its own assembly impl
set(string_x86_64_sources
	strlcat.c
	strlcpy.c
)

# i386 only, vice versa
set(string_i386_sources
	FreeBSD/strnlen.c)

set(string_sources ${string_sources}
	FreeBSD/index.c
	FreeBSD/memmem.c
	FreeBSD/rindex.c
	FreeBSD/strcasecmp.c
	FreeBSD/strcasestr.c
	FreeBSD/strcoll.c
	FreeBSD/strcspn.c
	FreeBSD/strdup.c
	FreeBSD/strerror.c
	#FreeBSD/strlen.c
	FreeBSD/strmode.c
	FreeBSD/strndup.c
	#FreeBSD/strnlen.c
	FreeBSD/strnstr.c
	FreeBSD/strpbrk.c
	FreeBSD/strrchr.c
	FreeBSD/strsep.c
	FreeBSD/strsignal.c
	FreeBSD/strspn.c
	FreeBSD/strstr.c
	FreeBSD/strtok.c
	FreeBSD/strxfrm.c
	FreeBSD/swab.c
	FreeBSD/timingsafe_bcmp.c
	FreeBSD/wcpcpy.c
	FreeBSD/wcpncpy.c
	FreeBSD/wcscasecmp.c
	FreeBSD/wcscat.c
	FreeBSD/wcscmp.c
	FreeBSD/wcscoll.c
	FreeBSD/wcscpy.c
	FreeBSD/wcscspn.c
	FreeBSD/wcsdup.c
	FreeBSD/wcschr.c
	FreeBSD/wcslcat.c
	FreeBSD/wcslcpy.c
	FreeBSD/wcslen.c
	FreeBSD/wcsncasecmp.c
	FreeBSD/wcsncat.c
	FreeBSD/wcsncmp.c
	FreeBSD/wcsncpy.c
	FreeBSD/wcsnlen.c
	FreeBSD/wcspbrk.c
	FreeBSD/wcsrchr.c
	FreeBSD/wcsspn.c
	FreeBSD/wcsstr.c
	FreeBSD/wcstok.c
	FreeBSD/wcswidth.c
	FreeBSD/wcsxfrm.c
	FreeBSD/wmemcmp.c
	FreeBSD/wmemcpy.c
	FreeBSD/wmemchr.c
	FreeBSD/wmemmove.c
	FreeBSD/wmemset.c
	NetBSD/memset_s.c
)

SET_SOURCE_FILES_PROPERTIES(FreeBSD/strerror.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_STRERROR")

add_library(libc-string OBJECT ${string_sources})
add_library(libc-string-x86_64 OBJECT ${string_x86_64_sources})
set_target_properties(libc-string-x86_64 PROPERTIES COMPILE_FLAGS "-arch x86_64")

add_library(libc-string-i386 OBJECT ${string_i386_sources})
set_target_properties(libc-string-i386 PROPERTIES COMPILE_FLAGS "-arch i386")

# Legacy variant
set(string-legacy_sources
	FreeBSD/strerror.c)
add_library(libc-string_legacy OBJECT ${string-legacy_sources})
SET_TARGET_PROPERTIES(libc-string_legacy PROPERTIES COMPILE_DEFINITIONS "BUILDING_VARIANT;VARIANT_LEGACY")

# DYLD variant
add_library(libc-string_dyld OBJECT strcat.c strcpy.c FreeBSD/strdup.c strlcat.c strlcpy.c FreeBSD/strnlen.c FreeBSD/strrchr.c FreeBSD/strstr.c FreeBSD/strtok.c
	FreeBSD/wcscpy.c FreeBSD/wcslen.c FreeBSD/wcsncmp.c FreeBSD/wcsncpy.c FreeBSD/strerror.c
	strlcat.c strlcpy.c strncat.c strncpy.c)
SET_TARGET_PROPERTIES(libc-string_dyld PROPERTIES COMPILE_FLAGS "-UBUILDING_VARIANT -DVARIANT_DYLD -DVARIANT_CANCELABLE -DVARIANT_DARWINEXTSN")


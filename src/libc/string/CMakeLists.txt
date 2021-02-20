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
	strncat.c
)

set(string_sources ${string_sources}
	FreeBSD/memmem.c
	FreeBSD/rindex.c
	FreeBSD/rpmatch.c
	FreeBSD/strcasecmp.c
	FreeBSD/strcasestr.c
	FreeBSD/strcoll.c
	FreeBSD/strcspn.c
	FreeBSD/strdup.c
	FreeBSD/strerror.c
	FreeBSD/strmode.c
	FreeBSD/strndup.c
	FreeBSD/strnstr.c
	FreeBSD/strpbrk.c
	FreeBSD/strrchr.c
	FreeBSD/strsep.c
	FreeBSD/strsignal.c
	FreeBSD/strspn.c
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

add_darling_object_library(libc-string ${string_sources})

# Legacy variant
if (TARGET_i386)
	set(string-legacy_sources
		FreeBSD/strerror.c)
	add_library(libc-string_legacy OBJECT ${string-legacy_sources})
	SET_TARGET_PROPERTIES(libc-string_legacy PROPERTIES COMPILE_FLAGS "-DBUILDING_VARIANT -DVARIANT_LEGACY -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0 -U__DARWIN_64_BIT_INO_T -D_DARWIN_NO_64_BIT_INODE")
endif (TARGET_i386)

# DYLD variant
add_library(libc-string_dyld OBJECT
	strcat.c
	strncat.c
	FreeBSD/strdup.c
	FreeBSD/strrchr.c
	FreeBSD/strtok.c
	FreeBSD/wcscpy.c
	FreeBSD/wcslen.c
	FreeBSD/wcsncmp.c
	FreeBSD/wcsncpy.c
	FreeBSD/strerror.c
)
SET_TARGET_PROPERTIES(libc-string_dyld PROPERTIES COMPILE_FLAGS "-UBUILDING_VARIANT -DVARIANT_DYLD -DVARIANT_CANCELABLE -DVARIANT_DARWINEXTSN")


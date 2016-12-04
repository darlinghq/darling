project(darling)

cmake_minimum_required(VERSION 2.8)
enable_language(ASM)

set(DARLING_TOP_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set(DARLING_NO_EXECUTABLES OFF)
set(CMAKE_C_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")

if (${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "i686|i386")
	message(FATAL_ERROR "This software can only be built on x86-64 systems")
endif (${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "i686|i386")

if (NOT BITS)
	message(FATAL_ERROR "BITS is not specified (32/64)")
endif (NOT BITS)

SET(IGNORED_WARNINGS "")

if (${CMAKE_C_COMPILER_ID} STREQUAL "Clang" AND NOT ${CMAKE_C_COMPILER_VERSION} VERSION_LESS "3.9")
	SET(IGNORED_WARNINGS "${IGNORED_WARNINGS} -Wno-expansion-to-defined")
endif (${CMAKE_C_COMPILER_ID} STREQUAL "Clang" AND NOT ${CMAKE_C_COMPILER_VERSION} VERSION_LESS "3.9")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m${BITS} -D__APPLE_CC__ ${IGNORED_WARNINGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m${BITS} -D__APPLE_CPP__ ${IGNORED_WARNINGS}")

SET(CMAKE_ASM_FLAGS "-m${BITS}")
SET(CMAKE_ASM-ATT_FLAGS "-m${BITS}")

message(STATUS "Building for ${CMAKE_SYSTEM_PROCESSOR}, CFLAGS: ${CMAKE_C_FLAGS}, libdir: ${CMAKE_INSTALL_LIBDIR}")

# --as-needed breaks Darling
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed")

if (BITS EQUAL 32)
	# Prevent building Darling executables (e.g. Bash)
	set(DARLING_NO_EXECUTABLES ON)

	SET(SUFFIX "32")
	add_definitions(-DTARGET_CPU_X86=1 -D__LITTLE_ENDIAN__)
	set(DARLING_NO_EXECUTABLES ON)

else()

	set(SUFFIX "64")
	add_definitions(-DTARGET_CPU_X86_64=1 -D__LITTLE_ENDIAN__)

endif()

enable_language(ASM-ATT)

option(FRAMEWORK_COREAUDIO "Enable audio-related functionality" OFF)
option(FRAMEWORK_APPKIT "Enable AppKit development code" OFF)

add_subdirectory(src)

if (NOT DARLING_NO_EXECUTABLES)
	install(FILES etc/dylib.conf etc/version.conf
		DESTINATION libexec/darling/etc/darling)
endif (NOT DARLING_NO_EXECUTABLES)


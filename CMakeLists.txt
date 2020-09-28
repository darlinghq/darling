SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR "x86-64")

# This allows setting the compiler with -DCMAKE_C_COMPILER when configuring.
if (NOT DEFINED CMAKE_C_COMPILER)
	find_program(CMAKE_C_COMPILER NAMES
		"clang"
		"clang-4.0"
		"clang-6.0"
		"clang-7"
		"clang-9"
		"clang-10"
)
endif(NOT DEFINED CMAKE_C_COMPILER)

if (NOT DEFINED CMAKE_CXX_COMPILER)
	find_program(CMAKE_CXX_COMPILER NAMES
		"clang++"
		"clang++-4.0"
		"clang++-6.0"
		"clang++-7"
		"clang++-9"
		"clang++-10"
)
endif(NOT DEFINED CMAKE_CXX_COMPILER)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

SET(CMAKE_SKIP_RPATH TRUE)

# technically ignored by CMake when building on non-Apple platforms, but it's already a standard variable for the
# SDK deployment target, so we'll just use it and then add it to the compiler flags manually
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "The version of macOS we're simulating")

project(darling)

cmake_minimum_required(VERSION 3.10)
enable_language(ASM)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(InstallSymlink)
include(MacroEnsureOutOfSourceBuild)
include(dsym)
include(xcproj)
include(architecture)

MACRO_ENSURE_OUT_OF_SOURCE_BUILD()

set(DARLING_TOP_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set(DARLING_NO_EXECUTABLES OFF)
set(CMAKE_C_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")

if (${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "i686|i386")
	message(FATAL_ERROR "This software can only be built on x86-64 systems")
endif (${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "i686|i386")

SET(IGNORED_WARNINGS "-Wno-nullability-completeness -Wno-deprecated-declarations -Wno-availability")

if (${CMAKE_C_COMPILER_ID} STREQUAL "Clang" AND NOT ${CMAKE_C_COMPILER_VERSION} VERSION_LESS "3.9")
	SET(IGNORED_WARNINGS "${IGNORED_WARNINGS} -Wno-expansion-to-defined")
endif (${CMAKE_C_COMPILER_ID} STREQUAL "Clang" AND NOT ${CMAKE_C_COMPILER_VERSION} VERSION_LESS "3.9")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${IGNORED_WARNINGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${IGNORED_WARNINGS}")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")

# set macOS deployment version
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
set(CMAKE_OBJC_FLAGS "${CMAKE_OBJC_FLAGS} -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")

# prevent object filename conflicts for two source files that differ only in extension (e.g. `object.c` and `object.m`)
set(CMAKE_C_OUTPUT_EXTENSION ".c.o")
set(CMAKE_CXX_OUTPUT_EXTENSION ".cpp.o")
set(CMAKE_OBJC_OUTPUT_EXTENSION ".m.o")

if(CMAKE_POSITION_INDEPENDENT_CODE)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")
endif(CMAKE_POSITION_INDEPENDENT_CODE)

enable_language(ASM-ATT)

option(FULL_BUILD "Include large items in the build" ON)
option(TARGET_i386 "Enable i386 slices" ON)
option(TARGET_x86_64 "Enable x86_64 slices" ON)
option(DEBIAN_PACKAGING "Packaging for Debian" OFF)
option(ENABLE_TESTS "Install in-prefix unit tests" OFF)
option(ADDITIONAL_PACKAGES "Include additional pacakges not included by default in a standard macOS installation" OFF)

FindDsymutil()
find_package(Setcap REQUIRED)

# Missing CMakeLists.txt must trigger an error
cmake_policy(SET CMP0014 NEW)

generate_architecture()

add_subdirectory(src)

install(DIRECTORY DESTINATION libexec/darling/private)
install(DIRECTORY DESTINATION libexec/darling/private/etc)
install(DIRECTORY DESTINATION libexec/darling/private/var)
install(DIRECTORY DESTINATION libexec/darling/private/tmp)
InstallSymlink(private/etc ${CMAKE_INSTALL_PREFIX}/libexec/darling/etc)
InstallSymlink(private/var ${CMAKE_INSTALL_PREFIX}/libexec/darling/var)

install(FILES etc/resolv.conf
	DESTINATION libexec/darling/etc)
InstallSymlink(/Volumes/SystemRoot/etc/machine-id ${CMAKE_INSTALL_PREFIX}/libexec/darling/etc/machine-id)
InstallSymlink(/Volumes/SystemRoot/etc/nsswitch.conf ${CMAKE_INSTALL_PREFIX}/libexec/darling/etc/nsswitch.conf)

install(DIRECTORY DESTINATION libexec/darling/Volumes)
install(DIRECTORY DESTINATION libexec/darling/Volumes/SystemRoot)
InstallSymlink(/ ${CMAKE_INSTALL_PREFIX}/libexec/darling/Volumes/DarlingEmulatedDrive)

install(DIRECTORY DESTINATION libexec/darling/proc)

install(DIRECTORY DESTINATION libexec/darling/var/root)
install(DIRECTORY DESTINATION libexec/darling/var/run)
InstallSymlink(/dev/log ${CMAKE_INSTALL_PREFIX}/libexec/darling/var/run/syslog)

install(DIRECTORY DESTINATION libexec/darling/usr)
install(DIRECTORY DESTINATION libexec/darling/usr/local)
install(DIRECTORY DESTINATION libexec/darling/usr/local/share)

InstallSymlink(/Volumes/SystemRoot/dev ${CMAKE_INSTALL_PREFIX}/libexec/darling/dev)
InstallSymlink(private/tmp ${CMAKE_INSTALL_PREFIX}/libexec/darling/tmp)

InstallSymlink(/proc/self/mounts ${CMAKE_INSTALL_PREFIX}/libexec/darling/etc/mtab)
InstallSymlink(/Volumes/SystemRoot/etc/passwd ${CMAKE_INSTALL_PREFIX}/libexec/darling/etc/passwd)
InstallSymlink(/Volumes/SystemRoot/etc/group ${CMAKE_INSTALL_PREFIX}/libexec/darling/etc/group)
InstallSymlink(/Volumes/SystemRoot/etc/localtime ${CMAKE_INSTALL_PREFIX}/libexec/darling/etc/localtime)

InstallSymlink(/Volumes/SystemRoot/usr/share/zoneinfo ${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/share/zoneinfo)

if(NOT DEBIAN_PACKAGING)
	install(CODE "execute_process(COMMAND bash ${DARLING_TOP_DIRECTORY}/tools/shutdown-user.sh)")
endif(NOT DEBIAN_PACKAGING)

add_custom_target(uninstall
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/uninstall
	COMMENT "Uninstall Darling and kernel module")

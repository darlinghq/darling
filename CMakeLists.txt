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
		"clang-11"
		"clang-12"
		"clang-13"
		"clang-14"
		"clang-15"
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
		"clang++-11"
		"clang++-12"
		"clang++-13"
		"clang++-14"
		"clang++-15"
)
endif(NOT DEFINED CMAKE_CXX_COMPILER)

option(DARLING_NO_CCACHE "Disable ccache usage" OFF)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM AND NOT DARLING_NO_CCACHE)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

SET(CMAKE_SKIP_RPATH TRUE)

# technically ignored by CMake when building on non-Apple platforms, but it's already a standard variable for the
# SDK deployment target, so we'll just use it and then add it to the compiler flags manually
set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0 CACHE STRING "The version of macOS we're simulating")

project(darling)

cmake_minimum_required(VERSION 3.10)
enable_language(ASM)

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "core")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(InstallSymlink)
include(MacroEnsureOutOfSourceBuild)
include(dsym)
include(xcproj)
include(architecture)
include(create_symlink)

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

if(${CMAKE_C_COMPILER_ID} STREQUAL "Clang" AND ${CMAKE_C_COMPILER_VERSION} VERSION_GREATER_EQUAL "11")
	# newer Clang chokes on the idiomatic way to use CF_ENUM; e.g. like this:
	#     typedef CF_ENUM(int, MyEnum) {
	#       MY_ENUM_THING,
	#       MY_ENUM_OTHER_THING,
	#       # etc...
	#     }
	set(IGNORED_WARNINGS "${IGNORED_WARNINGS} -Wno-elaborated-enum-base -Wno-undef-prefix")
	set(ASM_IGNORED_WARNINGS "${ASM_IGNORED_WARNINGS} -Wno-undef-prefix")
endif()

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${IGNORED_WARNINGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${IGNORED_WARNINGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${ASM_IGNORED_WARNINGS}")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")

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

option(TARGET_i386 "Enable i386 slices" ON)
option(TARGET_x86_64 "Enable x86_64 slices" ON)
option(DEBIAN_PACKAGING "Packaging for Debian" OFF)
option(ENABLE_TESTS "Install in-prefix unit tests" OFF)
option(REGENERATE_SDK "Regenerate Header Files For Open Source SDK" OFF)

if (DEBIAN_PACKAGING)
	set(COMPONENTS_DEFAULT "all")
else()
	set(COMPONENTS_DEFAULT "stock")
endif()

set(COMPONENTS "${COMPONENTS_DEFAULT}" CACHE STRING "Choose which components of Darling to build")
include(darling_parse_components)
darling_parse_components("${COMPONENTS}")

set(COMPILE_PY2_BYTECODE AUTO CACHE STRING "Pre-compile bytecode for Python 2 packages")
set_property(CACHE COMPILE_PY2_BYTECODE PROPERTY STRINGS AUTO ON OFF)

string(TOLOWER "${COMPILE_PY2_BYTECODE}" COMPILE_PY2_BYTECODE)

if (COMPILE_PY2_BYTECODE STREQUAL "auto")
	find_package(Python2 QUIET COMPONENTS Interpreter)

	if (Python2_Interpreter_FOUND)
		set(COMPILE_PY2_BYTECODE ON)
		message("Found Python 2; enabling pre-compilation of Python bytecode")
	else()
		set(COMPILE_PY2_BYTECODE OFF)
		message("Python 2 not available; bytecode compilation is disabled")
	endif (Python2_Interpreter_FOUND)
endif (COMPILE_PY2_BYTECODE STREQUAL "auto")

if (COMPILE_PY2_BYTECODE)
	find_package(Python2 REQUIRED COMPONENTS Interpreter)
endif (COMPILE_PY2_BYTECODE)

set(ENABLE_METAL AUTO CACHE STRING "Build Darling with Metal support")
set_property(CACHE ENABLE_METAL PROPERTY STRINGS AUTO ON OFF)

string(TOLOWER "${ENABLE_METAL}" BUILD_METAL)

if(BUILD_METAL STREQUAL "auto")
	# check if Vulkan and LLVM are available
	find_program(LLVM_CONFIG_PROGRAM llvm-config)
	find_package(Vulkan)

	if(LLVM_CONFIG_PROGRAM AND Vulkan_FOUND)
		set(BUILD_METAL ON)
		message("Found required libraries; building with Metal support")
	else()
		set(BUILD_METAL OFF)
		message("Did not find required libraries (Vulkan and LLVM); building without Metal support")
	endif()
endif()

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
	DESTINATION libexec/darling/private/etc)
InstallSymlink(/Volumes/SystemRoot/etc/machine-id ${CMAKE_INSTALL_PREFIX}/libexec/darling/private/etc/machine-id)
InstallSymlink(/Volumes/SystemRoot/etc/nsswitch.conf ${CMAKE_INSTALL_PREFIX}/libexec/darling/private/etc/nsswitch.conf)

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

InstallSymlink(/proc/self/mounts ${CMAKE_INSTALL_PREFIX}/libexec/darling/private/etc/mtab)
#InstallSymlink(/Volumes/SystemRoot/etc/passwd ${CMAKE_INSTALL_PREFIX}/libexec/darling/private/etc/passwd)
#InstallSymlink(/Volumes/SystemRoot/etc/group ${CMAKE_INSTALL_PREFIX}/libexec/darling/private/etc/group)
InstallSymlink(/Volumes/SystemRoot/etc/localtime ${CMAKE_INSTALL_PREFIX}/libexec/darling/private/etc/localtime)

InstallSymlink(/Volumes/SystemRoot/usr/share/zoneinfo ${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/share/zoneinfo)

install(DIRECTORY DESTINATION libexec/darling/Library/Preferences)

if(NOT DEBIAN_PACKAGING)
	install(CODE "execute_process(COMMAND bash ${DARLING_TOP_DIRECTORY}/tools/shutdown-user.sh)")
endif(NOT DEBIAN_PACKAGING)

add_custom_target(uninstall
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tools/uninstall
	COMMENT "Uninstall Darling and kernel module")

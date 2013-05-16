project(darling)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

if (SUFFIX)
	set(MULTILIB TRUE)
endif (SUFFIX)
if(CMAKE_SIZEOF_VOID_P EQUAL 4 OR SUFFIX STREQUAL "32")
	set(BITS 32)
else(CMAKE_SIZEOF_VOID_P EQUAL 4 OR SUFFIX STREQUAL "32")
	set(BITS 64)
endif(CMAKE_SIZEOF_VOID_P EQUAL 4 OR SUFFIX STREQUAL "32")

if (NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected, default to Debug")
	set(CMAKE_BUILD_TYPE "Debug")
endif(NOT CMAKE_BUILD_TYPE)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(DEBUG TRUE)
endif (${CMAKE_BUILD_TYPE} STREQUAL "Debug")

message(STATUS "This is a ${BITS}-bit build")

include(FindPkgConfig)
include(FindGnustepBase)
include(FindGnustepCorebase)
include(FindGnustepGui)
include(FindLibkqueue)
include(FindLibobjc2)
include(FindOpal)

pkg_check_modules(libbsd REQUIRED "libbsd >= 0.4.2")
pkg_check_modules(libudev REQUIRED "libudev")
#set(CMAKE_ASM_NASM_COMPILER "nasm")

find_package(GnustepBase REQUIRED)
find_package(GnustepCorebase REQUIRED)
find_package(GnustepGui REQUIRED)
find_package(Libkqueue REQUIRED)
find_package(Libobjc2 REQUIRED)
#find_package(Opal REQUIRED)

if (SUFFIX STREQUAL "32")
	set(CMAKE_CPP_FLAGS "${CMAKE_CPP_FLAGS} -m32")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
	set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf")
	SET(CMAKE_C_SIZEOF_DATA_PTR 4)
endif(SUFFIX STREQUAL "32")

enable_language(ASM-ATT)
enable_language(ASM_NASM)

if (DEBUG)
	ADD_DEFINITIONS(-ggdb -DDEBUG)
endif (DEBUG)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/xnu)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/xnu/bsd)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/darwin)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/libmach-o)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/util)
include_directories(${CMAKE_BINARY_DIR}) # for config.h

include_directories(${LIBGNUSTEPBASE_INCLUDE_DIR})
include_directories(${LIBGNUSTEPCOREBASE_INCLUDE_DIR})
include_directories(${LIBKQUEUE_INCLUDE_DIR})
include_directories(${LIBOBJC2_INCLUDE_DIR})
include_directories(${LIBOPAL_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

configure_file(config.h.in config.h)

# If building on a 64bit system, only install these for 64bits
if (NOT DEFINED SUFFIX OR SUFFIX STREQUAL "64")
	install(FILES etc/dylib.conf DESTINATION /etc/darling)
	add_subdirectory(src/motool)
	add_subdirectory(src/macbinary)
	add_subdirectory(src/libmacarchive)
#	add_subdirectory(src/crash)
endif(NOT DEFINED SUFFIX OR SUFFIX STREQUAL "64")

add_subdirectory(src/util)
add_subdirectory(src/libmach-o)
add_subdirectory(src/dyld)
add_subdirectory(src/libSystem)
add_subdirectory(src/Cocoa)
add_subdirectory(src/libobjcdarwin)
add_subdirectory(src/CoreFoundation)
add_subdirectory(src/libncurses)
add_subdirectory(src/CoreSecurity)
add_subdirectory(src/CoreServices)
add_subdirectory(src/CoreGraphics)
add_subdirectory(src/IOKit)
#add_subdirectory(src/libcrashhandler)
add_subdirectory(src/thin)
add_subdirectory(src/libstdc++darwin)

add_dependencies(dyld${SUFFIX} mach-o)
add_dependencies(Cocoa crashhandler)


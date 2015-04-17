project(darling-libsystem_c)

cmake_minimum_required(VERSION 2.4.0)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
	cmake_policy(SET CMP0018 NEW)
endif(COMMAND cmake_policy)


# -DHAVE_STDINT_H=1 -I libc/include -I platform-include/ -I kernel-include/ -I
# libc/locale/ -I libc/locale/FreeBSD/ -I libc/stdtime/FreeBSD -I
# platform-include/pthread/ -I libc/fbsdcompat -I libc/pthreads -nostdinc
# -DTARGET_CPU_X86_64=1 -DTARGET_OS_MAC=1

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__=1080)
add_definitions(-D__LIBC__ -D__DARWIN_UNIX03=1 -D__DARWIN_64_BIT_INO_T=1 -D__DARWIN_NON_CANCELABLE=1 -D__DARWIN_VERS_1050=1)
#add_definitions("'-D__weak_reference(sym,alias)=__asm__(\".weak \" \\\#alias );__asm__(\".equ \" \\\#alias \", \" \\\#sym)'")
#add_definitions("'-D__weak_reference(sym,alias)='")
add_definitions("'-D__warn_references(name,warning)='")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w ")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/weak_reference.h")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,--unresolved-symbols=ignore-all -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/darwin.map")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/fbsdcompat")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/pthreads")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/darwin")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../libmalloc/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/locale")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/gen")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/locale/FreeBSD")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/NetBSD")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/stdtime/FreeBSD")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../external/libdispatch")

add_subdirectory(db) # produces libc-db
add_subdirectory(compat-43) # produces libc-compat
add_subdirectory(emulated) # produces libc-emulated
add_subdirectory(gdtoa) # produces libc-gdtoa
add_subdirectory(gen)
add_subdirectory(gmon)
add_subdirectory(uuid)
add_subdirectory(util)
add_subdirectory(string)
add_subdirectory(stdtime)
add_subdirectory(sys)
add_subdirectory(stdlib)
add_subdirectory(stdio)
add_subdirectory(secure)
add_subdirectory(regex)
add_subdirectory(pthreads)
add_subdirectory(posix1e)
add_subdirectory(nls)
add_subdirectory(net)
add_subdirectory(locale)

# The following generate libc-hwplat
if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
	add_subdirectory(i386)
endif()

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
	add_subdirectory(x86_64)
endif()

add_subdirectory(darwin)

add_library(system_c SHARED
	$<TARGET_OBJECTS:libc-db>
	$<TARGET_OBJECTS:libc-compat>
	$<TARGET_OBJECTS:libc-compat_cancelable>
	$<TARGET_OBJECTS:libc-emulated>
	$<TARGET_OBJECTS:libc-gdtoa>
	$<TARGET_OBJECTS:libc-gen>
	$<TARGET_OBJECTS:libc-gen_cancelable>
	$<TARGET_OBJECTS:libc-gmon>
	$<TARGET_OBJECTS:libc-uuid>
	$<TARGET_OBJECTS:libc-util>
	$<TARGET_OBJECTS:libc-string>
	$<TARGET_OBJECTS:libc-stdtime>
	$<TARGET_OBJECTS:libc-sys>
	$<TARGET_OBJECTS:libc-stdlib>
	$<TARGET_OBJECTS:libc-stdlib_cancelable>
	$<TARGET_OBJECTS:libc-stdio>
	$<TARGET_OBJECTS:libc-secure>
	$<TARGET_OBJECTS:libc-regex>
	$<TARGET_OBJECTS:libc-pthreads>
	$<TARGET_OBJECTS:libc-pthreads_cancelable>
	$<TARGET_OBJECTS:libc-posix1e>
	$<TARGET_OBJECTS:libc-nls>
	$<TARGET_OBJECTS:libc-net>
	$<TARGET_OBJECTS:libc-locale>
	$<TARGET_OBJECTS:libc-hwplat>
	$<TARGET_OBJECTS:libc-darwin>)

# TODO: see ./xcodescripts/variants.xcconfig

#SET_TARGET_PROPERTIES(system_c PROPERTIES LINKER_LANGUAGE C)

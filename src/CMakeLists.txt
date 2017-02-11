project(darling-src)

cmake_minimum_required(VERSION 2.4.0)
cmake_policy(SET CMP0005 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(mig)

add_definitions(-D_DARWIN_C_SOURCE -D_POSIX_C_SOURCE -DDARLING)
set(DARLING TRUE)

add_definitions(-D__APPLE__ -D__DYNAMIC__)

execute_process(
	COMMAND git rev-parse --abbrev-ref HEAD
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_BRANCH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
	COMMAND git log -1 --format=%h
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_COMMIT_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(darling-config.h.in darling-config.h)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(bootstrap_cmds)
add_subdirectory(kernel)
add_subdirectory(util)
add_subdirectory(libmach-o)
add_subdirectory(libdyld)
add_subdirectory(dyld)

if (NOT DARLING_NO_EXECUTABLES)
	add_subdirectory(motool)
	add_subdirectory(external/darling-dmg)
endif (NOT DARLING_NO_EXECUTABLES)

if (FRAMEWORK_COREAUDIO)
	add_subdirectory(AudioToolbox)
	add_subdirectory(AudioUnit)
endif (FRAMEWORK_COREAUDIO)
add_subdirectory(external/AppKit)
add_subdirectory(IOKit)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libc/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libm/Source")
include_directories("${DARLING_TOP_DIRECTORY}/platform-include")
include_directories("${DARLING_TOP_DIRECTORY}/platform-include/pthread")
include_directories("${DARLING_TOP_DIRECTORY}/kernel-include")
include_directories("${DARLING_TOP_DIRECTORY}/compiler-include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/duct/include")

add_subdirectory(libc)
add_subdirectory(libm)
add_subdirectory(libgcc)
add_subdirectory(copyfile)
add_subdirectory(libinfo)
add_subdirectory(quarantine)
add_subdirectory(libmalloc)
add_subdirectory(libunwind)
add_subdirectory(libsystem)
add_subdirectory(libsystem_coreservices)
add_subdirectory(libutil)
add_subdirectory(libnotify)
add_subdirectory(libremovefile)
add_subdirectory(launchd/liblaunch)
add_subdirectory(keymgr)
add_subdirectory(ncurses)
add_subdirectory(libiconv)
add_subdirectory(duct/src)
add_subdirectory(libresolv)
add_subdirectory(libstdcxx)
add_subdirectory(libffi)
add_subdirectory(external/libobjc2)
add_subdirectory(external/libkqueue)
add_subdirectory(external/libdispatch)
add_subdirectory(external/zlib)
add_subdirectory(external/bzip2)
add_subdirectory(external/libxml2)
add_subdirectory(external/libxslt)
add_subdirectory(external/icu/icuSources)
add_subdirectory(external/corefoundation)
add_subdirectory(external/openssl/src)
add_subdirectory(external/foundation)
add_subdirectory(external/curl)
add_subdirectory(external/liblzma)
add_subdirectory(external/cfnetwork)
add_subdirectory(external/pcre)
add_subdirectory(external/sqlite)
add_subdirectory(external/openpam)
add_subdirectory(CFF)
add_subdirectory(SystemConfiguration)
add_subdirectory(CoreServices)
add_subdirectory(ApplicationServices)
add_subdirectory(VideoDecodeAcceleration)
add_subdirectory(DiskArbitration)
add_subdirectory(DebugSymbols)
add_subdirectory(xar)
add_subdirectory(libedit)
add_subdirectory(external/compiler-rt/lib/builtins)
add_subdirectory(CommonCrypto)
add_subdirectory(csu)
add_subdirectory(external/python/2.6/Python-2.6.9)
add_subdirectory(external/python/2.7/Python-2.7.10)
add_subdirectory(external/ruby)
add_subdirectory(external/expat)
add_subdirectory(external/libauto)
add_subdirectory(external/libarchive/libarchive)
add_subdirectory(external/apr)
add_subdirectory(external/corecrypto)
add_subdirectory(external/security) # work in progress
add_subdirectory(sandbox)
add_subdirectory(Cocoa)
add_subdirectory(external/file/file)
add_subdirectory(external/libxpc)
add_subdirectory(external/openssl_certificates)

if (NOT DARLING_NO_EXECUTABLES)
	add_subdirectory(external/shell_cmds)
	add_subdirectory(external/file_cmds)
	add_subdirectory(external/text_cmds)
	add_subdirectory(external/adv_cmds)
	add_subdirectory(external/network_cmds)
	add_subdirectory(external/bash)
	add_subdirectory(external/less)
	add_subdirectory(external/grep)
	add_subdirectory(external/awk)
	add_subdirectory(external/groff)
	add_subdirectory(external/nano)
	add_subdirectory(external/man)
	add_subdirectory(external/bc)
	add_subdirectory(external/zip/zip/zip30)
	add_subdirectory(external/zip/unzip/unzip-5.52)
	add_subdirectory(tools)
	add_subdirectory(external/installer)
	add_subdirectory(external/gnutar/gnutar)
	add_subdirectory(external/gpatch)
	add_subdirectory(external/gnudiff)
	add_subdirectory(launchd/src)
	add_subdirectory(launchd/support)
	add_subdirectory(external/openssh)
	add_subdirectory(external/top)
	add_subdirectory(external/perl)
endif (NOT DARLING_NO_EXECUTABLES)

######################
# libc++ & libc++abi #
######################
set(CMAKE_CXX_FLAGS_SAVED ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -nostdinc -nostdinc++")
set(CMAKE_SHARED_LINKER_FLAGS "-nostdlib")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/external/libcxx/include")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libc/locale)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libc/locale/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libc/stdtime/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libc/pthreads)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libc/include/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/libcxxabi/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libunwind/include)
set(LIBCXXABI_LIBCXX_PATH "{CMAKE_CURRENT_SOURCE_DIR}/external/libcxx")
set(LIBCXXABI_LIBCXX_INCLUDES "-I${CMAKE_CURRENT_SOURCE_DIR}/external/libcxx/include")

add_subdirectory(external/libcxxabi)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVED} -std=c++11 -msse -msse2 -msse3 -nostdinc -nostdinc++ -include ${CMAKE_CURRENT_SOURCE_DIR}/libc/include/xlocale.h")
set(LIBCXX_HAS_STDCXX11_FLAG TRUE)
add_definitions(-D_USE_EXTENDED_LOCALES_)
add_subdirectory(external/libcxx)

#######################
# Linux kernel module #
#######################

add_custom_target(lkm
	COMMAND gmake
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lkm")

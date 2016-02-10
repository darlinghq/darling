project(libxar)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


# Reason for -U__APPLE__:
# We need CommonCrypro/corecrypto for some of the extra stuff
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w -U__APPLE__") 
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,--unresolved-symbols=ignore-all -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${DARLING_TOP_DIRECTORY}/platform-include)
#include_directories(${DARLING_TOP_DIRECTORY}/src/libc/include)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/zlib)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/bzip2)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/libxml2/include)
include_directories(${CMAKE_BINARY_DIR}/src/external/libxml2/include)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/icu/icuSources/common)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/openssl/src/include)

set(xar_SRCS
	lib/archive.c
	lib/arcmod.c
	lib/b64.c
	lib/bzxar.c
	lib/darwinattr.c
	lib/data.c
	lib/ea.c
	lib/err.c
	lib/ext2.c
	lib/fbsdattr.c
	lib/filetree.c
	lib/hash.c
	lib/io.c
	lib/linuxattr.c
	lib/lzmaxar.c
	lib/macho.c
	lib/script.c
	lib/signature.c
	lib/stat.c
	lib/subdoc.c
	lib/util.c
	lib/zxar.c
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(xar SHARED ${xar_SRCS})
target_link_libraries(xar system z bz2 xml2 ssl098 crypto098)

install(TARGETS xar DESTINATION ${CMAKE_INSTALL_LIBDIR}/darling)

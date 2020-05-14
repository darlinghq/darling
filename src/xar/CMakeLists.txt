project(libxar)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


# Reason for -U__APPLE__:
# We need CommonCrypro/corecrypto for some of the extra stuff
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w -U__APPLE__") 
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
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

set(DYLIB_INSTALL_NAME "/usr/lib/libxar.1.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
add_darling_library(xar SHARED ${xar_SRCS})
set_target_properties(xar PROPERTIES OUTPUT_NAME "xar.1")
target_link_libraries(xar system z bz2 xml2 ssl098 crypto098)
make_fat(xar)

add_darling_executable(xarexe src/xar.c)
set_target_properties(xarexe PROPERTIES OUTPUT_NAME "xar")
target_link_libraries(xarexe system xar)

install(TARGETS xar DESTINATION libexec/darling/usr/lib)
install(TARGETS xarexe DESTINATION libexec/darling/usr/bin)
install(FILES src/xar.1 DESTINATION libexec/darling/usr/share/man/man1)
InstallSymlink("libxar.1.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libxar.dylib")

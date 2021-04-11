project(libxar)

cmake_minimum_required(VERSION 3.10)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w") 
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/xar/include
	${CMAKE_CURRENT_SOURCE_DIR}/xar/lib
)

set(xar_SRCS
	xar/lib/archive.c
	xar/lib/arcmod.c
	xar/lib/b64.c
	xar/lib/bzxar.c
	xar/lib/darwinattr.c
	xar/lib/data.c
	xar/lib/ea.c
	xar/lib/err.c
	xar/lib/ext2.c
	xar/lib/fbsdattr.c
	xar/lib/filetree.c
	xar/lib/hash.c
	xar/lib/io.c
	xar/lib/linuxattr.c
	xar/lib/lzmaxar.c
	xar/lib/script.c
	xar/lib/signature.c
	xar/lib/stat.c
	xar/lib/subdoc.c
	xar/lib/util.c
	xar/lib/zxar.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/libxar.1.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")
add_darling_library(xar SHARED ${xar_SRCS})
set_target_properties(xar PROPERTIES OUTPUT_NAME "xar.1")
target_link_libraries(xar system z bz2 xml2)
make_fat(xar)

add_darling_executable(xarexe xar/src/xar.c)
set_target_properties(xarexe PROPERTIES OUTPUT_NAME "xar")
target_link_libraries(xarexe system xar)

install(TARGETS xar DESTINATION libexec/darling/usr/lib)
install(TARGETS xarexe DESTINATION libexec/darling/usr/bin)
install(FILES xar/src/xar.1 DESTINATION libexec/darling/usr/share/man/man1)
InstallSymlink("libxar.1.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libxar.dylib")

project(libiconv)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__=1080)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w -include ${CMAKE_CURRENT_BINARY_DIR}/include.h -O0 -ggdb")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

add_definitions(-DHAVE_CONFIG_H)
configure_file(include.h.in include.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libcharset/include)

set(iconv_SRCS
	src/allocsa.c
	src/canonicalize.c
	src/error.c
	src/memmove.c
	src/progname.c
	src/progreloc.c
	src/readlink.c
	src/relocatable.c
	#src/relocwrapper.c
	src/setenv.c
	src/strerror.c
	src/unsetenv.c
	src/width.c
	src/xmalloc.c
	src/xreadlink.c
	src/xstrdup.c

	#lib/genaliases2.c
	#lib/genaliases.c
	#lib/genflags.c
	#lib/gentranslit.c
	lib/iconv.c
	#lib/relocatable.c
	
	libcharset/src/localcharset.c
	#libcharset/src/relocatable.c
)

set(DYLIB_COMPAT_VERSION "7.0.0")
set(DYLIB_CURRENT_VERSION "7.0.0")
set(DYLIB_INSTALL_NAME "/usr/lib/libiconv.2.dylib")
add_darling_library(iconv SHARED ${iconv_SRCS})
make_fat(iconv)
target_link_libraries(iconv system)
set_target_properties(iconv PROPERTIES OUTPUT_NAME "iconv.2")
InstallSymlink("libiconv.2.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libiconv.dylib")

install(TARGETS iconv DESTINATION libexec/darling/usr/lib)
install(FILES charset.alias DESTINATION libexec/darling/usr/lib)

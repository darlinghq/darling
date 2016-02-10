project(libiconv)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


if(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")
	set(BITS 32)
	add_definitions(-DTARGET_CPU_X86=1)
else(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")
	set(BITS 64)
	add_definitions(-DTARGET_CPU_X86_64=1)
endif(CMAKE_SIZEOF_VOID_P EQUAL 4 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib32")

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)
add_definitions(-D__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__=1080)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w -include ${CMAKE_CURRENT_BINARY_DIR}/include.h -O0 -ggdb")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

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

add_library(iconv SHARED ${iconv_SRCS}
)
target_link_libraries(iconv system)

install(TARGETS iconv DESTINATION ${CMAKE_INSTALL_LIBDIR}/darling)
install(FILES charset.alias DESTINATION ${CMAKE_INSTALL_LIBDIR}/darling)

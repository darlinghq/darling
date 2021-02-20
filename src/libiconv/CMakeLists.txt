project(libiconv)

cmake_minimum_required(VERSION 3.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_definitions(-DHAVE_STDINT_H=1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w -O0 -ggdb")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

add_definitions(
	-DHAVE_CONFIG_H
	-DBUILDING_DLL
	-DENABLE_RELOCATABLE
	-DIN_LIBRARY
	-DNO_XMALLOC
	-DPIC
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libiconv)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libiconv/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libiconv/srclib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libiconv/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libiconv/libcharset/include)

set(iconv_SRCS
	libiconv/lib/iconv.c
	libiconv/libcharset/lib/relocatable.c
)

set(charset_SRCS
	libiconv/libcharset/lib/localcharset.c
	libiconv/libcharset/lib/relocatable.c
)

set(DYLIB_COMPAT_VERSION "7.0.0")
set(DYLIB_CURRENT_VERSION "7.0.0")
set(DYLIB_INSTALL_NAME "/usr/lib/libiconv.2.dylib")
add_darling_library(iconv SHARED ${iconv_SRCS})
make_fat(iconv)
target_link_libraries(iconv PRIVATE system)
set_target_properties(iconv PROPERTIES OUTPUT_NAME "iconv.2")
InstallSymlink("libiconv.2.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libiconv.dylib")

install(TARGETS iconv DESTINATION libexec/darling/usr/lib)

target_compile_definitions(iconv PRIVATE
	BUILDING_LIBICONV
	relocate=libiconv_relocate
	set_relocation_prefix=libiconv_set_relocation_prefix
)

set_property(TARGET iconv APPEND_STRING PROPERTY LINK_FLAGS
	" -sub_library libcharset"
)

set(DYLIB_COMPAT_VERSION "2.0.0")
set(DYLIB_CURRENT_VERSION "2.0.0")
set(DYLIB_INSTALL_NAME "/usr/lib/libcharset.1.dylib")
add_darling_library(charset SHARED ${charset_SRCS})
make_fat(charset)
target_link_libraries(charset PRIVATE system)
set_target_properties(charset PROPERTIES OUTPUT_NAME "charset.1")
InstallSymlink("libcharset.1.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libcharset.dylib")

install(TARGETS charset DESTINATION libexec/darling/usr/lib)

target_compile_definitions(charset PRIVATE
	BUILDING_LIBCHARSET
	relocate=libcharset_relocate
	set_relocation_prefix=libcharset_set_relocation_prefix
)

target_link_libraries(iconv PRIVATE charset)

install(FILES libiconv/lib/charset.alias DESTINATION libexec/darling/usr/lib)

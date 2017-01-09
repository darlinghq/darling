project(darling-libstdcxx)

cmake_minimum_required(VERSION 2.4.0)
enable_language(C ASM)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -msse2 -msse3 -w -nostdinc -nostdinc++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__DARWIN_UNIX03 -fPIC -w -include ${CMAKE_CURRENT_SOURCE_DIR}/build.h")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/c++/4.2.1)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/c++/4.2.1/backward)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../kernel-include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libc/locale)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libc/locale/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libc/stdtime/FreeBSD)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libc/pthreads)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../external/libcxxabi/include)

add_definitions(-D__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__=1080)
add_definitions(-DBUILDING_LIBSTDCXX -D_LIBCPPABI_VERSION=1)

set(libstdcxx_sources
	src/allocator-inst.cc
	src/atomicity.cc
	src/basic_file.cc
	src/bitmap_allocator.cc
	src/c++locale.cc
	src/codecvt.cc
	src/codecvt_members.cc
	src/collate_members.cc
	src/compatibility.cc
	src/compatibility-ldbl.cc
	src/complex_io.cc
	src/concept-inst.cc
	src/ctype.cc
	src/ctype_members.cc
	src/debug.cc
	#src/debug_list.cc # Doesn't build, text relocation?!
	src/del_op.cc
	src/del_opnt.cc
	src/del_opv.cc
	src/del_opvnt.cc
	src/eh_exception.cc
	src/eh_terminate.cc
	src/ext-inst.cc
	src/fstream-inst.cc
	src/functexcept.cc
	src/globals_io.cc
	src/ios.cc
	src/ios_failure.cc
	src/ios_init.cc
	src/ios-inst.cc
	src/ios_locale.cc
	src/iostream-inst.cc
	src/istream.cc
	src/istream-inst.cc
	src/limits.cc
	src/list.cc
	src/locale.cc
	src/locale_facets.cc
	src/locale_init.cc
	src/locale-inst.cc
	src/localename.cc
	src/messages_members.cc
	src/misc-inst.cc
	src/monetary_members.cc
	src/mt_allocator.cc
	src/new_handler.cc
	src/new_op.cc
	src/new_opnt.cc
	src/new_opv.cc
	src/new_opvnt.cc
	src/numeric_members.cc
	src/ostream-inst.cc
	src/pool_allocator.cc
	src/sstream-inst.cc
	src/stdexcept.cc
	src/streambuf.cc
	src/streambuf-inst.cc
	src/string-inst.cc
	src/strstream.cc
	src/time_members.cc
	src/tinfo.cc
	src/tree.cc
	src/valarray-inst.cc
	src/wlocale-inst.cc
	src/wstring-inst.cc
	src/dso_handle.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/libstdc++.6.dylib")
add_darling_library(stdcxx SHARED ${libstdcxx_sources})
target_link_libraries(stdcxx PRIVATE system cxxabi_shared)
set_target_properties(stdcxx PROPERTIES OUTPUT_NAME "stdc++.6")

install(TARGETS stdcxx DESTINATION ${CMAKE_INSTALL_LIBDIR}/darling)


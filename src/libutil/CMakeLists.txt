project(libutil)

cmake_minimum_required(VERSION 3.10)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

include(InstallSymlink)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdinc -nostdinc++ -D__DARWIN_UNIX03 -fPIC -w")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(util_SRCS
	ExtentManager.cpp
	getmntopts.c
	humanize_number.c
	pidfile.c
	realhostname.c
	reexec_to_match_kernel.c
	trimdomain.c
	tzlink.c
	wipefs.cpp
)

set(DYLIB_INSTALL_NAME "/usr/lib/libutil1.0.dylib")
set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_darling_library(util SHARED ${util_SRCS})
make_fat(util)
target_link_libraries(util PRIVATE system cxx)
set_target_properties(util PROPERTIES OUTPUT_NAME "util1.0")

install(TARGETS util DESTINATION libexec/darling/usr/lib)
install(FILES
	reexec_to_match_kernel.3
	realhostname.3
	humanize_number.3
	getmntopts.3
	realhostname_sa.3
	pidfile.3
	wipefs.3
	trimdomain.3
	DESTINATION
		libexec/darling/usr/share/man/man3
)
InstallSymlink("libutil1.0.dylib" "${CMAKE_INSTALL_PREFIX}/libexec/darling/usr/lib/libutil.dylib")

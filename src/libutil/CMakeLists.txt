project(libutil)

cmake_minimum_required(VERSION 2.4.0)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -D__DARWIN_UNIX03 -fPIC -w")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdinc -nostdinc++ -D__DARWIN_UNIX03 -fPIC -w")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,--unresolved-symbols=ignore-all -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${DARLING_TOP_DIRECTORY}/src/libc/include)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/libcxx/include)

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

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(util SHARED ${util_SRCS})
target_link_libraries(util system)

install(TARGETS util DESTINATION ${CMAKE_INSTALL_LIBDIR}/darling)

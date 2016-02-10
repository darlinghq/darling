project(copyfile)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdinc -fblocks")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")

include_directories(${DARLING_TOP_DIRECTORY}/platform-include)
include_directories(${DARLING_TOP_DIRECTORY}/src/libinfo/membership.subproj/)
include_directories(${DARLING_TOP_DIRECTORY}/src/external/libdispatch)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DTARGET_OS_MAC=1)
add_definitions(-DHAVE_STDINT_H=1)
add_definitions(-D__APPLE__ -D__DYNAMIC__)

set(copyfile_sources
	copyfile.c
	# xattr_flags.c # Doesn't build yet (XPC)
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(system_copyfile SHARED ${copyfile_sources})
target_link_libraries(system_copyfile system_kernel system_c)

install(TARGETS system_copyfile DESTINATION ${CMAKE_INSTALL_LIBDIR}/darling)

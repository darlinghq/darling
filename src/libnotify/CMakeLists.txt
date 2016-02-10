project(libnotify)

cmake_minimum_required(VERSION 2.4.0)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../external/libdispatch/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../external/libdispatch/private)
include_directories(${DARLING_TOP_DIRECTORY}/platform-include)
include_directories(${DARLING_TOP_DIRECTORY}/src/launchd/liblaunch)
include_directories(${DARLING_TOP_DIRECTORY}/src/libc/gen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks -nostdinc -include ${DARLING_TOP_DIRECTORY}/kernel-include/sys/fileport.h")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,--version-script=${DARLING_TOP_DIRECTORY}/darwin.map")

mig(notify_ipc.defs)

set(notify_sources
	libnotify.c
	notify_client.c
	table.c
	${CMAKE_CURRENT_BINARY_DIR}/notify_ipcUser.c
)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/darling")
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_library(system_notify SHARED ${notify_sources})
target_link_libraries(system_notify system_c system_kernel)

install(TARGETS system_notify DESTINATION ${CMAKE_INSTALL_LIBDIR}/darling)


project(liblaunch)

cmake_minimum_required(VERSION 2.4.0)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../external/libdispatch/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../external/libdispatch/private)
include_directories(${DARLING_TOP_DIRECTORY}/platform-include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/)
include_directories(${DARLING_TOP_DIRECTORY}/src/libc/darwin)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks -nostdinc -include ${DARLING_TOP_DIRECTORY}/kernel-include/sys/fileport.h -ggdb")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,-flat_namespace -Wl,-undefined,suppress")

mig(job.defs)
mig(helper.defs)

set(launch_sources
	libbootstrap.c
	liblaunch.c
	libvproc.c
	${CMAKE_CURRENT_BINARY_DIR}/jobUser.c
	${CMAKE_CURRENT_BINARY_DIR}/helperServer.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/system/liblaunch.dylib")
add_darling_library(launch SHARED ${launch_sources})
target_link_libraries(launch PRIVATE system_c system_kernel)

install(TARGETS launch DESTINATION ${CMAKE_INSTALL_LIBDIR}/darling)


project(libnotify)

cmake_minimum_required(VERSION 2.4.0)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../external/libdispatch/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../external/libdispatch/private)
include_directories(${DARLING_TOP_DIRECTORY}/platform-include)
include_directories(${DARLING_TOP_DIRECTORY}/src/launchd/liblaunch)
include_directories(${DARLING_TOP_DIRECTORY}/src/libc/gen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src/libc/darwin)
include_directories(${CMAKE_SOURCE_DIR}/src/kernel/libsyscall/wrappers)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks -nostdinc -include ${DARLING_TOP_DIRECTORY}/kernel-include/sys/fileport.h")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

mig(notify_ipc.defs)

set(notify_sources
	libnotify.c
	notify_client.c
	table.c
	${CMAKE_CURRENT_BINARY_DIR}/notify_ipcUser.c
)

set(DYLIB_INSTALL_NAME "/usr/lib/system/libsystem_notify.dylib")
add_circular(system_notify FAT
	SOURCES
		${notify_sources}
	SIBLINGS
		system_c
		system_kernel
		system_blocks
		libdispatch_shared
		launch
		system_dyld
		system_malloc
		system_pthread
		platform
		system_asl
)
#target_link_libraries(system_notify system_c system_kernel system_blocks libdispatch_shared launch system_dyld system_malloc system_pthread)
#make_fat(system_notify)

install(TARGETS system_notify DESTINATION libexec/darling/usr/lib/system)


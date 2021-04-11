project(libnotify)

cmake_minimum_required(VERSION 3.10)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-nostdinc -w)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fblocks -include ${CMAKE_SOURCE_DIR}/src/external/lkm/bsd/sys/fileport.h")
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
	DEPENDENCIES
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
		xpc
)
#target_link_libraries(system_notify system_c system_kernel system_blocks libdispatch_shared launch system_dyld system_malloc system_pthread)

add_darling_executable(notifyutil notifyutil/notifyutil.c)

install(TARGETS system_notify DESTINATION libexec/darling/usr/lib/system)
install(TARGETS notifyutil DESTINATION libexec/darling/usr/bin)
install(FILES notifyutil/notifyutil.1 DESTINATION libexec/darling/usr/share/man/man1)

add_subdirectory(notifyd)

project(libc-sys)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
include_directories(${DARLING_TOP_DIRECTORY}/src/libnotify)

set(sys_sources chmodx_np.c
	crt_externs.c
	#errno.c # Moved by Apple into libsystem_kernel
	fork.c
	getgroups.c
	#getiopolicy_np.c
	gettimeofday.c
	_libc_fork_child.c
	__libc_init.c
	#msgctl.c
	openx_np.c
	#OSMemoryNotification.c
	#OSThermalNotification.c
	posix_spawn.c
	semctl.c
	settimeofday.c
	shmctl.c
	sigaction.c
	sigcatch.c
	sigtramp.c
	slot_name.c
	statx_np.c
	umaskx_np.c
	OpenBSD/stack_protector.c
)

#if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
#	set(sys_sources ${sys_sources} context-stubs.c)
#endif()

SET_SOURCE_FILES_PROPERTIES(semctl.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_SEMCTL -DKERNEL -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0")
SET_SOURCE_FILES_PROPERTIES(shmctl.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_SEMCTL -DKERNEL -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0")
SET_SOURCE_FILES_PROPERTIES(sigtramp.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -D_XOPEN_SOURCE")

add_library(libc-sys OBJECT ${sys_sources})

# ***
# No-INODE64 variants
# ***
set(sys-noinode64_sources statx_np.c)
add_library(libc-sys_noinode64 OBJECT ${sys-noinode64_sources})
SET_TARGET_PROPERTIES(libc-sys_noinode64 PROPERTIES COMPILE_FLAGS "-U__DARWIN_UNIX03 -U__DARWIN_64_BIT_INO_T -DVARIANT_INODE32 -DBUILDING_VARIANT -D_DARWIN_NO_64_BIT_INODE")


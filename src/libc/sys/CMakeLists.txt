project(libc-sys)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(sys_sources
	crt_externs.c
	fork.c
	getgroups.c
	gettimeofday.c
	chmodx_np.c
	_libc_fork_child.c
	_libc_init.c
	msgctl.c
	openx_np.c
	OSMemoryNotification.c
	OSThermalNotification.c
	posix_spawn.c
	semctl.c
	settimeofday.c
	shmctl.c
	sigaction.c
	statx_np.c
	umaskx_np.c
	OpenBSD/stack_protector.c
)

#if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
#	set(sys_sources ${sys_sources} context-stubs.c)
#endif()

SET_SOURCE_FILES_PROPERTIES(msgctl.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DKERNEL -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0")
SET_SOURCE_FILES_PROPERTIES(semctl.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_SEMCTL -DKERNEL -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0")
SET_SOURCE_FILES_PROPERTIES(shmctl.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_SEMCTL -DKERNEL -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0")
SET_SOURCE_FILES_PROPERTIES(sigtramp.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -D_XOPEN_SOURCE")
SET_SOURCE_FILES_PROPERTIES(OpenBSD/stack_protector.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -fno-stack-protector")

add_darling_object_library(libc-sys ${sys_sources})

# ***
# No-INODE64 variants
# ***
set(sys-noinode64_sources statx_np.c)
add_library(libc-sys_noinode64 OBJECT ${sys-noinode64_sources})
SET_TARGET_PROPERTIES(libc-sys_noinode64 PROPERTIES COMPILE_FLAGS "-U__DARWIN_UNIX03 -U__DARWIN_64_BIT_INO_T -DVARIANT_INODE32 -DBUILDING_VARIANT -D_DARWIN_NO_64_BIT_INODE")

add_library(libc-sys_dyld OBJECT gettimeofday.c crt_externs.c sigaction.c)
SET_TARGET_PROPERTIES(libc-sys_dyld PROPERTIES COMPILE_FLAGS "-UBUILDING_VARIANT -DVARIANT_DYLD -DVARIANT_CANCELABLE -DVARIANT_DARWINEXTSN")


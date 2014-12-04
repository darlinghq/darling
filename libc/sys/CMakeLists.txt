project(libc-sys)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(sys_sources chmodx_np.c
	context-stubs.c
	crt_externs.c
	errno.c
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
	#settimeofday.c
	shmctl.c
	sigaction.c
	sigcatch.c
	sigtramp.c
	slot_name.c
	statx_np.c
	umaskx_np.c
)

SET_SOURCE_FILES_PROPERTIES(semctl.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_SEMCTL -DKERNEL -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0")
SET_SOURCE_FILES_PROPERTIES(shmctl.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -DLIBC_ALIAS_SEMCTL -DKERNEL -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0")
SET_SOURCE_FILES_PROPERTIES(sigtramp.c PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -D_XOPEN_SOURCE")

add_library(libc-sys OBJECT ${sys_sources})

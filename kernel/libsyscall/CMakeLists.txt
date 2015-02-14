project(libsyscall)

cmake_minimum_required(VERSION 2.4.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -nostdinc")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/wrappers")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/mach")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/os")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../platform-include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../libc/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../duct/include")
add_definitions(-DPRIVATE)

mig(mach/exc.defs)

set(syscall_sources
	#wrappers/coalition.c
	# wrappers/__commpage_gettimeofday.c
	#wrappers/csr.c
	wrappers/gethostuuid.c
	wrappers/getiopolicy_np.c
	#wrappers/guarded_open_dprotected_np.c
	#wrappers/guarded_open_np.c
	wrappers/init_cpu_capabilities.c
	wrappers/ioctl.c
	wrappers/_libc_funcptr.c
	wrappers/_libkernel_init.c
	wrappers/mach_approximate_time.c
	wrappers/open_dprotected_np.c
	wrappers/posix_sem_obsolete.c
	wrappers/remove-counter.c
	wrappers/renameat.c
	wrappers/rename.c
	wrappers/rename_ext.c
	wrappers/rmdir.c
	#wrappers/select-base.c
	wrappers/setpriority.c
	#wrappers/sfi.c
	#wrappers/sigsuspend-base.c
	wrappers/unlinkat.c
	wrappers/unlink.c
	
	wrappers/unix03/chmod.c
	wrappers/unix03/fchmod.c
	wrappers/unix03/getrlimit.c
	wrappers/unix03/mmap.c
	wrappers/unix03/munmap.c
	wrappers/unix03/setrlimit.c
	
	#wrappers/legacy/accept.c
	#wrappers/legacy/bind.c
	#wrappers/legacy/connect.c
	#wrappers/legacy/getattrlist.c
	#wrappers/legacy/getaudit.c
	#wrappers/legacy/getpeername.c
	#wrappers/legacy/getsockname.c
	#wrappers/legacy/kill.c
	#wrappers/legacy/lchown.c
	#wrappers/legacy/listen.c
	#wrappers/legacy/mprotect.c
	#wrappers/legacy/msync.c
	#wrappers/legacy/munmap.c
	#wrappers/legacy/open.c
	#wrappers/legacy/recvfrom.c
	#wrappers/legacy/recvmsg.c
	#wrappers/legacy/select.c
	#wrappers/legacy/select-pre1050.c
	#wrappers/legacy/sendmsg.c
	#wrappers/legacy/sendto.c
	#wrappers/legacy/setattrlist.c
	#wrappers/legacy/sigsuspend.c
	#wrappers/legacy/socketpair.c

	custom/errno.c
	${CMAKE_CURRENT_BINARY_DIR}/excUser.c
	)

add_library(libsyscall OBJECT ${syscall_sources})

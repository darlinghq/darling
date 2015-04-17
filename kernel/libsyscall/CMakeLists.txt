project(libsyscall)

cmake_minimum_required(VERSION 2.4.0)

enable_language(C ASM)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -nostdinc")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/wrappers")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/mach")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../iokit")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/os")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../platform-include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../libc/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../duct/include")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
add_definitions(-DPRIVATE)
add_definitions('-D__XNU_PRIVATE_EXTERN=')
#add_definitions(-DKERNEL)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/darwin.map")

set(MIG_USER_HEADER_SUFFIX "_internal.h")
mig(mach/exc.defs)
mig(mach/host_priv.defs)
mig(mach/host_security.defs)
mig(mach/ledger.defs)
mig(mach/lock_set.defs)
mig(mach/mach_host.defs)
mig(mach/mach_port.defs)
mig(mach/mach_vm.defs)
mig(mach/mach_voucher.defs)
mig(mach/notify.defs)
mig(mach/processor_set.defs)
mig(mach/processor.defs)
mig(mach/task.defs)
mig(mach/thread_act.defs)
mig(mach/vm_map.defs)

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
	
	# MIG generated files
	${CMAKE_CURRENT_BINARY_DIR}/mach/excUser.c
	${CMAKE_CURRENT_BINARY_DIR}/mach/host_privUser.c
	${CMAKE_CURRENT_BINARY_DIR}/mach/host_securityUser.c
	${CMAKE_CURRENT_BINARY_DIR}/mach/ledgerUser.c
	${CMAKE_CURRENT_BINARY_DIR}/mach/lock_setUser.c
	${CMAKE_CURRENT_BINARY_DIR}/mach/mach_hostUser.c
	${CMAKE_CURRENT_BINARY_DIR}/mach/mach_portUser.c
	${CMAKE_CURRENT_BINARY_DIR}/mach/mach_vmUser.c
	${CMAKE_CURRENT_BINARY_DIR}/mach/mach_voucherUser.c
	${CMAKE_CURRENT_BINARY_DIR}/mach/notifyUser.c
	${CMAKE_CURRENT_BINARY_DIR}/mach/processor_setUser.c
	${CMAKE_CURRENT_BINARY_DIR}/mach/processorUser.c
	${CMAKE_CURRENT_BINARY_DIR}/mach/taskUser.c
	${CMAKE_CURRENT_BINARY_DIR}/mach/thread_actUser.c
	${CMAKE_CURRENT_BINARY_DIR}/mach/vm_mapUser.c
	
	mach/mach_init.c
	mach/clock_sleep.c
	mach/dylib_link.c
	mach/error_codes.c
	mach/exc_catcher.c
	mach/exc_catcher_state.c
	mach/exc_catcher_state_identity.c
	mach/fprintf_stderr.c
	mach/mach_error.c
	mach/mach_error_string.c
	mach/mach_init.c
	mach/mach_legacy.c
	mach/mach_msg.c
	mach/mach_port.c
	mach/mach_vm.c
	mach/mig_allocate.c
	mach/mig_deallocate.c
	mach/mig_reply_port.c
	mach/mig_reply_setup.c
	mach/mig_strncpy.c
	mach/ms_thread_switch.c
	mach/panic.c
	mach/port_obj.c
	mach/semaphore.c
	mach/slot_name.c
	mach/string.c
	mach/thread_act.c

	Platforms/syscall.s
	Platforms/MacOSX/x86_64/syscall.s
)

mig(mach/host_priv.defs)
mig(mach/host_security.defs)
mig(mach/ledger.defs)
mig(mach/lock_set.defs)
mig(mach/mach_host.defs)
mig(mach/mach_port.defs)
mig(mach/mach_vm.defs)
mig(mach/mach_voucher.defs)
mig(mach/notify.defs)
mig(mach/processor_set.defs)
mig(mach/processor.defs)
mig(mach/task.defs)
mig(mach/thread_act.defs)
mig(mach/vm_map.defs)

add_library(libsyscall OBJECT ${syscall_sources})

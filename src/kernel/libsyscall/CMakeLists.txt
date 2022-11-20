project(libsyscall)

cmake_minimum_required(VERSION 3.10)

enable_language(C ASM)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -nostdinc -include ${CMAKE_CURRENT_SOURCE_DIR}/nolegacy.h -Wno-error=int-conversion")
include_directories(BEFORE
	"${CMAKE_CURRENT_SOURCE_DIR}/wrappers"
	"${CMAKE_CURRENT_SOURCE_DIR}/mach"
	"${CMAKE_CURRENT_SOURCE_DIR}/os"
	"${CMAKE_CURRENT_SOURCE_DIR}/wrappers/libproc"
	"${CMAKE_CURRENT_SOURCE_DIR}/wrappers/spawn"
	"${CMAKE_CURRENT_SOURCE_DIR}/wrappers/string"
)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
add_definitions(-DPRIVATE=1 -D__DARWIN_C_LEVEL=20150101)
add_definitions('-D__XNU_PRIVATE_EXTERN=')
add_definitions(-DIOKIT_ALL_IPC=1 -DIOKIT=1)

include_directories(${CMAKE_SOURCE_DIR}/src/external/lkm/osfmk)

include_directories(
	"${CMAKE_BINARY_DIR}/src/external/darlingserver/include"
	"${CMAKE_SOURCE_DIR}/src/external/darlingserver/include"
)

set(LIBSYSCALL_MIGS
	mach/clock
	mach/clock_priv
	mach/clock_reply
	mach/exc
	mach/host_priv
	mach/host_security
	mach/lock_set
	mach/mach_host
	mach/mach_port
	mach/mach_voucher
	mach/memory_entry
	mach/processor
	mach/processor_set
	mach/task
	mach/thread_act
	mach/vm_map
	mach/mach_vm
)

set(LIBSYSCALL_MIGS_INTERNAL
	mach/mach_port
	mach/mach_vm
	mach/thread_act
	mach/vm_map
)

set(syscall_mig_sources "")
set(syscall_mig_sources_32 "")
set(syscall_mig_sources_64 "")
set(syscall_mig_headers "")
set(syscall_mig_internal_headers "")

add_definitions(-DLIBSYSCALL_INTERFACE=1)
set(MIG_USER_HEADER_SUFFIX ".h")
set(MIG_USER_SOURCE_SUFFIX "_unused_user.c")
set(MIG_SERVER_HEADER_SUFFIX "_unused_server.h")
set(MIG_SERVER_SOURCE_SUFFIX "_unused_server.c")
foreach(MIG_FILENAME IN LISTS LIBSYSCALL_MIGS)
	mig("${MIG_FILENAME}.defs")
	list(APPEND syscall_mig_headers "${CMAKE_CURRENT_BINARY_DIR}/${MIG_FILENAME}.h")
endforeach()

mig(mach/servers/netname.defs)
list(APPEND syscall_mig_headers "${CMAKE_CURRENT_BINARY_DIR}/mach/servers/netname.h")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${DARLING_SDK_RELATIVE_PATH}/usr/include/servers")
create_symlink(
	"${DARLING_ROOT_RELATIVE_TO_SDK}/../../../src/kernel/libsyscall/mach/servers/netname.h"
	"${CMAKE_BINARY_DIR}/${DARLING_SDK_RELATIVE_PATH}/usr/include/servers/netname.h"
)

remove_definitions(-DLIBSYSCALL_INTERFACE=1)
set(MIG_MULTIARCH i386 x86_64)
set(MIG_USER_HEADER_SUFFIX "_unused_user.h")
set(MIG_USER_SOURCE_SUFFIX)
set(MIG_SERVER_HEADER_SUFFIX)
set(MIG_SERVER_SOURCE_SUFFIX)
set(MIG_XTRACE_SUFFIX "_unused_xtrace1.c")
set(MIG_NO_XTRACE)
foreach(MIG_FILENAME IN LISTS LIBSYSCALL_MIGS)
	mig("${MIG_FILENAME}.defs")

	list(APPEND syscall_mig_sources_32 "${CMAKE_CURRENT_BINARY_DIR}/${MIG_FILENAME}-i386-User.c")
	list(APPEND syscall_mig_sources_64 "${CMAKE_CURRENT_BINARY_DIR}/${MIG_FILENAME}-x86_64-User.c")

	get_filename_component(MIG_HEADER_DIR ${MIG_FILENAME} DIRECTORY)

	file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${DARLING_SDK_RELATIVE_PATH}/usr/include/${MIG_HEADER_DIR}")
	create_symlink(
		"${DARLING_ROOT_RELATIVE_TO_SDK}/../../../src/kernel/libsyscall/${MIG_FILENAME}.h"
		"${CMAKE_BINARY_DIR}/${DARLING_SDK_RELATIVE_PATH}/usr/include/${MIG_FILENAME}.h"
	)
endforeach()

# mach/exc also needs the server component
list(APPEND syscall_mig_sources_32 "${CMAKE_CURRENT_BINARY_DIR}/mach/exc-i386-Server.c")
list(APPEND syscall_mig_sources_64 "${CMAKE_CURRENT_BINARY_DIR}/mach/exc-x86_64-Server.c")

unset(MIG_MULTIARCH)
set(MIG_USER_HEADER_SUFFIX "_internal.h")
set(MIG_USER_SOURCE_SUFFIX "_unused_internal_user.c")
set(MIG_SERVER_HEADER_SUFFIX "_unused_internal_server.h")
set(MIG_SERVER_SOURCE_SUFFIX "_unused_internal_server.c")
set(MIG_XTRACE_SUFFIX "_unused_xtrace2.c")
foreach(MIG_FILENAME IN LISTS LIBSYSCALL_MIGS_INTERNAL)
	mig("${MIG_FILENAME}.defs")
	list(APPEND syscall_mig_internal_headers "${CMAKE_CURRENT_BINARY_DIR}/${MIG_FILENAME}_internal.h")

	get_filename_component(MIG_HEADER_DIR ${MIG_FILENAME} DIRECTORY)

	file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${DARLING_SDK_RELATIVE_PATH}/usr/include/${MIG_HEADER_DIR}")
	create_symlink(
		"${DARLING_ROOT_RELATIVE_TO_SDK}/../../../src/kernel/libsyscall/${MIG_FILENAME}_internal.h"
		"${CMAKE_BINARY_DIR}/${DARLING_SDK_RELATIVE_PATH}/usr/include/${MIG_FILENAME}_internal.h"
	)
endforeach()

add_custom_target(libsyscall_generate_mig_headers
	DEPENDS
		${syscall_mig_headers}
)

add_custom_target(libsyscall_generate_mig_internal_headers
	DEPENDS
		${syscall_mig_internal_headers}
)

set(syscall_legacy_sources
	wrappers/legacy/accept.c
	wrappers/legacy/bind.c
	wrappers/legacy/connect.c
	wrappers/legacy/getattrlist.c
	wrappers/legacy/getaudit.c
	wrappers/legacy/getpeername.c
	wrappers/legacy/getsockname.c
	wrappers/legacy/kill.c
	wrappers/legacy/lchown.c
	wrappers/legacy/listen.c
	wrappers/legacy/mprotect.c
	wrappers/legacy/msync.c
	wrappers/legacy/munmap.c
	wrappers/legacy/open.c
	wrappers/legacy/recvfrom.c
	wrappers/legacy/recvmsg.c
	wrappers/legacy/select.c
	#wrappers/legacy/select-pre1050.c
	wrappers/legacy/sendmsg.c
	wrappers/legacy/sendto.c
	wrappers/legacy/setattrlist.c
	wrappers/legacy/sigsuspend.c
	wrappers/legacy/socketpair.c
	)

set(syscall_dynamic_sources
	wrappers/_libc_funcptr.c
	wrappers/_libkernel_init.c)

set(syscall_sources
	mach/mach_traps.S
	os/alloc_once.c
	os/log_data.c
	wrappers/libproc/libproc.c
	wrappers/libproc/proc_listpidspath.c

	wrappers/mach_approximate_time_i386.S
	wrappers/carbon_delete.c
	wrappers/clonefile.c
	wrappers/coalition.c
	wrappers/__commpage_gettimeofday.c
	wrappers/__get_cpu_capabilities.S
	wrappers/csr.c
	wrappers/fs_snapshot.c
	wrappers/gethostuuid.c
	wrappers/getiopolicy_np.c
	wrappers/guarded_open_dprotected_np.c
	wrappers/guarded_open_np.c
	wrappers/init_cpu_capabilities.c
	wrappers/ioctl.c
	wrappers/kdebug_trace.c
	wrappers/kill.c
	wrappers/mach_approximate_time.c
	wrappers/mach_boottime.c
	wrappers/mach_continuous_time.c
	wrappers/mach_get_times.c
	wrappers/mach_timebase_info.c
	wrappers/open_dprotected_np.c
	wrappers/persona.c
	wrappers/posix_sem_obsolete.c
	wrappers/quota_obsolete.c
	wrappers/remove-counter.c
	wrappers/renameat.c
	wrappers/rename.c
	wrappers/renamex.c
	wrappers/rmdir.c
	wrappers/setpriority.c
	wrappers/sfi.c
	wrappers/stackshot.c
	wrappers/terminate_with_reason.c
	wrappers/thread_register_state.c
	wrappers/unlinkat.c
	wrappers/unlink.c
	wrappers/work_interval.c
	wrappers/utimensat.c
	wrappers/mach_bridge_remote_time.c
	wrappers/pid_shutdown_networking.c
	wrappers/proc.c
	wrappers/reboot.c

	wrappers/legacy/select-pre1050.c
	
	wrappers/cancelable/fcntl.c
	wrappers/cancelable/fcntl-cancel.c
	wrappers/cancelable/select.c # defined in sys_x86-64/
	wrappers/cancelable/select-cancel.c
	wrappers/cancelable/sigsuspend.c
	wrappers/cancelable/sigsuspend-cancel.c
	wrappers/cancelable/pselect-darwinext-cancel.c
	wrappers/cancelable/pselect-darwinext.c
	
	wrappers/unix03/chmod.c
	wrappers/unix03/fchmod.c
	wrappers/unix03/getrlimit.c
	wrappers/unix03/mmap.c
	wrappers/unix03/munmap.c
	wrappers/unix03/setrlimit.c

	wrappers/spawn/posix_spawn.c

	wrappers/string/index.c
	wrappers/string/memcpy.c
	wrappers/string/memset.c
	wrappers/string/strcmp.c
	wrappers/string/strcpy.c
	wrappers/string/strlcpy.c
	wrappers/string/strlen.c
	wrappers/string/strsep.c

	${syscall_legacy_sources}

	${syscall_mig_sources}

	custom/errno.c

	mach/mach_init.c
	mach/clock_sleep.c
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
	mach/host.c
	mach/mach_right.c
	mach/port_descriptions.c

	bsdsyscalls/___abort_with_payload.S
	bsdsyscalls/___accept_nocancel.S
	bsdsyscalls/___accept.S
	bsdsyscalls/___access_extended.S
	bsdsyscalls/_access.S
	bsdsyscalls/_acct.S
	bsdsyscalls/_adjtime.S
	bsdsyscalls/_aio_cancel.S
	bsdsyscalls/_aio_error.S
	bsdsyscalls/_aio_fsync.S
	bsdsyscalls/_aio_read.S
	bsdsyscalls/_aio_return.S
	bsdsyscalls/___aio_suspend_nocancel.S
	bsdsyscalls/_aio_suspend.S
	bsdsyscalls/_aio_write.S
	bsdsyscalls/_auditctl.S
	bsdsyscalls/_auditon.S
	bsdsyscalls/_audit.S
	bsdsyscalls/_audit_session_join.S
	bsdsyscalls/_audit_session_port.S
	bsdsyscalls/_audit_session_self.S
	bsdsyscalls/___bind.S
	bsdsyscalls/___bsdthread_create.S
	bsdsyscalls/___bsdthread_ctl.S
	bsdsyscalls/___bsdthread_register.S
	bsdsyscalls/___bsdthread_terminate.S
	bsdsyscalls/_clonefileat.S
	bsdsyscalls/___close_nocancel.S
	bsdsyscalls/_close.S
	bsdsyscalls/___coalition_info.S
	bsdsyscalls/___coalition.S
	bsdsyscalls/___coalition_ledger.S
	bsdsyscalls/___connect_nocancel.S
	bsdsyscalls/___connect.S
	bsdsyscalls/_connectx.S
	bsdsyscalls/___copyfile.S
	bsdsyscalls/_csops_audittoken.S
	bsdsyscalls/_csops.S
	bsdsyscalls/___csrctl.S
	bsdsyscalls/custom.S
	bsdsyscalls/___delete.S
	bsdsyscalls/___disable_threadsignal.S
	bsdsyscalls/_disconnectx.S
	bsdsyscalls/_dup.S
	bsdsyscalls/_dup2.S
	bsdsyscalls/_execve.S
	bsdsyscalls/_exchangedata.S
	bsdsyscalls/___exit.S
	bsdsyscalls/_faccessat.S
	bsdsyscalls/_fclonefileat.S
	bsdsyscalls/___fcntl_nocancel.S
	bsdsyscalls/___fcntl.S
	bsdsyscalls/_fdatasync.S
	bsdsyscalls/_ffsctl.S
	bsdsyscalls/_fgetattrlist.S
	bsdsyscalls/_fgetxattr.S
	bsdsyscalls/_fhopen.S
	bsdsyscalls/_fchdir.S
	bsdsyscalls/_fchflags.S
	bsdsyscalls/_fchmodat.S
	bsdsyscalls/___fchmod_extended.S
	bsdsyscalls/___fchmod.S
	bsdsyscalls/_fchownat.S
	bsdsyscalls/_fchown.S
	bsdsyscalls/_fileport_makefd.S
	bsdsyscalls/_fileport_makeport.S
	bsdsyscalls/_flistxattr.S
	bsdsyscalls/_flock.S
	bsdsyscalls/___fork.S
	bsdsyscalls/_fpathconf.S
	bsdsyscalls/_fremovexattr.S
	bsdsyscalls/_fsctl.S
	bsdsyscalls/_fsetattrlist.S
	bsdsyscalls/_setattrlistat.S
	bsdsyscalls/_fsetxattr.S
	bsdsyscalls/_fsgetpath.S
	bsdsyscalls/___fs_snapshot.S
	bsdsyscalls/_fstatat.S
	bsdsyscalls/_fstatat64.S
	bsdsyscalls/___fstat_extended.S
	bsdsyscalls/_fstatfs.S
	bsdsyscalls/_fstatfs64.S
	bsdsyscalls/_fstat.S
	bsdsyscalls/___fstat64_extended.S
	bsdsyscalls/_fstat64.S
	bsdsyscalls/___fsync_nocancel.S
	bsdsyscalls/_fsync.S
	bsdsyscalls/_ftruncate.S
	bsdsyscalls/_futimes.S
	bsdsyscalls/_getattrlistat.S
	bsdsyscalls/_getattrlistbulk.S
	bsdsyscalls/___getattrlist.S
	bsdsyscalls/_getaudit_addr.S
	bsdsyscalls/_getauid.S
	bsdsyscalls/_getdirentriesattr.S
	bsdsyscalls/_getdirentries.S
	bsdsyscalls/___getdirentries64.S
	bsdsyscalls/_getdtablesize.S
	bsdsyscalls/_getegid.S
	bsdsyscalls/_getentropy.S
	bsdsyscalls/_geteuid.S
	bsdsyscalls/_getfh.S
	bsdsyscalls/_getfsstat.S
	bsdsyscalls/_getfsstat64.S
	bsdsyscalls/_getgid.S
	bsdsyscalls/_getgroups.S
	bsdsyscalls/___gethostuuid.S
	bsdsyscalls/_getitimer.S
	bsdsyscalls/___getlogin.S
	bsdsyscalls/___getpeername.S
	bsdsyscalls/_getpgid.S
	bsdsyscalls/_getpgrp.S
	bsdsyscalls/___getpid.S
	bsdsyscalls/_getppid.S
	bsdsyscalls/_getpriority.S
	bsdsyscalls/___getrlimit.S
	bsdsyscalls/_getrusage.S
	bsdsyscalls/___getsgroups.S
	bsdsyscalls/_getsid.S
	bsdsyscalls/___getsockname.S
	bsdsyscalls/_getsockopt.S
	bsdsyscalls/___gettid.S
	bsdsyscalls/___gettimeofday.S
	bsdsyscalls/_getuid.S
	bsdsyscalls/___getwgroups.S
	bsdsyscalls/_getxattr.S
	bsdsyscalls/_grab_pgo_data.S
	bsdsyscalls/_guarded_close_np.S
	bsdsyscalls/_guarded_kqueue_np.S
	bsdsyscalls/___guarded_open_dprotected_np.S
	bsdsyscalls/___guarded_open_np.S
	bsdsyscalls/_guarded_pwrite_np.S
	bsdsyscalls/_guarded_write_np.S
	bsdsyscalls/_guarded_writev_np.S
	bsdsyscalls/_change_fdguard_np.S
	bsdsyscalls/_chdir.S
	bsdsyscalls/_chflags.S
	bsdsyscalls/___chmod_extended.S
	bsdsyscalls/___chmod.S
	bsdsyscalls/_chown.S
	bsdsyscalls/_chroot.S
	bsdsyscalls/___identitysvc.S
	bsdsyscalls/___initgroups.S
	bsdsyscalls/___ioctl.S
	bsdsyscalls/___iopolicysys.S
	bsdsyscalls/_issetugid.S
	bsdsyscalls/_kas_info.S
	bsdsyscalls/___kdebug_trace.S
	bsdsyscalls/___kdebug_trace_string.S
	bsdsyscalls/___kdebug_trace64.S
	bsdsyscalls/___kdebug_typefilter.S
	bsdsyscalls/_kevent_qos.S
	bsdsyscalls/_kevent.S
	bsdsyscalls/_kevent64.S
	bsdsyscalls/___kill.S
	bsdsyscalls/_kqueue.S
	bsdsyscalls/_ledger.S
	bsdsyscalls/___lchown.S
	bsdsyscalls/_linkat.S
	bsdsyscalls/_link.S
	bsdsyscalls/_lio_listio.S
	bsdsyscalls/___listen.S
	bsdsyscalls/_listxattr.S
	bsdsyscalls/___log_data.S
	bsdsyscalls/___lseek.S
	bsdsyscalls/___lstat_extended.S
	bsdsyscalls/_lstat.S
	bsdsyscalls/___lstat64_extended.S
	bsdsyscalls/_lstat64.S
	bsdsyscalls/___mac_execve.S
	bsdsyscalls/___mac_get_fd.S
	bsdsyscalls/___mac_get_file.S
	bsdsyscalls/___mac_getfsstat.S
	bsdsyscalls/___mac_get_link.S
	bsdsyscalls/___mac_get_mount.S
	bsdsyscalls/___mac_get_pid.S
	bsdsyscalls/___mac_get_proc.S
	bsdsyscalls/___mac_mount.S
	bsdsyscalls/___mac_set_fd.S
	bsdsyscalls/___mac_set_file.S
	bsdsyscalls/___mac_set_link.S
	bsdsyscalls/___mac_set_proc.S
	bsdsyscalls/___mac_syscall.S
	bsdsyscalls/_madvise.S
	bsdsyscalls/_memorystatus_control.S
	bsdsyscalls/_memorystatus_get_level.S
	bsdsyscalls/___microstackshot.S
	bsdsyscalls/_mincore.S
	bsdsyscalls/_minherit.S
	bsdsyscalls/_mkdirat.S
	bsdsyscalls/___mkdir_extended.S
	bsdsyscalls/_mkdir.S
	bsdsyscalls/___mkfifo_extended.S
	bsdsyscalls/_mkfifo.S
	bsdsyscalls/_mknod.S
	bsdsyscalls/_mlockall.S
	bsdsyscalls/_mlock.S
	bsdsyscalls/___mmap.S
	bsdsyscalls/_modwatch.S
	bsdsyscalls/_mount.S
	bsdsyscalls/___mprotect.S
	bsdsyscalls/_mremap_encrypted.S
	bsdsyscalls/___msgctl.S
	bsdsyscalls/_msgget.S
	bsdsyscalls/___msgrcv_nocancel.S
	bsdsyscalls/_msgrcv.S
	bsdsyscalls/___msgsnd_nocancel.S
	bsdsyscalls/_msgsnd.S
	bsdsyscalls/___msgsys.S
	bsdsyscalls/___msync_nocancel.S
	bsdsyscalls/___msync.S
	bsdsyscalls/_munlockall.S
	bsdsyscalls/_munlock.S
	bsdsyscalls/___munmap.S
	bsdsyscalls/_necp_client_action.S
	bsdsyscalls/_necp_match_policy.S
	bsdsyscalls/_necp_open.S
	bsdsyscalls/_netagent_trigger.S
	bsdsyscalls/_nfsclnt.S
	bsdsyscalls/_nfssvc.S
	bsdsyscalls/_____old_semwait_signal_nocancel.S
	bsdsyscalls/___old_semwait_signal.S
	bsdsyscalls/___openat_nocancel.S
	bsdsyscalls/___openat.S
	bsdsyscalls/_openbyid_np.S
	bsdsyscalls/___open_dprotected_np.S
	bsdsyscalls/___open_extended.S
	bsdsyscalls/___open_nocancel.S
	bsdsyscalls/___open.S
	bsdsyscalls/_pathconf.S
	bsdsyscalls/_peeloff.S
	bsdsyscalls/___persona.S
	bsdsyscalls/_pid_resume.S
	bsdsyscalls/_pid_suspend.S
	bsdsyscalls/___pipe.S
	bsdsyscalls/___poll_nocancel.S
	bsdsyscalls/_poll.S
	bsdsyscalls/___posix_spawn.S
	bsdsyscalls/___pread_nocancel.S
	bsdsyscalls/_pread.S
	bsdsyscalls/___process_policy.S
	bsdsyscalls/___proc_info.S
	bsdsyscalls/_proc_rlimit_control.S
	bsdsyscalls/_proc_trace_log.S
	bsdsyscalls/_proc_uuid_policy.S
	bsdsyscalls/___pselect_nocancel.S
	bsdsyscalls/___pselect.S
	bsdsyscalls/___psynch_cvbroad.S
	bsdsyscalls/___psynch_cvclrprepost.S
	bsdsyscalls/___psynch_cvsignal.S
	bsdsyscalls/___psynch_cvwait.S
	bsdsyscalls/___psynch_mutexdrop.S
	bsdsyscalls/___psynch_mutexwait.S
	bsdsyscalls/___psynch_rw_downgrade.S
	bsdsyscalls/___psynch_rw_longrdlock.S
	bsdsyscalls/___psynch_rw_rdlock.S
	bsdsyscalls/___psynch_rw_unlock.S
	bsdsyscalls/___psynch_rw_unlock2.S
	bsdsyscalls/___psynch_rw_upgrade.S
	bsdsyscalls/___psynch_rw_wrlock.S
	bsdsyscalls/___psynch_rw_yieldwrlock.S
	bsdsyscalls/___pthread_canceled.S
	bsdsyscalls/___pthread_fchdir.S
	bsdsyscalls/___pthread_chdir.S
	bsdsyscalls/___pthread_kill.S
	bsdsyscalls/___pthread_markcancel.S
	bsdsyscalls/___pthread_sigmask.S
	bsdsyscalls/___ptrace.S
	bsdsyscalls/___pwrite_nocancel.S
	bsdsyscalls/_pwrite.S
	bsdsyscalls/_quotactl.S
	bsdsyscalls/_readlinkat.S
	bsdsyscalls/_readlink.S
	bsdsyscalls/___read_nocancel.S
	bsdsyscalls/_read.S
	bsdsyscalls/___readv_nocancel.S
	bsdsyscalls/_readv.S
	bsdsyscalls/___recvfrom_nocancel.S
	bsdsyscalls/___recvfrom.S
	bsdsyscalls/___recvmsg_nocancel.S
	bsdsyscalls/___recvmsg.S
	bsdsyscalls/_recvmsg_x.S
	bsdsyscalls/_removexattr.S
	bsdsyscalls/___renameat.S
	bsdsyscalls/___renameatx_np.S
	bsdsyscalls/___rename.S
	bsdsyscalls/_revoke.S
	bsdsyscalls/___rmdir.S
	bsdsyscalls/_searchfs.S
	bsdsyscalls/___select_nocancel.S
	bsdsyscalls/___select.S
	bsdsyscalls/_sem_close.S
	bsdsyscalls/___semctl.S
	bsdsyscalls/_semget.S
	bsdsyscalls/___sem_open.S
	bsdsyscalls/_semop.S
	bsdsyscalls/_sem_post.S
	bsdsyscalls/___semsys.S
	bsdsyscalls/_sem_trywait.S
	bsdsyscalls/_sem_unlink.S
	bsdsyscalls/___sem_wait_nocancel.S
	bsdsyscalls/_sem_wait.S
	bsdsyscalls/___semwait_signal_nocancel.S
	bsdsyscalls/___semwait_signal.S
	bsdsyscalls/_sendfile.S
	bsdsyscalls/___sendmsg_nocancel.S
	bsdsyscalls/___sendmsg.S
	bsdsyscalls/_sendmsg_x.S
	bsdsyscalls/___sendto_nocancel.S
	bsdsyscalls/___sendto.S
	bsdsyscalls/___setattrlist.S
	bsdsyscalls/_setaudit_addr.S
	bsdsyscalls/_setauid.S
	bsdsyscalls/_setegid.S
	bsdsyscalls/_seteuid.S
	bsdsyscalls/_setgid.S
	bsdsyscalls/_setgroups.S
	bsdsyscalls/_setitimer.S
	bsdsyscalls/___setlogin.S
	bsdsyscalls/_setpgid.S
	bsdsyscalls/___setpriority.S
	bsdsyscalls/_setprivexec.S
	bsdsyscalls/___setregid.S
	bsdsyscalls/___setreuid.S
	bsdsyscalls/___setrlimit.S
	bsdsyscalls/___setsgroups.S
	bsdsyscalls/_setsid.S
	bsdsyscalls/_setsockopt.S
	bsdsyscalls/___settid.S
	bsdsyscalls/___settid_with_pid.S
	bsdsyscalls/___settimeofday.S
	bsdsyscalls/_setuid.S
	bsdsyscalls/___setwgroups.S
	bsdsyscalls/_setxattr.S
	bsdsyscalls/___sfi_ctl.S
	bsdsyscalls/___sfi_pidctl.S
	bsdsyscalls/___shared_region_check_np.S
	bsdsyscalls/___shared_region_map_and_slide_np.S
	bsdsyscalls/_shmat.S
	bsdsyscalls/___shmctl.S
	bsdsyscalls/_shmdt.S
	bsdsyscalls/_shmget.S
	bsdsyscalls/___shm_open.S
	bsdsyscalls/___shmsys.S
	bsdsyscalls/_shm_unlink.S
	bsdsyscalls/_shutdown.S
	bsdsyscalls/___sigaction.S
	bsdsyscalls/___sigaltstack.S
	bsdsyscalls/_sigpending.S
	bsdsyscalls/_sigprocmask.S
	bsdsyscalls/___sigreturn.S
	bsdsyscalls/___sigsuspend_nocancel.S
	bsdsyscalls/___sigsuspend.S
	bsdsyscalls/_____sigwait_nocancel.S
	bsdsyscalls/___sigwait.S
	bsdsyscalls/_socket_delegate.S
	bsdsyscalls/___socketpair.S
	bsdsyscalls/_socket.S
	bsdsyscalls/___stack_snapshot_with_config.S
	bsdsyscalls/___stat_extended.S
	bsdsyscalls/_statfs.S
	bsdsyscalls/_statfs64.S
	bsdsyscalls/_stat.S
	bsdsyscalls/___stat64_extended.S
	bsdsyscalls/_stat64.S
	bsdsyscalls/_swapon.S
	bsdsyscalls/_symlinkat.S
	bsdsyscalls/_symlink.S
	bsdsyscalls/_sync.S
	bsdsyscalls/___syscall.S
	bsdsyscalls/___sysctlbyname.S
	bsdsyscalls/___sysctl.S
	bsdsyscalls/_system_override.S
	bsdsyscalls/___telemetry.S
	bsdsyscalls/___terminate_with_payload.S
	bsdsyscalls/___thread_selfid.S
	bsdsyscalls/___thread_selfusage.S
	bsdsyscalls/_truncate.S
	bsdsyscalls/___ulock_wait.S
	bsdsyscalls/___ulock_wake.S
	bsdsyscalls/___umask_extended.S
	bsdsyscalls/_umask.S
	bsdsyscalls/_undelete.S
	bsdsyscalls/___unlinkat.S
	bsdsyscalls/___unlink.S
	bsdsyscalls/_unmount.S
	bsdsyscalls/_usrctl.S
	bsdsyscalls/_utimes.S
	bsdsyscalls/___vfork.S
	bsdsyscalls/_vfs_purge.S
	bsdsyscalls/_vm_pressure_monitor.S
	bsdsyscalls/_waitevent.S
	bsdsyscalls/___waitid_nocancel.S
	bsdsyscalls/_waitid.S
	bsdsyscalls/___wait4_nocancel.S
	bsdsyscalls/___wait4.S
	bsdsyscalls/_watchevent.S
	bsdsyscalls/___work_interval_ctl.S
	bsdsyscalls/___workq_kernreturn.S
	bsdsyscalls/___workq_open.S
	bsdsyscalls/___write_nocancel.S
	bsdsyscalls/_write.S
	bsdsyscalls/___writev_nocancel.S
	bsdsyscalls/_writev.S
	bsdsyscalls/___kqueue_workloop_ctl.S
	bsdsyscalls/___mach_bridge_remote_time.S
	bsdsyscalls/___reboot.S
	bsdsyscalls/_fmount.S
	bsdsyscalls/_kevent_id.S
	bsdsyscalls/_necp_session_action.S
	bsdsyscalls/_necp_session_open.S
	bsdsyscalls/_net_qos_guideline.S
	bsdsyscalls/_ntp_adjtime.S
	bsdsyscalls/_ntp_gettime.S
	bsdsyscalls/_os_fault_with_payload.S
	bsdsyscalls/_pid_hibernate.S
	bsdsyscalls/_pid_shutdown_sockets.S
	bsdsyscalls/_thread_selfcounts.S
)

set_source_files_properties(${syscall_legacy_sources} PROPERTIES COMPILE_FLAGS "-U_DARWIN_C_SOURCE -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0 -U__DARWIN_C_LEVEL")
set_source_files_properties(wrappers/legacy/select-pre1050.c PROPERTIES COMPILE_FLAGS "-U_DARWIN_C_SOURCE -DSYSCALL_PRE1050")
set_source_files_properties(wrappers/cancelable/select-cancel.c PROPERTIES COMPILE_FLAGS "-U_DARWIN_C_SOURCE")
set_source_files_properties(wrappers/cancelable/select.c PROPERTIES COMPILE_FLAGS "-U_DARWIN_C_SOURCE")

foreach (file IN ITEMS ${syscall_legacy_sources})
	set_source_files_properties(${file} PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS} -U_DARWIN_C_SOURCE -U_POSIX_C_SOURCE -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0  -U__DARWIN_C_LEVEL")
endforeach(file)

add_compile_options(
	#-fdollars-in-identifiers
	-no-cpp-precomp
	-fno-common
	-fno-stack-protector
	-fno-stack-check
	-momit-leaf-frame-pointer
)

add_darling_object_library(libsyscall_32 32BIT_ONLY ${syscall_mig_sources_32})
add_dependencies(libsyscall_32
	libsyscall_generate_mig_headers
	libsyscall_generate_mig_internal_headers
)

add_darling_object_library(libsyscall_64 64BIT_ONLY ${syscall_mig_sources_64})
add_dependencies(libsyscall_64
	libsyscall_generate_mig_headers
	libsyscall_generate_mig_internal_headers
)

add_darling_object_library(libsyscall ${syscall_sources})
add_dependencies(libsyscall
	libsyscall_generate_mig_headers
	libsyscall_generate_mig_internal_headers
)

add_darling_object_library(libsyscall_dynamic ${syscall_dynamic_sources})
add_dependencies(libsyscall_dynamic
	libsyscall_generate_mig_headers
	libsyscall_generate_mig_internal_headers
)

make_fat(libsyscall)
make_fat(libsyscall_dynamic)

#add_darling_object_library(libsyscall_static wrappers/_libc_funcptr.c)
#set_target_properties(libsyscall_static PROPERTIES COMPILE_FLAGS "-DVARIANT_DYLD")
#make_fat(libsyscall_static)

# For i386
#add_darling_object_library(libsyscall_legacy_extra wrappers/legacy/select.c)
#set_target_properties(libsyscall_legacy_extra PROPERTIES COMPILE_FLAGS
#	"${CMAKE_C_FLAGS} -U_DARWIN_C_SOURCE -U_POSIX_C_SOURCE -U__DARWIN_UNIX03 -D__DARWIN_UNIX03=0 -U__DARWIN_C_LEVEL")




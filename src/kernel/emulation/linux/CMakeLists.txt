project(emulation)

add_subdirectory(mach)

cmake_policy(SET CMP0005 NEW)
enable_language(ASM)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -fvisibility=hidden -fPIC -fno-builtin -ggdb -Wno-int-conversion")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

add_definitions(-DBSDTHREAD_WRAP_LINUX_PTHREAD
	-DEMULATED_SYSNAME="Darwin"
	-DEMULATED_RELEASE="16.0.0"
	-DEMULATED_VERSION="Darwin Kernel Version 16.0.0")


include_directories(${CMAKE_SOURCE_DIR}/src/startup)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(emulation_sources
	elfcalls_wrapper.c
	base.c
	syscalls.c
	simple.c
	errno.c
	readline.c
	common_at.c
	guarded/guarded_open_np.c
	guarded/guarded_close_np.c
	guarded/guarded_kqueue_np.c
	machdep/machdeps.c
	machdep/machdep-table.S
	machdep/tls.c
	unistd/write.c
	unistd/read.c
	unistd/pread.c
	unistd/pwrite.c
	unistd/readv.c
	unistd/writev.c
	unistd/flock.c
	mman/mman.c
	mman/madvise.c
	mman/msync.c
	kqueue/kqueue.c
	kqueue/kevent.c
	kqueue/kevent64.c
	kqueue/kevent_qos.c
	unistd/getsid.c
	unistd/fsync.c
	unistd/sync.c
	unistd/fdatasync.c
	unistd/dup.c
	unistd/dup2.c
	unistd/exit.c
	unistd/close.c
	unistd/fchdir.c
	unistd/fchown.c
	unistd/fchownat.c
	unistd/fchmod.c
	unistd/fchmodat.c
	unistd/getegid.c
	unistd/setgid.c
	unistd/setuid.c
	unistd/setegid.c
	unistd/seteuid.c
	unistd/setsid.c
	unistd/getuid.c
	unistd/geteuid.c
	unistd/getpid.c
	unistd/lseek.c
	unistd/ftruncate.c
	unistd/truncate.c
	unistd/access.c
	unistd/faccessat.c
	unistd/readlink.c
	unistd/readlinkat.c
	unistd/symlink.c
	unistd/symlinkat.c
	unistd/link.c
	unistd/linkat.c
	unistd/unlink.c
	unistd/unlinkat.c
	unistd/chdir.c
	unistd/mknod.c
	unistd/chmod.c
	unistd/chown.c
	unistd/lchown.c
	unistd/umask.c
	unistd/chroot.c
	unistd/getgid.c
	unistd/getppid.c
	unistd/rename.c
	unistd/renameat.c
	unistd/getpgrp.c
	unistd/getdtablesize.c
	unistd/setpgid.c
	unistd/setgroups.c
	unistd/getgroups.c
	unistd/getpgid.c
	unistd/pipe.c
	unistd/chmod_extended.c
	unistd/fchmod_extended.c
	unistd/fchflags.c
	unistd/chflags.c
	unistd/issetugid.c
	select/select.c
	select/pselect.c
	select/poll.c
	process/vfork.c
	process/fork.c
	process/wait4.c
	process/waitid.c
	process/execve.c
	process/posix_spawn.c
	signal/duct_signals.c
	signal/kill.c
	signal/sigaltstack.c
	signal/sigaction.c
	signal/sigreturn.c
	signal/sigprocmask.c
	signal/sig_restorer.S
	signal/sigsuspend.c
	misc/getentropy.c
	misc/syscall.c
	misc/shared_region_check_np.c
	misc/ioctl.c
	misc/thread_selfid.c
	misc/proc_info.c
	misc/sysctl.c
	misc/sysctl_proc.c
	misc/sysctl_hw.c
	misc/sysctl_kern.c
	misc/sysctl_unspec.c
	misc/sysctl_machdep.c
	misc/getrlimit.c
	misc/setrlimit.c
	misc/gethostuuid.c
	misc/getrusage.c
	misc/getlogin.c
	fcntl/open.c
	fcntl/openat.c
	fcntl/fcntl.c
	network/socket.c
	network/socketpair.c
	network/connect.c
	network/recvmsg.c
	network/sendmsg.c
	network/duct.c
	network/recvfrom.c
	network/accept.c
	network/getpeername.c
	network/getsockname.c
	network/shutdown.c
	network/getsockopt.c
	network/setsockopt.c
	network/sendto.c
	network/bind.c
	network/listen.c
	stat/fstat.c
	stat/fstatat.c
	stat/lstat.c
	stat/stat.c
	stat/stat64_extended.c
	stat/lstat64_extended.c
	stat/fstat64_extended.c
	stat/getfsstat.c
	stat/statfs.c
	stat/fstatfs.c
	stat/mkdir.c
	stat/mkdirat.c
	stat/mkfifo.c
	stat/rmdir.c
	stat/common.c
	xattr/getattrlistbulk.c
	xattr/getattrlistat.c
	xattr/listxattr.c
	xattr/flistxattr.c
	xattr/removexattr.c
	xattr/fremovexattr.c
	xattr/getxattr.c
	xattr/fgetxattr.c
	xattr/setxattr.c
	xattr/fsetxattr.c
	synch/semwait_signal.c
	hfs/stub.c
	dirent/getdirentries.c
	time/gettimeofday.c
	time/utimes.c
	time/futimes.c
	time/setitimer.c
	time/getitimer.c
	ioctl/ioctl.c
	ioctl/termios.c
	ioctl/filio.c
	ext/sysinfo.c
	ext/uname.c
	ext/epoll_create.c
	ext/epoll_create1.c
	ext/epoll_ctl.c
	ext/epoll_wait.c
	ext/inotify_init.c
	ext/inotify_init1.c
	ext/inotify_add_watch.c
	ext/inotify_rm_watch.c
	ext/eventfd.c
	ext/eventfd_read.c
	ext/eventfd_write.c
	ext/signalfd.c
	ext/timerfd_create.c
	ext/timerfd_settime.c
	ext/timerfd_gettime.c
	ext/clock_nanosleep.c
	ext/clock_gettime.c
	ext/sched_yield.c
	ext/syslog.c
	ext/futex.c
	ext/mremap.c
	bsdthread/bsdthread_register.c
	bsdthread/bsdthread_create.c
	bsdthread/bsdthread_terminate.c
	bsdthread/disable_threadsignal.c
	bsdthread/workq_kernreturn.c
	bsdthread/workq_open.c
	bsdthread/pthread_kill.c
	bsdthread/__pthread_canceled.c
	psynch/psynch_mutexwait.c
	psynch/psynch_mutexdrop.c
	psynch/psynch_cvwait.c
	psynch/psynch_cvbroad.c
	psynch/psynch_cvsignal.c
	psynch/ulock_wait.c
	psynch/ulock_wake.c
	conf/pathconf.c
	conf/fpathconf.c
	syscalls-table.S
	linux-syscall.S
)

set_source_files_properties(mman/mman.c PROPERTIES COMPILE_FLAGS
	"-I${DARLING_TOP_DIRECTORY}/platform-include")
set_source_files_properties(bsdthread/bsdthread_create.c PROPERTIES COMPILE_FLAGS
	"-I${DARLING_TOP_DIRECTORY}/platform-include")
set_source_files_properties(signal/duct_signals.c PROPERTIES COMPILE_FLAGS
	"-I${DARLING_TOP_DIRECTORY}/platform-include -I${DARLING_TOP_DIRECTORY}/src/libc/include -nostdinc")
set_source_files_properties(network/socket.c PROPERTIES COMPILE_FLAGS
	"-I${DARLING_TOP_DIRECTORY}/platform-include -I${DARLING_TOP_DIRECTORY}/kernel-include")
set_source_files_properties(network/socketpair.c PROPERTIES COMPILE_FLAGS
	"-I${DARLING_TOP_DIRECTORY}/platform-include -I${DARLING_TOP_DIRECTORY}/kernel-include")
set_source_files_properties(network/duct.c PROPERTIES COMPILE_FLAGS
	"-I${DARLING_TOP_DIRECTORY}/platform-include -I${DARLING_TOP_DIRECTORY}/kernel-include")
set_source_files_properties(network/connect.c PROPERTIES COMPILE_FLAGS
	"-I${DARLING_TOP_DIRECTORY}/platform-include -I${DARLING_TOP_DIRECTORY}/kernel-include")
set_source_files_properties(dirent/getdirentries.c PROPERTIES COMPILE_FLAGS
	"-I${DARLING_TOP_DIRECTORY}/platform-include -I${DARLING_TOP_DIRECTORY}/kernel-include")
set_source_files_properties(stat/common.c PROPERTIES COMPILE_FLAGS
	"-I${DARLING_TOP_DIRECTORY}/platform-include -I${DARLING_TOP_DIRECTORY}/kernel-include")
set_source_files_properties(ext/timerfd_create.c PROPERTIES COMPILE_FLAGS
	"-I${DARLING_TOP_DIRECTORY}/platform-include -I${DARLING_TOP_DIRECTORY}/kernel-include")
set_source_files_properties(ext/timerfd_gettime.c PROPERTIES COMPILE_FLAGS
	"-I${DARLING_TOP_DIRECTORY}/platform-include -I${DARLING_TOP_DIRECTORY}/kernel-include")
set_source_files_properties(ext/timerfd_settime.c PROPERTIES COMPILE_FLAGS
	"-I${DARLING_TOP_DIRECTORY}/platform-include -I${DARLING_TOP_DIRECTORY}/kernel-include")
set_source_files_properties(misc/sysctl.c PROPERTIES COMPILE_FLAGS
	"-I${DARLING_TOP_DIRECTORY}/platform-include -I${DARLING_TOP_DIRECTORY}/kernel-include")

add_library(emulation OBJECT ${emulation_sources})
add_library(emulation_dyld OBJECT ${emulation_sources})
set_target_properties(emulation_dyld PROPERTIES COMPILE_FLAGS "-ffunction-sections -DVARIANT_DYLD")

make_fat(emulation)
make_fat(emulation_dyld)


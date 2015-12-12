project(emulation)

cmake_minimum_required(VERSION 2.4.0)

enable_language(C ASM)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -fvisibility=hidden -fPIC -ggdb -Wno-int-conversion")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib")

add_definitions(-DBSDTHREAD_WRAP_LINUX_PTHREAD)

set(emulation_sources
	base.c
	syscalls.c
	simple.c
	errno.c
	path.c
	tls.c
	unistd/write.c
	unistd/read.c
	unistd/pread.c
	unistd/pwrite.c
	unistd/readv.c
	unistd/writev.c
	mman/mman.c
	mman/madvise.c
	unistd/fsync.c
	unistd/sync.c
	unistd/fdatasync.c
	unistd/dup.c
	unistd/dup2.c
	unistd/exit.c
	unistd/close.c
	unistd/fchdir.c
	unistd/fchown.c
	unistd/fchmod.c
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
	unistd/readlink.c
	unistd/symlink.c
	unistd/link.c
	unistd/unlink.c
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
	unistd/getpgrp.c
	unistd/getdtablesize.c
	unistd/setpgid.c
	unistd/setgroups.c
	unistd/getgroups.c
	unistd/pipe.c
	select/select.c
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
	misc/ioctl.c
	misc/thread_selfid.c
	misc/proc_info.c
	misc/sysctl.c
	misc/getrlimit.c
	misc/gethostuuid.c
	misc/getrusage.c
	fcntl/open.c
	fcntl/fcntl.c
	network/socket.c
	network/connect.c
	network/recvmsg.c
	network/sendmsg.c
	network/duct.c
	network/recvfrom.c
	network/accept.c
	network/getpeername.c
	network/getsockname.c
	network/shutdown.c
	stat/fstat.c
	stat/lstat.c
	stat/stat.c
	stat/stat64_extended.c
	stat/lstat64_extended.c
	stat/fstat64_extended.c
	stat/getfsstat.c
	stat/statfs.c
	stat/fstatfs.c
	stat/mkdir.c
	stat/mkfifo.c
	stat/rmdir.c
	stat/common.c
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
	ioctl/ioctl.c
	ioctl/termios.c
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
	bsdthread/bsdthread_register.c
	bsdthread/bsdthread_create.c
	bsdthread/bsdthread_terminate.c
	bsdthread/disable_threadsignal.c
	bsdthread/workq_kernreturn.c
	bsdthread/workq_open.c
	bsdthread/pthread_kill.c
	psynch/psynch_mutexwait.c
	psynch/psynch_mutexdrop.c
	syscalls-table.S
)

set_source_files_properties(mman/mman.c PROPERTIES COMPILE_FLAGS
	"-I${DARLING_TOP_DIRECTORY}/platform-include")
set_source_files_properties(bsdthread/bsdthread_create.c PROPERTIES COMPILE_FLAGS
	"-I${DARLING_TOP_DIRECTORY}/platform-include")
set_source_files_properties(signal/duct_signals.c PROPERTIES COMPILE_FLAGS
	"-I${DARLING_TOP_DIRECTORY}/platform-include -I${DARLING_TOP_DIRECTORY}/src/libc/include -nostdinc")
set_source_files_properties(network/socket.c PROPERTIES COMPILE_FLAGS
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

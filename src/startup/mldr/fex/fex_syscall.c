#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "fex_bridge.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <sys/xattr.h>
#include <time.h>
#include <unistd.h>

#define X86_ARCH_SET_GS 0x1001
#define X86_ARCH_SET_FS 0x1002
#define X86_ARCH_GET_FS 0x1003
#define X86_ARCH_GET_GS 0x1004

struct x86_64_linux_stat {
	uint64_t st_dev;
	uint64_t st_ino;
	uint64_t st_nlink;
	uint32_t st_mode;
	uint32_t st_uid;
	uint32_t st_gid;
	uint32_t __pad0;
	uint64_t st_rdev;
	int64_t  st_size;
	int64_t  st_blksize;
	int64_t  st_blocks;
	uint64_t st_atime_sec;
	uint64_t st_atime_nsec;
	uint64_t st_mtime_sec;
	uint64_t st_mtime_nsec;
	uint64_t st_ctime_sec;
	uint64_t st_ctime_nsec;
	int64_t  __unused[3];
};

_Static_assert(sizeof(struct x86_64_linux_stat) == 144, "x86_64 linux_stat must be 144 bytes");
_Static_assert(offsetof(struct x86_64_linux_stat, st_nlink) == 16, "st_nlink offset");
_Static_assert(offsetof(struct x86_64_linux_stat, st_mode) == 24, "st_mode offset");
_Static_assert(offsetof(struct x86_64_linux_stat, st_size) == 48, "st_size offset");

struct __attribute__((packed)) x86_64_epoll_event {
	uint32_t events;
	uint64_t data;
};

_Static_assert(sizeof(struct x86_64_epoll_event) == 12, "x86_64 epoll_event must be 12 bytes");

static __thread uint64_t t_current_gs_base = 0;

void fex_set_current_thread_gs_base(uint64_t gs_base)
{
	t_current_gs_base = gs_base;
}

uint64_t fex_get_current_thread_gs_base(void)
{
	return t_current_gs_base;
}

static long raw_host_syscall(long nr, long a1, long a2, long a3, long a4, long a5, long a6)
{
#if defined(__aarch64__)
	register long x8 asm("x8") = nr;
	register long x0 asm("x0") = a1;
	register long x1 asm("x1") = a2;
	register long x2 asm("x2") = a3;
	register long x3 asm("x3") = a4;
	register long x4 asm("x4") = a5;
	register long x5 asm("x5") = a6;
	__asm__ volatile(
		"svc #0"
		: "+r"(x0)
		: "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5), "r"(x8)
		: "memory", "cc"
	);
	return x0;
#else
	long ret = syscall(nr, a1, a2, a3, a4, a5, a6);
	return (ret == -1) ? -errno : ret;
#endif
}

struct arm64_kernel_stat {
	uint64_t st_dev;
	uint64_t st_ino;
	uint32_t st_mode;
	uint32_t st_nlink;
	uint32_t st_uid;
	uint32_t st_gid;
	uint64_t st_rdev;
	uint64_t __pad1;
	int64_t  st_size;
	int32_t  st_blksize;
	int32_t  __pad2;
	int64_t  st_blocks;
	int64_t  st_atime_sec;
	uint64_t st_atime_nsec;
	int64_t  st_mtime_sec;
	uint64_t st_mtime_nsec;
	int64_t  st_ctime_sec;
	uint64_t st_ctime_nsec;
	uint32_t __unused4;
	uint32_t __unused5;
};

_Static_assert(sizeof(struct arm64_kernel_stat) == 128, "arm64 kernel_stat must be 128 bytes");

static void convert_host_stat_to_x86_64(const struct arm64_kernel_stat* src, struct x86_64_linux_stat* dst)
{
	memset(dst, 0, sizeof(*dst));
	dst->st_dev = src->st_dev;
	dst->st_ino = src->st_ino;
	dst->st_nlink = src->st_nlink;
	dst->st_mode = src->st_mode;
	dst->st_uid = src->st_uid;
	dst->st_gid = src->st_gid;
	dst->st_rdev = src->st_rdev;
	dst->st_size = src->st_size;
	dst->st_blksize = src->st_blksize;
	dst->st_blocks = src->st_blocks;
	dst->st_atime_sec = (uint64_t)src->st_atime_sec;
	dst->st_atime_nsec = src->st_atime_nsec;
	dst->st_mtime_sec = (uint64_t)src->st_mtime_sec;
	dst->st_mtime_nsec = src->st_mtime_nsec;
	dst->st_ctime_sec = (uint64_t)src->st_ctime_sec;
	dst->st_ctime_nsec = src->st_ctime_nsec;
}

static int map_x86_64_nr_to_arm64(uint64_t x86_nr)
{
	switch (x86_nr) {
		case 0:   return 63;
		case 1:   return 64;
		case 3:   return 57;
		case 8:   return 62;
		case 9:   return 222;
		case 10:  return 226;
		case 11:  return 215;
		case 12:  return 214;
		case 13:  return 134;
		case 14:  return 135;
		case 15:  return 139;
		case 16:  return 29;
		case 17:  return 67;
		case 18:  return 68;
		case 19:  return 65;
		case 20:  return 66;
		case 24:  return 124;
		case 25:  return 216;
		case 26:  return 227;
		case 27:  return 232;
		case 28:  return 233;
		case 32:  return 23;
		case 35:  return 101;
		case 36:  return 102;
		case 38:  return 103;
		case 39:  return 172;
		case 40:  return 71;
		case 41:  return 198;
		case 42:  return 203;
		case 43:  return 202;
		case 44:  return 206;
		case 45:  return 207;
		case 46:  return 211;
		case 47:  return 212;
		case 48:  return 210;
		case 49:  return 200;
		case 50:  return 201;
		case 51:  return 204;
		case 52:  return 205;
		case 53:  return 199;
		case 54:  return 208;
		case 55:  return 209;
		case 59:  return 221;
		case 60:  return 93;
		case 61:  return 260;
		case 62:  return 129;
		case 63:  return 160;
		case 72:  return 25;
		case 73:  return 32;
		case 74:  return 82;
		case 75:  return 83;
		case 76:  return 45;
		case 77:  return 46;
		case 79:  return 17;
		case 80:  return 49;
		case 81:  return 50;
		case 91:  return 52;
		case 93:  return 55;
		case 95:  return 166;
		case 96:  return 169;
		case 97:  return 163;
		case 98:  return 165;
		case 99:  return 179;
		case 102: return 174;
		case 104: return 176;
		case 105: return 146;
		case 106: return 144;
		case 107: return 175;
		case 108: return 177;
		case 109: return 154;
		case 110: return 173;
		case 112: return 157;
		case 115: return 158;
		case 116: return 159;
		case 117: return 147;
		case 118: return 148;
		case 119: return 149;
		case 120: return 150;
		case 121: return 155;
		case 124: return 156;
		case 127: return 136;
		case 128: return 137;
		case 129: return 138;
		case 130: return 133;
		case 131: return 132;
		case 137: return 43;
		case 138: return 44;
		case 140: return 141;
		case 141: return 140;
		case 149: return 228;
		case 150: return 229;
		case 157: return 167;
		case 160: return 164;
		case 162: return 81;
		case 186: return 178;
		case 188: return 5;
		case 189: return 6;
		case 190: return 7;
		case 191: return 8;
		case 192: return 9;
		case 193: return 10;
		case 194: return 11;
		case 195: return 12;
		case 196: return 13;
		case 197: return 14;
		case 198: return 15;
		case 199: return 16;
		case 200: return 130;
		case 202: return 98;
		case 203: return 122;
		case 204: return 123;
		case 217: return 61;
		case 218: return 96;
		case 228: return 113;
		case 229: return 114;
		case 230: return 115;
		case 231: return 94;
		case 234: return 131;
		case 247: return 95;
		case 257: return 56;
		case 258: return 34;
		case 259: return 33;
		case 260: return 54;
		case 263: return 35;
		case 264: return 38;
		case 265: return 37;
		case 266: return 36;
		case 267: return 78;
		case 268: return 53;
		case 269: return 48;
		case 270: return 72;
		case 271: return 73;
		case 273: return 99;
		case 274: return 100;
		case 280: return 88;
		case 288: return 242;
		case 290: return 19;
		case 291: return 20;
		case 292: return 24;
		case 293: return 59;
		case 295: return 69;
		case 296: return 70;
		case 297: return 240;
		case 302: return 261;
		case 316: return 276;
		case 318: return 278;
		case 319: return 279;
		case 326: return 285;
		case 332: return 291;
		case 424: return 424;
		case 434: return 434;
		default:  return -1;
	}
}

static int copy_guest_memory(void* local, void* guest, size_t len, bool to_guest)
{
	if (len == 0)
		return 0;
	if (!local || !guest)
		return -EFAULT;

	struct iovec local_iov = {
		.iov_base = local,
		.iov_len = len,
	};
	struct iovec remote_iov = {
		.iov_base = guest,
		.iov_len = len,
	};

	ssize_t n = to_guest
		? process_vm_writev(getpid(), &local_iov, 1, &remote_iov, 1, 0)
		: process_vm_readv(getpid(), &local_iov, 1, &remote_iov, 1, 0);

	if (n < 0 || (size_t)n != len) {
		return (n >= 0 || errno == EFAULT) ? -EFAULT : -errno;
	}
	return 0;
}

int fex_copy_to_guest(void* guest, const void* src, size_t len)
{
	return copy_guest_memory((void*)src, guest, len, true);
}

int fex_copy_from_guest(void* dst, const void* guest, size_t len)
{
	return copy_guest_memory(dst, (void*)guest, len, false);
}

static long copy_host_stat_to_guest(const struct arm64_kernel_stat* host_st, uintptr_t guest_addr)
{
	struct x86_64_linux_stat guest_st;
	convert_host_stat_to_x86_64(host_st, &guest_st);
	return fex_copy_to_guest((void*)guest_addr, &guest_st, sizeof(guest_st));
}

// Guest signal actions. The guest's rt_sigaction cannot go to the host kernel: the x86_64
// struct sigaction has a sa_restorer field the host's lacks, and a guest handler is x86_64 code
// that the kernel would run natively. A handler installed that way also replaces the host
// SIGSEGV/SIGBUS/... handlers that FEX's generated code depends on. So the guest's actions are
// kept in a table and the host only sees a handler that notes the signal, which is enough to
// interrupt a blocked host syscall with EINTR. Delivering a signal to a guest handler is not
// implemented.
struct x86_64_ksigaction {
	uint64_t handler;
	uint64_t flags;
	uint64_t restorer;
	uint64_t mask;
};

#define GUEST_SIG_DFL 0
#define GUEST_SIG_IGN 1
#define GUEST_NSIG 65

static struct x86_64_ksigaction g_guest_sigactions[GUEST_NSIG];
static pthread_mutex_t g_guest_sigactions_lock = PTHREAD_MUTEX_INITIALIZER;

static void guest_signal_noted(int sig)
{
	(void)sig;
}

// Faults raised by the generated code itself: the host keeps its own handler for them.
static bool signal_owned_by_jit(int sig)
{
	return sig == SIGILL || sig == SIGTRAP || sig == SIGBUS || sig == SIGFPE || sig == SIGSEGV;
}

// glibc keeps the signals between 31 and SIGRTMIN for its own use and refuses to install handlers for
// them, so the guest's request is only recorded in the table.
static bool signal_reserved_by_libc(int sig)
{
	return sig > 31 && sig < SIGRTMIN;
}

static long guest_rt_sigaction(long sig, uintptr_t new_action, uintptr_t old_action, long sigsetsize)
{
	if (sig <= 0 || sig >= GUEST_NSIG || (new_action && (sig == SIGKILL || sig == SIGSTOP)))
		return -EINVAL;
	if (sigsetsize != sizeof(uint64_t))
		return -EINVAL;

	struct x86_64_ksigaction requested;
	if (new_action) {
		long rc = fex_copy_from_guest(&requested, (void*)new_action, sizeof(requested));
		if (rc != 0)
			return rc;
	}

	pthread_mutex_lock(&g_guest_sigactions_lock);
	struct x86_64_ksigaction previous = g_guest_sigactions[sig];
	long ret = 0;
	if (new_action) {
		if (!signal_owned_by_jit((int)sig) && !signal_reserved_by_libc((int)sig)) {
			struct sigaction host;
			memset(&host, 0, sizeof(host));
			if (requested.handler == GUEST_SIG_DFL) {
				host.sa_handler = SIG_DFL;
			} else if (requested.handler == GUEST_SIG_IGN) {
				host.sa_handler = SIG_IGN;
			} else {
				host.sa_handler = guest_signal_noted;
				host.sa_flags = (int)(requested.flags & (SA_RESTART | SA_NOCLDSTOP | SA_NOCLDWAIT));
				memcpy(&host.sa_mask, &requested.mask, sizeof(requested.mask));
			}
			if (sigaction((int)sig, &host, NULL) != 0)
				ret = -errno;
		}
		if (ret == 0)
			g_guest_sigactions[sig] = requested;
	}
	pthread_mutex_unlock(&g_guest_sigactions_lock);

	if (ret == 0 && old_action)
		ret = fex_copy_to_guest((void*)old_action, &previous, sizeof(previous));
	return ret;
}

// The guest may block signals, but the host's SIGBUS/SIGSEGV/... handlers must stay deliverable: a
// synchronous fault in generated code with its signal blocked kills the process instead of being
// patched, so those signals are dropped from every mask the guest sets.
static long guest_rt_sigprocmask(long how, uintptr_t new_set, uintptr_t old_set, long sigsetsize)
{
	if (sigsetsize != sizeof(uint64_t))
		return -EINVAL;

	sigset_t host_new;
	sigset_t host_old;
	sigemptyset(&host_new);
	if (new_set) {
		uint64_t mask;
		long rc = fex_copy_from_guest(&mask, (void*)new_set, sizeof(mask));
		if (rc != 0)
			return rc;
		for (int sig = 1; sig < GUEST_NSIG; sig++) {
			if ((mask & (1ull << (sig - 1))) && !signal_owned_by_jit(sig))
				sigaddset(&host_new, sig);
		}
	}

	int err = pthread_sigmask((int)how, new_set ? &host_new : NULL, &host_old);
	if (err != 0)
		return -err;

	if (old_set) {
		uint64_t mask = 0;
		for (int sig = 1; sig < GUEST_NSIG; sig++) {
			if (sigismember(&host_old, sig) == 1)
				mask |= 1ull << (sig - 1);
		}
		return fex_copy_to_guest((void*)old_set, &mask, sizeof(mask));
	}
	return 0;
}

// x86_64 and arm64 Linux number four of the open flags differently. The guest uses the x86_64 values.
#define X86_O_DIRECT     0040000L
#define X86_O_LARGEFILE  0100000L
#define X86_O_DIRECTORY  0200000L
#define X86_O_NOFOLLOW   0400000L
#define ARM64_O_DIRECTORY 0040000L
#define ARM64_O_NOFOLLOW  0100000L
#define ARM64_O_DIRECT    0200000L
#define ARM64_O_LARGEFILE 0400000L
#define OPEN_FLAGS_DIFFERENT (X86_O_DIRECT | X86_O_LARGEFILE | X86_O_DIRECTORY | X86_O_NOFOLLOW)

static long open_flags_x86_to_host(long flags)
{
	long out = flags & ~OPEN_FLAGS_DIFFERENT;
	if (flags & X86_O_DIRECT)
		out |= ARM64_O_DIRECT;
	if (flags & X86_O_LARGEFILE)
		out |= ARM64_O_LARGEFILE;
	if (flags & X86_O_DIRECTORY)
		out |= ARM64_O_DIRECTORY;
	if (flags & X86_O_NOFOLLOW)
		out |= ARM64_O_NOFOLLOW;
	return out;
}

static long open_flags_host_to_x86(long flags)
{
	long out = flags & ~OPEN_FLAGS_DIFFERENT;
	if (flags & ARM64_O_DIRECT)
		out |= X86_O_DIRECT;
	if (flags & ARM64_O_LARGEFILE)
		out |= X86_O_LARGEFILE;
	if (flags & ARM64_O_DIRECTORY)
		out |= X86_O_DIRECTORY;
	if (flags & ARM64_O_NOFOLLOW)
		out |= X86_O_NOFOLLOW;
	return out;
}

long fex_linux_syscall_dispatch(struct fex_x86_64_state* state)
{
	if (!state)
		return -EINVAL;

	uint64_t nr = state->rax;

	if (nr >= DARLING_FEX_ELFCALL_SYSCALL_BASE &&
	    nr < DARLING_FEX_ELFCALL_SYSCALL_BASE + DARLING_FEX_ELFCALL_COUNT) {
		return fex_elfcalls_dispatch((uint32_t)(nr - DARLING_FEX_ELFCALL_SYSCALL_BASE), state);
	}

	if (nr >= DARLING_FEX_DLSYM_SYSCALL_BASE &&
	    nr < DARLING_FEX_DLSYM_SYSCALL_BASE + DARLING_FEX_DLSYM_MAX_SLOTS) {
		return fex_dlsym_dispatch((uint32_t)(nr - DARLING_FEX_DLSYM_SYSCALL_BASE), state);
	}

	long a1 = (long)state->rdi;
	long a2 = (long)state->rsi;
	long a3 = (long)state->rdx;
	long a4 = (long)state->r10;
	long a5 = (long)state->r8;
	long a6 = (long)state->r9;
	long ret = -ENOSYS;

	switch (nr) {
		case 13: // rt_sigaction
			ret = guest_rt_sigaction(a1, (uintptr_t)a2, (uintptr_t)a3, a4);
			break;

		case 14: // rt_sigprocmask
			ret = guest_rt_sigprocmask(a1, (uintptr_t)a2, (uintptr_t)a3, a4);
			break;

		case 158: // arch_prctl
			if (a1 == X86_ARCH_SET_GS) {
				state->gs_base = (uint64_t)a2;
				fex_set_current_thread_gs_base((uint64_t)a2);
				ret = 0;
			} else if (a1 == X86_ARCH_GET_GS) {
				ret = fex_copy_to_guest((void*)(uintptr_t)a2, &state->gs_base, sizeof(state->gs_base));
			} else if (a1 == X86_ARCH_SET_FS) {
				state->fs_base = (uint64_t)a2;
				ret = 0;
			} else if (a1 == X86_ARCH_GET_FS) {
				ret = fex_copy_to_guest((void*)(uintptr_t)a2, &state->fs_base, sizeof(state->fs_base));
			} else {
				ret = -EINVAL;
			}
			break;

		case 2: // open -> openat(AT_FDCWD, path, flags, mode)
			ret = raw_host_syscall(56, AT_FDCWD, a1, open_flags_x86_to_host(a2), a3, 0, 0);
			break;
		case 257: // openat
			ret = raw_host_syscall(56, a1, a2, open_flags_x86_to_host(a3), a4, 0, 0);
			break;
		case 72: // fcntl: the file status flags use the host numbering
			if (a2 == F_SETFL) {
				ret = raw_host_syscall(25, a1, a2, open_flags_x86_to_host(a3), 0, 0, 0);
			} else {
				ret = raw_host_syscall(25, a1, a2, a3, 0, 0, 0);
				if (a2 == F_GETFL && ret >= 0)
					ret = open_flags_host_to_x86(ret);
			}
			break;
		case 85: // creat -> openat(AT_FDCWD, path, O_CREAT|O_WRONLY|O_TRUNC, mode)
			ret = raw_host_syscall(56, AT_FDCWD, a1, O_CREAT | O_WRONLY | O_TRUNC, a2, 0, 0);
			break;
		case 21: // access -> faccessat(AT_FDCWD, path, mode, 0)
			ret = raw_host_syscall(48, AT_FDCWD, a1, a2, 0, 0, 0);
			break;
		case 22: // pipe -> pipe2(fds, 0)
			ret = raw_host_syscall(59, a1, 0, 0, 0, 0, 0);
			break;
		case 33: // dup2(oldfd, newfd)
			if (a1 == a2) {
				long check = raw_host_syscall(25, a1, F_GETFD, 0, 0, 0, 0);
				ret = (check < 0) ? -EBADF : a1;
			} else {
				ret = raw_host_syscall(24, a1, a2, 0, 0, 0, 0);
			}
			break;
		case 7: { // poll(fds, nfds, timeout_ms) -> ppoll
			struct timespec ts;
			struct timespec* tsp = NULL;
			int timeout_ms = (int)a3;
			if (timeout_ms >= 0) {
				ts.tv_sec = timeout_ms / 1000;
				ts.tv_nsec = (long)(timeout_ms % 1000) * 1000000L;
				tsp = &ts;
			}
			ret = raw_host_syscall(73, a1, a2, (long)(uintptr_t)tsp, 0, 0, 0);
			break;
		}
		case 4: // stat(path, statbuf)
		case 6: { // lstat(path, statbuf)
			struct arm64_kernel_stat host_st;
			int flags = (nr == 6) ? AT_SYMLINK_NOFOLLOW : 0;
			ret = raw_host_syscall(79, AT_FDCWD, a1, (long)(uintptr_t)&host_st, flags, 0, 0);
			if (ret == 0)
				ret = copy_host_stat_to_guest(&host_st, (uintptr_t)a2);
			break;
		}
		case 5: { // fstat(fd, statbuf)
			struct arm64_kernel_stat host_st;
			ret = raw_host_syscall(80, a1, (long)(uintptr_t)&host_st, 0, 0, 0, 0);
			if (ret == 0)
				ret = copy_host_stat_to_guest(&host_st, (uintptr_t)a2);
			break;
		}
		case 262: { // newfstatat(dfd, path, statbuf, flag)
			struct arm64_kernel_stat host_st;
			ret = raw_host_syscall(79, a1, a2, (long)(uintptr_t)&host_st, a4, 0, 0);
			if (ret == 0)
				ret = copy_host_stat_to_guest(&host_st, (uintptr_t)a3);
			break;
		}
		case 82: // rename -> renameat2(AT_FDCWD, old, AT_FDCWD, new, 0)
			ret = raw_host_syscall(276, AT_FDCWD, a1, AT_FDCWD, a2, 0, 0);
			break;
		case 83: // mkdir -> mkdirat(AT_FDCWD, path, mode)
			ret = raw_host_syscall(34, AT_FDCWD, a1, a2, 0, 0, 0);
			break;
		case 84: // rmdir -> unlinkat(AT_FDCWD, path, AT_REMOVEDIR)
			ret = raw_host_syscall(35, AT_FDCWD, a1, AT_REMOVEDIR, 0, 0, 0);
			break;
		case 86: // link -> linkat(AT_FDCWD, old, AT_FDCWD, new, 0)
			ret = raw_host_syscall(37, AT_FDCWD, a1, AT_FDCWD, a2, 0, 0);
			break;
		case 87: // unlink -> unlinkat(AT_FDCWD, path, 0)
			ret = raw_host_syscall(35, AT_FDCWD, a1, 0, 0, 0, 0);
			break;
		case 88: // symlink -> symlinkat(target, AT_FDCWD, linkpath)
			ret = raw_host_syscall(36, a1, AT_FDCWD, a2, 0, 0, 0);
			break;
		case 89: // readlink -> readlinkat(AT_FDCWD, path, buf, bufsiz)
			ret = raw_host_syscall(78, AT_FDCWD, a1, a2, a3, 0, 0);
			break;
		case 90: // chmod -> fchmodat(AT_FDCWD, path, mode, 0)
			ret = raw_host_syscall(53, AT_FDCWD, a1, a2, 0, 0, 0);
			break;
		case 92: // chown -> fchownat(AT_FDCWD, path, owner, group, 0)
			ret = raw_host_syscall(54, AT_FDCWD, a1, a2, a3, 0, 0);
			break;
		case 94: // lchown -> fchownat(AT_FDCWD, path, owner, group, AT_SYMLINK_NOFOLLOW)
			ret = raw_host_syscall(54, AT_FDCWD, a1, a2, a3, AT_SYMLINK_NOFOLLOW, 0);
			break;
		case 213: // epoll_create(size) -> epoll_create1(0)
			ret = (a1 <= 0) ? -EINVAL : raw_host_syscall(20, 0, 0, 0, 0, 0, 0);
			break;
		case 233: { // epoll_ctl(epfd, op, fd, event)
			struct epoll_event host_ev;
			struct epoll_event* host_ev_ptr = NULL;
			if (a2 != EPOLL_CTL_DEL) {
				struct x86_64_epoll_event guest_ev;
				int copy_rc = fex_copy_from_guest(&guest_ev, (const void*)(uintptr_t)a4, sizeof(guest_ev));
				if (copy_rc != 0) {
					ret = copy_rc;
					break;
				}
				host_ev.events = guest_ev.events;
				host_ev.data.u64 = guest_ev.data;
				host_ev_ptr = &host_ev;
			}
			ret = raw_host_syscall(21, a1, a2, a3, (long)(uintptr_t)host_ev_ptr, 0, 0);
			break;
		}
		case 56:  // clone
		case 435: // clone3
			// Thread creation in Darling is handled via the darling_thread_create ELF call
			// (bsdthread_create), and process fork via native_fork. Direct host clone/clone3
			// from emulated code cannot safely initialize FEX execution context or Darling
			// Mach thread state. Return -ENOSYS so callers fail cleanly rather than creating
			// unmanaged host threads or corrupted state.
			ret = -ENOSYS;
			break;
		case 232: // epoll_wait
		case 281: { // epoll_pwait
			int maxevents = (int)a3;
			if (maxevents <= 0) {
				ret = -EINVAL;
				break;
			}
			struct epoll_event stack_evs[64];
			struct epoll_event* host_evs = stack_evs;
			size_t alloc_sz = 0;
			if (maxevents > 64) {
				alloc_sz = (size_t)maxevents * sizeof(struct epoll_event);
				host_evs = mmap(NULL, alloc_sz, PROT_READ | PROT_WRITE,
					MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
				if (host_evs == MAP_FAILED) {
					ret = -ENOMEM;
					break;
				}
			}
			long sigmask_arg = (nr == 281) ? a5 : 0;
			long sigsetsize_arg = (nr == 281) ? a6 : (long)sizeof(sigset_t);
			ret = raw_host_syscall(22, a1, (long)(uintptr_t)host_evs, maxevents,
				a4, sigmask_arg, sigsetsize_arg);
			if (ret > 0) {
				struct x86_64_epoll_event stack_guest_evs[64];
				struct x86_64_epoll_event* guest_evs = stack_guest_evs;
				size_t guest_alloc_sz = 0;
				if (ret > 64) {
					guest_alloc_sz = (size_t)ret * sizeof(struct x86_64_epoll_event);
					guest_evs = mmap(NULL, guest_alloc_sz, PROT_READ | PROT_WRITE,
						MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
					if (guest_evs == MAP_FAILED) {
						if (alloc_sz > 0)
							munmap(host_evs, alloc_sz);
						ret = -ENOMEM;
						break;
					}
				}
				for (long i = 0; i < ret; i++) {
					guest_evs[i].events = host_evs[i].events;
					guest_evs[i].data = host_evs[i].data.u64;
				}
				int copy_rc = fex_copy_to_guest((void*)(uintptr_t)a2, guest_evs,
					(size_t)ret * sizeof(struct x86_64_epoll_event));
				if (guest_alloc_sz > 0)
					munmap(guest_evs, guest_alloc_sz);
				if (copy_rc != 0)
					ret = copy_rc;
			}
			if (alloc_sz > 0)
				munmap(host_evs, alloc_sz);
			break;
		}
		default: {
			int arm64_nr = map_x86_64_nr_to_arm64(nr);
			if (arm64_nr >= 0)
				ret = raw_host_syscall(arm64_nr, a1, a2, a3, a4, a5, a6);
			else
				ret = -ENOSYS;
			break;
		}
	}

	state->rax = (uint64_t)ret;
	state->unwind_thread = fex_elfcalls_has_pending_terminate() ? 1 : 0;
	return ret;
}

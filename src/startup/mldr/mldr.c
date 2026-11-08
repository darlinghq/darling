/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <dlfcn.h>
#include <endian.h>
#include "commpage.h"
#include "loader.h"
#include "socket_bitmap.h"
#include <sys/resource.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <darlingserver/rpc.h>
#include <darlingserver/rpc-supplement.h>
#include <sys/ptrace.h>
#include <pthread.h>
#include <sys/utsname.h>
#include <signal.h>
#include <kern/cs_blobs.h>

#ifndef PAGE_SIZE
#	define PAGE_SIZE	4096
#endif
#define PAGE_ALIGN(x) (x & ~(PAGE_SIZE-1))

static const char* dyld_path = INSTALL_PREFIX "/libexec/darling/usr/lib/dyld";

struct sockaddr_un __dserver_socket_address_data = {
	.sun_family = AF_UNIX,
	.sun_path = "\0",
};

int __dserver_main_thread_socket_fd = -1;
int __dserver_process_lifetime_pipe_fd = -1;

// The idea of mldr is to load dyld_path into memory and set up the stack
// as described in dyldStartup.S.
// After that, we pass control over to dyld.
//
// Additionally, mldr providers access to native platforms libdl.so APIs (ELF loader).

#if defined(__x86_64__) || defined(__aarch64__)
static void load64(int fd, bool expect_dylinker, struct load_results* lr);
#endif
#ifdef __x86_64__
static void reexec32(char** argv);
#endif
static void load32(int fd, bool expect_dylinker, struct load_results* lr);
static void load_fat(int fd, cpu_type_t cpu, bool expect_dylinker, char** argv, struct load_results* lr);
static void load(const char* path, cpu_type_t cpu, bool expect_dylinker, char** argv, struct load_results* lr);
static int native_prot(int prot);
static void setup_space(struct load_results* lr, bool is_64_bit);
static int code_signing_gate(int fd, uint64_t image_offset, struct load_results* lr);
static void process_special_env(struct load_results* lr);
static void start_thread(struct load_results* lr);
static bool is_kernel_at_least(int major, int minor);
static void* compatible_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
#if defined(__x86_64__) || defined(__aarch64__)
static void setup_stack64(const char* filepath, struct load_results* lr);
#endif
static void setup_stack32(const char* filepath, struct load_results* lr);

// this is called when argv[0] specifies an interpreter and we need to "unexpand" it (i.e. convert it from a Linux path to a vchrooted path)
static void vchroot_unexpand_interpreter(struct load_results* lr);

// UUID of the main executable
uint8_t exe_uuid[16];

// globally visible for debugging/core-dumping purposes
// however, this should not be relied on; a pointer to this should passed around to whoever needs the load_results structure
__attribute__((used))
struct load_results mldr_load_results = {0};

static uint32_t stack_size = 0;

// the path of the executable being started, for messages
static const char* exec_path = NULL;

#ifdef __i386__
// how many times mldr32 executed itself again to place the top of the address space
#define LAYOUT_RETRY_ENV "__mldr_layout_retry"
#define LAYOUT_RETRY_MAX 8
#endif

static const char* const skip_env_vars[] = {
	"__mldr_bprefs=",
	"__mldr_sockpath=",
	"__mldr_lifetime_pipe",
	"__mldr_nofile=",
};

void* __mldr_main_stack_top = NULL;

static int kernel_major = -1;
static int kernel_minor = -1;

static unsigned int inherited_nofile[2];
static bool have_inherited_nofile = false;

int main(int argc, char** argv, char** envp)
{
	void** sp;
	int pushCount = 0;
	char *filename, *p = NULL;
	size_t arg_strings_total_size_after = 0;
	size_t orig_argv0_len = 0;
	const char* orig_argv1 = NULL;

	mldr_load_results.kernfd = -1;
	mldr_load_results.argc = argc;
	mldr_load_results.argv = argv;

	while (envp[mldr_load_results.envc] != NULL) {
		++mldr_load_results.envc;
	}
	mldr_load_results.envp = envp;

	// sys_execve() passes the original file path appended to the mldr path in argv[0].
	if (argc > 0)
		p = strchr(argv[0], '!');

	if (argc <= 1)
	{
		if (p == NULL) {
			fprintf(stderr, "mldr is part of Darling. It is not to be executed directly.\n");
			return 1;
		}
		else
		{
			fprintf(stderr, "mldr: warning: Executing with no argv[0]. Continuing anyway, but this is probably a bug.\n");
		}
	}

	if (p != NULL)
	{
		filename = (char*) __builtin_alloca(strlen(argv[0])+1);
		strcpy(filename, p + 1);
	}
	else
	{
		filename = (char*) __builtin_alloca(strlen(argv[1])+1);
		strcpy(filename, argv[1]);
	}

	// allow any process to ptrace us
	// the only process we really care about being able to do this is the server,
	// but we can't just use the server's PID, since it lies outside our PID namespace.
	prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY, 0, 0, 0);

	process_special_env(&mldr_load_results);

	exec_path = filename;

#ifdef __i386__
	load(filename, CPU_TYPE_X86, false, argv, &mldr_load_results); // accept i386 only
#else
	load(filename, 0, false, argv, &mldr_load_results);
#endif

	// this was previously necessary when we were loading the binary from the LKM
	// (presumably because the break was detected incorrectly)
	// but this shouldn't be necessary for loading Mach-O's from userspace (the heap space should already be set up properly).
	// see https://github.com/darlinghq/darling/issues/469 for the issue this originally fixed in the LKM
#if 0
	if (prctl(PR_SET_MM, PR_SET_MM_BRK, PAGE_ALIGN(mldr_load_results.vm_addr_max), 0, 0) < 0) {
		fprintf(stderr, "Failed to set BRK value\n");
		return 1;
	}

	if (prctl(PR_SET_MM, PR_SET_MM_START_BRK, PAGE_ALIGN(mldr_load_results.vm_addr_max), 0, 0) < 0) {
		fprintf(stderr, "Failed to set BRK start\n");
		return 1;
	}
#endif

	// adjust argv (remove mldr's argv[0])
	// NOTE: this code assumes that the current argv array points to contiguous strings.
	//       this is not necessarily true, although AFAIK this is always true on Linux.
	// also note: we do it this way (moving the string contents in addition to the pointers)
	//            so that Linux sees our modified argv array without having to use PR_SET_MM_ARG_START
	//            and PR_SET_MM_ARG_END (since those require CAP_SYS_RESOURCE)

	--mldr_load_results.argc;

	orig_argv0_len = strlen(mldr_load_results.argv[0]) + 1;
	orig_argv1 = mldr_load_results.argv[1];

	for (size_t i = 0; i < mldr_load_results.argc; ++i) {
		mldr_load_results.argv[i] = mldr_load_results.argv[0] + arg_strings_total_size_after;
		arg_strings_total_size_after += strlen(mldr_load_results.argv[i + 1]) + 1;
	}
	mldr_load_results.argv[mldr_load_results.argc] = NULL;

	memmove(mldr_load_results.argv[0], orig_argv1, arg_strings_total_size_after);
	memset(mldr_load_results.argv[0] + arg_strings_total_size_after, 0, orig_argv0_len);

	if (p == NULL) {
		vchroot_unexpand_interpreter(&mldr_load_results);
	}

	// adjust envp (remove special mldr variables)
	// NOTE: same as for argv; here we assume the envp strings are contiguous
	for (size_t i = 0; i < mldr_load_results.envc; ++i) {
		if (!mldr_load_results.envp[i]) {
			mldr_load_results.envc = i;
			break;
		}

		size_t len = strlen(mldr_load_results.envp[i]) + 1;

		#define ENV_VAR_MATCHES(_name) \
			(len > sizeof(_name) - 1 && strncmp(mldr_load_results.envp[i], _name, sizeof(_name) - 1) == 0)

		// Don't pass these special env vars down to userland
		if (
			ENV_VAR_MATCHES("__mldr_bprefs=")   ||
			ENV_VAR_MATCHES("__mldr_sockpath=") ||
			ENV_VAR_MATCHES("__mldr_nofile=")   ||
			ENV_VAR_MATCHES("__mldr_layout_retry=")
		) {
			size_t len_after = 0;
			const char* orig_envp_i_plus_one = mldr_load_results.envp[i + 1];

			--mldr_load_results.envc;

			for (size_t j = i; j < mldr_load_results.envc; ++j) {
				mldr_load_results.envp[j] = mldr_load_results.envp[i] + len_after;
				len_after += strlen(mldr_load_results.envp[j + 1]) + 1;
			}
			mldr_load_results.envp[mldr_load_results.envc] = NULL;

			memmove(mldr_load_results.envp[i], orig_envp_i_plus_one, len_after);
			memset(mldr_load_results.envp[i] + len_after, 0, len);

			// we have to check this index again because it now points to a different string
			--i;
			continue;
		}
		// If we were passed __mldr_DYLD_ROOT_PATH, it is a special case of DYLD_ROOT_PATH needing to be set,
		// so we remove the prefix, so dyld reads it as DYLD_ROOT_PATH
		else if (ENV_VAR_MATCHES("__mldr_DYLD_ROOT_PATH=")) {
			const char* env_p = mldr_load_results.envp[i];
			env_p += (sizeof("__mldr_") - 1);
			size_t len_remaining = strlen(env_p) + 1;
			memmove(mldr_load_results.envp[i], env_p, len_remaining);
		}
	}

	if (mldr_load_results._32on64)
		setup_stack32(filename, &mldr_load_results);
	else
#if defined(__x86_64__) || defined(__aarch64__)
		setup_stack64(filename, &mldr_load_results);
#else
		abort();
#endif

	int status = dserver_rpc_set_dyld_info(mldr_load_results.dyld_all_image_location, mldr_load_results.dyld_all_image_size);
	if (status < 0) {
		fprintf(stderr, "Failed to tell darlingserver about our dyld info\n");
		exit(1);
	}

	if (dserver_rpc_set_executable_path(filename, strlen(filename)) < 0) {
		fprintf(stderr, "Failed to tell darlingserver about our executable path\n");
		exit(1);
	}

	__mldr_main_stack_top = (void*)mldr_load_results.stack_top;

	start_thread(&mldr_load_results);

	__builtin_unreachable();
}

void load(const char* path, cpu_type_t forced_arch, bool expect_dylinker, char** argv, struct load_results* lr)
{
	int fd;
	uint32_t magic;

	fd = open(path, O_RDONLY);
	if (fd == -1)
	{
		fprintf(stderr, "Cannot open %s: %s\n", path, strerror(errno));
		exit(1);
	}

	// We need to read argv[1] and detect whether it's a 32 or 64-bit application.
	// Then load the appropriate version of dyld from the fat file.
	// In case the to-be-executed executable contains both, we prefer the 64-bit version,
	// unless a special property has been passed to sys_posix_spawn() to force the 32-bit
	// version. See posix_spawnattr_setbinpref_np().

	if (read(fd, &magic, sizeof(magic)) != sizeof(magic))
	{
		fprintf(stderr, "Cannot read the file header of %s.\n", path);
		exit(1);
	}

	if (magic == MH_MAGIC_64 || magic == MH_CIGAM_64)
	{
#if defined(__x86_64__) || defined(__aarch64__)
		lseek(fd, 0, SEEK_SET);
		load64(fd, expect_dylinker, lr);
#else
		abort();
#endif
	}
	else if (magic == MH_MAGIC || magic == MH_CIGAM)
	{
#if !__x86_64__
		lseek(fd, 0, SEEK_SET);
		load32(fd, expect_dylinker, lr);
#else
		// Re-run self as mldr32
		reexec32(argv);
#endif
	}
	else if (magic == FAT_MAGIC || magic == FAT_CIGAM)
	{
		lseek(fd, 0, SEEK_SET);
		load_fat(fd, forced_arch, expect_dylinker, argv, lr);
	}
	else
	{
		fprintf(stderr, "Unknown file format: %s.\n", path);
		exit(1);
	}

	close(fd);
}

static void load_fat(int fd, cpu_type_t forced_arch, bool expect_dylinker, char** argv, struct load_results* lr) {
	struct fat_header fhdr;
	struct fat_arch best_arch = {0};
	int bpref_index = -1;

	best_arch.cputype = CPU_TYPE_ANY;

	if (read(fd, &fhdr, sizeof(fhdr)) != sizeof(fhdr))
	{
		fprintf(stderr, "Cannot read fat file header.\n");
		exit(1);
	}

	const bool swap = fhdr.magic == FAT_CIGAM;

#define SWAP32(x) x = __bswap_32(x)

	if (swap)
		SWAP32(fhdr.nfat_arch);

	uint32_t i;
	for (i = 0; i < fhdr.nfat_arch; i++)
	{
		struct fat_arch arch;

		if (read(fd, &arch, sizeof(arch)) != sizeof(arch))
		{
			fprintf(stderr, "Cannot read fat_arch header.\n");
			exit(1);
		}

		if (swap)
		{
			SWAP32(arch.cputype);
			SWAP32(arch.cpusubtype);
			SWAP32(arch.offset);
			SWAP32(arch.size);
			SWAP32(arch.align);
		}

		if (!forced_arch)
		{
			int j;
			for (j = 0; j < 4; j++)
			{
				if (lr->bprefs[j] && arch.cputype == lr->bprefs[j])
				{
					if (bpref_index == -1 || bpref_index > j)
					{
						best_arch = arch;
						bpref_index = j;
						break;
					}
				}
			}

			if (bpref_index == -1)
			{
#if defined(__x86_64__)
				if (arch.cputype == CPU_TYPE_X86_64)
					best_arch = arch;
				else if (best_arch.cputype == CPU_TYPE_ANY && arch.cputype == CPU_TYPE_X86)
					best_arch = arch;
#elif defined(__i386__)
				if (arch.cputype == CPU_TYPE_X86)
					best_arch = arch;
#elif defined(__aarch64__)
				if (arch.cputype == CPU_TYPE_ARM64)
					best_arch = arch;
				else if (best_arch.cputype == CPU_TYPE_ANY && arch.cputype == CPU_TYPE_ARM)
					best_arch = arch;
#else
	#error Unsupported CPU architecture
#endif
			}
		}
		else
		{
			if (arch.cputype == forced_arch)
				best_arch = arch;
		}
	}

	if (best_arch.cputype == CPU_TYPE_ANY)
	{
		fprintf(stderr, "No supported architecture found in fat binary.\n");
		exit(1);
	}

	if (lseek(fd, best_arch.offset, SEEK_SET) == -1)
	{
		fprintf(stderr, "Cannot seek to selected arch in fat binary.\n");
		exit(1);
	}

	if (best_arch.cputype & CPU_ARCH_ABI64) {
#if defined(__x86_64__) || defined(__aarch64__)
		load64(fd, expect_dylinker, lr);
#else
		abort();
#endif
	} else {
#if !__x86_64__
		load32(fd, expect_dylinker, lr);
#else
		// Re-run self as mldr32
		reexec32(argv);
#endif
	}
};

#if defined(__x86_64__) || defined(__aarch64__)
#define GEN_64BIT
#include "loader.c"
#include "stack.c"
#undef GEN_64BIT
#endif

#define GEN_32BIT
#include "loader.c"
#include "stack.c"
#undef GEN_32BIT

int native_prot(int prot)
{
	int protOut = 0;

	if (prot & VM_PROT_READ)
		protOut |= PROT_READ;
	if (prot & VM_PROT_WRITE)
		protOut |= PROT_WRITE;
	if (prot & VM_PROT_EXECUTE)
		protOut |= PROT_EXEC;

	return protOut;
}

static void reexec32(char** argv)
{
	char selfpath[1024];
	ssize_t len;

	len = readlink("/proc/self/exe", selfpath, sizeof(selfpath)-3);
	if (len == -1)
	{
		perror("Cannot readlink /proc/self/exe");
		abort();
	}

	selfpath[len] = '\0';
	strcat(selfpath, "32");

	execv(selfpath, argv);

	perror("Cannot re-execute as 32-bit process");
	abort();
}

// Given that there's no proper way of passing special parameters to the binary loader
// via execve(), we must do this via env variables
static void process_special_env(struct load_results* lr) {
	const char* str;
	static char root_path[4096];

	lr->bprefs[0] = lr->bprefs[1] = lr->bprefs[2] = lr->bprefs[3] = 0;
	str = getenv("__mldr_bprefs");

	if (str != NULL) {
		sscanf(str, "%x,%x,%x,%x", &lr->bprefs[0], &lr->bprefs[1], &lr->bprefs[2], &lr->bprefs[3]);
	}

	str = getenv("__mldr_sockpath");

	if (str != NULL) {
		if (strlen(str) > sizeof(__dserver_socket_address_data.sun_path) - 1) {
			fprintf(stderr, "darlingserver socket path is too long\n");
			exit(1);
		}
		strncpy(__dserver_socket_address_data.sun_path, str, sizeof(__dserver_socket_address_data.sun_path) - 1);
		__dserver_socket_address_data.sun_path[sizeof(__dserver_socket_address_data.sun_path) - 1] = '\0';

		lr->socket_path = __dserver_socket_address_data.sun_path;
	}

	lr->lifetime_pipe = -1;
	str = getenv("__mldr_lifetime_pipe");

	if (str != NULL) {
		sscanf(str, "%i", &lr->lifetime_pipe);
	}

	// the Darwin RLIMIT_NOFILE of the process that exec'd us; libsystem_kernel
	// keeps it apart from the Linux limit, which must stay above our RPC sockets
	str = getenv("__mldr_nofile");

	if (str != NULL) {
		unsigned int cur, max;
		if (sscanf(str, "%u,%u", &cur, &max) == 2 && cur <= max) {
			inherited_nofile[0] = cur;
			inherited_nofile[1] = max;
			have_inherited_nofile = true;
		}
	}

	str = getenv("__mldr_DYLD_ROOT_PATH");

	if (str != NULL && lr->root_path == NULL) {
		strncpy(root_path, str, sizeof(root_path) - 1);
		root_path[sizeof(root_path) - 1] = '\0';
		lr->root_path = root_path;
		lr->root_path_length = strlen(lr->root_path);
	}
};

static void unset_special_env() {
	unsetenv("__mldr_bprefs");
	unsetenv("__mldr_sockpath");
	unsetenv("__mldr_lifetime_pipe");
	unsetenv("__mldr_nofile");
#ifdef __i386__
	unsetenv(LAYOUT_RETRY_ENV);
#endif
};

int __mldr_inherited_nofile_limit(unsigned int limits[2]) {
	if (!have_inherited_nofile) {
		return -1;
	}

	limits[0] = inherited_nofile[0];
	limits[1] = inherited_nofile[1];
	return 0;
};

static socket_bitmap_t socket_bitmap = {
	.mutex = PTHREAD_MUTEX_INITIALIZER,
	.next_index = 0,
	.bits = NULL,
	.bit_length = 0,
	.highest = -1,
};

void __mldr_socket_bitmap_prefork(void) {
	socket_bitmap_fork_prepare(&socket_bitmap);
}

void __mldr_socket_bitmap_postfork_parent(void) {
	socket_bitmap_fork_parent(&socket_bitmap);
}

void __mldr_socket_bitmap_postfork_child(void) {
	socket_bitmap_fork_child(&socket_bitmap);
}

int __mldr_create_rpc_socket(void) {
	int pre_fd = -1;
	int fd = -1;

	pre_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (pre_fd < 0) {
		goto err_out;
	}

	fd = socket_bitmap_place(&socket_bitmap, pre_fd);
	if (fd < 0) {
		goto err_out;
	}

	close(pre_fd);
	pre_fd = -1;

	int fd_flags = fcntl(fd, F_GETFD);
	if (fd_flags < 0) {
		goto err_out;
	}
	if (fcntl(fd, F_SETFD, fd_flags | FD_CLOEXEC) < 0) {
		goto err_out;
	}

	sa_family_t family = AF_UNIX;
	if (bind(fd, (const struct sockaddr*)&family, sizeof(family)) < 0) {
		goto err_out;
	}

out:
	return fd;

err_out:
	if (fd >= 0) {
		socket_bitmap_put(&socket_bitmap, fd);
		close(fd);
	}

	if (pre_fd >= 0) {
		close(pre_fd);
	}

	return -1;
};

void __mldr_close_rpc_socket(int socket) {
	close(socket);
	socket_bitmap_put(&socket_bitmap, socket);
};

int __mldr_create_process_lifetime_pipe(int* fds) {
	// These pipes are not required for Linux 5.3 or newer,
	// we already have pidfd_open.
	if (is_kernel_at_least(5, 3)) {
		fds[0] = fds[1] = -1;
		return 0;
	}

	int pre_fds[2];
	if (pipe(pre_fds) == -1) {
		goto err_out;
	}

	for (int i = 0; i < 2; ++i) {
		fds[i] = socket_bitmap_place(&socket_bitmap, pre_fds[i]);
		if (fds[i] < 0) {
			goto err_out;
		}

		close(pre_fds[i]);
		pre_fds[i] = -1;
	}

	return 0;

err_out:
	for (int i = 0; i < 2; ++i) {
		if (fds[i] >= 0) {
			socket_bitmap_put(&socket_bitmap, fds[i]);
			close(fds[i]);
		}

		if (pre_fds[i] >= 0) {
			close(pre_fds[i]);
		}
	}

	return -1;
}

void __mldr_close_process_lifetime_pipe(int fd) {
	if (fd != -1) {
		close(fd);
		socket_bitmap_put(&socket_bitmap, fd);
	}
}

#ifdef __i386__
// A 32-bit process gets its Linux stack (and the mappings next to it) at a
// random place just below the top of the address space, which is where the
// Darwin commpage and the main thread stack below it have to go. If that range
// is taken, execute ourselves again to get another placement.
static void ensure_top_range_free(struct load_results* lr, unsigned long stack_size) {
	unsigned long start = commpage_address(false) - stack_size;
	unsigned long length = commpage_length(false) + stack_size;

	void* probe = compatible_mmap((void*)start, length, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
	if (probe != MAP_FAILED) {
		munmap(probe, length);
		if (probe == (void*)start)
			return;
	} else if (errno != EEXIST) {
		return; // let the mappings below report the error
	}

	const char* str = getenv(LAYOUT_RETRY_ENV);
	int retries = str ? atoi(str) : 0;
	if (retries >= LAYOUT_RETRY_MAX)
		return; // the mappings below report the conflict

	char selfpath[1024];
	ssize_t len = readlink("/proc/self/exe", selfpath, sizeof(selfpath) - 1);
	if (len == -1)
		return;
	selfpath[len] = '\0';

	char count[16];
	snprintf(count, sizeof(count), "%d", retries + 1);
	setenv(LAYOUT_RETRY_ENV, count, 1);

	execv(selfpath, lr->argv);
	unsetenv(LAYOUT_RETRY_ENV);
}
#endif

static void setup_space(struct load_results* lr, bool is_64_bit) {
	struct rlimit limit;
	getrlimit(RLIMIT_STACK, &limit);
	// allocate a few pages 16 pages if it's less than the limit; otherwise, allocate the limit
	unsigned long size = PAGE_SIZE * 16;
	if (limit.rlim_cur != RLIM_INFINITY && limit.rlim_cur < size) {
		size = limit.rlim_cur;
	}

#ifdef __i386__
	ensure_top_range_free(lr, size);
#endif

	commpage_setup(is_64_bit);

	// Using the default stack top would cause the stack to be placed just above the commpage
	// and would collide with it eventually.
	// Instead, we manually allocate a new stack below the commpage.
#if __x86_64__ || __aarch64__
	lr->stack_top = commpage_address(true);
#elif __i386__
	lr->stack_top = commpage_address(false);
#else
	#error Unsupported architecture
#endif

	if (compatible_mmap((void*)(lr->stack_top - size), size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE | MAP_GROWSDOWN, -1, 0) == MAP_FAILED) {
		fprintf(stderr, "Failed to allocate stack of %lu bytes: %d (%s)\n", size, errno, strerror(errno));
		exit(1);
	}

	unset_special_env();

	lr->kernfd = __mldr_create_rpc_socket();
	if (lr->kernfd < 0) {
		fprintf(stderr, "Failed to create socket\n");
		exit(1);
	}

	__dserver_main_thread_socket_fd = lr->kernfd;

	int lifetime_pipe[2];

	// this process is created using exec from another Darling process.
	// darlingserver should already have the read pipe, so we don't need
	// to check that in.
	if (lr->lifetime_pipe != -1) {
		lifetime_pipe[1] = socket_bitmap_get(&socket_bitmap);

		if (lr->lifetime_pipe != lifetime_pipe[1]) {
			// move the existing pipe to a higher fd number, and invalidate
			// the old fd to prevent interfering with fds provided by
			// socket_bitmap_get
			if (dup2(lr->lifetime_pipe, lifetime_pipe[1]) == -1) {
				fprintf(stderr, "Failed to dup process lifetime pipe: %d (%s)\n", errno, strerror(errno));
				exit(1);
			}
			close(lr->lifetime_pipe);
		}

		lifetime_pipe[0] = -1;
	} else {
		if (__mldr_create_process_lifetime_pipe(lifetime_pipe) == -1) {
			fprintf(stderr, "Failed to create process lifetime pipe: %d (%s)\n", errno, strerror(errno));
			exit(1);
		}
	}

	lr->lifetime_pipe = lifetime_pipe[1];

	// store the write end of the pipe; the read end is sent to darlingserver.
	__dserver_process_lifetime_pipe_fd = lifetime_pipe[1];

	int dummy_stack_variable;
	if (dserver_rpc_checkin(false, &dummy_stack_variable, lifetime_pipe[0]) < 0) {
		fprintf(stderr, "Failed to checkin with darlingserver\n");
		exit(1);
	}

	// keep our write end while closing the unused read end.
	__mldr_close_process_lifetime_pipe(lifetime_pipe[0]);

	if (!lr->root_path) {
		static char vchroot_buffer[4096];
		uint64_t vchroot_path_length = 0;

		int code = dserver_rpc_vchroot_path(vchroot_buffer, sizeof(vchroot_buffer), &vchroot_path_length);
		if (code < 0) {
			fprintf(stderr, "Failed to retrieve vchroot path from darlingserver: %d\n", code);
			exit(1);
		}

		if (vchroot_path_length >= sizeof(vchroot_buffer)) {
			fprintf(stderr, "Vchroot path is too large for buffer\n");
			exit(1);
		} else if (vchroot_path_length > 0) {
			lr->root_path = vchroot_buffer;
			lr->root_path_length = vchroot_path_length;
		}
	}
};

// The code signing state darlingserver recorded for this process; false when
// it cannot be read.
static bool own_code_signing_state(dserver_cs_state_t* state) {
	return dserver_rpc_cs_get_state(0, state, sizeof(*state)) == 0;
}

// What is wrong with the registered image's signature, or NULL if nothing is.
static const char* code_signing_problem(void) {
	dserver_cs_state_t state;

	if (!own_code_signing_state(&state))
		return "could not be checked";

	switch (state.status) {
		case dserver_cs_status_valid: return NULL;
		case dserver_cs_status_unsigned: return "is missing";
		case dserver_cs_status_invalid: return "is not valid";
		default: return "could not be checked";
	}
}

// Refuses to start the executable. On macOS, a program whose code signature
// is rejected at launch does not make exec fail with an errno: the new process
// is killed with SIGKILL (crash reports name the CODESIGNING termination
// namespace, see Apple's "Understanding the exception types in a crash
// report"). By the time mldr runs, the Linux exec has happened as well, so
// mldr does the same, before anything of the image is mapped. The message is
// Darling's addition; macOS logs the reason to the system log instead.
static void __attribute__((noreturn)) refuse_executable(const char* problem) {
	fprintf(stderr, "mldr: %s: refusing to run it: its code signature %s\n",
		exec_path ? exec_path : "(unknown)", problem);
	fflush(stderr);
	kill(getpid(), SIGKILL);
	_exit(1);
}

// Hands the executable about to be mapped (the slice at `image_offset`) to
// darlingserver, which reads its code signature and records it as the
// process's signing state, the state csops() reports. This runs on every exec,
// after the check-in and before anything is mapped. What happens next depends
// on the process's code signing policy (`CodeSigningPolicy` in the
// org.darlinghq.security domain, as darlingserver recorded it when the
// process started):
//   - off (the default): nothing; the result is not looked at;
//   - warn: an image that `enforce` would refuse is reported on stderr, and
//     runs;
//   - enforce: an image darlingserver does not report as validly signed is
//     not started (refuse_executable()).
// An unknown policy value is treated as `enforce`. If darlingserver cannot be
// asked for the policy, it is taken to be `off`, as in dyld.
// Returns the descriptor to map the image from.
static int code_signing_gate(int fd, uint64_t image_offset, struct load_results* lr) {
	dserver_cs_state_t state;
	uint32_t policy = dserver_code_signing_policy_off;
	uint32_t flags = 0;
	int code;

	(void) lr;

	if (own_code_signing_state(&state))
		policy = state.code_signing_policy;
	if (policy > dserver_code_signing_policy_enforce)
		policy = dserver_code_signing_policy_enforce;

	code = dserver_rpc_cs_register_image(fd, image_offset, &flags);

	if (policy == dserver_code_signing_policy_enforce) {
		// under enforce, darlingserver reports CS_VALID only for a valid signature
		if (code < 0)
			refuse_executable("could not be checked");
		if (!(flags & CS_VALID)) {
			const char* problem = code_signing_problem();
			refuse_executable(problem ? problem : "is not valid");
		}
	} else if (policy == dserver_code_signing_policy_warn) {
		// without enforce, darlingserver reports CS_VALID for every image, so the status tells
		const char* problem = (code < 0) ? "could not be checked" : code_signing_problem();
		if (problem) {
			fprintf(stderr, "mldr: warning: %s: its code signature %s; "
				"code signing enforcement would refuse to run it\n",
				exec_path ? exec_path : "(unknown)", problem);
		}
	}

	return fd;
}

static void start_thread(struct load_results* lr) {
#ifdef __x86_64__
	__asm__ volatile(
		"mov %1, %%rsp\n"
		"jmpq *%0"
		::
		"m"(lr->entry_point),
		"r"(lr->stack_top)
		:
	);
#elif defined(__i386__)
	__asm__ volatile(
		"mov %1, %%esp\n"
		"jmp *%0"
		::
		"m"(lr->entry_point),
		"r"(lr->stack_top)
		:
	);
#elif defined(__aarch64__)
	__asm__ volatile(
		"mov sp, %1\n"
		"br %0"
		::
		"r"(lr->entry_point),
		"r"(lr->stack_top)
		:
	);
#elif defined(__arm__)
	__asm__ volatile(
		"mov sp, %1\n"
		"bx %0"
		::
		"r"(lr->entry_point),
		"r"(lr->stack_top)
		:
	);
#else
#       error Unsupported platform!
#endif
};

static bool is_kernel_at_least(int major, int minor) {
	if (kernel_major == -1) {
		struct utsname uname_info;
		if (uname(&uname_info) == -1) {
			return false;
		}
		kernel_major = 0;
		kernel_minor = 0;
		size_t pos = 0;
		while (uname_info.release[pos] != '\0' && uname_info.release[pos] != '.') {
			kernel_major = kernel_major * 10 + uname_info.release[pos] - '0';
			++pos;
		}
		++pos;
		while (uname_info.release[pos] != '\0' && uname_info.release[pos] != '.') {
			kernel_minor = kernel_minor * 10 + uname_info.release[pos] - '0';
			++pos;
		}
	}

	if (major != kernel_major) {
		return kernel_major > major;
	}

	return kernel_minor >= minor;
}

void* compatible_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
	// MAP_FIXED_NOREPLACE is not supported on WSL1 (Linux < 4.17).
	bool fixed_noreplace_hack = false;
	if ((flags & MAP_FIXED_NOREPLACE) && !is_kernel_at_least(4, 17)) {
		flags &= ~MAP_FIXED_NOREPLACE;
		fixed_noreplace_hack = true;
	}
	void* result = mmap(addr, length, prot, flags, fd, offset);
	// MAP_GROWSDOWN is not supported on WSL1. See https://github.com/microsoft/WSL/issues/8095.
	if ((result == (void*)MAP_FAILED) && (flags & MAP_GROWSDOWN) && (errno == EOPNOTSUPP)) {
		result = mmap(addr, length, prot, (flags & ~MAP_GROWSDOWN), fd, offset);
	}
	if (fixed_noreplace_hack) {
		if (result != addr && result != (void*)MAP_FAILED) {
			errno = ESRCH;
			munmap(addr, length);
			return MAP_FAILED;
		}
	}
	return result;
}

static void vchroot_unexpand_interpreter(struct load_results* lr) {
	static char unexpanded[4096];
	size_t length;

	if (lr->root_path) {
		length = strlen(lr->argv[0]);

		if (strncmp(lr->argv[0], lr->root_path, lr->root_path_length) == 0) {
			memmove(unexpanded, lr->argv[0] + lr->root_path_length, length - lr->root_path_length + 1);
		} else {
			if (length + sizeof(SYSTEM_ROOT) > sizeof(unexpanded)) {
				fprintf(stderr, "Interpreter path was too big\n");
				exit(1);
			}
			memmove(unexpanded + sizeof(SYSTEM_ROOT) - 1, lr->argv[0], length + 1);
			memcpy(unexpanded, SYSTEM_ROOT, sizeof(SYSTEM_ROOT) - 1);
		}

		lr->argv[0] = unexpanded;
	}
};

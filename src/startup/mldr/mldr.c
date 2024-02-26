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
#include <sys/resource.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <darlingserver/rpc.h>
#include <sys/ptrace.h>
#include <pthread.h>
#include <sys/utsname.h>

#ifndef PAGE_SIZE
#	define PAGE_SIZE	4096
#endif
#define PAGE_ALIGN(x) (x & ~(PAGE_SIZE-1))

static const char* dyld_path = INSTALL_PREFIX "/libexec/usr/lib/dyld";

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

#ifdef __x86_64__
static void load64(int fd, bool expect_dylinker, struct load_results* lr);
static void reexec32(char** argv);
#endif
static void load32(int fd, bool expect_dylinker, struct load_results* lr);
static void load_fat(int fd, cpu_type_t cpu, bool expect_dylinker, char** argv, struct load_results* lr);
static void load(const char* path, cpu_type_t cpu, bool expect_dylinker, char** argv, struct load_results* lr);
static int native_prot(int prot);
static void setup_space(struct load_results* lr, bool is_64_bit);
static void process_special_env(struct load_results* lr);
static void start_thread(struct load_results* lr);
static bool is_kernel_at_least(int major, int minor);
static void* compatible_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
#ifdef __x86_64__
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

static const char* const skip_env_vars[] = {
	"__mldr_bprefs=",
	"__mldr_sockpath=",
	"__mldr_lifetime_pipe",
};

void* __mldr_main_stack_top = NULL;

static int kernel_major = -1;
static int kernel_minor = -1;

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
	ptrace(PR_SET_PTRACER, PR_SET_PTRACER_ANY, 0, 0, 0);

	process_special_env(&mldr_load_results);

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

		// Don't pass these special env vars down to userland
		#define SKIP_VAR(_name) \
			(len > sizeof(_name) - 1 && strncmp(mldr_load_results.envp[i], _name, sizeof(_name) - 1) == 0)

		if (
			SKIP_VAR("__mldr_bprefs=")   ||
			SKIP_VAR("__mldr_sockpath=")
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
	}

	if (mldr_load_results._32on64)
		setup_stack32(filename, &mldr_load_results);
	else
#ifdef __x86_64__
		setup_stack64(filename, &mldr_load_results);
#elif __aarch64__
	#error TODO: aarch64
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
#ifdef __x86_64__
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
#elif defined (__aarch64__)
	#error TODO: arm
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
#ifdef __x86_64__
		load64(fd, expect_dylinker, lr);
#elif __aarch64__
	#error TODO: aarch64
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

#ifdef __x86_64__
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

	str = getenv("DARLING_DYLD_ROOT_PATH");

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
};

typedef struct socket_bitmap {
	pthread_mutex_t mutex;
	/**
	 * This is always next lowest available index.
	 * If this is equal to #bit_length, then the bitmap is full.
	 */
	size_t next_index;
	uint8_t* bits;
	size_t bit_length;
	int highest;
} socket_bitmap_t;

static socket_bitmap_t socket_bitmap = {
	.mutex = PTHREAD_MUTEX_INITIALIZER,
	.next_index = 0,
	.bits = NULL,
	.bit_length = 0,
	.highest = -1,
};

static int socket_bitmap_get(socket_bitmap_t* bitmap) {
	int fd = -1;
	bool updated = false;

	pthread_mutex_lock(&bitmap->mutex);

	if (bitmap->highest == -1) {
		// we need to initialize this bitmap
		struct rlimit limit;

		if (getrlimit(RLIMIT_NOFILE, &limit) < 0) {
			goto out;
		}

		if (limit.rlim_cur == RLIM_INFINITY) {
			// just default to 1024
			limit.rlim_cur = 1024;
		}

		bitmap->highest = limit.rlim_cur - 1;
	}

	if (bitmap->next_index >= bitmap->bit_length) {
		// we need to grow the bitmap

		if ((bitmap->bit_length % 8) == 0) {
			// we need to allocate an additional byte

			void* ptr = realloc(bitmap->bits, (bitmap->bit_length / 8) + 1);
			if (!ptr) {
				goto out;
			}

			bitmap->bits = ptr;

			bitmap->bits[bitmap->bit_length / 8] = 0;
		} else {
			// we just need to increment the bit length
		}

		++bitmap->bit_length;
	}

	fd = bitmap->highest - bitmap->next_index;

	bitmap->bits[bitmap->next_index / 8] |= 1 << (bitmap->next_index % 8);

	// update the next available index
	for (size_t i = bitmap->next_index + 1; i < bitmap->bit_length; ++i) {
		size_t byte = i / 8;
		uint8_t bit = i % 8;

		if (bit == 0) {
			// check the entire byte at once so we can avoid unnecessary iteration
			if (bitmap->bits[byte] == 0xff) {
				// this byte is full, skip it
				i += 7;
				continue;
			}
		}

		if ((bitmap->bits[byte] & (1 << bit)) == 0) {
			// this index is unused
			bitmap->next_index = i;
			updated = true;
			break;
		}
	}

	if (!updated) {
		// all of our entries are currently in-use
		bitmap->next_index = bitmap->bit_length;
	}

out:
	pthread_mutex_unlock(&bitmap->mutex);

	return fd;
};

static void socket_bitmap_put(socket_bitmap_t* bitmap, int socket) {
	size_t index;

	pthread_mutex_lock(&bitmap->mutex);

	index = bitmap->highest - socket;

	bitmap->bits[index / 8] &= ~(1 << (index % 8));

	if (index < bitmap->next_index) {
		bitmap->next_index = index;
	}

	if (index == bitmap->bit_length - 1) {
		// we can shrink the bitmap
		size_t old_byte_size = (bitmap->bit_length + 7) / 8;
		size_t new_byte_size = old_byte_size;

		while (bitmap->bit_length > 0) {
			size_t index = bitmap->bit_length - 1;

			if ((bitmap->bit_length % 8) == 0) {
				// check the entire byte at once to avoid unnecessary iteration
				if (bitmap->bits[(bitmap->bit_length / 8) - 1] == 0) {
					// remove this entire byte
					bitmap->bit_length -= 8;
					continue;
				}
			}

			if ((bitmap->bits[index / 8] & (1 << (index % 8))) == 0) {
				// this bit is in-use, so we can't shrink any further
				break;
			}

			--bitmap->bit_length;
		}

		new_byte_size = (bitmap->bit_length + 7) / 8;

		if (old_byte_size != new_byte_size) {
			// we can free one or more bytes from the bitmap
			void* ptr = realloc(bitmap->bits, new_byte_size);
			if (!ptr) {
				goto out;
			}

			bitmap->bits = ptr;
		}
	}

out:
	pthread_mutex_unlock(&bitmap->mutex);
};

int __mldr_create_rpc_socket(void) {
	int pre_fd = -1;
	int fd = -1;

	pre_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (pre_fd < 0) {
		goto err_out;
	}

	fd = socket_bitmap_get(&socket_bitmap);
	if (fd < 0) {
		goto err_out;
	}

	if (dup2(pre_fd, fd) < 0) {
		// we have to put it away ourselves here because `fd` is not yet valid, so we can't close() it in the error handler
		socket_bitmap_put(&socket_bitmap, fd);
		fd = -1;
		goto err_out;
	}

	close(pre_fd);
	pre_fd = -1;

	// `fd` now contains the socket with the desired FD number returned by `socket_bitmap_get`

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
		fds[i] = socket_bitmap_get(&socket_bitmap);
		if (fds[i] < 0) {
			goto err_out;
		}

		if (dup2(pre_fds[i], fds[i]) < 0) {
			socket_bitmap_put(&socket_bitmap, fds[i]);
			fds[i] = -1;
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

static void setup_space(struct load_results* lr, bool is_64_bit) {
	commpage_setup(is_64_bit);

	// Using the default stack top would cause the stack to be placed just above the commpage
	// and would collide with it eventually.
	// Instead, we manually allocate a new stack below the commpage.
#if __x86_64__
	lr->stack_top = commpage_address(true);
#elif __i386__
	lr->stack_top = commpage_address(false);
#else
	#error Unsupported architecture
#endif

	struct rlimit limit;
	getrlimit(RLIMIT_STACK, &limit);
	// allocate a few pages 16 pages if it's less than the limit; otherwise, allocate the limit
	unsigned long size = PAGE_SIZE * 16;
	if (limit.rlim_cur != RLIM_INFINITY && limit.rlim_cur < size) {
		size = limit.rlim_cur;
	}

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
			// FIXME: potential buffer overflow
			memmove(unexpanded + sizeof(SYSTEM_ROOT) - 1, lr->argv[0], length + 1);
			memcpy(unexpanded, SYSTEM_ROOT, sizeof(SYSTEM_ROOT) - 1);
		}

		lr->argv[0] = unexpanded;
	}
};

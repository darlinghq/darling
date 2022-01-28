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
#ifdef __x86_64__
static void setup_stack64(const char* filepath, struct load_results* lr);
#endif
static void setup_stack32(const char* filepath, struct load_results* lr);

// UUID of the main executable
uint8_t exe_uuid[16];

// globally visible for debugging/core-dumping purposes
// however, this should not be relied on; a pointer to this should passed around to whoever needs the load_results structure
__attribute__((used))
struct load_results mldr_load_results = {0};

static uint32_t stack_size = 0;

int main(int argc, char** argv, char** envp)
{
	void** sp;
	int pushCount = 0;
	char *filename, *p = NULL;

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

	if (prctl(PR_SET_MM, PR_SET_MM_START_STACK, mldr_load_results.stack_top, 0, 0) < 0) {
		fprintf(stderr, "Failed to set stack start\n");
		return 1;
	}

	// adjust argv (remove mldr's argv[0])
	--mldr_load_results.argc;
	for (size_t i = 0; i < mldr_load_results.argc; ++i) {
		mldr_load_results.argv[i] = mldr_load_results.argv[i + 1];
	}
	mldr_load_results.argv[mldr_load_results.argc] = NULL;

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

	// TODO: tell darlingserver about our dyld info

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
		unsetenv("__mldr_bprefs");
	}

	str = getenv("__mldr_sockpath");

	if (str != NULL) {
		if (strlen(str) > sizeof(__dserver_socket_address_data.sun_path) - 1) {
			fprintf(stderr, "darlingserver socket path is too long\n");
			exit(1);
		}
		strncpy(__dserver_socket_address_data.sun_path, str, sizeof(__dserver_socket_address_data.sun_path) - 1);
		__dserver_socket_address_data.sun_path[sizeof(__dserver_socket_address_data.sun_path) - 1] = '\0';
		unsetenv("__mldr_sockpath");

		lr->socket_path = __dserver_socket_address_data.sun_path;
	}

	str = getenv("DYLD_ROOT_PATH");

	if (str != NULL && lr->root_path == NULL) {
		strncpy(root_path, str, sizeof(root_path) - 1);
		root_path[sizeof(root_path) - 1] = '\0';
		lr->root_path = root_path;
	}
};

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

	if (mmap((void*)(lr->stack_top - size), size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE | MAP_GROWSDOWN, -1, 0) == MAP_FAILED) {
		fprintf(stderr, "Failed to allocate stack of %lu bytes: %d (%s)\n", size, errno, strerror(errno));
		exit(1);
	}

	process_special_env(lr);

	lr->kernfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (lr->kernfd < 0) {
		fprintf(stderr, "Failed to create socket\n");
		exit(1);
	}

	int fd_flags = fcntl(lr->kernfd, F_GETFD);
	if (fd_flags < 0) {
		fprintf(stderr, "Failed to read socket FD flags\n");
		exit(1);
	}
	if (fcntl(lr->kernfd, F_SETFD, fd_flags | FD_CLOEXEC) < 0) {
		fprintf(stderr, "Failed to set close-on-exec flag on socket FD\n");
		exit(1);
	}

	sa_family_t family = AF_UNIX;
	if (bind(lr->kernfd, (const struct sockaddr*)&family, sizeof(family)) < 0) {
		fprintf(stderr, "Failed to autobind socket\n");
		exit(1);
	}

	__dserver_main_thread_socket_fd = lr->kernfd;

	if (dserver_rpc_checkin(false) < 0) {
		fprintf(stderr, "Failed to checkin with darlingserver\n");
		exit(1);
	}

	static char vchroot_buffer[4096];
	uint64_t vchroot_path_length = 0;

	int code = dserver_rpc_vchroot_path(vchroot_buffer, sizeof(vchroot_buffer), &vchroot_path_length);
	if (code < 0) {
		fprintf(stderr, "Failed to retrieve vchroot path from darlingserver: %d\n", code);
		exit(1);
	}

	if (vchroot_path_length > 0) {
		lr->root_path = vchroot_buffer;
	} else if (vchroot_path_length >= sizeof(vchroot_buffer)) {
		fprintf(stderr, "Vchroot path is too large for buffer\n");
		exit(1);
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

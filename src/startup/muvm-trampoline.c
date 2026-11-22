/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

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

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "muvm-trampoline.h"

extern char** environ;

// The launcher target is built without the mach-o include directory mldr uses,
// so only the few header fields the decision needs are declared here.
#ifndef MH_MAGIC_64
#define MH_MAGIC_64 0xfeedfacfU
#define MH_CIGAM_64 0xcffaedfeU
#define MH_MAGIC 0xfeedfaceU
#define MH_CIGAM 0xcefaedfeU
#define FAT_MAGIC 0xcafebabeU
#define FAT_CIGAM 0xbebafecaU
#define FAT_MAGIC_64 0xcafebabfU
#define FAT_CIGAM_64 0xbfbafecaU

#define CPU_TYPE_ARM 12U
#define CPU_TYPE_X86 7U
#define CPU_ARCH_ABI64 0x01000000U
#define CPU_TYPE_ARM64 (CPU_TYPE_ARM | CPU_ARCH_ABI64)
#define CPU_TYPE_X86_64 (CPU_TYPE_X86 | CPU_ARCH_ABI64)
#endif

// Both mldr (MLDR_FAT_MAX_ARCHES) and the launcher cap universal binary
// architecture tables at 64 entries.
#define FAT_MAX_ARCHS 64U

// muvm passes only named variables into the guest; mldr needs the prefix and
// the architecture preference, and a GUI program needs its session. GDK_SCALE
// is the scale an X11 desktop exports for its clients (AppKit's X11 backend
// reads it, as X11 has no scale protocol), so the guest draws at the desktop's
// scale.
static const char* const kForwardedVariables[] =
{
	"HOME",
	"USER",
	"LOGNAME",
	"SHELL",
	"PATH",
	"TERM",
	"TMPDIR",
	"LANG",
	"LC_ALL",
	"XDG_RUNTIME_DIR",
	"WAYLAND_DISPLAY",
	"GDK_SCALE",
	"DPREFIX",
	"__mldr_bprefs",
};

static bool read_exact(int fd, void* buf, size_t len)
{
	uint8_t* p = (uint8_t*)buf;
	size_t total = 0;
	while (total < len)
	{
		ssize_t n = read(fd, p + total, len - total);
		if (n < 0)
		{
			if (errno == EINTR)
				continue;
			return false;
		}
		if (n == 0)
			return false;
		total += (size_t)n;
	}
	return true;
}

static uint32_t swap32(uint32_t value)
{
	return (value >> 24) | ((value >> 8) & 0xff00U) | ((value << 8) & 0xff0000U) | (value << 24);
}

static enum darling_target_slice sliceFromCpuType(uint32_t cputype)
{
	switch (cputype)
	{
		case CPU_TYPE_ARM64:
			return DARLING_TARGET_SLICE_ARM64;
		case CPU_TYPE_X86_64:
			return DARLING_TARGET_SLICE_X86_64;
		case CPU_TYPE_ARM:
			return DARLING_TARGET_SLICE_ARM32;
		default:
			return DARLING_TARGET_SLICE_NONE;
	}
}

enum darling_target_slice darling_target_slice_arch(const char* path, const uint32_t bprefs[4])
{
	uint8_t header[8];
	uint32_t magic;
	int fd;

	if (!path || !path[0])
		return DARLING_TARGET_SLICE_NONE;

	// The launcher is root at this point and the path comes from the user, so a
	// redirect to a FIFO must not block it and anything but a regular file is
	// left for mldr to reject.
	fd = open(path, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
	if (fd < 0)
		return DARLING_TARGET_SLICE_NONE;

	struct stat st;

	if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) || st.st_size < 8)
	{
		close(fd);
		return DARLING_TARGET_SLICE_NONE;
	}

	if (!read_exact(fd, header, sizeof(header)))
	{
		close(fd);
		return DARLING_TARGET_SLICE_NONE;
	}

	memcpy(&magic, header, sizeof(magic));

	if (magic == MH_CIGAM_64 || magic == MH_MAGIC_64)
	{
		close(fd);
		uint32_t cputype;

		memcpy(&cputype, header + 4, sizeof(cputype));
		if (magic == MH_CIGAM_64)
			cputype = swap32(cputype);
		return sliceFromCpuType(cputype);
	}

	bool swap = false;
	unsigned width = 0;

	if (magic == FAT_MAGIC)
	{
		swap = false;
		width = 20;
	}
	else if (magic == FAT_CIGAM)
	{
		swap = true;
		width = 20;
	}
	else if (magic == FAT_MAGIC_64)
	{
		swap = false;
		width = 32;
	}
	else if (magic == FAT_CIGAM_64)
	{
		swap = true;
		width = 32;
	}
	else
	{
		close(fd);
		return DARLING_TARGET_SLICE_NONE;
	}

	uint32_t nfat_arch;
	memcpy(&nfat_arch, header + 4, sizeof(nfat_arch));
	if (swap)
		nfat_arch = swap32(nfat_arch);

	// Both mldr and muvm-trampoline cap universal binaries at 64 architectures.
	// A file declaring 0 architectures, more than FAT_MAX_ARCHS, or a table
	// extending beyond the file size is malformed.
	if (nfat_arch == 0 || nfat_arch > FAT_MAX_ARCHS)
	{
		close(fd);
		return DARLING_TARGET_SLICE_NONE;
	}

	if ((uint64_t)8 + (uint64_t)nfat_arch * width > (uint64_t)st.st_size)
	{
		close(fd);
		return DARLING_TARGET_SLICE_NONE;
	}

	// Stream entries in small fixed-size chunks on the stack (O(1) memory).
	bool bprefs_found[4] = { false, false, false, false };
	bool found_arm64 = false;
	bool found_x86_64 = false;
	bool found_arm32 = false;

	uint8_t chunk[16 * 32];
	uint32_t remaining = nfat_arch;

	while (remaining > 0)
	{
		uint32_t count = remaining < 16 ? remaining : 16;
		size_t to_read = count * width;
		if (!read_exact(fd, chunk, to_read))
		{
			close(fd);
			return DARLING_TARGET_SLICE_NONE;
		}

		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t cputype;
			memcpy(&cputype, chunk + i * width, sizeof(cputype));
			if (swap)
				cputype = swap32(cputype);

			for (int j = 0; j < 4; j++)
			{
				if (bprefs && bprefs[j] && cputype == bprefs[j])
					bprefs_found[j] = true;
			}

			if (cputype == CPU_TYPE_ARM64)
				found_arm64 = true;
			else if (cputype == CPU_TYPE_X86_64)
				found_x86_64 = true;
			else if (cputype == CPU_TYPE_ARM)
				found_arm32 = true;
		}
		remaining -= count;
	}
	close(fd);

	// 1. Check user/environment architecture preferences (__mldr_bprefs).
	for (int j = 0; j < 4; j++)
	{
		if (bprefs && bprefs[j] && bprefs_found[j])
			return sliceFromCpuType(bprefs[j]);
	}

	// 2. Default preference: arm64, then x86_64, then 32-bit arm.
	if (found_arm64)
		return DARLING_TARGET_SLICE_ARM64;
	if (found_x86_64)
		return DARLING_TARGET_SLICE_X86_64;
	if (found_arm32)
		return DARLING_TARGET_SLICE_ARM32;

	return DARLING_TARGET_SLICE_NONE;
}

void darling_read_mldr_bprefs(uint32_t bprefs[4])
{
	const char* value = getenv("__mldr_bprefs");
	unsigned int entries[4] = { 0, 0, 0, 0 };
	int parsed;

	bprefs[0] = bprefs[1] = bprefs[2] = bprefs[3] = 0;

	if (value == NULL)
		return;

	parsed = sscanf(value, "%x,%x,%x,%x", &entries[0], &entries[1], &entries[2], &entries[3]);
	if (parsed < 1)
		return;

	// A preference shorter than four entries leaves the rest at zero, as mldr
	// reads it.
	for (int i = 0; i < parsed; i++)
		bprefs[i] = entries[i];
}

bool darling_muvm_needed(long page_size, enum darling_target_slice slice)
{
	return page_size > 4096 && slice == DARLING_TARGET_SLICE_X86_64;
}

bool darling_muvm_stdio_ok(int stdin_fd, int stdout_fd, int stderr_fd)
{
	return isatty(stdin_fd) && isatty(stdout_fd) && isatty(stderr_fd);
}

// An environ entry for name, or NULL when the variable is not set. An entry
// that does not match the whole name (PATHX for PATH) does not count.
static const char* environEntry(const char* name)
{
	size_t length = strlen(name);

	for (char** entry = environ; *entry != NULL; entry++)
	{
		if (strncmp(*entry, name, length) == 0 && (*entry)[length] == '=')
			return *entry;
	}

	return NULL;
}

static size_t countForwardedVariables(void)
{
	size_t count = 0;

	for (size_t i = 0; i < sizeof(kForwardedVariables) / sizeof(kForwardedVariables[0]); i++)
	{
		if (environEntry(kForwardedVariables[i]) != NULL)
			count++;
	}

	// Every DARLING_* variable the user set, for local paths the launcher's
	// configuration uses.
	for (char** entry = environ; *entry != NULL; entry++)
	{
		if (strncmp(*entry, "DARLING_", 8) == 0 && (*entry)[8] != '=')
			count++;
	}

	return count;
}

bool darling_wayland_display_is_plain(const char* wayland_display)
{
	return wayland_display != NULL && wayland_display[0] != '\0'
		&& strchr(wayland_display, '/') == NULL
		&& strcmp(wayland_display, ".") != 0 && strcmp(wayland_display, "..") != 0;
}

// The first executable regular file called program in a PATH value, as muvm
// searches. Returns a malloc'ed path or NULL.
static char* findInPath(const char* path_env, const char* program)
{
	size_t program_length = strlen(program);
	const char* start = path_env;

	while (start != NULL && *start != '\0')
	{
		const char* end = strchr(start, ':');
		size_t directory_length = end != NULL ? (size_t)(end - start) : strlen(start);
		struct stat st;
		char* candidate;

		// An empty element is the current directory in a shell, and muvm joins
		// it the same way, so it is searched as ".".
		candidate = malloc(directory_length + program_length + 3);
		if (candidate == NULL)
			return NULL;
		if (directory_length == 0)
			snprintf(candidate, directory_length + program_length + 3, "./%s", program);
		else
			snprintf(candidate, directory_length + program_length + 3, "%.*s/%s",
				(int) directory_length, start, program);

		if (stat(candidate, &st) == 0 && S_ISREG(st.st_mode) && access(candidate, X_OK) == 0)
			return candidate;

		free(candidate);
		start = end != NULL ? end + 1 : NULL;
	}

	return NULL;
}

// Whether the file contains needle, read in blocks that overlap by one needle
// less a byte so a match across a block boundary is found.
static bool fileContains(const char* path, const char* needle)
{
	static const size_t kBlock = 1 << 16;
	size_t needle_length = strlen(needle);
	size_t kept = 0;
	bool found = false;
	uint8_t* buffer;
	int fd;

	fd = open(path, O_RDONLY | O_CLOEXEC);
	if (fd < 0)
		return false;

	buffer = malloc(kBlock + needle_length);
	if (buffer == NULL)
	{
		close(fd);
		return false;
	}

	for (;;)
	{
		ssize_t got = read(fd, buffer + kept, kBlock);
		size_t total;

		if (got < 0 && errno == EINTR)
			continue;
		if (got <= 0)
			break;

		total = kept + (size_t) got;
		for (size_t at = 0; at + needle_length <= total; at++)
		{
			if (buffer[at] == (uint8_t) needle[0] && memcmp(buffer + at, needle, needle_length) == 0)
			{
				found = true;
				break;
			}
		}
		if (found)
			break;

		kept = needle_length - 1 < total ? needle_length - 1 : total;
		memmove(buffer, buffer + total - kept, kept);
	}

	free(buffer);
	close(fd);
	return found;
}

// The program muvm starts in the guest to serve the host's compositor.
#define WAYLAND_PROXY "wl-cross-domain-proxy"

bool darling_muvm_wayland_capable(const char* path_env)
{
	char* proxy = findInPath(path_env, WAYLAND_PROXY);
	char* guest;
	bool capable;

	if (proxy == NULL)
		return false;
	free(proxy);

	guest = findInPath(path_env, "muvm-guest");
	if (guest == NULL)
		return false;

	capable = fileContains(guest, WAYLAND_PROXY);
	free(guest);
	return capable;
}

bool darling_muvm_wayland_wanted(const char* wayland_display, const char* path_env,
	bool (*probe)(const char* path_env))
{
	return darling_wayland_display_is_plain(wayland_display) && probe(path_env);
}

bool darling_reexec_under_muvm(char* const argv[], size_t target_index, const char* target_path)
{
	static char launcher[4096];
	ssize_t launcher_length;
	size_t argc, forwarded, slots, at, name_bytes;
	char** muvm_argv;
	char* names;
	char* cursor;

	for (argc = 0; argv[argc] != NULL; argc++)
		;

	launcher_length = readlink("/proc/self/exe", launcher, sizeof(launcher) - 1);
	if (launcher_length <= 0 || (size_t) launcher_length >= sizeof(launcher) - 1)
		return false;
	launcher[launcher_length] = '\0';

	forwarded = countForwardedVariables();

	// muvm, one "-e NAME" pair per forwarded variable, "--", the launcher, every
	// argument but argv[0], and the null terminator.
	slots = 2 * forwarded + argc + 3;
	muvm_argv = calloc(slots, sizeof(*muvm_argv));
	if (muvm_argv == NULL)
		return false;

	// The DARLING_* names have to be split out of their "NAME=value" entries.
	name_bytes = 0;
	for (char** entry = environ; *entry != NULL; entry++)
	{
		if (strncmp(*entry, "DARLING_", 8) == 0 && (*entry)[8] != '=')
			name_bytes += strcspn(*entry, "=") + 1;
	}
	names = malloc(name_bytes ? name_bytes : 1);
	if (names == NULL)
	{
		free(muvm_argv);
		return false;
	}
	cursor = names;

	at = 0;
	muvm_argv[at++] = (char*) "muvm";

	for (size_t i = 0; i < sizeof(kForwardedVariables) / sizeof(kForwardedVariables[0]); i++)
	{
		if (environEntry(kForwardedVariables[i]) != NULL)
		{
			muvm_argv[at++] = (char*) "-e";
			muvm_argv[at++] = (char*) kForwardedVariables[i];
		}
	}

	for (char** entry = environ; *entry != NULL; entry++)
	{
		if (strncmp(*entry, "DARLING_", 8) == 0 && (*entry)[8] != '=')
		{
			size_t length = strcspn(*entry, "=");

			memcpy(cursor, *entry, length);
			cursor[length] = '\0';
			muvm_argv[at++] = (char*) "-e";
			muvm_argv[at++] = cursor;
			cursor += length + 1;
		}
	}

	// Everything between the launcher and the target is the subcommand, if any,
	// and stays as it is; only the target becomes the resolved absolute path.
	muvm_argv[at++] = (char*) "--";
	muvm_argv[at++] = launcher;

	for (size_t i = 1; i < target_index; i++)
		muvm_argv[at++] = argv[i];
	muvm_argv[at++] = (char*) target_path;

	for (size_t i = target_index + 1; i < argc; i++)
		muvm_argv[at++] = argv[i];

	muvm_argv[at] = NULL;

	execvp("muvm", muvm_argv);
	return false;
}

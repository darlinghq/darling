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

#include "dmabuf.h"
#include "linux_dmabuf.h"
#include <CoreFoundation/CoreFoundation.h>
#include <elfcalls.h>
#include <pthread.h>
#include <unistd.h>

extern struct elf_calls* _elfcalls;

// The defaults domain and key that turn DMA-BUF export off. The value is a
// boolean and defaults to true: surfaces are exportable whenever the host
// supports it. Read with the usual search list, so both
// /Library/Preferences/org.darlinghq.iosurface.plist and a user's own
// setting apply.
#define PREFERENCES_DOMAIN CFSTR("org.darlinghq.iosurface")
#define PREFERENCES_KEY_DMABUF CFSTR("DMABuf")

// memfd_create, fcntl and ioctl with Linux commands have no Darwin
// counterpart, so they are called in the host C library, the way other
// frameworks reach host libraries.
static int (*host_memfd_create)(const char* name, unsigned int flags);
static int (*host_fcntl)(int fd, int cmd, ...);
static int (*host_ioctl)(int fd, unsigned long request, ...);
static int (*host_open)(const char* path, int flags, ...);

// The udmabuf device, opened once per process.
static int g_udmabuf = -1;
static bool g_available;

static bool enabledInDefaults(void)
{
	bool enabled = true;
	CFPropertyListRef value = CFPreferencesCopyAppValue(PREFERENCES_KEY_DMABUF, PREFERENCES_DOMAIN);
	if (value != NULL)
	{
		if (CFGetTypeID(value) == CFBooleanGetTypeID())
			enabled = CFBooleanGetValue((CFBooleanRef) value);
		else if (CFGetTypeID(value) == CFNumberGetTypeID())
		{
			int number = 1;
			CFNumberGetValue((CFNumberRef) value, kCFNumberIntType, &number);
			enabled = number != 0;
		}
		CFRelease(value);
	}
	return enabled;
}

static void probe(void)
{
	if (!enabledInDefaults() || _elfcalls == NULL)
		return;

	void* libc = _elfcalls->dlopen("libc.so.6");
	if (libc == NULL)
		return;
	host_memfd_create = _elfcalls->dlsym(libc, "memfd_create");
	host_fcntl = _elfcalls->dlsym(libc, "fcntl");
	host_ioctl = _elfcalls->dlsym(libc, "ioctl");
	host_open = _elfcalls->dlsym(libc, "open");
	if (!host_memfd_create || !host_fcntl || !host_ioctl || !host_open)
		return;

	// The host path: the device is not part of the prefix.
	g_udmabuf = host_open("/dev/udmabuf", LINUX_O_RDWR | LINUX_O_CLOEXEC);
	g_available = g_udmabuf >= 0;
}

bool _IOSurfaceDMABufAvailable(void)
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;
	pthread_once(&once, probe);
	return g_available;
}

int _IOSurfaceDMABufCreateMemory(void)
{
	if (!_IOSurfaceDMABufAvailable())
		return -1;
	return host_memfd_create("IOSurface", LINUX_MFD_CLOEXEC | LINUX_MFD_ALLOW_SEALING);
}

int _IOSurfaceDMABufExport(int fd, uint64_t offset, uint64_t size)
{
	if (!_IOSurfaceDMABufAvailable() || fd < 0)
		return -1;

	// udmabuf only takes memory files whose size cannot shrink under it.
	// Sealing a file that already has the seal is harmless; one that is
	// not a sealable memory file fails here, and the surface stays
	// without a DMA-BUF.
	if (host_fcntl(fd, LINUX_F_ADD_SEALS, LINUX_F_SEAL_SHRINK) != 0)
		return -1;

	struct linux_udmabuf_create create = {
		.memfd = (uint32_t) fd,
		.flags = LINUX_UDMABUF_FLAGS_CLOEXEC,
		.offset = offset,
		.size = size,
	};
	int dmabuf = host_ioctl(g_udmabuf, LINUX_UDMABUF_CREATE, &create);
	return dmabuf >= 0 ? dmabuf : -1;
}

static void syncCPUAccess(int dmabuf, uint64_t flags)
{
	struct linux_dma_buf_sync sync = { .flags = flags };
	// The kernel asks to repeat the call when it was interrupted.
	for (int attempt = 0; attempt < 16; attempt++)
	{
		if (host_ioctl(dmabuf, LINUX_DMA_BUF_IOCTL_SYNC, &sync) == 0)
			return;
		int error = _elfcalls->get_errno();
		if (error != LINUX_EINTR && error != LINUX_EAGAIN)
			return;
	}
}

void _IOSurfaceDMABufBeginCPUAccess(int dmabuf, bool write)
{
	if (dmabuf >= 0)
		syncCPUAccess(dmabuf, LINUX_DMA_BUF_SYNC_START | (write ? LINUX_DMA_BUF_SYNC_RW : LINUX_DMA_BUF_SYNC_READ));
}

void _IOSurfaceDMABufEndCPUAccess(int dmabuf, bool write)
{
	if (dmabuf >= 0)
		syncCPUAccess(dmabuf, LINUX_DMA_BUF_SYNC_END | (write ? LINUX_DMA_BUF_SYNC_RW : LINUX_DMA_BUF_SYNC_READ));
}

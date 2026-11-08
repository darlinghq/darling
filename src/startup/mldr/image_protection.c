/*
This file is part of Darling.

Copyright (C) 2026 Darling Developers

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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE // memfd_create(), F_ADD_SEALS, mremap()
#endif
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#include "image_protection.h"

// Linux implements mseal(2) for 64-bit processes only. Its number is the same
// on x86_64 and aarch64; older headers lack it.
#if defined(__x86_64__) || defined(__aarch64__)
#	define MLDR_HAVE_MSEAL 1
#	ifndef __NR_mseal
#		define __NR_mseal 462
#	endif
#endif

int mldr_snapshot_image(int fd) {
	char buffer[16384];
	struct stat st;
	unsigned long long offset = 0;
	int snapshot, ret;

	if (fstat(fd, &st) != 0)
		return -errno;
	if (!S_ISREG(st.st_mode) || st.st_size <= 0)
		return -EINVAL;
	if ((unsigned long long)st.st_size > MLDR_SNAPSHOT_IMAGE_MAX)
		return -EFBIG;

	snapshot = memfd_create("darling-image-snapshot", MFD_CLOEXEC | MFD_ALLOW_SEALING);
	if (snapshot < 0)
		return -errno;

	while (offset < (unsigned long long)st.st_size) {
		unsigned long long left = (unsigned long long)st.st_size - offset;
		ssize_t got, written = 0;

		got = pread(fd, buffer, left < sizeof(buffer) ? (size_t)left : sizeof(buffer), (off_t)offset);
		if (got < 0 && errno == EINTR)
			continue;
		if (got <= 0) {
			// a read error, or the file became shorter while it was copied
			ret = (got < 0) ? -errno : -EIO;
			goto fail;
		}

		while (written < got) {
			ssize_t put = write(snapshot, buffer + written, (size_t)(got - written));
			if (put < 0 && errno == EINTR)
				continue;
			if (put <= 0) {
				ret = (put < 0) ? -errno : -EIO;
				goto fail;
			}
			written += put;
		}

		offset += (unsigned long long)got;
	}

	if (fcntl(snapshot, F_ADD_SEALS, F_SEAL_SEAL | F_SEAL_SHRINK | F_SEAL_GROW | F_SEAL_WRITE) != 0) {
		ret = -errno;
		goto fail;
	}

	if (lseek(snapshot, 0, SEEK_SET) != 0) {
		ret = -errno;
		goto fail;
	}

	return snapshot;

fail:
	close(snapshot);
	return ret;
}

int mldr_seal_range(void* addr, size_t len, int prot) {
	long page_size = sysconf(_SC_PAGESIZE);
	uintptr_t page_mask;

	if (page_size <= 0)
		return -EINVAL;
	page_mask = (uintptr_t)page_size - 1;

	if (len == 0 || ((uintptr_t)addr & page_mask) != 0 || len > SIZE_MAX - page_mask ||
			!(prot & PROT_READ) || (prot & PROT_WRITE))
		return -EINVAL;

#ifndef MLDR_HAVE_MSEAL
	// nothing is copied in a 32-bit process
	return -ENOSYS;
#else
	void* copy;
	int ret;

	len = (len + page_mask) & ~page_mask;

	copy = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (copy == MAP_FAILED)
		return -errno;

	memcpy(copy, addr, len);

	if (mprotect(copy, len, prot) != 0) {
		ret = -errno;
		goto fail;
	}

	// move the copy over the original mapping
	if (mremap(copy, len, len, MREMAP_MAYMOVE | MREMAP_FIXED, addr) == MAP_FAILED) {
		ret = -errno;
		goto fail;
	}

	if (syscall(__NR_mseal, addr, len, 0UL) != 0)
		return -errno;
	return 0;

fail:
	munmap(copy, len);
	return ret;
#endif
}

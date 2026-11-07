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

#ifndef IOSURFACE_DMABUF_H
#define IOSURFACE_DMABUF_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>

// Host support for surfaces whose memory can be exported as a Linux DMA-BUF.
//
// Such a surface lives in a sealable memory file (memfd) instead of a POSIX
// shared memory object. Its data pages are exported, when a GPU API asks for
// them, through the kernel's udmabuf device, which makes a DMA-BUF out of a
// range of a memfd without copying: the CPU keeps using its mapping of the
// memfd, and the GPU driver that imports the DMA-BUF sees the same pages.
// Another process that receives the memfd makes its own DMA-BUF from it.

__BEGIN_DECLS

// Whether this process backs new surfaces with memfds: the
// org.darlinghq.iosurface "DMABuf" default does not turn it off, the host
// C library has memfd_create, and /dev/udmabuf can be opened. Decided once.
__attribute__((visibility("hidden")))
bool _IOSurfaceDMABufAvailable(void);

// Creates an empty sealable memory file, or returns -1.
__attribute__((visibility("hidden")))
int _IOSurfaceDMABufCreateMemory(void);

// Makes a DMA-BUF of size bytes at offset in memory file fd, both multiples
// of the page size. Returns its descriptor, or -1 if fd is not a memory file
// that can be exported or the kernel refuses.
__attribute__((visibility("hidden")))
int _IOSurfaceDMABufExport(int fd, uint64_t offset, uint64_t size);

// Bracket CPU access to the memory of a DMA-BUF, so that the kernel can
// wait for and keep caches coherent with devices that use it.
__attribute__((visibility("hidden")))
void _IOSurfaceDMABufBeginCPUAccess(int dmabuf, bool write);
__attribute__((visibility("hidden")))
void _IOSurfaceDMABufEndCPUAccess(int dmabuf, bool write);

__END_DECLS

#endif

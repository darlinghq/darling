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

#ifndef IOSURFACE_LINUX_DMABUF_H
#define IOSURFACE_LINUX_DMABUF_H
#include <stdint.h>

// The parts of the Linux user-space ABI that IOSurface uses to export
// surface memory as a DMA-BUF. Darling code is built against the Darwin SDK,
// where the Linux headers cannot be included, so the values are spelled out
// here; they are stable kernel ABI.

// <linux/memfd.h>
#define LINUX_MFD_CLOEXEC 0x0001U
#define LINUX_MFD_ALLOW_SEALING 0x0002U

// <linux/fcntl.h>
#define LINUX_F_ADD_SEALS (1024 + 9)
#define LINUX_F_SEAL_SHRINK 0x0002
#define LINUX_O_RDWR 02
#define LINUX_O_CLOEXEC 02000000

// <asm-generic/errno-base.h>
#define LINUX_EINTR 4
#define LINUX_EAGAIN 11

// <asm-generic/ioctl.h>: a command that passes a structure to the kernel.
#define LINUX_IOW(type, nr, size) \
	((1U << 30) | ((uint32_t) (size) << 16) | ((uint32_t) (type) << 8) | (uint32_t) (nr))

// <linux/udmabuf.h>
struct linux_udmabuf_create
{
	uint32_t memfd;
	uint32_t flags;
	uint64_t offset;
	uint64_t size;
};

#define LINUX_UDMABUF_FLAGS_CLOEXEC 0x01
#define LINUX_UDMABUF_CREATE LINUX_IOW('u', 0x42, sizeof(struct linux_udmabuf_create))

// <linux/dma-buf.h>
struct linux_dma_buf_sync
{
	uint64_t flags;
};

#define LINUX_DMA_BUF_SYNC_READ (1 << 0)
#define LINUX_DMA_BUF_SYNC_WRITE (2 << 0)
#define LINUX_DMA_BUF_SYNC_RW (LINUX_DMA_BUF_SYNC_READ | LINUX_DMA_BUF_SYNC_WRITE)
#define LINUX_DMA_BUF_SYNC_START (0 << 2)
#define LINUX_DMA_BUF_SYNC_END (1 << 2)
#define LINUX_DMA_BUF_IOCTL_SYNC LINUX_IOW('b', 0, sizeof(struct linux_dma_buf_sync))

_Static_assert(sizeof(struct linux_udmabuf_create) == 24, "udmabuf_create layout");
_Static_assert(sizeof(struct linux_dma_buf_sync) == 8, "dma_buf_sync layout");

#endif

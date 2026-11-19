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

#ifndef IOSURFACE_GPUBUFFER_H
#define IOSURFACE_GPUBUFFER_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>

// GPU buffers in the GPU's own layout, for surfaces that a GPU renders into
// but whose memory, in IOSurface's linear layout, the GPU's driver can only
// sample (see "GPU buffers" in IOSurfaceObjC.m).
//
// A buffer is a buffer object of the host's generic buffer manager (libgbm)
// on a DRM render node, allocated with a layout modifier the caller's driver
// can render into. The CPU cannot map such a buffer usefully, so its
// contents are copied to and from the surface's memory on the GPU, by a GL
// context of the host EGL that this file keeps on a thread of its own: it
// never becomes current on an application's thread, so the application's
// GL state is never touched. The host libraries are loaded at run time and
// are optional; without them no buffer is ever made.

__BEGIN_DECLS

// DRM formats (<drm_fourcc.h>) that buffers are made in: four bytes per
// pixel, given in memory order.
#define IOSURFACE_DRM_FOURCC(a, b, c, d) \
	((uint32_t) (a) | ((uint32_t) (b) << 8) | ((uint32_t) (c) << 16) | ((uint32_t) (d) << 24))
#define IOSURFACE_DRM_FORMAT_ARGB8888 IOSURFACE_DRM_FOURCC('A', 'R', '2', '4') // B G R A
#define IOSURFACE_DRM_FORMAT_ABGR8888 IOSURFACE_DRM_FOURCC('A', 'B', '2', '4') // R G B A
// DRM_FORMAT_MOD_LINEAR.
#define IOSURFACE_DRM_FORMAT_MOD_LINEAR 0ULL

typedef struct _IOSurfaceGPUBo _IOSurfaceGPUBo;

typedef struct
{
	// The buffer's DMA-BUF; it belongs to the buffer.
	int fd;
	uint32_t fourcc;
	uint64_t modifier;
	uint32_t offset;
	uint32_t stride;
	uint32_t width, height;
} _IOSurfaceGPUBoDescription;

// Makes a buffer of width x height pixels in DRM format fourcc, in one of
// the given layout modifiers (never a linear one), on the DRM render node at
// renderNode, a host path. All buffers of a process are on the render node
// of the first one. Returns NULL if the host lacks the libraries or the
// driver refuses.
__attribute__((visibility("hidden")))
_IOSurfaceGPUBo* _IOSurfaceGPUBoCreate(const char* renderNode, uint32_t width, uint32_t height,
	uint32_t fourcc, const uint64_t* modifiers, size_t modifierCount);

// Uses a buffer another process made, given by its description: the
// DMA-BUF (duplicated, the caller keeps its descriptor), its format,
// modifier, stride and size, with one plane at offset 0. The buffer must be
// on the render node at renderNode. Returns NULL if it cannot be used.
__attribute__((visibility("hidden")))
_IOSurfaceGPUBo* _IOSurfaceGPUBoImport(const char* renderNode, const _IOSurfaceGPUBoDescription* description);

__attribute__((visibility("hidden")))
void _IOSurfaceGPUBoDestroy(_IOSurfaceGPUBo* bo);

__attribute__((visibility("hidden")))
void _IOSurfaceGPUBoDescribe(const _IOSurfaceGPUBo* bo, _IOSurfaceGPUBoDescription* description);

// Whether the buffer is on the render node at renderNode.
__attribute__((visibility("hidden")))
bool _IOSurfaceGPUBoOnRenderNode(const _IOSurfaceGPUBo* bo, const char* renderNode);

// Copy the buffer's contents into memory whose rows are bytesPerRow apart,
// or the other way. The caller makes sure that GPU work writing to the
// buffer has completed before a download; an upload has completed for every
// GPU user of the buffer when it returns. Both return false if the copy
// could not be made.
__attribute__((visibility("hidden")))
bool _IOSurfaceGPUBoDownload(_IOSurfaceGPUBo* bo, void* memory, size_t bytesPerRow);
__attribute__((visibility("hidden")))
bool _IOSurfaceGPUBoUpload(_IOSurfaceGPUBo* bo, const void* memory, size_t bytesPerRow);

// Called in the child after fork(). A child of a process that had opened
// the GPU makes no buffers and copies nothing: the driver state it
// inherited cannot be used, and buffers made before the fork are left
// alone (their memory is released when the child exits).
__attribute__((visibility("hidden")))
void _IOSurfaceGPUBoForked(void);

__END_DECLS

#endif

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

#ifndef OPENGL_DRM_FORMATS_H
#define OPENGL_DRM_FORMATS_H
#include <stdint.h>

// Linux DRM pixel formats (<drm_fourcc.h>) of the DMA-BUFs that
// CGLTexImageIOSurface2D imports. The kernel header cannot be included in a
// Darwin build, so the codes are spelled out the way the header defines
// them. The comment gives each format's bytes in memory, first to last.
#define CGL_DRM_FOURCC(a, b, c, d) \
	((uint32_t) (a) | ((uint32_t) (b) << 8) | ((uint32_t) (c) << 16) | ((uint32_t) (d) << 24))

#define CGL_DRM_FORMAT_ARGB8888 CGL_DRM_FOURCC('A', 'R', '2', '4') // B G R A
#define CGL_DRM_FORMAT_XRGB8888 CGL_DRM_FOURCC('X', 'R', '2', '4') // B G R x
#define CGL_DRM_FORMAT_ABGR8888 CGL_DRM_FOURCC('A', 'B', '2', '4') // R G B A
#define CGL_DRM_FORMAT_XBGR8888 CGL_DRM_FOURCC('X', 'B', '2', '4') // R G B x
#define CGL_DRM_FORMAT_BGRA8888 CGL_DRM_FOURCC('B', 'A', '2', '4') // A R G B
#define CGL_DRM_FORMAT_BGRX8888 CGL_DRM_FOURCC('B', 'X', '2', '4') // x R G B
#define CGL_DRM_FORMAT_RGBA8888 CGL_DRM_FOURCC('R', 'A', '2', '4') // A B G R
#define CGL_DRM_FORMAT_RGBX8888 CGL_DRM_FOURCC('R', 'X', '2', '4') // x B G R
#define CGL_DRM_FORMAT_R8 CGL_DRM_FOURCC('R', '8', ' ', ' ') // R
#define CGL_DRM_FORMAT_GR88 CGL_DRM_FOURCC('G', 'R', '8', '8') // R G
#define CGL_DRM_FORMAT_ABGR16161616F CGL_DRM_FOURCC('A', 'B', '4', 'H') // R G B A, half floats
#define CGL_DRM_FORMAT_XBGR16161616F CGL_DRM_FOURCC('X', 'B', '4', 'H') // R G B x, half floats

// DRM_FORMAT_MOD_LINEAR: rows one after another, no tiling.
#define CGL_DRM_FORMAT_MOD_LINEAR 0ULL

#endif

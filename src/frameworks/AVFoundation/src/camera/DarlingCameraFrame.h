/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Darling developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DARLING_CAMERA_FRAME_H_
#define DARLING_CAMERA_FRAME_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Source layouts. The first four match darling_pw_video_format; the rest are
 * the other raw formats offered to PipeWire during format fixation. */
enum darling_camera_pixel_layout
{
	DARLING_CAMERA_PIXEL_UNKNOWN = 0,
	DARLING_CAMERA_PIXEL_BGRX = 1,
	DARLING_CAMERA_PIXEL_RGBX = 2,
	DARLING_CAMERA_PIXEL_XRGB = 3,
	DARLING_CAMERA_PIXEL_XBGR = 4,
	DARLING_CAMERA_PIXEL_RGB24 = 5,
	DARLING_CAMERA_PIXEL_BGR24 = 6,
	DARLING_CAMERA_PIXEL_I420 = 7,
	DARLING_CAMERA_PIXEL_NV12 = 8,
	DARLING_CAMERA_PIXEL_YUY2 = 9,
	DARLING_CAMERA_PIXEL_UYVY = 10,
	DARLING_CAMERA_PIXEL_P010 = 11,
};

/* Destination layouts, all of which can be backed by a CoreVideo pixel
 * buffer. 420v/420f are biplanar NV12 (video/full range) and y420 is planar
 * I420; Darling's CoreVideo does not allocate planar full-range buffers. */
enum darling_camera_output_layout
{
	DARLING_CAMERA_OUTPUT_BGRA = 0,
	DARLING_CAMERA_OUTPUT_RGBA = 1,
	DARLING_CAMERA_OUTPUT_ARGB = 2,
	DARLING_CAMERA_OUTPUT_ABGR = 3,
	DARLING_CAMERA_OUTPUT_RGB24 = 4,
	DARLING_CAMERA_OUTPUT_BGR24 = 5,
	DARLING_CAMERA_OUTPUT_I420 = 6,
	DARLING_CAMERA_OUTPUT_NV12 = 7,
	DARLING_CAMERA_OUTPUT_NV12_FULL = 8,
};

/* Converts a captured frame into the layout requested by the client.
 *
 * srcPlanes/srcStrides describe the source planes (I420 uses 0/1/2, NV12 uses
 * 0/1, packed formats use 0). dstPlanes/dstStrides describe the CoreVideo
 * planes; unused entries are NULL/0. Strides are in bytes. Returns 0 on
 * success and -1 when an argument, stride, or format combination is invalid. */
int DarlingCameraConvertFrame(const uint8_t *const srcPlanes[4], const uint32_t srcStrides[4],
	enum darling_camera_pixel_layout source,
	const uint8_t *const dstPlanes[4], const uint32_t dstStrides[4],
	enum darling_camera_output_layout destination, uint32_t width, uint32_t height);

/* Convenience wrapper for a tightly packed 32-bpp source and BGRA output. */
void DarlingCameraCopyFrame(const uint8_t *src, size_t srcStride,
	enum darling_camera_pixel_layout layout, uint8_t *dst, size_t dstStride,
	uint32_t width, uint32_t height);

#ifdef __cplusplus
}
#endif

#endif

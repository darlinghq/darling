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

#include "DarlingCameraFrame.h"

static int clampByte(int value)
{
	if (value < 0)
		return 0;
	if (value > 255)
		return 255;
	return value;
}

static void yuvToRgb(int y, int cb, int cr, int *r, int *g, int *b)
{
	/* BT.601, studio range. Integer form of the documented Y'CbCr -> R'G'B'
	 * conversion; the shifts round to the nearest byte. */
	int c = y - 16, d = cb - 128, e = cr - 128;

	*r = clampByte((298 * c + 409 * e + 128) >> 8);
	*g = clampByte((298 * c - 100 * d - 208 * e + 128) >> 8);
	*b = clampByte((298 * c + 516 * d + 128) >> 8);
}

static void rgbToYuv(int r, int g, int b, int *y, int *cb, int *cr)
{
	/* BT.601, studio range. */
	*y = clampByte(((66 * r + 129 * g + 25 * b + 128) >> 8) + 16);
	*cb = clampByte(((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128);
	*cr = clampByte(((112 * r - 94 * g - 18 * b + 128) >> 8) + 128);
}

static void rgbToYuvFull(int r, int g, int b, int *y, int *cb, int *cr)
{
	/* BT.601, full range. Used for kCVPixelFormatType_420YpCbCr8BiPlanarFullRange. */
	*y = clampByte((77 * r + 150 * g + 29 * b + 128) >> 8);
	*cb = clampByte(((-43 * r - 85 * g + 128 * b + 128) >> 8) + 128);
	*cr = clampByte(((128 * r - 107 * g - 21 * b + 128) >> 8) + 128);
}

static uint32_t chromaWidth(uint32_t width)
{
	return (width + 1) / 2;
}

static void readPixel(const uint8_t *const planes[4], const uint32_t strides[4],
	enum darling_camera_pixel_layout source, uint32_t x, uint32_t y,
	int *r, int *g, int *b)
{
	const uint8_t *p = planes[0] + (size_t) y * strides[0];

	switch (source)
	{
		case DARLING_CAMERA_PIXEL_RGBX:
			p += (size_t) x * 4; *r = p[0]; *g = p[1]; *b = p[2]; break;
		case DARLING_CAMERA_PIXEL_XBGR:
			p += (size_t) x * 4; *r = p[3]; *g = p[2]; *b = p[1]; break;
		case DARLING_CAMERA_PIXEL_XRGB:
			p += (size_t) x * 4; *r = p[1]; *g = p[2]; *b = p[3]; break;
		case DARLING_CAMERA_PIXEL_RGB24:
			p += (size_t) x * 3; *r = p[0]; *g = p[1]; *b = p[2]; break;
		case DARLING_CAMERA_PIXEL_BGR24:
			p += (size_t) x * 3; *r = p[2]; *g = p[1]; *b = p[0]; break;
		case DARLING_CAMERA_PIXEL_YUY2:
			p += (size_t) (x / 2) * 4;
			yuvToRgb((x & 1) ? p[2] : p[0], p[1], p[3], r, g, b);
			break;
		case DARLING_CAMERA_PIXEL_UYVY:
			p += (size_t) (x / 2) * 4;
			yuvToRgb((x & 1) ? p[3] : p[1], p[0], p[2], r, g, b);
			break;
		case DARLING_CAMERA_PIXEL_I420:
		{
			const uint8_t *cb = planes[1] + (size_t) (y / 2) * strides[1] + x / 2;
			const uint8_t *cr = planes[2] + (size_t) (y / 2) * strides[2] + x / 2;
			yuvToRgb(p[x], cb[0], cr[0], r, g, b);
			break;
		}
		case DARLING_CAMERA_PIXEL_NV12:
		{
			const uint8_t *uv = planes[1] + (size_t) (y / 2) * strides[1] + (size_t) (x / 2) * 2;
			yuvToRgb(p[x], uv[0], uv[1], r, g, b);
			break;
		}
		case DARLING_CAMERA_PIXEL_P010:
		{
			const uint16_t *py = (const uint16_t *) (planes[0] + (size_t) y * strides[0]);
			const uint16_t *uv = (const uint16_t *) (planes[1] + (size_t) (y / 2) * strides[1] + (size_t) (x / 2) * 4);
			int y10 = (int) (py[x] >> 6);
			int cb10 = (int) (uv[0] >> 6);
			int cr10 = (int) (uv[1] >> 6);
			{
				double yy = (y10 - 64.0) / 876.0;
				double cb = (cb10 - 512.0) / 896.0;
				double cr = (cr10 - 512.0) / 896.0;
				double rr = yy + 1.4746 * cr;
				double gg = yy - 0.16455312684366 * cb - 0.57135312684366 * cr;
				double bb = yy + 1.8814 * cb;
				double luma = 0.2627 * rr + 0.6780 * gg + 0.0593 * bb;
				double scale = 1.0 / (1.0 + luma);
				*r = clampByte((int)(rr * scale * 1.8 * 255.0 + 0.5));
				*g = clampByte((int)(gg * scale * 1.8 * 255.0 + 0.5));
				*b = clampByte((int)(bb * scale * 1.8 * 255.0 + 0.5));
			}
			break;
		}
		case DARLING_CAMERA_PIXEL_BGRX:
		case DARLING_CAMERA_PIXEL_UNKNOWN:
		default:
			p += (size_t) x * 4; *r = p[2]; *g = p[1]; *b = p[0]; break;
	}
}

static int validateSource(const uint8_t *const planes[4], const uint32_t strides[4],
	enum darling_camera_pixel_layout source, uint32_t width)
{
	uint32_t cw = chromaWidth(width);

	if (!planes || !strides || !planes[0])
		return -1;
	switch (source)
	{
		case DARLING_CAMERA_PIXEL_RGB24:
		case DARLING_CAMERA_PIXEL_BGR24:
			return strides[0] >= (uint32_t) width * 3 ? 0 : -1;
		case DARLING_CAMERA_PIXEL_YUY2:
		case DARLING_CAMERA_PIXEL_UYVY:
			return strides[0] >= (uint32_t) width * 2 ? 0 : -1;
		case DARLING_CAMERA_PIXEL_I420:
			if (strides[0] < width || !planes[1] || !planes[2])
				return -1;
			return strides[1] >= cw && strides[2] >= cw ? 0 : -1;
		case DARLING_CAMERA_PIXEL_NV12:
			if (strides[0] < width || !planes[1])
				return -1;
			return strides[1] >= cw * 2 ? 0 : -1;
		case DARLING_CAMERA_PIXEL_P010:
			if (strides[0] < width * 2 || !planes[1])
				return -1;
			return strides[1] >= cw * 4 ? 0 : -1;
		default:
			return strides[0] >= (uint32_t) width * 4 ? 0 : -1;
	}
}

static int validateDestination(const uint8_t *const planes[4], const uint32_t strides[4],
	enum darling_camera_output_layout destination, uint32_t width)
{
	uint32_t cw = chromaWidth(width);

	if (!planes || !strides || !planes[0])
		return -1;
	switch (destination)
	{
		case DARLING_CAMERA_OUTPUT_RGB24:
		case DARLING_CAMERA_OUTPUT_BGR24:
			return strides[0] >= (uint32_t) width * 3 ? 0 : -1;
		case DARLING_CAMERA_OUTPUT_I420:
			if (strides[0] < width || !planes[1] || !planes[2])
				return -1;
			return strides[1] >= cw && strides[2] >= cw ? 0 : -1;
		case DARLING_CAMERA_OUTPUT_NV12:
		case DARLING_CAMERA_OUTPUT_NV12_FULL:
			if (strides[0] < width || !planes[1])
				return -1;
			return strides[1] >= cw * 2 ? 0 : -1;
		default:
			return strides[0] >= (uint32_t) width * 4 ? 0 : -1;
	}
}

int DarlingCameraConvertFrame(const uint8_t *const srcPlanes[4], const uint32_t srcStrides[4],
	enum darling_camera_pixel_layout source,
	const uint8_t *const dstPlanes[4], const uint32_t dstStrides[4],
	enum darling_camera_output_layout destination, uint32_t width, uint32_t height)
{
	uint32_t x, y;

	if (!width || !height || source < DARLING_CAMERA_PIXEL_UNKNOWN
		|| source > DARLING_CAMERA_PIXEL_P010
		|| destination > DARLING_CAMERA_OUTPUT_NV12_FULL)
		return -1;
	if (validateSource(srcPlanes, srcStrides, source, width) < 0
		|| validateDestination(dstPlanes, dstStrides, destination, width) < 0)
		return -1;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			int r, g, b;
			uint8_t *out = (uint8_t *) dstPlanes[0] + (size_t) y * dstStrides[0];

			readPixel(srcPlanes, srcStrides, source, x, y, &r, &g, &b);

			switch (destination)
			{
				case DARLING_CAMERA_OUTPUT_BGRA:
					out += (size_t) x * 4;
					out[0] = (uint8_t) b; out[1] = (uint8_t) g; out[2] = (uint8_t) r; out[3] = 255;
					break;
				case DARLING_CAMERA_OUTPUT_RGBA:
					out += (size_t) x * 4;
					out[0] = (uint8_t) r; out[1] = (uint8_t) g; out[2] = (uint8_t) b; out[3] = 255;
					break;
				case DARLING_CAMERA_OUTPUT_ARGB:
					out += (size_t) x * 4;
					out[0] = 255; out[1] = (uint8_t) r; out[2] = (uint8_t) g; out[3] = (uint8_t) b;
					break;
				case DARLING_CAMERA_OUTPUT_ABGR:
					out += (size_t) x * 4;
					out[0] = 255; out[1] = (uint8_t) b; out[2] = (uint8_t) g; out[3] = (uint8_t) r;
					break;
				case DARLING_CAMERA_OUTPUT_RGB24:
					out += (size_t) x * 3;
					out[0] = (uint8_t) r; out[1] = (uint8_t) g; out[2] = (uint8_t) b;
					break;
				case DARLING_CAMERA_OUTPUT_BGR24:
					out += (size_t) x * 3;
					out[0] = (uint8_t) b; out[1] = (uint8_t) g; out[2] = (uint8_t) r;
					break;
				case DARLING_CAMERA_OUTPUT_I420:
				case DARLING_CAMERA_OUTPUT_NV12:
				case DARLING_CAMERA_OUTPUT_NV12_FULL:
				{
					int yy, cb, cr;
					if (destination == DARLING_CAMERA_OUTPUT_NV12_FULL)
						rgbToYuvFull(r, g, b, &yy, &cb, &cr);
					else
						rgbToYuv(r, g, b, &yy, &cb, &cr);
					out[x] = (uint8_t) yy;
					if ((x & 1) == 0 && (y & 1) == 0)
					{
						if (destination == DARLING_CAMERA_OUTPUT_I420)
						{
							size_t row = (size_t) (y / 2) * dstStrides[1];
							((uint8_t *) dstPlanes[1])[row + x / 2] = (uint8_t) cb;
							((uint8_t *) dstPlanes[2])[(size_t) (y / 2) * dstStrides[2] + x / 2] = (uint8_t) cr;
						}
						else
						{
							uint8_t *uv = (uint8_t *) dstPlanes[1]
								+ (size_t) (y / 2) * dstStrides[1] + (size_t) (x / 2) * 2;
							uv[0] = (uint8_t) cb;
							uv[1] = (uint8_t) cr;
						}
					}
					break;
				}
			}
		}
	}
	return 0;
}

void DarlingCameraCopyFrame(const uint8_t *src, size_t srcStride,
	enum darling_camera_pixel_layout layout, uint8_t *dst, size_t dstStride,
	uint32_t width, uint32_t height)
{
	const uint8_t *srcPlanes[4] = { src, NULL, NULL, NULL };
	const uint8_t *dstPlanes[4] = { dst, NULL, NULL, NULL };
	uint32_t srcStrides[4] = { (uint32_t) srcStride, 0, 0, 0 };
	uint32_t dstStrides[4] = { (uint32_t) dstStride, 0, 0, 0 };

	if (srcStride > UINT32_MAX || dstStride > UINT32_MAX)
		return;
	DarlingCameraConvertFrame(srcPlanes, srcStrides, layout, dstPlanes, dstStrides,
		DARLING_CAMERA_OUTPUT_BGRA, width, height);
}

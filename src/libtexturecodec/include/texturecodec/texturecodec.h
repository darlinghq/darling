/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DTC_TEXTURECODEC_H
#define DTC_TEXTURECODEC_H

#include <stddef.h>
#include <stdint.h>

/* Public entry points stay visible although the library builds with
 * -fvisibility=hidden. A build that links the library into another image
 * and must not export the entry points from it defines DTC_API to nothing. */
#ifndef DTC_API
#if defined(__GNUC__)
#define DTC_API __attribute__((visibility("default")))
#else
#define DTC_API
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum dtc_status {
	DTC_OK = 0,
	DTC_E_ARG,
	DTC_E_UNSUPPORTED,
	DTC_E_TRUNCATED,
	DTC_E_OVERFLOW,
	DTC_E_CORRUPT
} dtc_status;

#define DTC_ASTC_FILE_HEADER_SIZE 16
#define DTC_ASTC_BLOCK_BYTES 16
#define DTC_ASTC_MAX_BLOCK_DIM 12

/* Fields of the 16-byte header of the .astc container. */
typedef struct dtc_astc_header {
	uint8_t block_w;
	uint8_t block_h;
	uint8_t block_d;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
} dtc_astc_header;

/* Out parameter of the decoders: lets a caller fail loudly instead of
 * showing the magenta error colour silently. */
typedef struct dtc_decode_report {
	uint64_t blocks;
	uint64_t error_blocks;
} dtc_decode_report;

/* Returns 1 when block_w x block_h is one of the 14 2D footprints of the
 * ASTC specification, else 0. */
DTC_API int dtc_astc_footprint_valid(uint32_t block_w, uint32_t block_h);

/* Parses the 16-byte .astc header. The payload starts at
 * DTC_ASTC_FILE_HEADER_SIZE. 3D footprints and depth > 1 parse but the
 * decoder rejects them (DTC_E_UNSUPPORTED from dtc_astc_file_parse_2d). */
DTC_API dtc_status dtc_astc_file_parse(const uint8_t *data, size_t len,
			       dtc_astc_header *out);

/* Same as dtc_astc_file_parse, then requires a 2D image (block_d == 1,
 * depth == 1); anything else is DTC_E_UNSUPPORTED. */
DTC_API dtc_status dtc_astc_file_parse_2d(const uint8_t *data, size_t len,
				  dtc_astc_header *out);

/* Decode modes, the `srgb` argument of the decoders is a mask of these.
 * DTC_SRGB: sRGB operation mode (see below). DTC_COMPAT: the behaviour of macOS
 * ImageIO where it parts from the specification text, for callers that stand
 * behind ImageIO: a void-extent block with D = 0 is a constant colour block
 * whatever its reserved bits and extent hold (the specification calls cleared
 * reserved bits and a low extent coordinate not below the high one illegal),
 * and its 16-bit colour is rounded to 8 bits instead of cut to its top byte.
 * Everything else is unchanged by it. 0 and 1 mean what they always did. */
#define DTC_SRGB 1
#define DTC_COMPAT 2

/* Decodes one 128-bit block into block_w * block_h RGBA8 texels in raster
 * order. srgb & DTC_SRGB selects sRGB operation mode: RGB bytes are the stored
 * sRGB-encoded values (no transfer function is applied), alpha is straight
 * and linear. *error_block is set to 1 when the block was illegal and every
 * affected texel is opaque magenta. It is also set when only some texels got
 * the error colour (a partition with an HDR endpoint mode). May be NULL. */
DTC_API dtc_status dtc_astc_decode_block(const uint8_t block[16], uint32_t block_w,
				 uint32_t block_h, int srgb,
				 uint8_t *rgba_out, int *error_block);

/* Decodes the payload of a 2D LDR ASTC image (no file header) into
 * width * height * 4 bytes of straight-alpha RGBA8. data holds the blocks in
 * raster order; trailing bytes are ignored. report may be NULL; when it is
 * given it is zeroed first, on every return path.
 * Status: DTC_E_ARG for NULL pointers, zero dimensions or a footprint that is
 * not one of the 14 2D footprints; DTC_E_OVERFLOW when width * height * 4 does
 * not fit in size_t; DTC_E_TRUNCATED when len holds fewer blocks than the
 * image needs (nothing is written then). */
DTC_API dtc_status dtc_astc_decode_ex(const uint8_t *data, size_t len,
			      uint32_t block_w, uint32_t block_h,
			      uint32_t width, uint32_t height, int srgb,
			      uint8_t *rgba_out, dtc_decode_report *report);

/* dtc_astc_decode_ex without the report. */
DTC_API dtc_status dtc_astc_decode(const uint8_t *data, size_t len, uint32_t block_w,
			   uint32_t block_h, uint32_t width, uint32_t height,
			   int srgb, uint8_t *rgba_out);

#ifdef __cplusplus
}
#endif

#endif

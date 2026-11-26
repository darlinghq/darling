/* SPDX-License-Identifier: GPL-3.0-only */
#include "texturecodec/texturecodec.h"

static const uint8_t magic[4] = { 0x13, 0xAB, 0xA1, 0x5C };

/* Footprints of the ASTC specification (2D and 3D tables). */
static const uint8_t foot2d[14][2] = {
	{ 4, 4 },   { 5, 4 },  { 5, 5 },   { 6, 5 },   { 6, 6 },
	{ 8, 5 },   { 8, 6 },  { 10, 5 },  { 10, 6 },  { 8, 8 },
	{ 10, 8 },  { 10, 10 }, { 12, 10 }, { 12, 12 }
};

static const uint8_t foot3d[10][3] = {
	{ 3, 3, 3 }, { 4, 3, 3 }, { 4, 4, 3 }, { 4, 4, 4 }, { 5, 4, 4 },
	{ 5, 5, 4 }, { 5, 5, 5 }, { 6, 5, 5 }, { 6, 6, 5 }, { 6, 6, 6 }
};

int dtc_astc_footprint_valid(uint32_t block_w, uint32_t block_h)
{
	size_t i;

	for (i = 0; i < sizeof(foot2d) / sizeof(foot2d[0]); i++)
		if (foot2d[i][0] == block_w && foot2d[i][1] == block_h)
			return 1;
	return 0;
}

static int footprint_3d_valid(uint32_t w, uint32_t h, uint32_t d)
{
	size_t i;

	for (i = 0; i < sizeof(foot3d) / sizeof(foot3d[0]); i++)
		if (foot3d[i][0] == w && foot3d[i][1] == h &&
		    foot3d[i][2] == d)
			return 1;
	return 0;
}

static uint32_t le24(const uint8_t *p)
{
	return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16;
}

dtc_status dtc_astc_file_parse(const uint8_t *data, size_t len,
			       dtc_astc_header *out)
{
	dtc_astc_header h;
	int i;

	if (!data || !out)
		return DTC_E_ARG;
	if (len < DTC_ASTC_FILE_HEADER_SIZE)
		return DTC_E_TRUNCATED;
	for (i = 0; i < 4; i++)
		if (data[i] != magic[i])
			return DTC_E_CORRUPT;
	h.block_w = data[4];
	h.block_h = data[5];
	h.block_d = data[6];
	h.width = le24(data + 7);
	h.height = le24(data + 10);
	h.depth = le24(data + 13);
	if (!h.width || !h.height || !h.depth)
		return DTC_E_CORRUPT;
	if (h.block_d == 1 ? !dtc_astc_footprint_valid(h.block_w, h.block_h)
			   : !footprint_3d_valid(h.block_w, h.block_h,
						 h.block_d))
		return DTC_E_CORRUPT;
	*out = h;
	return DTC_OK;
}

dtc_status dtc_astc_file_parse_2d(const uint8_t *data, size_t len,
				  dtc_astc_header *out)
{
	dtc_astc_header h;
	dtc_status st = dtc_astc_file_parse(data, len, &h);

	if (st != DTC_OK)
		return st;
	if (h.block_d != 1 || h.depth != 1)
		return DTC_E_UNSUPPORTED;
	*out = h;
	return DTC_OK;
}

/* SPDX-License-Identifier: GPL-3.0-only */
#include "astc_internal.h"

static int clamp8(int v)
{
	return v < 0 ? 0 : (v > 255 ? 255 : v);
}

/* bit_transfer_signed(a, b) of spec 23.14.1. */
static void bit_transfer(int *a, int *b)
{
	*b >>= 1;
	*b |= *a & 0x80;
	*a >>= 1;
	*a &= 0x3F;
	if (*a & 0x20)
		*a -= 0x40;
}

static void set_rgba(int e[4], int r, int g, int b, int a)
{
	e[0] = r;
	e[1] = g;
	e[2] = b;
	e[3] = a;
}

/* blue_contract() on the arithmetic shift of a possibly negative sum. */
static void set_contracted(int e[4], int r, int g, int b, int a)
{
	set_rgba(e, (r + b) >> 1, (g + b) >> 1, b, a);
}

static void clamp_pair(int e[2][4])
{
	int i;

	for (i = 0; i < 4; i++) {
		e[0][i] = clamp8(e[0][i]);
		e[1][i] = clamp8(e[1][i]);
	}
}

static void luminance(int e[2][4], int cem, int *v)
{
	int l0, l1;

	if (cem == 0) {
		l0 = v[0];
		l1 = v[1];
	} else {
		l0 = (v[0] >> 2) | (v[1] & 0xC0);
		l1 = clamp8(l0 + (v[1] & 0x3F));
	}
	set_rgba(e[0], l0, l0, l0, 255);
	set_rgba(e[1], l1, l1, l1, 255);
}

static void luminance_alpha(int e[2][4], int cem, int *v)
{
	if (cem == 4) {
		set_rgba(e[0], v[0], v[0], v[0], v[2]);
		set_rgba(e[1], v[1], v[1], v[1], v[3]);
		return;
	}
	bit_transfer(&v[1], &v[0]);
	bit_transfer(&v[3], &v[2]);
	set_rgba(e[0], v[0], v[0], v[0], v[2]);
	set_rgba(e[1], v[0] + v[1], v[0] + v[1], v[0] + v[1], v[2] + v[3]);
	clamp_pair(e);
}

/* Modes 6 and 10: base plus scale; mode 10 carries two alpha values. */
static void base_scale(int e[2][4], int cem, int *v)
{
	int a0 = cem == 10 ? v[4] : 255, a1 = cem == 10 ? v[5] : 255;

	set_rgba(e[0], v[0] * v[3] >> 8, v[1] * v[3] >> 8, v[2] * v[3] >> 8,
		 a0);
	set_rgba(e[1], v[0], v[1], v[2], a1);
}

/* Modes 8 and 12: direct, with blue contraction when the sum drops. */
static void direct(int e[2][4], int cem, int *v)
{
	int has_a = cem == 12;
	int a0 = has_a ? v[6] : 255, a1 = has_a ? v[7] : 255;

	if (v[1] + v[3] + v[5] >= v[0] + v[2] + v[4]) {
		set_rgba(e[0], v[0], v[2], v[4], a0);
		set_rgba(e[1], v[1], v[3], v[5], a1);
	} else {
		set_contracted(e[0], v[1], v[3], v[5], a1);
		set_contracted(e[1], v[0], v[2], v[4], a0);
	}
}

/* Modes 9 and 13: base plus signed offset. */
static void base_offset(int e[2][4], int cem, int *v)
{
	int has_a = cem == 13, a0 = 255, a1 = 255;

	bit_transfer(&v[1], &v[0]);
	bit_transfer(&v[3], &v[2]);
	bit_transfer(&v[5], &v[4]);
	if (has_a) {
		bit_transfer(&v[7], &v[6]);
		a0 = v[6];
		a1 = v[6] + v[7];
	}
	if (v[1] + v[3] + v[5] >= 0) {
		set_rgba(e[0], v[0], v[2], v[4], a0);
		set_rgba(e[1], v[0] + v[1], v[2] + v[3], v[4] + v[5], a1);
	} else {
		set_contracted(e[0], v[0] + v[1], v[2] + v[3], v[4] + v[5], a1);
		set_contracted(e[1], v[0], v[2], v[4], a0);
	}
	clamp_pair(e);
}

/* Returns -1 for the HDR modes, which LDR operation mode cannot decode. */
static int decode_mode(int cem, int *v, int e[2][4])
{
	switch (cem) {
	case 0: case 1: luminance(e, cem, v); break;
	case 4: case 5: luminance_alpha(e, cem, v); break;
	case 6: case 10: base_scale(e, cem, v); break;
	case 8: case 12: direct(e, cem, v); break;
	case 9: case 13: base_offset(e, cem, v); break;
	default: return -1;
	}
	return 0;
}

uint32_t astc_decode_endpoints(const uint8_t blk[16], const astc_block *b,
			       int ep[4][2][4])
{
	uint8_t raw[18];
	int v[8];
	uint32_t i, k, n, pos = 0, hdr = 0;

	astc_ise_decode(blk, b->cstart, b->ncvals, &b->crange, raw);
	for (i = 0; i < b->nparts; i++) {
		n = ((uint32_t)(b->cem[i] >> 2) + 1) * 2;
		for (k = 0; k < n; k++)
			v[k] = (int)astc_unquant_color(&b->crange, raw[pos + k]);
		pos += n;
		if (decode_mode(b->cem[i], v, ep[i]))
			hdr |= 1u << i;
	}
	return hdr;
}

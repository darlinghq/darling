/* SPDX-License-Identifier: GPL-3.0-only */
#include "astc_internal.h"

const astc_range astc_color_ranges[ASTC_NUM_COLOR_RANGES] = {
	{ 1, 0, 1, 5 },   { 0, 0, 3, 7 },   { 0, 1, 1, 9 },
	{ 1, 0, 2, 11 },  { 0, 0, 4, 15 },  { 0, 1, 2, 19 },
	{ 1, 0, 3, 23 },  { 0, 0, 5, 31 },  { 0, 1, 3, 39 },
	{ 1, 0, 4, 47 },  { 0, 0, 6, 63 },  { 0, 1, 4, 79 },
	{ 1, 0, 5, 95 },  { 0, 0, 7, 127 }, { 0, 1, 5, 159 },
	{ 1, 0, 6, 191 }, { 0, 0, 8, 255 }
};

/* Rows are rho 2..7; columns are low precision (p = 0), high precision. */
static const astc_range weight_ranges[2][6] = {
	{ { 0, 0, 1, 1 }, { 1, 0, 0, 2 }, { 0, 0, 2, 3 },
	  { 0, 1, 0, 4 }, { 1, 0, 1, 5 }, { 0, 0, 3, 7 } },
	{ { 0, 1, 1, 9 }, { 1, 0, 2, 11 }, { 0, 0, 4, 15 },
	  { 0, 1, 2, 19 }, { 1, 0, 3, 23 }, { 0, 0, 5, 31 } }
};

const astc_range *astc_weight_range(uint32_t p, uint32_t rho)
{
	if (rho < 2 || rho > 7 || p > 1)
		return 0;
	return &weight_ranges[p][rho - 2];
}

uint32_t astc_ise_size(uint32_t count, const astc_range *r)
{
	uint32_t bits = count * r->bits;

	if (r->trit)
		bits += (8 * count + 4) / 5;
	if (r->quint)
		bits += (7 * count + 2) / 3;
	return bits;
}

/* Reads n bits at pos, treating every bit at or past `end` (or past the
 * 128-bit block) as zero. */
static uint32_t get_bits(const uint8_t *buf, uint32_t pos, uint32_t n,
			 uint32_t end)
{
	uint32_t v = 0, i, p;

	for (i = 0; i < n; i++) {
		p = pos + i;
		if (p >= end || p >= 128)
			break;
		v |= (uint32_t)((buf[p >> 3] >> (p & 7)) & 1) << i;
	}
	return v;
}

static void unpack_trits(uint32_t T, uint8_t t[5])
{
	uint32_t c, t0, t1, t2, t3, t4;

	if (((T >> 2) & 7) == 7) {
		c = (((T >> 5) & 7) << 2) | (T & 3);
		t4 = 2;
		t3 = 2;
	} else {
		c = T & 31;
		if (((T >> 5) & 3) == 3) {
			t4 = 2;
			t3 = (T >> 7) & 1;
		} else {
			t4 = (T >> 7) & 1;
			t3 = (T >> 5) & 3;
		}
	}
	if ((c & 3) == 3) {
		t2 = 2;
		t1 = (c >> 4) & 1;
		t0 = (((c >> 3) & 1) << 1) | (((c >> 2) & 1) & ~(c >> 3) & 1);
	} else if (((c >> 2) & 3) == 3) {
		t2 = 2;
		t1 = 2;
		t0 = c & 3;
	} else {
		t2 = (c >> 4) & 1;
		t1 = (c >> 2) & 3;
		t0 = (((c >> 1) & 1) << 1) | ((c & 1) & ~(c >> 1) & 1);
	}
	t[0] = (uint8_t)t0;
	t[1] = (uint8_t)t1;
	t[2] = (uint8_t)t2;
	t[3] = (uint8_t)t3;
	t[4] = (uint8_t)t4;
}

static void unpack_quints(uint32_t Q, uint8_t q[3])
{
	uint32_t c, q0, q1, q2;

	if (((Q >> 1) & 3) == 3 && ((Q >> 5) & 3) == 0) {
		q2 = ((Q & 1) << 2) | (((Q >> 4) & 1) & ~Q & 1) << 1 |
		     (((Q >> 3) & 1) & ~Q & 1);
		q1 = 4;
		q0 = 4;
	} else {
		if (((Q >> 1) & 3) == 3) {
			q2 = 4;
			c = (((Q >> 3) & 3) << 3) | ((~(Q >> 5) & 3) << 1) |
			    (Q & 1);
		} else {
			q2 = (Q >> 5) & 3;
			c = Q & 31;
		}
		if ((c & 7) == 5) {
			q1 = 4;
			q0 = (c >> 3) & 3;
		} else {
			q1 = (c >> 3) & 3;
			q0 = c & 7;
		}
	}
	q[0] = (uint8_t)q0;
	q[1] = (uint8_t)q1;
	q[2] = (uint8_t)q2;
}

/* One block of 5 trit values: layout of table 169, low bit first. */
static void trit_block(const uint8_t *buf, uint32_t pos, uint32_t end,
		       uint32_t n, uint8_t *out)
{
	static const uint8_t tbits[5] = { 2, 2, 1, 2, 1 };
	uint32_t m[5], T = 0, tshift = 0, i;
	uint8_t t[5];

	for (i = 0; i < 5; i++) {
		m[i] = get_bits(buf, pos, n, end);
		pos += n;
		T |= get_bits(buf, pos, tbits[i], end) << tshift;
		pos += tbits[i];
		tshift += tbits[i];
	}
	unpack_trits(T, t);
	for (i = 0; i < 5; i++)
		out[i] = (uint8_t)((uint32_t)t[i] << n | m[i]);
}

/* One block of 3 quint values: layout of tables 170 and 171. */
static void quint_block(const uint8_t *buf, uint32_t pos, uint32_t end,
			uint32_t n, uint8_t *out)
{
	static const uint8_t qbits[3] = { 3, 2, 2 };
	uint32_t m[3], Q = 0, qshift = 0, i;
	uint8_t q[3];

	for (i = 0; i < 3; i++) {
		m[i] = get_bits(buf, pos, n, end);
		pos += n;
		Q |= get_bits(buf, pos, qbits[i], end) << qshift;
		pos += qbits[i];
		qshift += qbits[i];
	}
	unpack_quints(Q, q);
	for (i = 0; i < 3; i++)
		out[i] = (uint8_t)((uint32_t)q[i] << n | m[i]);
}

void astc_ise_decode(const uint8_t buf[16], uint32_t start, uint32_t count,
		     const astc_range *r, uint8_t *out)
{
	uint32_t end = start + astc_ise_size(count, r);
	uint32_t i, per, blk_bits, k;
	uint8_t tmp[5];

	if (!r->trit && !r->quint) {
		for (i = 0; i < count; i++)
			out[i] = (uint8_t)get_bits(buf, start + i * r->bits,
						   r->bits, end);
		return;
	}
	per = r->trit ? 5 : 3;
	blk_bits = r->trit ? 8 + 5 * r->bits : 7 + 3 * r->bits;
	for (i = 0; i < count; i += per) {
		uint32_t pos = start + (i / per) * blk_bits;

		if (r->trit)
			trit_block(buf, pos, end, r->bits, tmp);
		else
			quint_block(buf, pos, end, r->bits, tmp);
		for (k = 0; k < per && i + k < count; k++)
			out[i + k] = tmp[k];
	}
}

static uint32_t replicate(uint32_t v, uint32_t n, uint32_t target)
{
	uint32_t r = v, k = n;

	while (k < target) {
		r = (r << n) | v;
		k += n;
	}
	return r >> (k - target);
}

/* Letters a..f name bits 0..5 of the stored low bits, as in tables 172 and
 * 182; the string is the B constant from the most significant bit down. */
static uint32_t pattern(const char *s, uint32_t m)
{
	uint32_t b = 0;

	for (; *s; s++)
		b = (b << 1) | (*s == '0' ? 0 : (m >> (*s - 'a')) & 1);
	return b;
}

typedef struct unq_row {
	uint8_t trit;
	uint8_t bits;
	uint8_t c;
	const char *b;
} unq_row;

static const unq_row color_rows[] = {
	{ 1, 1, 204, "000000000" }, { 0, 1, 113, "000000000" },
	{ 1, 2, 93, "b000b0bb0" },  { 0, 2, 54, "b0000bb00" },
	{ 1, 3, 44, "cb000cbcb" },  { 0, 3, 26, "cb0000cbc" },
	{ 1, 4, 22, "dcb000dcb" },  { 0, 4, 13, "dcb0000dc" },
	{ 1, 5, 11, "edcb000ed" },  { 0, 5, 6, "edcb0000e" },
	{ 1, 6, 5, "fedcb000f" }
};

static const unq_row weight_rows[] = {
	{ 1, 1, 50, "0000000" }, { 0, 1, 28, "0000000" },
	{ 1, 2, 23, "b000b0b" }, { 0, 2, 13, "b0000b0" },
	{ 1, 3, 11, "cb000cb" }
};

static const unq_row *find_row(const unq_row *rows, uint32_t nrows,
			       const astc_range *r)
{
	uint32_t i;

	for (i = 0; i < nrows; i++)
		if (rows[i].trit == r->trit && rows[i].bits == r->bits)
			return &rows[i];
	return 0;
}

/* Shared D*C+B, xor A, keep-top-bit step of tables 172 and 182. */
static uint32_t unq_step(const unq_row *row, const astc_range *r, uint32_t v,
			 uint32_t width, uint32_t keep)
{
	uint32_t m = v & ((1u << r->bits) - 1), d = v >> r->bits;
	uint32_t a = (m & 1) ? (1u << width) - 1 : 0;
	uint32_t unq = d * row->c + pattern(row->b, m);

	unq ^= a;
	return (a & keep) | (unq >> 2);
}

uint32_t astc_unquant_color(const astc_range *r, uint32_t v)
{
	const unq_row *row;

	if (!r->trit && !r->quint)
		return replicate(v, r->bits, 8);
	row = find_row(color_rows, sizeof(color_rows) / sizeof(color_rows[0]),
		       r);
	return row ? unq_step(row, r, v, 9, 0x80) : 0;
}

/* Table 181: trit-only and quint-only weights have no extra bits. */
static uint32_t weight_unq63(const astc_range *r, uint32_t v)
{
	static const uint8_t trit_only[3] = { 0, 32, 63 };
	static const uint8_t quint_only[5] = { 0, 16, 32, 47, 63 };
	const unq_row *row;

	if (!r->trit && !r->quint)
		return replicate(v, r->bits, 6);
	if (r->bits == 0)
		return r->trit ? trit_only[v % 3] : quint_only[v % 5];
	row = find_row(weight_rows,
		       sizeof(weight_rows) / sizeof(weight_rows[0]), r);
	return row ? unq_step(row, r, v, 7, 0x20) : 0;
}

uint32_t astc_unquant_weight(const astc_range *r, uint32_t v)
{
	uint32_t unq = weight_unq63(r, v);

	return unq > 32 ? unq + 1 : unq;
}

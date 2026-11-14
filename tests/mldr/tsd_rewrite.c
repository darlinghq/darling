/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// Host test of the ARM64 TSD read rewriting helpers (tsd_rewrite.h). It runs on
// any host: it only manipulates instruction words. The encodings are checked
// against an independent decoder written from the Arm Architecture Reference
// Manual for A-profile (DDI 0487): MRS (register), AND (immediate) with its
// DecodeBitMasks bitmask-immediate rule, and LDR (immediate, unsigned offset).
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "tsd_rewrite.h"

static int failures;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

// --- reference decoders -----------------------------------------------------

struct mrs { int ok; unsigned op0, op1, crn, crm, op2, rt; };

// MRS Xt, <sysreg>: 1101 0101 0011 (o0=1) op1 CRn CRm op2 Rt; op0 = 2 + o0.
static struct mrs decode_mrs(uint32_t insn)
{
	struct mrs r = {0};
	if ((insn >> 20) != 0xD53)
		return r;
	r.ok = 1;
	r.op0 = 2 + ((insn >> 19) & 1);
	r.op1 = (insn >> 16) & 7;
	r.crn = (insn >> 12) & 15;
	r.crm = (insn >> 8) & 15;
	r.op2 = (insn >> 5) & 7;
	r.rt = insn & 31;
	return r;
}

static uint64_t ror64(uint64_t v, unsigned n, unsigned size)
{
	uint64_t mask = size == 64 ? ~0ull : (1ull << size) - 1;
	v &= mask;
	n %= size;
	return n ? ((v >> n) | (v << (size - n))) & mask : v;
}

// DecodeBitMasks for a 64-bit logical immediate (immediate = TRUE).
static int decode_bitmask(unsigned n, unsigned imms, unsigned immr, uint64_t* out)
{
	unsigned notimms = (~imms) & 63;
	unsigned combined = (n << 6) | notimms; // 7 bits
	int len = -1;
	for (int i = 6; i >= 0; i--)
		if (combined & (1u << i)) { len = i; break; }
	if (len < 1)
		return 0;
	unsigned esize = 1u << len;
	unsigned levels = esize - 1;
	unsigned s = imms & levels;
	unsigned r = immr & levels;
	if (s == levels)
		return 0;
	uint64_t welem = (1ull << (s + 1)) - 1;
	uint64_t elem = ror64(welem, r, esize);
	uint64_t result = 0;
	for (unsigned i = 0; i < 64; i += esize)
		result |= elem << i;
	*out = result;
	return 1;
}

struct and_imm { int ok; unsigned rd, rn; uint64_t imm; };

// AND Xd, Xn, #imm: 1 00 100100 N immr imms Rn Rd
static struct and_imm decode_and_x(uint32_t insn)
{
	struct and_imm r = {0};
	if ((insn >> 23) != ((1u << 8) | (0u << 7) | 0x24)) // sf=1 opc=00 100100
		return r;
	unsigned n = (insn >> 22) & 1;
	unsigned immr = (insn >> 16) & 63;
	unsigned imms = (insn >> 10) & 63;
	if (!decode_bitmask(n, imms, immr, &r.imm))
		return r;
	r.ok = 1;
	r.rn = (insn >> 5) & 31;
	r.rd = insn & 31;
	return r;
}

struct ldr_imm { int ok; unsigned rt, rn; uint64_t offset; };

// LDR Xt, [Xn, #pimm]: 11 111 0 01 01 imm12 Rn Rt, offset = imm12 * 8
static struct ldr_imm decode_ldr_x(uint32_t insn)
{
	struct ldr_imm r = {0};
	if ((insn >> 22) != 0x3E5)
		return r;
	r.ok = 1;
	r.offset = (uint64_t) ((insn >> 10) & 0xFFF) * 8;
	r.rn = (insn >> 5) & 31;
	r.rt = insn & 31;
	return r;
}

// --- tests ------------------------------------------------------------------

static uint32_t encode_and_not7(unsigned rd, unsigned rn)
{
	return MLDR_A64_AND_X_CLEAR_LOW3 | (rn << 5) | rd;
}

static void test_encodings(void)
{
	int ok = 1;
	for (unsigned r = 0; r < 32; r++) {
		// the constants describe the registers and bit patterns they claim
		struct mrs ro = decode_mrs(MLDR_A64_MRS_TPIDRRO_EL0 | r);
		struct mrs rw = decode_mrs(MLDR_A64_MRS_TPIDR_EL0 | r);
		ok &= ro.ok && ro.op0 == 3 && ro.op1 == 3 && ro.crn == 13 && ro.crm == 0 && ro.op2 == 3 && ro.rt == r;
		ok &= rw.ok && rw.op0 == 3 && rw.op1 == 3 && rw.crn == 13 && rw.crm == 0 && rw.op2 == 2 && rw.rt == r;
		struct and_imm a = decode_and_x(encode_and_not7(r, 31 - r));
		ok &= a.ok && a.rd == r && a.rn == 31 - r && a.imm == ~7ull;
		struct ldr_imm l = decode_ldr_x(MLDR_A64_LDR_X_UOFFSET | (8u << 10) | (r << 5) | (31 - r));
		ok &= l.ok && l.offset == 64 && l.rn == r && l.rt == 31 - r;
	}
	check(ok, "MRS TPIDRRO_EL0/TPIDR_EL0, AND #~7 and LDR constants decode to the documented fields");

	// the reference decoder itself: well-known logical immediates
	uint64_t v;
	check(decode_bitmask(1, 60, 61, &v) && v == ~7ull, "reference decoder: N=1 immr=61 imms=60 is ~7");
	check(decode_bitmask(1, 2, 0, &v) && v == 7, "reference decoder: N=1 immr=0 imms=2 is 7");
	check(decode_bitmask(0, 0x3C, 0, &v) && v == 0x5555555555555555ull, "reference decoder: N=0 imms=0x3c repeats 0b01");
	check(!decode_bitmask(1, 63, 0, &v), "reference decoder: an all-ones run is not encodable");
}

static void test_pair_detection(void)
{
	uint32_t reg;
	uint32_t mrs8 = MLDR_A64_MRS_TPIDRRO_EL0 | 8;
	check(mldr_tsd_read_pair(mrs8, encode_and_not7(8, 8), &reg) && reg == 8, "pair: mrs x8 + and x8, x8, #~7 matches");
	check(!mldr_tsd_read_pair(mrs8, encode_and_not7(9, 8), &reg), "pair: different destination does not match");
	check(!mldr_tsd_read_pair(mrs8, encode_and_not7(8, 9), &reg), "pair: different source does not match");
	// AND Xd, Xn, #7 (the CPU number), N=1 immr=0 imms=2
	uint32_t and7 = 0x92400000u | (0u << 16) | (2u << 10) | (8 << 5) | 8;
	check(!mldr_tsd_read_pair(mrs8, and7, &reg), "pair: and #7 (CPU number) does not match");
	check(!mldr_tsd_read_pair(mrs8, 0xD503201Fu /* nop */, &reg), "pair: lone mrs does not match");
	check(!mldr_tsd_read_pair(MLDR_A64_MRS_TPIDR_EL0 | 8, encode_and_not7(8, 8), &reg), "pair: mrs TPIDR_EL0 does not match");
	check(!mldr_tsd_read_pair(MLDR_A64_MRS_TPIDRRO_EL0 | 31, encode_and_not7(31, 31), &reg), "pair: XZR/SP never matches");
	// 32-bit AND (sf=0) with the same fields must not match
	check(!mldr_tsd_read_pair(mrs8, encode_and_not7(8, 8) & 0x7FFFFFFFu, &reg), "pair: 32-bit and does not match");
	// the registers named by the pair are reported for every register 0..30
	int ok = 1;
	for (uint32_t r = 0; r < 31; r++)
		ok &= mldr_tsd_read_pair(MLDR_A64_MRS_TPIDRRO_EL0 | r, encode_and_not7(r, r), &reg) && reg == r;
	check(ok, "pair: matches for x0..x30");
}

static void test_slot_offset(void)
{
	check(!mldr_tsd_slot_offset_valid(0) && !mldr_tsd_slot_offset_valid(8), "slot offset: below the thread control block is refused");
	check(mldr_tsd_slot_offset_valid(16) && mldr_tsd_slot_offset_valid(64) && mldr_tsd_slot_offset_valid(32760), "slot offset: aligned values in range accepted");
	check(!mldr_tsd_slot_offset_valid(32768) && !mldr_tsd_slot_offset_valid(65) && !mldr_tsd_slot_offset_valid((uintptr_t) -1), "slot offset: unaligned or beyond the LDR range refused");
}

static void test_rewrite(void)
{
	const uint32_t nop = 0xD503201Fu;
	uint32_t code[] = {
		nop,
		MLDR_A64_MRS_TPIDRRO_EL0 | 0, encode_and_not7(0, 0),            // pair, x0
		nop,
		MLDR_A64_MRS_TPIDRRO_EL0 | 9, 0x92400000u | (2u << 10) | (9 << 5) | 9, // CPU number: untouched
		MLDR_A64_MRS_TPIDRRO_EL0 | 30, encode_and_not7(30, 30),          // pair, x30
		MLDR_A64_MRS_TPIDRRO_EL0 | 5,                                     // pair split by the end of the buffer
	};
	uint32_t before[sizeof(code) / 4];
	memcpy(before, code, sizeof(code));
	size_t count = sizeof(code) / 4;

	check(mldr_tsd_count_reads(code, count) == 2, "rewrite: two pairs counted");
	check(mldr_tsd_rewrite_reads(code, count, 15) == 0 && memcmp(code, before, sizeof(code)) == 0, "rewrite: invalid slot offset changes nothing");
	check(mldr_tsd_rewrite_reads(code, count, 64) == 2, "rewrite: two pairs rewritten");

	int ok = 1;
	struct mrs m = decode_mrs(code[1]);
	struct ldr_imm l = decode_ldr_x(code[2]);
	ok &= m.ok && m.op2 == 2 && m.rt == 0 && l.ok && l.rn == 0 && l.rt == 0 && l.offset == 64;
	m = decode_mrs(code[6]);
	l = decode_ldr_x(code[7]);
	ok &= m.ok && m.op2 == 2 && m.rt == 30 && l.ok && l.rn == 30 && l.rt == 30 && l.offset == 64;
	check(ok, "rewrite: pairs become mrs TPIDR_EL0 + ldr [Xt, #slot]");

	check(code[0] == before[0] && code[3] == before[3] && code[4] == before[4] && code[5] == before[5] && code[8] == before[8],
		"rewrite: neighbours, the CPU number read and a trailing lone mrs are untouched");
	check(mldr_tsd_count_reads(code, count) == 0, "rewrite: no TPIDRRO_EL0 pairs remain");
	check(mldr_tsd_rewrite_reads(code, count, 64) == 0, "rewrite: idempotent");

	uint32_t big_slot[] = { MLDR_A64_MRS_TPIDRRO_EL0 | 1, encode_and_not7(1, 1) };
	mldr_tsd_rewrite_reads(big_slot, 2, 32760);
	l = decode_ldr_x(big_slot[1]);
	check(l.ok && l.offset == 32760, "rewrite: largest slot offset encodes exactly");

	check(mldr_tsd_rewrite_reads(NULL, 0, 64) == 0 && mldr_tsd_rewrite_reads(big_slot, 1, 64) == 0, "rewrite: empty and one-word buffers are fine");
}

int main(void)
{
	test_encodings();
	test_pair_detection();
	test_slot_offset();
	test_rewrite();
	if (failures) {
		printf("FAIL: %d checks failed\n", failures);
		return 1;
	}
	puts("PASS tsd_rewrite");
	return 0;
}

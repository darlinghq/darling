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

#ifndef _MLDR_TSD_REWRITE_H_
#define _MLDR_TSD_REWRITE_H_

// Instruction-level helpers for the ARM64 Darwin thread-specific data (TSD)
// base. On Darwin the base lives in TPIDRRO_EL0 together with the CPU number
// in the low 3 bits, and code reads it as a pair of adjacent instructions:
//
//     mrs Xt, TPIDRRO_EL0
//     and Xt, Xt, #~7          (the documented _os_tsd_get_base() sequence)
//
// A Linux process cannot set TPIDRRO_EL0 (it reads as zero), so mldr turns the
// pair into
//
//     mrs Xt, TPIDR_EL0
//     ldr Xt, [Xt, #slot]
//
// where `slot` is the thread-pointer-relative offset of an ELF TLS slot that
// libsystem_kernel fills with the Darwin TSD base. Both rewritten words keep
// their position, so no branch or relocation in the image changes.
//
// A lone TPIDRRO_EL0 read (for example the CPU number, which masks with #7)
// is left alone: it already yields zero, which is a valid CPU number.
//
// Encodings are from the Arm Architecture Reference Manual for A-profile
// (DDI 0487): the MRS (register) instruction with TPIDRRO_EL0 (op0=3, op1=3,
// CRn=13, CRm=0, op2=3) and TPIDR_EL0 (op2=2), AND (immediate) and
// LDR (immediate, unsigned offset). The AND bitmask immediate for ~7 is
// N=1, immr=61, imms=60.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MLDR_A64_MRS_TPIDRRO_EL0     0xD53BD060u
#define MLDR_A64_MRS_TPIDR_EL0       0xD53BD040u
#define MLDR_A64_MRS_MASK            0xFFFFFFE0u // all but Rt
#define MLDR_A64_AND_X_CLEAR_LOW3    0x927DF000u // AND Xd, Xn, #~7
#define MLDR_A64_AND_X_MASK          0xFFFFFC00u // all but Rn and Rd
#define MLDR_A64_LDR_X_UOFFSET       0xF9400000u // LDR Xt, [Xn, #imm]

// The load's scaled 12-bit unsigned offset limits the slot offset.
#define MLDR_TSD_SLOT_OFFSET_MAX     32760u

static inline bool mldr_tsd_slot_offset_valid(uintptr_t offset)
{
	// 8-byte aligned, within the LDR range, and past the thread control block
	// header that the host's thread pointer points at.
	return offset >= 16 && offset <= MLDR_TSD_SLOT_OFFSET_MAX && (offset & 7) == 0;
}

// Returns true when `first` and `second` are the TSD base read pair for the
// same register (never XZR/SP), and stores that register in `*reg`.
static inline bool mldr_tsd_read_pair(uint32_t first, uint32_t second, uint32_t* reg)
{
	if ((first & MLDR_A64_MRS_MASK) != MLDR_A64_MRS_TPIDRRO_EL0)
		return false;
	uint32_t rt = first & 0x1F;
	if (rt == 31)
		return false;
	if ((second & MLDR_A64_AND_X_MASK) != MLDR_A64_AND_X_CLEAR_LOW3)
		return false;
	if ((second & 0x1F) != rt || ((second >> 5) & 0x1F) != rt)
		return false;
	*reg = rt;
	return true;
}

// Number of TSD base read pairs in `insns`; changes nothing.
static inline size_t mldr_tsd_count_reads(const uint32_t* insns, size_t count)
{
	size_t found = 0;
	uint32_t rt;
	for (size_t i = 0; i + 1 < count; i++)
	{
		if (mldr_tsd_read_pair(insns[i], insns[i + 1], &rt))
		{
			found++;
			i++;
		}
	}
	return found;
}

// Rewrites every TSD base read pair in `insns` to read the native TLS slot at
// `slot_offset` and returns how many were rewritten. The caller makes the
// memory writable first and invalidates the instruction cache afterwards.
static inline size_t mldr_tsd_rewrite_reads(uint32_t* insns, size_t count, uintptr_t slot_offset)
{
	size_t rewritten = 0;
	uint32_t rt;
	if (!mldr_tsd_slot_offset_valid(slot_offset))
		return 0;
	for (size_t i = 0; i + 1 < count; i++)
	{
		if (mldr_tsd_read_pair(insns[i], insns[i + 1], &rt))
		{
			insns[i] = MLDR_A64_MRS_TPIDR_EL0 | rt;
			insns[i + 1] = MLDR_A64_LDR_X_UOFFSET | (uint32_t)((slot_offset / 8) << 10) | (rt << 5) | rt;
			rewritten++;
			i++;
		}
	}
	return rewritten;
}

#endif // _MLDR_TSD_REWRITE_H_

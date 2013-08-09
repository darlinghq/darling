/*
This file is part of Darling.

Copyright (C) 2013 Lubos Dolezel

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

#ifndef CFIWALKER_H
#define CFIWALKER_H
#include <stdint.h>
#include "BufReWriter.h"

class CFIWalker
{
public:
	CFIWalker(uint8_t* instructions, uintptr_t length, uint8_t dwarfPtrEncoding);
	
	void walk(void* opaque, int (*mapRegisterNumber)(void*, int));
private:
	void processNextReg(void* opaque, int (*mapRegisterNumber)(void*, int));
	uint8_t processMixedInstr(void* opaque, int (*mapRegisterNumber)(void*, int), uint8_t instr);
private:
	BufReWriter m_buffer;
	uint8_t m_dwarfPtrEncoding;
};

enum : uint8_t
{
	DW_CFA_advance_loc = 0x40,
	DW_CFA_offset = 0x80,
	DW_CFA_restore = 0xc0,
	DW_CFA_extended = 0,

	DW_CFA_nop = 0x00,
	DW_CFA_set_loc = 0x01,
	DW_CFA_advance_loc1 = 0x02,
	DW_CFA_advance_loc2 = 0x03,
	DW_CFA_advance_loc4 = 0x04,
	DW_CFA_offset_extended = 0x05,
	DW_CFA_restore_extended = 0x06,
	DW_CFA_undefined = 0x07,
	DW_CFA_same_value = 0x08,
	DW_CFA_register = 0x09,
	DW_CFA_remember_state = 0x0a,
	DW_CFA_restore_state = 0x0b,
	DW_CFA_def_cfa = 0x0c,
	DW_CFA_def_cfa_register = 0x0d,
	DW_CFA_def_cfa_offset = 0x0e,
	DW_CFA_def_cfa_expression = 0x0f,
	DW_CFA_expression = 0x10,
	DW_CFA_offset_extended_sf = 0x11,
	DW_CFA_def_cfa_sf = 0x12,
	DW_CFA_def_cfa_offset_sf = 0x13,
	DW_CFA_val_offset = 0x14,
	DW_CFA_val_offset_sf = 0x15,
	DW_CFA_val_expression = 0x16,

	DW_CFA_low_user = 0x1c,
	DW_CFA_MIPS_advance_loc8 = 0x1d,
	DW_CFA_GNU_window_save = 0x2d,
	DW_CFA_GNU_args_size = 0x2e,
	DW_CFA_GNU_negative_offset_extended = 0x2f,
	DW_CFA_high_user = 0x3f
};


#endif

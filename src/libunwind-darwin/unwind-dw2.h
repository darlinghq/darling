/* DWARF2 frame unwind data structure.
   Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2009
   Free Software Foundation, Inc.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* A target can override (perhaps for backward compatibility) how
   many dwarf2 columns are unwound.  */
#ifndef DWARF_FRAME_REGISTERS
#define DWARF_FRAME_REGISTERS FIRST_PSEUDO_REGISTER
#endif

/* The result of interpreting the frame unwind info for a frame.
   This is all symbolic at this point, as none of the values can
   be resolved until the target pc is located.  */
typedef struct
{
  /* Each register save state can be described in terms of a CFA slot,
     another register, or a location expression.  */
  struct frame_state_reg_info
  {
    struct {
      union {
	_Unwind_Word reg;
	_Unwind_Sword offset;
	const unsigned char *exp;
      } loc;
      enum {
	REG_UNSAVED,
	REG_SAVED_OFFSET,
	REG_SAVED_REG,
	REG_SAVED_EXP,
	REG_SAVED_VAL_OFFSET,
	REG_SAVED_VAL_EXP,
	REG_UNDEFINED
      } how;
    } reg[DWARF_FRAME_REGISTERS+1];

    /* Used to implement DW_CFA_remember_state.  */
    struct frame_state_reg_info *prev;

    /* The CFA can be described in terms of a reg+offset or a
       location expression.  */
    _Unwind_Sword cfa_offset;
    _Unwind_Word cfa_reg;
    const unsigned char *cfa_exp;
    enum {
      CFA_UNSET,
      CFA_REG_OFFSET,
      CFA_EXP
    } cfa_how;
  } regs;

  /* The PC described by the current frame state.  */
  void *pc;

  /* The information we care about from the CIE/FDE.  */
  _Unwind_Personality_Fn personality;
  _Unwind_Sword data_align;
  _Unwind_Word code_align;
  _Unwind_Word retaddr_column;
  unsigned char fde_encoding;
  unsigned char lsda_encoding;
  unsigned char saw_z;
  unsigned char signal_frame;
  void *eh_ptr;
} _Unwind_FrameState;


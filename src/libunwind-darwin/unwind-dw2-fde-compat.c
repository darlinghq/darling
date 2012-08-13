/* Backward compatibility unwind routines.
   Copyright (C) 2004, 2005, 2009
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

#if defined (USE_GAS_SYMVER) && defined (USE_LIBUNWIND_EXCEPTIONS)
#include "tconfig.h"
#include "tsystem.h"
#include "unwind.h"
#include "unwind-dw2-fde.h"
#include "unwind-compat.h"

extern const fde * __libunwind__Unwind_Find_FDE
  (void *, struct dwarf_eh_bases *);

const fde *
_Unwind_Find_FDE (void *pc, struct dwarf_eh_bases *bases)
{
  __libunwind__Unwind_Find_FDE (pc, bases);
}

symver (_Unwind_Find_FDE, GCC_3.0);
#endif

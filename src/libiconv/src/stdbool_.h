/* Copyright (C) 2001, 2002, 2003 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef _STDBOOL_H
#define _STDBOOL_H

/* ISO C 99 <stdbool.h> for platforms that lack it.  */

/* Usage suggestions:

   Programs that use <stdbool.h> should be aware of some limitations
   and standards compliance issues.

   Standards compliance:

       - <stdbool.h> must be #included before 'bool', 'false', 'true'
         can be used.

       - You cannot assume that sizeof (bool) == 1.

       - Programs should not undefine the macros bool, true, and false,
         as C99 lists that as an "obsolescent feature".

   Limitations of this substitute, when used in a C89 environment:

       - <stdbool.h> must be #included before the '_Bool' type can be used.

       - You cannot assume that _Bool is a typedef; it might be a macro.

       - In C99, casts and automatic conversions to '_Bool' or 'bool' are
         performed in such a way that every nonzero value gets converted
         to 'true', and zero gets converted to 'false'.  This doesn't work
         with this substitute.  With this substitute, only the values 0 and 1
         give the expected result when converted to _Bool' or 'bool'.

   Also, it is suggested that programs use 'bool' rather than '_Bool';
   this isn't required, but 'bool' is more common.  */


/* 7.16. Boolean type and values */

/* BeOS <sys/socket.h> already #defines false 0, true 1.  We use the same
   definitions below, but temporarily we have to #undef them.  */
#ifdef __BEOS__
# include <OS.h> /* defines bool but not _Bool */
# undef false
# undef true
#endif

/* For the sake of symbolic names in gdb, we define true and false as
   enum constants, not only as macros.
   It is tempting to write
      typedef enum { false = 0, true = 1 } _Bool;
   so that gdb prints values of type 'bool' symbolically. But if we do
   this, values of type '_Bool' may promote to 'int' or 'unsigned int'
   (see ISO C 99 6.7.2.2.(4)); however, '_Bool' must promote to 'int'
   (see ISO C 99 6.3.1.1.(2)).  So we add a negative value to the
   enum; this ensures that '_Bool' promotes to 'int'.  */
#if !(defined __cplusplus || defined __BEOS__)
# if !@HAVE__BOOL@
#  if defined __SUNPRO_C && (__SUNPRO_C < 0x550 || __STDC__ == 1)
    /* Avoid stupid "warning: _Bool is a keyword in ISO C99".  */
#   define _Bool signed char
enum { false = 0, true = 1 };
#  else
typedef enum { _Bool_must_promote_to_int = -1, false = 0, true = 1 } _Bool;
#  endif
# endif
#else
typedef bool _Bool;
#endif
#define bool _Bool

/* The other macros must be usable in preprocessor directives.  */
#define false 0
#define true 1
#define __bool_true_false_are_defined 1

#endif /* _STDBOOL_H */

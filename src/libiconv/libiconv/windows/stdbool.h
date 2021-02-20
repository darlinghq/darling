/* Copyright (C) 2001-2002 Free Software Foundation, Inc.
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

/* 7.16. Boolean type and values */

/* BeOS <sys/socket.h> already #defines false 0, true 1.  We use the same
   definitions below, but temporarily we have to #undef them.  */
#ifdef __BEOS__
# undef false
# undef true
#endif

/* For the sake of symbolic names in gdb, define _Bool as an enum type.  */
#ifndef __cplusplus
# if !0
typedef enum { false = 0, true = 1 } _Bool;
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

/* Safe automatic memory allocation.
   Copyright (C) 2003-2005 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2003.

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

#ifndef _ALLOCSA_H
#define _ALLOCSA_H

#include <alloca.h>
#include <stddef.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif


/* safe_alloca(N) is equivalent to alloca(N) when it is safe to call
   alloca(N); otherwise it returns NULL.  It either returns N bytes of
   memory allocated on the stack, that lasts until the function returns,
   or NULL.
   Use of safe_alloca should be avoided:
     - inside arguments of function calls - undefined behaviour,
     - in inline functions - the allocation may actually last until the
       calling function returns.
*/
#if HAVE_ALLOCA
/* The OS usually guarantees only one guard page at the bottom of the stack,
   and a page size can be as small as 4096 bytes.  So we cannot safely
   allocate anything larger than 4096 bytes.  Also care for the possibility
   of a few compiler-allocated temporary stack slots.
   This must be a macro, not an inline function.  */
# define safe_alloca(N) ((N) < 4032 ? alloca (N) : NULL)
#else
# define safe_alloca(N) ((N), NULL)
#endif

/* allocsa(N) is a safe variant of alloca(N).  It allocates N bytes of
   memory allocated on the stack, that must be freed using freesa() before
   the function returns.  Upon failure, it returns NULL.  */
#if HAVE_ALLOCA
# define allocsa(N) \
  ((N) < 4032 - sa_increment					    \
   ? (void *) ((char *) alloca ((N) + sa_increment) + sa_increment) \
   : mallocsa (N))
#else
# define allocsa(N) \
  mallocsa (N)
#endif
extern void * mallocsa (size_t n);

/* Free a block of memory allocated through allocsa().  */
#if HAVE_ALLOCA
extern void freesa (void *p);
#else
# define freesa free
#endif

/* Maybe we should also define a variant
    nallocsa (size_t n, size_t s) - behaves like allocsa (n * s)
   If this would be useful in your application. please speak up.  */


#ifdef __cplusplus
}
#endif


/* ------------------- Auxiliary, non-public definitions ------------------- */

/* Determine the alignment of a type at compile time.  */
#if defined __GNUC__
# define sa_alignof __alignof__
#elif defined __cplusplus
  template <class type> struct sa_alignof_helper { char __slot1; type __slot2; };
# define sa_alignof(type) offsetof (sa_alignof_helper<type>, __slot2)
#elif defined __hpux
  /* Work around a HP-UX 10.20 cc bug with enums constants defined as offsetof
     values.  */
# define sa_alignof(type) (sizeof (type) <= 4 ? 4 : 8)
#elif defined _AIX
  /* Work around an AIX 3.2.5 xlc bug with enums constants defined as offsetof
     values.  */
# define sa_alignof(type) 4
#else
# define sa_alignof(type) offsetof (struct { char __slot1; type __slot2; }, __slot2)
#endif

enum
{
/* The desired alignment of memory allocations is the maximum alignment
   among all elementary types.  */
  sa_alignment_long = sa_alignof (long),
  sa_alignment_double = sa_alignof (double),
#ifdef HAVE_LONG_LONG
  sa_alignment_longlong = sa_alignof (long long),
#endif
#ifdef HAVE_LONG_DOUBLE
  sa_alignment_longdouble = sa_alignof (long double),
#endif
  sa_alignment_max = ((sa_alignment_long - 1) | (sa_alignment_double - 1)
#ifdef HAVE_LONG_LONG
		      | (sa_alignment_longlong - 1)
#endif
#ifdef HAVE_LONG_DOUBLE
		      | (sa_alignment_longdouble - 1)
#endif
		     ) + 1,
/* The increment that guarantees room for a magic word must be >= sizeof (int)
   and a multiple of sa_alignment_max.  */
  sa_increment = ((sizeof (int) + sa_alignment_max - 1) / sa_alignment_max) * sa_alignment_max
};

#endif /* _ALLOCSA_H */

/* malloc with out of memory checking.
   Copyright (C) 2001-2004 Free Software Foundation, Inc.
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

#ifndef _XALLOC_H
#define _XALLOC_H

#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif


/* Defined in xmalloc.c.  */

/* Allocate SIZE bytes of memory dynamically, with error checking.  */
extern void *xmalloc (size_t size);

/* Allocate memory for NMEMB elements of SIZE bytes, with error checking.  */
extern void *xcalloc (size_t nmemb, size_t size);

/* Change the size of an allocated block of memory PTR to SIZE bytes,
   with error checking.  If PTR is NULL, run xmalloc.  */
extern void *xrealloc (void *ptr, size_t size);

/* This function is always triggered when memory is exhausted.  It is
   in charge of honoring the three previous items.  This is the
   function to call when one wants the program to die because of a
   memory allocation failure.  */
extern void xalloc_die (void)
#if (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 5)) && !__STRICT_ANSI__
     __attribute__ ((__noreturn__))
#endif
     ;


/* Defined in xstrdup.c.  */

/* Return a newly allocated copy of STRING.  */
extern char *xstrdup (const char *string);


/* Return 1 if an array of N objects, each of size S, cannot exist due
   to size arithmetic overflow.  S must be positive and N must be
   nonnegative.  This is a macro, not an inline function, so that it
   works correctly even when SIZE_MAX < N.

   By gnulib convention, SIZE_MAX represents overflow in size
   calculations, so the conservative dividend to use here is
   SIZE_MAX - 1, since SIZE_MAX might represent an overflowed value.
   However, malloc (SIZE_MAX) fails on all known hosts where
   sizeof (ptrdiff_t) <= sizeof (size_t), so do not bother to test for
   exactly-SIZE_MAX allocations on such hosts; this avoids a test and
   branch when S is known to be 1.  */
# define xalloc_oversized(n, s) \
    ((size_t) (sizeof (ptrdiff_t) <= sizeof (size_t) ? -1 : -2) / (s) < (n))


#ifdef __cplusplus
}
#endif


#endif /* _XALLOC_H */

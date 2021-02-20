/* Copyright (C) 2001-2002, 2004-2006 Free Software Foundation, Inc.
   Written by Bruno Haible, Sam Steingold, Peter Burwood.
   This file is part of gnulib.

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

#ifndef _STDINT_H
#define _STDINT_H

/*
 * ISO C 99 <stdint.h> for platforms that lack it.
 * <http://www.opengroup.org/susv3xbd/stdint.h.html>
 */

/* Get wchar_t, WCHAR_MIN, WCHAR_MAX.  */
#include <stddef.h>
/* BSD/OS 4.2 defines WCHAR_MIN, WCHAR_MAX in <wchar.h>, not <stddef.h>.  */
#if !(defined(WCHAR_MIN) && defined(WCHAR_MAX)) && @HAVE_WCHAR_H@
# include <wchar.h>
#endif

/* Get LONG_MIN, LONG_MAX, ULONG_MAX.  */
#include <limits.h>

/* Get those types that are already defined in other system include files.  */
#if defined(__FreeBSD__) && (__FreeBSD__ >= 3) && (__FreeBSD__ <= 4)
# include <sys/inttypes.h>
#endif
#if defined(__OpenBSD__) || defined(__bsdi__) || defined(__sgi)
  /* In OpenBSD 3.8, <sys/types.h> includes <machine/types.h>, which defines
     int{8,16,32,64}_t, uint{8,16,32,64}_t and __BIT_TYPES_DEFINED__.
     <inttypes.h> includes <machine/types.h> and also defines intptr_t and
     uintptr_t.  */
  /* BSD/OS 4.2 is similar, but doesn't have <inttypes.h> */
  /* IRIX 6.5 has <inttypes.h>, and <sys/types.h> defines some of these
     types as well.  */
# include <sys/types.h>
# if @HAVE_INTTYPES_H@
#  include @FULL_PATH_INTTYPES_H@
# endif
#endif
#if defined(__linux__) && @HAVE_SYS_BITYPES_H@
  /* Linux libc4 >= 4.6.7 and libc5 have a <sys/bitypes.h> that defines
     int{8,16,32,64}_t and __BIT_TYPES_DEFINED__.  In libc5 >= 5.2.2 it is
     included by <sys/types.h>.  */
# include <sys/bitypes.h>
#endif
#if defined(__sun) && @HAVE_SYS_INTTYPES_H@
  /* Solaris 7 <sys/inttypes.h> has the types except the *_fast*_t types, and
     the macros except for *_FAST*_*, INTPTR_MIN, PTRDIFF_MIN, PTRDIFF_MAX.
     But note that <sys/int_types.h> contains only the type definitions!  */
# include <sys/inttypes.h>
#endif
#if (defined(__hpux) || defined(_AIX)) && @HAVE_INTTYPES_H@
  /* HP-UX 10 <inttypes.h> has nearly everything, except UINT_LEAST8_MAX,
     UINT_FAST8_MAX, PTRDIFF_MIN, PTRDIFF_MAX.  */
  /* AIX 4 <inttypes.h> has nearly everything, except INTPTR_MIN, INTPTR_MAX,
     UINTPTR_MAX, PTRDIFF_MIN, PTRDIFF_MAX.  */
# include @FULL_PATH_INTTYPES_H@
#endif
#if @HAVE_STDINT_H@
  /* Other systems may have an incomplete <stdint.h>.  */
  /* On some versions of IRIX, the SGI C compiler comes with an <stdint.h>,
     but
       - in c99 mode, <inttypes.h> includes <stdint.h>,
       - in c89 mode, <stdint.h> spews warnings and defines nothing.
         <inttypes.h> defines only a subset of the types and macros that
         <stdint.h> would define in c99 mode.
     So we rely only on <inttypes.h> (included above).  It means that in
     c89 mode, we shadow the contents of warning-spewing <stdint.h>.  */
# if !(defined(__sgi) && @HAVE_INTTYPES_H@ && !defined(__c99))
#  include @FULL_PATH_STDINT_H@
# endif
#endif

/* 7.18.1.1. Exact-width integer types */

/* Here we assume a standard architecture where the hardware integer
   types have 8, 16, 32, optionally 64 bits.  */

#if !@HAVE_INT8_T@
typedef signed char    int8_t;
#endif
#if !@HAVE_UINT8_T@
typedef unsigned char  uint8_t;
# define _UINT8_T /* avoid collision with Solaris 2.5.1 <pthread.h> */
#endif

#if !@HAVE_INT16_T@
typedef short          int16_t;
#endif
#if !@HAVE_UINT16_T@
typedef unsigned short uint16_t;
#endif

#if !@HAVE_INT32_T@
typedef int            int32_t;
#endif
#if !@HAVE_UINT32_T@
typedef unsigned int   uint32_t;
# define _UINT32_T /* avoid collision with Solaris 2.5.1 <pthread.h> */
#endif

#if @HAVE_INT64_T@
# define _STDINT_H_HAVE_INT64 1
#else
# if @HAVE_LONG_64BIT@
typedef long           int64_t;
#  define _STDINT_H_HAVE_INT64 1
# elif @HAVE_LONG_LONG_64BIT@
typedef long long      int64_t;
#  define _STDINT_H_HAVE_INT64 1
# elif defined _MSC_VER
typedef __int64        int64_t;
#  define _STDINT_H_HAVE_INT64 1
# endif
#endif
#if @HAVE_UINT64_T@
# define _STDINT_H_HAVE_UINT64 1
#else
# if @HAVE_LONG_64BIT@
typedef unsigned long      uint64_t;
#  define _STDINT_H_HAVE_UINT64 1
# elif @HAVE_LONG_LONG_64BIT@
typedef unsigned long long uint64_t;
#  define _UINT64_T /* avoid collision with Solaris 2.5.1 <pthread.h> */
#  define _STDINT_H_HAVE_UINT64 1
# elif defined _MSC_VER
typedef unsigned __int64   uint64_t;
#  define _STDINT_H_HAVE_UINT64 1
# endif
#endif

/* 7.18.1.2. Minimum-width integer types */

/* Here we assume a standard architecture where the hardware integer
   types have 8, 16, 32, optionally 64 bits. Therefore the leastN_t types
   are the same as the corresponding N_t types.  */

#if !@HAVE_INT_LEAST8_T@
typedef int8_t   int_least8_t;
#endif
#if !@HAVE_UINT_LEAST8_T@
typedef uint8_t  uint_least8_t;
#endif

#if !@HAVE_INT_LEAST16_T@
typedef int16_t  int_least16_t;
#endif
#if !@HAVE_UINT_LEAST16_T@
typedef uint16_t uint_least16_t;
#endif

#if !@HAVE_INT_LEAST32_T@
typedef int32_t  int_least32_t;
#endif
#if !@HAVE_UINT_LEAST32_T@
typedef uint32_t uint_least32_t;
#endif

#if !@HAVE_INT_LEAST64_T@ && _STDINT_H_HAVE_INT64
typedef int64_t  int_least64_t;
#endif
#if !@HAVE_UINT_LEAST64_T@ && _STDINT_H_HAVE_UINT64
typedef uint64_t uint_least64_t;
#endif

/* 7.18.1.3. Fastest minimum-width integer types */

/* Note: Other <stdint.h> substitutes may define these types differently.
   It is not recommended to use these types in public header files. */

/* Here we assume a standard architecture where the hardware integer
   types have 8, 16, 32, optionally 64 bits. Therefore the fastN_t types
   are taken from the same list of types.  */

/* On alpha processors, int32_t variables are slower than int64_t variables,
   due to the necessary zap instructions.  */
#if defined __alpha
# define _STDINT_H_INT64_FASTER_THAN_INT32 1
#endif

#if !@HAVE_INT_FAST8_T@
# if _STDINT_H_INT64_FASTER_THAN_INT32
typedef int64_t  int_fast8_t;
# else
typedef int32_t  int_fast8_t;
# endif
#endif
#if !@HAVE_UINT_FAST8_T@
# if _STDINT_H_INT64_FASTER_THAN_INT32
typedef uint64_t uint_fast8_t;
# else
typedef uint32_t uint_fast8_t;
# endif
#endif

#if !@HAVE_INT_FAST16_T@
# if _STDINT_H_INT64_FASTER_THAN_INT32
typedef int64_t  int_fast16_t;
# else
typedef int32_t  int_fast16_t;
# endif
#endif
#if !@HAVE_UINT_FAST16_T@
# if _STDINT_H_INT64_FASTER_THAN_INT32
typedef uint64_t uint_fast16_t;
# else
typedef uint32_t uint_fast16_t;
# endif
#endif

#if !@HAVE_INT_FAST32_T@
# if _STDINT_H_INT64_FASTER_THAN_INT32
typedef int64_t  int_fast32_t;
# else
typedef int32_t  int_fast32_t;
# endif
#endif
#if !@HAVE_UINT_FAST32_T@
# if _STDINT_H_INT64_FASTER_THAN_INT32
typedef uint64_t uint_fast32_t;
# else
typedef uint32_t uint_fast32_t;
# endif
#endif

#if !@HAVE_INT_FAST64_T@ && _STDINT_H_HAVE_INT64
typedef int64_t  int_fast64_t;
#endif
#if !@HAVE_UINT_FAST64_T@ && _STDINT_H_HAVE_UINT64
typedef uint64_t uint_fast64_t;
#endif

/* 7.18.1.4. Integer types capable of holding object pointers */

/* On some platforms (like IRIX6 MIPS with -n32) sizeof(void*) < sizeof(long),
   but this doesn't matter here.  */
#if !@HAVE_INTPTR_T@
typedef long          intptr_t;
#endif
#if !@HAVE_UINTPTR_T@
typedef unsigned long uintptr_t;
#endif

/* 7.18.1.5. Greatest-width integer types */

/* Note: These types are compiler dependent. It may be unwise to use them in
   public header files. */

#if !@HAVE_INTMAX_T@
/* Remove possible redundant definition from gnulib's config.h first.  */
# undef intmax_t
# ifdef _STDINT_H_HAVE_INT64
typedef int64_t  intmax_t;
# else
typedef int32_t  intmax_t;
# endif
#endif
#if !@HAVE_UINTMAX_T@
/* Remove possible redundant definition from gnulib's config.h first.  */
# undef uintmax_t
# ifdef _STDINT_H_HAVE_UINT64
typedef uint64_t uintmax_t;
# else
typedef uint32_t uintmax_t;
# endif
#endif

/* 7.18.2. Limits of specified-width integer types */

#if !defined(__cplusplus) || defined(__STDC_LIMIT_MACROS)

/* 7.18.2.1. Limits of exact-width integer types */

/* Here we assume a standard architecture where the hardware integer
   types have 8, 16, 32, optionally 64 bits.  */

#if @HAVE_INT8_T@
# ifndef INT8_MIN
#  define INT8_MIN  (-1 << (@BITSIZEOF_INT8_T@ - 1))
# endif
#else
# define INT8_MIN  -128
#endif
#if @HAVE_INT8_T@
# ifndef INT8_MAX
#  define INT8_MAX  (~ (-1 << (@BITSIZEOF_INT8_T@ - 1)))
# endif
#else
# define INT8_MAX  127
#endif
#if @HAVE_UINT8_T@
# ifndef UINT8_MAX
#  if @BITSIZEOF_UINT8_T@ < @BITSIZEOF_UNSIGNED_INT@
#   define UINT8_MAX  (((1 << (@BITSIZEOF_UINT8_T@ - 1)) - 1) * 2 + 1)
#  else
#   define UINT8_MAX  (((1U << (@BITSIZEOF_UINT8_T@ - 1)) - 1) * 2 + 1)
#  endif
# endif
#else
# define UINT8_MAX  255
#endif

#if @HAVE_INT16_T@
# ifndef INT16_MIN
#  define INT16_MIN  (-1 << (@BITSIZEOF_INT16_T@ - 1))
# endif
#else
# define INT16_MIN  -32768
#endif
#if @HAVE_INT16_T@
# ifndef INT16_MAX
#  define INT16_MAX  (~ (-1 << (@BITSIZEOF_INT16_T@ - 1)))
# endif
#else
# define INT16_MAX  32767
#endif
#if @HAVE_UINT16_T@
# ifndef UINT16_MAX
#  if @BITSIZEOF_UINT16_T@ < @BITSIZEOF_UNSIGNED_INT@
#   define UINT16_MAX  (((1 << (@BITSIZEOF_UINT16_T@ - 1)) - 1) * 2 + 1)
#  else
#   define UINT16_MAX  (((1U << (@BITSIZEOF_UINT16_T@ - 1)) - 1) * 2 + 1)
#  endif
# endif
#else
# define UINT16_MAX  65535
#endif

#if @HAVE_INT32_T@
# ifndef INT32_MIN
#  define INT32_MIN  (-1 << (@BITSIZEOF_INT32_T@ - 1))
# endif
#else
# define INT32_MIN  (~INT32_MAX)
#endif
#if @HAVE_INT32_T@
# ifndef INT32_MAX
#  define INT32_MAX  (~ (-1 << (@BITSIZEOF_INT32_T@ - 1)))
# endif
#else
# define INT32_MAX  2147483647
#endif
#if @HAVE_UINT32_T@
# ifndef UINT32_MAX
#  if @BITSIZEOF_UINT32_T@ < @BITSIZEOF_UNSIGNED_INT@
#   define UINT32_MAX  (((1 << (@BITSIZEOF_UINT32_T@ - 1)) - 1) * 2 + 1)
#  else
#   define UINT32_MAX  (((1U << (@BITSIZEOF_UINT32_T@ - 1)) - 1) * 2 + 1)
#  endif
# endif
#else
# define UINT32_MAX  4294967295U
#endif

#if @HAVE_INT64_T@
# ifndef INT64_MIN
#  if @HAVE_LONG_64BIT@
#   define INT64_MIN  (-1L << (@BITSIZEOF_INT64_T@ - 1))
#  elif @HAVE_LONG_LONG_64BIT@
#   define INT64_MIN  (-1LL << (@BITSIZEOF_INT64_T@ - 1))
#  elif defined _MSC_VER
#   define INT64_MIN  (-1i64 << (@BITSIZEOF_INT64_T@ - 1))
#  endif
# endif
#else
# ifdef _STDINT_H_HAVE_INT64
#  define INT64_MIN  (~INT64_MAX)
# endif
#endif
#if @HAVE_INT64_T@
# ifndef INT64_MAX
#  if @HAVE_LONG_64BIT@
#   define INT64_MAX  (~ (-1L << (@BITSIZEOF_INT64_T@ - 1)))
#  elif @HAVE_LONG_LONG_64BIT@
#   define INT64_MAX  (~ (-1LL << (@BITSIZEOF_INT64_T@ - 1)))
#  elif defined _MSC_VER
#   define INT64_MAX  (~ (-1i64 << (@BITSIZEOF_INT64_T@ - 1)))
#  endif
# endif
#else
# ifdef _STDINT_H_HAVE_INT64
#  if @HAVE_LONG_64BIT@
#   define INT64_MAX  9223372036854775807L
#  elif @HAVE_LONG_LONG_64BIT@
#   define INT64_MAX  9223372036854775807LL
#  elif defined _MSC_VER
#   define INT64_MAX  9223372036854775807i64
#  endif
# endif
#endif
#if @HAVE_UINT64_T@
# ifndef UINT64_MAX
#  if @HAVE_LONG_64BIT@
#   define UINT64_MAX  (((1UL << (@BITSIZEOF_UINT64_T@ - 1)) - 1) * 2 + 1)
#  elif @HAVE_LONG_LONG_64BIT@
#   define UINT64_MAX  (((1ULL << (@BITSIZEOF_UINT64_T@ - 1)) - 1) * 2 + 1)
#  elif defined _MSC_VER
#   define UINT64_MAX  (((1ui64 << (@BITSIZEOF_UINT64_T@ - 1)) - 1) * 2 + 1)
#  endif
# endif
#else
# ifdef _STDINT_H_HAVE_UINT64
#  if @HAVE_LONG_64BIT@
#   define UINT64_MAX 18446744073709551615UL
#  elif @HAVE_LONG_LONG_64BIT@
#   define UINT64_MAX 18446744073709551615ULL
#  elif defined _MSC_VER
#   define UINT64_MAX 18446744073709551615ui64
#  endif
# endif
#endif

/* 7.18.2.2. Limits of minimum-width integer types */

/* Here we assume a standard architecture where the hardware integer
   types have 8, 16, 32, optionally 64 bits. Therefore the leastN_t types
   are the same as the corresponding N_t types.  */

#if @HAVE_INT_LEAST8_T@
# ifndef INT_LEAST8_MIN
#  define INT_LEAST8_MIN  (-1 << (@BITSIZEOF_INT_LEAST8_T@ - 1))
# endif
#else
# define INT_LEAST8_MIN  INT8_MIN
#endif
#if @HAVE_INT_LEAST8_T@
# ifndef INT_LEAST8_MAX
#  define INT_LEAST8_MAX  (~ (-1 << (@BITSIZEOF_INT_LEAST8_T@ - 1)))
# endif
#else
# define INT_LEAST8_MAX  INT8_MAX
#endif
#if @HAVE_UINT_LEAST8_T@
# ifndef UINT_LEAST8_MAX
#  if @BITSIZEOF_UINT_LEAST8_T@ < @BITSIZEOF_UNSIGNED_INT@
#   define UINT_LEAST8_MAX  (((1 << (@BITSIZEOF_UINT_LEAST8_T@ - 1)) - 1) * 2 + 1)
#  else
#   define UINT_LEAST8_MAX  (((1U << (@BITSIZEOF_UINT_LEAST8_T@ - 1)) - 1) * 2 + 1)
#  endif
# endif
#else
# define UINT_LEAST8_MAX  UINT8_MAX
#endif

#if @HAVE_INT_LEAST16_T@
# ifndef INT_LEAST16_MIN
#  define INT_LEAST16_MIN  (-1 << (@BITSIZEOF_INT_LEAST16_T@ - 1))
# endif
#else
# define INT_LEAST16_MIN  INT16_MIN
#endif
#if @HAVE_INT_LEAST16_T@
# ifndef INT_LEAST16_MAX
#  define INT_LEAST16_MAX  (~ (-1 << (@BITSIZEOF_INT_LEAST16_T@ - 1)))
# endif
#else
# define INT_LEAST16_MAX  INT16_MAX
#endif
#if @HAVE_UINT_LEAST16_T@
# ifndef UINT_LEAST16_MAX
#  if @BITSIZEOF_UINT_LEAST16_T@ < @BITSIZEOF_UNSIGNED_INT@
#   define UINT_LEAST16_MAX  (((1 << (@BITSIZEOF_UINT_LEAST16_T@ - 1)) - 1) * 2 + 1)
#  else
#   define UINT_LEAST16_MAX  (((1U << (@BITSIZEOF_UINT_LEAST16_T@ - 1)) - 1) * 2 + 1)
#  endif
# endif
#else
# define UINT_LEAST16_MAX  UINT16_MAX
#endif

#if @HAVE_INT_LEAST32_T@
# ifndef INT_LEAST32_MIN
#  define INT_LEAST32_MIN  (-1 << (@BITSIZEOF_INT_LEAST32_T@ - 1))
# endif
#else
# define INT_LEAST32_MIN  INT32_MIN
#endif
#if @HAVE_INT_LEAST32_T@
# ifndef INT_LEAST32_MAX
#  define INT_LEAST32_MAX  (~ (-1 << (@BITSIZEOF_INT_LEAST32_T@ - 1)))
# endif
#else
# define INT_LEAST32_MAX  INT32_MAX
#endif
#if @HAVE_UINT_LEAST32_T@
# ifndef UINT_LEAST32_MAX
#  if @BITSIZEOF_UINT_LEAST32_T@ < @BITSIZEOF_UNSIGNED_INT@
#   define UINT_LEAST32_MAX  (((1 << (@BITSIZEOF_UINT_LEAST32_T@ - 1)) - 1) * 2 + 1)
#  else
#   define UINT_LEAST32_MAX  (((1U << (@BITSIZEOF_UINT_LEAST32_T@ - 1)) - 1) * 2 + 1)
#  endif
# endif
#else
# define UINT_LEAST32_MAX  UINT32_MAX
#endif

#if @HAVE_INT_LEAST64_T@
# ifndef INT_LEAST64_MIN
#  if @HAVE_LONG_64BIT@
#   define INT_LEAST64_MIN  (-1L << (@BITSIZEOF_INT_LEAST64_T@ - 1))
#  elif @HAVE_LONG_LONG_64BIT@
#   define INT_LEAST64_MIN  (-1LL << (@BITSIZEOF_INT_LEAST64_T@ - 1))
#  elif defined _MSC_VER
#   define INT_LEAST64_MIN  (-1i64 << (@BITSIZEOF_INT_LEAST64_T@ - 1))
#  endif
# endif
#else
# ifdef _STDINT_H_HAVE_INT64
#  define INT_LEAST64_MIN  INT64_MIN
# endif
#endif
#if @HAVE_INT_LEAST64_T@
# ifndef INT_LEAST64_MAX
#  if @HAVE_LONG_64BIT@
#   define INT_LEAST64_MAX  (~ (-1L << (@BITSIZEOF_INT_LEAST64_T@ - 1)))
#  elif @HAVE_LONG_LONG_64BIT@
#   define INT_LEAST64_MAX  (~ (-1LL << (@BITSIZEOF_INT_LEAST64_T@ - 1)))
#  elif defined _MSC_VER
#   define INT_LEAST64_MAX  (~ (-1i64 << (@BITSIZEOF_INT_LEAST64_T@ - 1)))
#  endif
# endif
#else
# ifdef _STDINT_H_HAVE_INT64
#  define INT_LEAST64_MAX  INT64_MAX
# endif
#endif
#if @HAVE_UINT_LEAST64_T@
# ifndef UINT_LEAST64_MAX
#  if @HAVE_LONG_64BIT@
#   define UINT_LEAST64_MAX  (((1UL << (@BITSIZEOF_UINT_LEAST64_T@ - 1)) - 1) * 2 + 1)
#  elif @HAVE_LONG_LONG_64BIT@
#   define UINT_LEAST64_MAX  (((1ULL << (@BITSIZEOF_UINT_LEAST64_T@ - 1)) - 1) * 2 + 1)
#  elif defined _MSC_VER
#   define UINT_LEAST64_MAX  (((1ui64 << (@BITSIZEOF_UINT_LEAST64_T@ - 1)) - 1) * 2 + 1)
#  endif
# endif
#else
# ifdef _STDINT_H_HAVE_UINT64
#  define UINT_LEAST64_MAX  UINT64_MAX
# endif
#endif

/* 7.18.2.3. Limits of fastest minimum-width integer types */

/* Here we assume a standard architecture where the hardware integer
   types have 8, 16, 32, optionally 64 bits. Therefore the fastN_t types
   are taken from the same list of types.  */

#if @HAVE_INT_FAST8_T@
# ifndef INT_FAST8_MIN
#  define INT_FAST8_MIN  (-1L << (@BITSIZEOF_INT_FAST8_T@ - 1))
# endif
#else
# if _STDINT_H_INT64_FASTER_THAN_INT32
#  define INT_FAST8_MIN  INT64_MIN
# else
#  define INT_FAST8_MIN  INT32_MIN
# endif
#endif
#if @HAVE_INT_FAST8_T@
# ifndef INT_FAST8_MAX
#  define INT_FAST8_MAX  (~ (-1L << (@BITSIZEOF_INT_FAST8_T@ - 1)))
# endif
#else
# if _STDINT_H_INT64_FASTER_THAN_INT32
#  define INT_FAST8_MAX  INT64_MAX
# else
#  define INT_FAST8_MAX  INT32_MAX
# endif
#endif
#if @HAVE_UINT_FAST8_T@
# ifndef UINT_FAST8_MAX
#  if @BITSIZEOF_UINT_FAST8_T@ < @BITSIZEOF_UNSIGNED_INT@
#   define UINT_FAST8_MAX  (((1 << (@BITSIZEOF_UINT_FAST8_T@ - 1)) - 1) * 2 + 1)
#  else
#   define UINT_FAST8_MAX  (((1UL << (@BITSIZEOF_UINT_FAST8_T@ - 1)) - 1) * 2 + 1)
#  endif
# endif
#else
# if _STDINT_H_INT64_FASTER_THAN_INT32
#  define UINT_FAST8_MAX  UINT64_MAX
# else
#  define UINT_FAST8_MAX  UINT32_MAX
# endif
#endif

#if @HAVE_INT_FAST16_T@
# ifndef INT_FAST16_MIN
#  define INT_FAST16_MIN  (-1L << (@BITSIZEOF_INT_FAST16_T@ - 1))
# endif
#else
# if _STDINT_H_INT64_FASTER_THAN_INT32
#  define INT_FAST16_MIN  INT64_MIN
# else
#  define INT_FAST16_MIN  INT32_MIN
# endif
#endif
#if @HAVE_INT_FAST16_T@
# ifndef INT_FAST16_MAX
#  define INT_FAST16_MAX  (~ (-1L << (@BITSIZEOF_INT_FAST16_T@ - 1)))
# endif
#else
# if _STDINT_H_INT64_FASTER_THAN_INT32
#  define INT_FAST16_MAX  INT64_MAX
# else
#  define INT_FAST16_MAX  INT32_MAX
# endif
#endif
#if @HAVE_UINT_FAST16_T@
# ifndef UINT_FAST16_MAX
#  if @BITSIZEOF_UINT_FAST16_T@ < @BITSIZEOF_UNSIGNED_INT@
#   define UINT_FAST16_MAX  (((1 << (@BITSIZEOF_UINT_FAST16_T@ - 1)) - 1) * 2 + 1)
#  else
#   define UINT_FAST16_MAX  (((1UL << (@BITSIZEOF_UINT_FAST16_T@ - 1)) - 1) * 2 + 1)
#  endif
# endif
#else
# if _STDINT_H_INT64_FASTER_THAN_INT32
#  define UINT_FAST16_MAX  UINT64_MAX
# else
#  define UINT_FAST16_MAX  UINT32_MAX
# endif
#endif

#if @HAVE_INT_FAST32_T@
# ifndef INT_FAST32_MIN
#  define INT_FAST32_MIN  (-1L << (@BITSIZEOF_INT_FAST32_T@ - 1))
# endif
#else
# if _STDINT_H_INT64_FASTER_THAN_INT32
#  define INT_FAST32_MIN  INT64_MIN
# else
#  define INT_FAST32_MIN  INT32_MIN
# endif
#endif
#if @HAVE_INT_FAST32_T@
# ifndef INT_FAST32_MAX
#  define INT_FAST32_MAX  (~ (-1L << (@BITSIZEOF_INT_FAST32_T@ - 1)))
# endif
#else
# if _STDINT_H_INT64_FASTER_THAN_INT32
#  define INT_FAST32_MAX  INT64_MAX
# else
#  define INT_FAST32_MAX  INT32_MAX
# endif
#endif
#if @HAVE_UINT_FAST32_T@
# ifndef UINT_FAST32_MAX
#  if @BITSIZEOF_UINT_FAST32_T@ < @BITSIZEOF_UNSIGNED_INT@
#   define UINT_FAST32_MAX  (((1 << (@BITSIZEOF_UINT_FAST32_T@ - 1)) - 1) * 2 + 1)
#  else
#   define UINT_FAST32_MAX  (((1UL << (@BITSIZEOF_UINT_FAST32_T@ - 1)) - 1) * 2 + 1)
#  endif
# endif
#else
# if _STDINT_H_INT64_FASTER_THAN_INT32
#  define UINT_FAST32_MAX  UINT64_MAX
# else
#  define UINT_FAST32_MAX  UINT32_MAX
# endif
#endif

#if @HAVE_INT_FAST64_T@
# ifndef INT_FAST64_MIN
#  if @HAVE_LONG_64BIT@
#   define INT_FAST64_MIN  (-1L << (@BITSIZEOF_INT_FAST64_T@ - 1))
#  elif @HAVE_LONG_LONG_64BIT@
#   define INT_FAST64_MIN  (-1LL << (@BITSIZEOF_INT_FAST64_T@ - 1))
#  elif defined _MSC_VER
#   define INT_FAST64_MIN  (-1i64 << (@BITSIZEOF_INT_FAST64_T@ - 1))
#  endif
# endif
#else
# ifdef _STDINT_H_HAVE_INT64
#  define INT_FAST64_MIN  INT64_MIN
# endif
#endif
#if @HAVE_INT_FAST64_T@
# ifndef INT_FAST64_MAX
#  if @HAVE_LONG_64BIT@
#   define INT_FAST64_MAX  (~ (-1L << (@BITSIZEOF_INT_FAST64_T@ - 1)))
#  elif @HAVE_LONG_LONG_64BIT@
#   define INT_FAST64_MAX  (~ (-1LL << (@BITSIZEOF_INT_FAST64_T@ - 1)))
#  elif defined _MSC_VER
#   define INT_FAST64_MAX  (~ (-1i64 << (@BITSIZEOF_INT_FAST64_T@ - 1)))
#  endif
# endif
#else
# ifdef _STDINT_H_HAVE_INT64
#  define INT_FAST64_MAX  INT64_MAX
# endif
#endif
#if @HAVE_UINT_FAST64_T@
# ifndef UINT_FAST64_MAX
#  if @HAVE_LONG_64BIT@
#   define UINT_FAST64_MAX  (((1UL << (@BITSIZEOF_UINT_FAST64_T@ - 1)) - 1) * 2 + 1)
#  elif @HAVE_LONG_LONG_64BIT@
#   define UINT_FAST64_MAX  (((1ULL << (@BITSIZEOF_UINT_FAST64_T@ - 1)) - 1) * 2 + 1)
#  elif defined _MSC_VER
#   define UINT_FAST64_MAX  (((1ui64 << (@BITSIZEOF_UINT_FAST64_T@ - 1)) - 1) * 2 + 1)
#  endif
# endif
#else
# ifdef _STDINT_H_HAVE_UINT64
#  define UINT_FAST64_MAX  UINT64_MAX
# endif
#endif

/* 7.18.2.4. Limits of integer types capable of holding object pointers */

#if @HAVE_INTPTR_T@
# ifndef INTPTR_MIN
#  if @BITSIZEOF_INTPTR_T@ > @BITSIZEOF_LONG@
#   define INTPTR_MIN  (-1LL << (@BITSIZEOF_INTPTR_T@ - 1))
#  else
#   define INTPTR_MIN  (-1L << (@BITSIZEOF_INTPTR_T@ - 1))
#  endif
# endif
#else
# define INTPTR_MIN  LONG_MIN
#endif
#if @HAVE_INTPTR_T@
# ifndef INTPTR_MAX
#  if @BITSIZEOF_INTPTR_T@ > @BITSIZEOF_LONG@
#   define INTPTR_MAX  (~ (-1LL << (@BITSIZEOF_INTPTR_T@ - 1)))
#  else
#   define INTPTR_MAX  (~ (-1L << (@BITSIZEOF_INTPTR_T@ - 1)))
#  endif
# endif
#else
# define INTPTR_MAX  LONG_MAX
#endif
#if @HAVE_UINTPTR_T@
# ifndef UINTPTR_MAX
#  if @BITSIZEOF_UINTPTR_T@ > @BITSIZEOF_UNSIGNED_LONG@
#   define UINTPTR_MAX  (((1ULL << (@BITSIZEOF_UINTPTR_T@ - 1)) - 1) * 2 + 1)
#  else
#   define UINTPTR_MAX  (((1UL << (@BITSIZEOF_UINTPTR_T@ - 1)) - 1) * 2 + 1)
#  endif
# endif
#else
# define UINTPTR_MAX  ULONG_MAX
#endif

/* 7.18.2.5. Limits of greatest-width integer types */

#if @HAVE_INTMAX_T@
# ifndef INTMAX_MIN
#  if @BITSIZEOF_INTMAX_T@ > @BITSIZEOF_LONG@
#   define INTMAX_MIN  (-1LL << (@BITSIZEOF_INTMAX_T@ - 1))
#  else
#   define INTMAX_MIN  (-1L << (@BITSIZEOF_INTMAX_T@ - 1))
#  endif
# endif
#else
# ifdef _STDINT_H_HAVE_INT64
#  define INTMAX_MIN  INT64_MIN
# else
#  define INTMAX_MIN  INT32_MIN
# endif
#endif
#if @HAVE_INTMAX_T@
# ifndef INTMAX_MAX
#  if @BITSIZEOF_INTMAX_T@ > @BITSIZEOF_LONG@
#   define INTMAX_MAX  (~ (-1LL << (@BITSIZEOF_INTMAX_T@ - 1)))
#  else
#   define INTMAX_MAX  (~ (-1L << (@BITSIZEOF_INTMAX_T@ - 1)))
#  endif
# endif
#else
# ifdef _STDINT_H_HAVE_INT64
#  define INTMAX_MAX  INT64_MAX
# else
#  define INTMAX_MAX  INT32_MAX
# endif
#endif
#if @HAVE_UINTMAX_T@
# ifndef UINTMAX_MAX
#  if @BITSIZEOF_UINTMAX_T@ > @BITSIZEOF_UNSIGNED_LONG@
#   define UINTMAX_MAX  (((1ULL << (@BITSIZEOF_UINTMAX_T@ - 1)) - 1) * 2 + 1)
#  else
#   define UINTMAX_MAX  (((1UL << (@BITSIZEOF_UINTMAX_T@ - 1)) - 1) * 2 + 1)
#  endif
# endif
#else
# ifdef _STDINT_H_HAVE_INT64
#  define UINTMAX_MAX  UINT64_MAX
# else
#  define UINTMAX_MAX  UINT32_MAX
# endif
#endif

/* 7.18.3. Limits of other integer types */

/* ptrdiff_t limits */
#ifndef PTRDIFF_MIN
# if @BITSIZEOF_PTRDIFF_T@ > @BITSIZEOF_LONG@ || @SAME_TYPE_PTRDIFF_T_LONG@
#  define PTRDIFF_MIN  (-1L << (@BITSIZEOF_PTRDIFF_T@ - 1))
# else
#  define PTRDIFF_MIN  (-1 << (@BITSIZEOF_PTRDIFF_T@ - 1))
# endif
#endif
#ifndef PTRDIFF_MAX
# if @BITSIZEOF_PTRDIFF_T@ > @BITSIZEOF_LONG@ || @SAME_TYPE_PTRDIFF_T_LONG@
#  define PTRDIFF_MAX  (~ (-1L << (@BITSIZEOF_PTRDIFF_T@ - 1)))
# else
#  define PTRDIFF_MAX  (~ (-1 << (@BITSIZEOF_PTRDIFF_T@ - 1)))
# endif
#endif

/* sig_atomic_t limits */
#ifndef SIG_ATOMIC_MIN
# if @HAVE_SIGNED_SIG_ATOMIC_T@
#  if @BITSIZEOF_SIG_ATOMIC_T@ > @BITSIZEOF_LONG@ || @SAME_TYPE_SIG_ATOMIC_T_LONG@
#   define SIG_ATOMIC_MIN  (-1L << (@BITSIZEOF_SIG_ATOMIC_T@ - 1))
#  else
#   define SIG_ATOMIC_MIN  (-1 << (@BITSIZEOF_SIG_ATOMIC_T@ - 1))
#  endif
# else
#  if @BITSIZEOF_SIG_ATOMIC_T@ > @BITSIZEOF_UNSIGNED_LONG@ || @SAME_TYPE_SIG_ATOMIC_T_UNSIGNED_LONG@
#   define SIG_ATOMIC_MIN  0UL
#  elif @BITSIZEOF_SIG_ATOMIC_T@ >= @BITSIZEOF_UNSIGNED_INT@
#   define SIG_ATOMIC_MIN  0U
#  else
#   define SIG_ATOMIC_MIN  0
#  endif
# endif
#endif
#ifndef SIG_ATOMIC_MAX
# if @HAVE_SIGNED_SIG_ATOMIC_T@
#  if @BITSIZEOF_SIG_ATOMIC_T@ > @BITSIZEOF_LONG@ || @SAME_TYPE_SIG_ATOMIC_T_LONG@
#   define SIG_ATOMIC_MAX  (~ (-1L << (@BITSIZEOF_SIG_ATOMIC_T@ - 1)))
#  else
#   define SIG_ATOMIC_MAX  (~ (-1 << (@BITSIZEOF_SIG_ATOMIC_T@ - 1)))
#  endif
# else
#  if @BITSIZEOF_SIG_ATOMIC_T@ > @BITSIZEOF_UNSIGNED_LONG@ || @SAME_TYPE_SIG_ATOMIC_T_UNSIGNED_LONG@
#   define SIG_ATOMIC_MAX  (((1UL << (@BITSIZEOF_SIG_ATOMIC_T@ - 1)) - 1) * 2 + 1)
#  elif @BITSIZEOF_SIG_ATOMIC_T@ >= @BITSIZEOF_UNSIGNED_INT@
#   define SIG_ATOMIC_MAX  (((1U << (@BITSIZEOF_SIG_ATOMIC_T@ - 1)) - 1) * 2 + 1)
#  else
#   define SIG_ATOMIC_MAX  (((1 << (@BITSIZEOF_SIG_ATOMIC_T@ - 1)) - 1) * 2 + 1)
#  endif
# endif
#endif

/* size_t limit */
#ifndef SIZE_MAX /* SIZE_MAX may also be defined in config.h. */
# if @BITSIZEOF_SIZE_T@ > @BITSIZEOF_UNSIGNED_LONG@ || @SAME_TYPE_SIZE_T_UNSIGNED_LONG@
#  define SIZE_MAX  (((1UL << (@BITSIZEOF_SIZE_T@ - 1)) - 1) * 2 + 1)
# else
#  define SIZE_MAX  (((1U << (@BITSIZEOF_SIZE_T@ - 1)) - 1) * 2 + 1)
# endif
#endif

/* wchar_t limits may already be defined in <stddef.h>.  */
#ifndef WCHAR_MIN
# if @HAVE_SIGNED_WCHAR_T@
#  if @BITSIZEOF_WCHAR_T@ > @BITSIZEOF_LONG@ || @SAME_TYPE_WCHAR_T_LONG@
#   define WCHAR_MIN  (-1L << (@BITSIZEOF_WCHAR_T@ - 1))
#  else
#   define WCHAR_MIN  (-1 << (@BITSIZEOF_WCHAR_T@ - 1))
#  endif
# else
#  if @BITSIZEOF_WCHAR_T@ > @BITSIZEOF_UNSIGNED_LONG@ || @SAME_TYPE_WCHAR_T_UNSIGNED_LONG@
#   define WCHAR_MIN  0UL
#  elif @BITSIZEOF_WCHAR_T@ >= @BITSIZEOF_UNSIGNED_INT@
#   define WCHAR_MIN  0U
#  else
#   define WCHAR_MIN  0
#  endif
# endif
#endif
#ifndef WCHAR_MAX
# if @HAVE_SIGNED_WCHAR_T@
#  if @BITSIZEOF_WCHAR_T@ > @BITSIZEOF_LONG@ || @SAME_TYPE_WCHAR_T_LONG@
#   define WCHAR_MAX  (~ (-1L << (@BITSIZEOF_WCHAR_T@ - 1)))
#  else
#   define WCHAR_MAX  (~ (-1 << (@BITSIZEOF_WCHAR_T@ - 1)))
#  endif
# else
#  if @BITSIZEOF_WCHAR_T@ > @BITSIZEOF_UNSIGNED_LONG@ || @SAME_TYPE_WCHAR_T_UNSIGNED_LONG@
#   define WCHAR_MAX  (((1UL << (@BITSIZEOF_WCHAR_T@ - 1)) - 1) * 2 + 1)
#  elif @BITSIZEOF_WCHAR_T@ >= @BITSIZEOF_UNSIGNED_INT@
#   define WCHAR_MAX  (((1U << (@BITSIZEOF_WCHAR_T@ - 1)) - 1) * 2 + 1)
#  else
#   define WCHAR_MAX  (((1 << (@BITSIZEOF_WCHAR_T@ - 1)) - 1) * 2 + 1)
#  endif
# endif
#endif

/* wint_t limits */
#ifndef WINT_MIN
# if @HAVE_SIGNED_WINT_T@
#  if @BITSIZEOF_WINT_T@ > @BITSIZEOF_LONG@ || @SAME_TYPE_WINT_T_LONG@
#   define WINT_MIN  (-1L << (@BITSIZEOF_WINT_T@ - 1))
#  else
#   define WINT_MIN  (-1 << (@BITSIZEOF_WINT_T@ - 1))
#  endif
# else
#  if @BITSIZEOF_WINT_T@ > @BITSIZEOF_UNSIGNED_LONG@ || @SAME_TYPE_WINT_T_UNSIGNED_LONG@
#   define WINT_MIN  0UL
#  elif @BITSIZEOF_WINT_T@ >= @BITSIZEOF_UNSIGNED_INT@
#   define WINT_MIN  0U
#  else
#   define WINT_MIN  0
#  endif
# endif
#endif
#ifndef WINT_MAX
# if @HAVE_SIGNED_WINT_T@
#  if @BITSIZEOF_WINT_T@ > @BITSIZEOF_LONG@ || @SAME_TYPE_WINT_T_LONG@
#   define WINT_MAX  (~ (-1L << (@BITSIZEOF_WINT_T@ - 1)))
#  else
#   define WINT_MAX  (~ (-1 << (@BITSIZEOF_WINT_T@ - 1)))
#  endif
# else
#  if @BITSIZEOF_WINT_T@ > @BITSIZEOF_UNSIGNED_LONG@ || @SAME_TYPE_WINT_T_UNSIGNED_LONG@
#   define WINT_MAX  (((1UL << (@BITSIZEOF_WINT_T@ - 1)) - 1) * 2 + 1)
#  elif @BITSIZEOF_WINT_T@ >= @BITSIZEOF_UNSIGNED_INT@
#   define WINT_MAX  (((1U << (@BITSIZEOF_WINT_T@ - 1)) - 1) * 2 + 1)
#  else
#   define WINT_MAX  (((1 << (@BITSIZEOF_WINT_T@ - 1)) - 1) * 2 + 1)
#  endif
# endif
#endif

#endif

/* 7.18.4. Macros for integer constants */

#if !defined(__cplusplus) || defined(__STDC_CONSTANT_MACROS)

/* 7.18.4.1. Macros for minimum-width integer constants */
/* According to ISO C 99 Technical Corrigendum 1 */

#undef INT8_C
#undef UINT8_C
#define INT8_C(x) x
#if @HAVE_UINT8_T@
# if @BITSIZEOF_UINT8_T@ < @BITSIZEOF_UNSIGNED_INT@
#  define UINT8_C(x) x
# else
#  define UINT8_C(x) x##U
# endif
#else
# define UINT8_C(x) x
#endif

#undef INT16_C
#undef UINT16_C
#define INT16_C(x) x
#if @HAVE_UINT16_T@
# if @BITSIZEOF_UINT16_T@ < @BITSIZEOF_UNSIGNED_INT@
#  define UINT16_C(x) x
# else
#  define UINT16_C(x) x##U
# endif
#else
# define UINT16_C(x) x
#endif

#undef INT32_C
#undef UINT32_C
#define INT32_C(x) x
#if @HAVE_UINT32_T@
# if @BITSIZEOF_UINT32_T@ < @BITSIZEOF_UNSIGNED_INT@
#  define UINT32_C(x) x
# else
#  define UINT32_C(x) x##U
# endif
#else
# define UINT32_C(x) x
#endif

#undef INT64_C
#undef UINT64_C
#if @HAVE_LONG_64BIT@
# define INT64_C(x) x##L
# define UINT64_C(x) x##UL
#elif @HAVE_LONG_LONG_64BIT@
# define INT64_C(x) x##LL
# define UINT64_C(x) x##ULL
#elif defined(_MSC_VER)
# define INT64_C(x) x##i64
# define UINT64_C(x) x##ui64
#endif

/* 7.18.4.2. Macros for greatest-width integer constants */

#undef INTMAX_C
#undef UINTMAX_C
#if @HAVE_LONG_64BIT@
# define INTMAX_C(x) x##L
# define UINTMAX_C(x) x##UL
#elif @HAVE_LONG_LONG_64BIT@
# define INTMAX_C(x) x##LL
# define UINTMAX_C(x) x##ULL
#elif defined(_MSC_VER)
# define INTMAX_C(x) x##i64
# define UINTMAX_C(x) x##ui64
#else
# define INTMAX_C(x) x
# define UINTMAX_C(x) x##U
#endif

#endif

#endif /* _STDINT_H */

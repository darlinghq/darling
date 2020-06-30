# stdint.m4 serial 11
dnl Copyright (C) 2001-2002, 2004-2006 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Bruno Haible.
dnl Test whether <stdint.h> is supported or must be substituted.

AC_DEFUN([gl_STDINT_H],
[
  dnl Check for <wchar.h>.
  AC_CHECK_HEADERS_ONCE([wchar.h])
  if test $ac_cv_header_wchar_h = yes; then
    HAVE_WCHAR_H=1
  else
    HAVE_WCHAR_H=0
  fi
  AC_SUBST([HAVE_WCHAR_H])

  dnl Check for <stdint.h> that doesn't clash with <sys/types.h>.
  gl_HEADER_STDINT_H
  if test $gl_cv_header_stdint_h = yes; then
    ac_cv_header_stdint_h=yes; dnl Hack for gl_FULL_HEADER_PATH.
    gl_FULL_HEADER_PATH([stdint.h])
    FULL_PATH_STDINT_H='<'$gl_cv_full_path_stdint_h'>'
    AC_SUBST([FULL_PATH_STDINT_H])
    HAVE_STDINT_H=1
  else
    HAVE_STDINT_H=0
  fi
  AC_SUBST([HAVE_STDINT_H])

  dnl Check for <inttypes.h> that doesn't clash with <sys/types.h>.
  gl_HEADER_INTTYPES_H
  if test $gl_cv_header_inttypes_h = yes; then
    ac_cv_header_inttypes_h=yes; dnl Hack for gl_FULL_HEADER_PATH.
    gl_FULL_HEADER_PATH([inttypes.h])
    FULL_PATH_INTTYPES_H='<'$gl_cv_full_path_inttypes_h'>'
    AC_SUBST([FULL_PATH_INTTYPES_H])
    HAVE_INTTYPES_H=1
  else
    HAVE_INTTYPES_H=0
  fi
  AC_SUBST([HAVE_INTTYPES_H])

  dnl Check for <sys/inttypes.h>.
  AC_CHECK_HEADERS([sys/inttypes.h])
  if test $ac_cv_header_sys_inttypes_h = yes; then
    HAVE_SYS_INTTYPES_H=1
  else
    HAVE_SYS_INTTYPES_H=0
  fi
  AC_SUBST([HAVE_SYS_INTTYPES_H])

  dnl Check for <sys/bitypes.h> (used in Linux libc4 >= 4.6.7 and libc5).
  AC_CHECK_HEADERS([sys/bitypes.h])
  if test $ac_cv_header_sys_bitypes_h = yes; then
    HAVE_SYS_BITYPES_H=1
  else
    HAVE_SYS_BITYPES_H=0
  fi
  AC_SUBST([HAVE_SYS_BITYPES_H])

  dnl Is long == int64_t ?
  AC_CACHE_CHECK([whether 'long' is 64 bit wide], gl_cv_long_bitsize_64, [
    AC_TRY_COMPILE([
#define POW63  ((((((long) 1 << 15) << 15) << 15) << 15) << 3)
#define POW64  ((((((long) 1 << 15) << 15) << 15) << 15) << 4)
typedef int array [2 * (POW63 != 0 && POW64 == 0) - 1];
], , gl_cv_long_bitsize_64=yes, gl_cv_long_bitsize_64=no)])
  if test $gl_cv_long_bitsize_64 = yes; then
    HAVE_LONG_64BIT=1
  else
    HAVE_LONG_64BIT=0
  fi
  AC_SUBST(HAVE_LONG_64BIT)

  dnl Is long long == int64_t ?
  AC_CACHE_CHECK([whether 'long long' is 64 bit wide], gl_cv_longlong_bitsize_64, [
    AC_TRY_COMPILE([
#define POW63  ((((((long long) 1 << 15) << 15) << 15) << 15) << 3)
#define POW64  ((((((long long) 1 << 15) << 15) << 15) << 15) << 4)
typedef int array [2 * (POW63 != 0 && POW64 == 0) - 1];
], , gl_cv_longlong_bitsize_64=yes, gl_cv_longlong_bitsize_64=no)])
  if test $gl_cv_longlong_bitsize_64 = yes; then
    HAVE_LONG_LONG_64BIT=1
  else
    HAVE_LONG_LONG_64BIT=0
  fi
  AC_SUBST(HAVE_LONG_LONG_64BIT)

  dnl Here we use FULL_PATH_INTTYPES_H and FULL_PATH_STDINT_H, not just
  dnl <inttypes.h> and <stdint.h>, so that it also works during a
  dnl "config.status --recheck" if an inttypes.h or stdint.h have been
  dnl created in the build directory.
  other_includes='
/* Get those types that are already defined in other system include files.  */
#if defined(__FreeBSD__) && (__FreeBSD__ >= 3) && (__FreeBSD__ <= 4)
# include <sys/inttypes.h>
#endif
#if defined(__OpenBSD__) || defined(__bsdi__) || defined(__sgi)
# include <sys/types.h>
# if HAVE_INTTYPES_H
#  include FULL_PATH_INTTYPES_H
# endif
#endif
#if defined(__linux__) && HAVE_SYS_BITYPES_H
# include <sys/bitypes.h>
#endif
#if defined(__sun) && HAVE_SYS_INTTYPES_H
# include <sys/inttypes.h>
#endif
#if (defined(__hpux) || defined(_AIX)) && HAVE_INTTYPES_H
# include FULL_PATH_INTTYPES_H
#endif
#if HAVE_STDINT_H && !(defined(__sgi) && HAVE_INTTYPES_H && !defined(__c99))
# include FULL_PATH_STDINT_H
#endif
'
  gl_STDINT_CHECK_TYPES(
    [int8_t int16_t int32_t int64_t \
     uint8_t uint16_t uint32_t uint64_t \
     int_least8_t int_least16_t int_least32_t int_least64_t \
     uint_least8_t uint_least16_t uint_least32_t uint_least64_t \
     int_fast8_t int_fast16_t int_fast32_t int_fast64_t \
     uint_fast8_t uint_fast16_t uint_fast32_t uint_fast64_t \
     intptr_t uintptr_t \
     intmax_t uintmax_t],
    [$other_includes],
    [gl_cv_type_], [], [])

  dnl Now see if we need a substitute <stdint.h>.
  gl_cv_header_working_stdint_h=no
  if test $gl_cv_header_stdint_h = yes; then
    gl_STDINT_CHECK_TYPES(
      [int64_t uint64_t \
       int_least64_t uint_least64_t \
       int_fast64_t uint_fast64_t],
      [#include <stdint.h>],
      [gl_cv_stdint_], [_IN_STDINT_H], [in <stdint.h>])
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM([
#define __STDC_LIMIT_MACROS 1 /* to make it work also in C++ mode */
#include <stdint.h>
int8_t a1 = INT8_C (17);
int16_t a2 = INT16_C (17);
int32_t a3 = INT32_C (17);
#if HAVE_INT64_T_IN_STDINT_H
int64_t a4 = INT64_C (17);
#endif
uint8_t b1 = UINT8_C (17);
uint16_t b2 = UINT16_C (17);
uint32_t b3 = UINT32_C (17);
#if HAVE_UINT64_T_IN_STDINT_H
uint64_t b4 = UINT64_C (17);
#endif
int_least8_t c1 = 17;
int_least16_t c2 = 17;
int_least32_t c3 = 17;
#if HAVE_INT_LEAST64_T_IN_STDINT_H
int_least64_t c4 = 17;
#endif
uint_least8_t d1 = 17;
uint_least16_t d2 = 17;
uint_least32_t d3 = 17;
#if HAVE_UINT_LEAST64_T_IN_STDINT_H
uint_least64_t d4 = 17;
#endif
int_fast8_t e1 = 17;
int_fast16_t e2 = 17;
int_fast32_t e3 = 17;
#if HAVE_INT_FAST64_T_IN_STDINT_H
int_fast64_t e4 = 17;
#endif
uint_fast8_t f1 = 17;
uint_fast16_t f2 = 17;
uint_fast32_t f3 = 17;
#if HAVE_UINT_FAST64_T_IN_STDINT_H
uint_fast64_t f4 = 17;
#endif
intptr_t g = 17;
uintptr_t h = 17;
intmax_t i = INTMAX_C (17);
uintmax_t j = UINTMAX_C (17);
      ])],
      [gl_cv_header_working_stdint_h=yes])
  fi
  if test $gl_cv_header_working_stdint_h = yes; then
    dnl Use the existing <stdint.h>, adding missing macro definitions.
    suff64=
    suffu64=
    if test $HAVE_LONG_64BIT = 1; then
      suff64=L
      suffu64=UL
    else
      if test $HAVE_LONG_LONG_64BIT = 1; then
        suff64=LL
        suffu64=ULL
      else
        AC_EGREP_CPP([msvc compiler], [
#ifdef _MSC_VER
msvc compiler
#endif
          ], [
          suff64=i64
          suffu64=ui64
        ])
      fi
    fi
    dnl Here we assume a standard architecture where the hardware integer
    dnl types have 8, 16, 32, optionally 64 bits.
    gl_STDINT_MISSING_BOUND([INT8_MIN], [-128],
      [Define if <stdint.h> doesn't define it.])
    gl_STDINT_MISSING_BOUND([INT8_MAX], [127],
      [Define if <stdint.h> doesn't define it.])
    gl_STDINT_MISSING_BOUND([UINT8_MAX], [255],
      [Define if <stdint.h> doesn't define it.])
    gl_STDINT_MISSING_BOUND([INT16_MIN], [-32768],
      [Define if <stdint.h> doesn't define it.])
    gl_STDINT_MISSING_BOUND([INT16_MAX], [32767],
      [Define if <stdint.h> doesn't define it.])
    gl_STDINT_MISSING_BOUND([UINT16_MAX], [65535],
      [Define if <stdint.h> doesn't define it.])
    gl_STDINT_MISSING_BOUND([INT32_MIN], [(~INT32_MAX)],
      [Define if <stdint.h> doesn't define it.])
    gl_STDINT_MISSING_BOUND([INT32_MAX], [2147483647],
      [Define if <stdint.h> doesn't define it.])
    gl_STDINT_MISSING_BOUND([UINT32_MAX], [4294967295U],
      [Define if <stdint.h> doesn't define it.])
    if test $gl_cv_stdint_int64_t = yes; then
      gl_STDINT_MISSING_BOUND([INT64_MIN], [(~INT64_MAX)],
        [Define if <stdint.h> doesn't define it but has the int64_t type.])
      gl_STDINT_MISSING_BOUND([INT64_MAX], [9223372036854775807${suff64}],
        [Define if <stdint.h> doesn't define it but has the int64_t type.])
    fi
    if test $gl_cv_stdint_uint64_t = yes; then
      gl_STDINT_MISSING_BOUND([UINT64_MAX], [18446744073709551615${suffu64}],
        [Define if <stdint.h> doesn't define it but has the uint64_t type.])
    fi
    dnl Here we assume a standard architecture where the hardware integer
    dnl types have 8, 16, 32, optionally 64 bits. Therefore the leastN_t types
    dnl are the same as the corresponding N_t types.
    gl_STDINT_MISSING_BOUND([INT_LEAST8_MIN], [INT8_MIN],
      [Define if <stdint.h> doesn't define it.])
    gl_STDINT_MISSING_BOUND([INT_LEAST8_MAX], [INT8_MAX],
      [Define if <stdint.h> doesn't define it.])
    gl_STDINT_MISSING_BOUND([UINT_LEAST8_MAX], [UINT8_MAX],
      [Define if <stdint.h> doesn't define it.])
    gl_STDINT_MISSING_BOUND([INT_LEAST16_MIN], [INT16_MIN],
      [Define if <stdint.h> doesn't define it.])
    gl_STDINT_MISSING_BOUND([INT_LEAST16_MAX], [INT16_MAX],
      [Define if <stdint.h> doesn't define it.])
    gl_STDINT_MISSING_BOUND([UINT_LEAST16_MAX], [UINT16_MAX],
      [Define if <stdint.h> doesn't define it.])
    gl_STDINT_MISSING_BOUND([INT_LEAST32_MIN], [INT32_MIN],
      [Define if <stdint.h> doesn't define it.])
    gl_STDINT_MISSING_BOUND([INT_LEAST32_MAX], [INT32_MAX],
      [Define if <stdint.h> doesn't define it.])
    gl_STDINT_MISSING_BOUND([UINT_LEAST32_MAX], [UINT32_MAX],
      [Define if <stdint.h> doesn't define it.])
    if test $gl_cv_stdint_int_least64_t = yes; then
      gl_STDINT_MISSING_BOUND([INT_LEAST64_MIN], [INT64_MIN],
        [Define if <stdint.h> doesn't define it but has the int_least64_t type.])
      gl_STDINT_MISSING_BOUND([INT_LEAST64_MAX], [INT64_MAX],
        [Define if <stdint.h> doesn't define it but has the int_least64_t type.])
    fi
    if test $gl_cv_stdint_uint_least64_t = yes; then
      gl_STDINT_MISSING_BOUND([UINT_LEAST64_MAX], [UINT64_MAX],
        [Define if <stdint.h> doesn't define it but has the uint_least64_t type.])
    fi
    dnl Here we assume a standard architecture where the hardware integer
    dnl types have 8, 16, 32, optionally 64 bits. Therefore the fastN_t types
    dnl are taken from the same list of types.
    gl_STDINT_MISSING_BOUNDS([INT_FAST8_MIN INT_FAST8_MAX UINT_FAST8_MAX \
                              INT_FAST16_MIN INT_FAST16_MAX UINT_FAST16_MAX \
                              INT_FAST32_MIN INT_FAST32_MAX UINT_FAST32_MAX])
    if test $gl_cv_stdint_uint_fast64_t = yes; then
      gl_STDINT_MISSING_BOUNDS([INT_FAST64_MIN INT_FAST64_MAX])
    fi
    if test $gl_cv_stdint_uint_fast64_t = yes; then
      gl_STDINT_MISSING_BOUNDS([UINT_FAST64_MAX])
    fi
    gl_STDINT_MISSING_BOUNDS([INTPTR_MIN INTPTR_MAX UINTPTR_MAX \
                              INTMAX_MIN INTMAX_MAX UINTMAX_MAX])
    gl_STDINT_MISSING_BOUNDS([PTRDIFF_MIN PTRDIFF_MAX], [#include <stddef.h>])
    gl_SIZE_MAX
    gl_STDINT_MISSING_BOUNDS2([SIG_ATOMIC_MIN SIG_ATOMIC_MAX],
      [#include <signal.h>])
    dnl Don't bother defining WCHAR_MIN and WCHAR_MAX, since they should
    dnl already be defined in <stddef.h> or <wchar.h>.
    dnl For wint_t we need <wchar.h>.
    dnl Tru64 with Desktop Toolkit C has a bug: <stdio.h> must be included
    dnl before <wchar.h>.
    dnl BSD/OS 4.1 has a bug: <stdio.h> and <time.h> must be included before
    dnl <wchar.h>.
    gl_STDINT_MISSING_BOUNDS2([WINT_MIN WINT_MAX], [
#include <stdio.h>
#include <time.h>
#include <wchar.h>
])
    STDINT_H=''
  else

    gl_STDINT_BITSIZEOF(
      [int8_t int16_t int32_t int64_t \
       uint8_t uint16_t uint32_t uint64_t \
       int_least8_t int_least16_t int_least32_t int_least64_t \
       uint_least8_t uint_least16_t uint_least32_t uint_least64_t \
       int_fast8_t int_fast16_t int_fast32_t int_fast64_t \
       uint_fast8_t uint_fast16_t uint_fast32_t uint_fast64_t \
       intptr_t uintptr_t \
       intmax_t uintmax_t],
      [$other_includes])

    gl_cv_type_unsigned_int=yes
    gl_cv_type_long=yes
    gl_cv_type_unsigned_long=yes
    gl_STDINT_BITSIZEOF([unsigned_int long unsigned_long],
      [typedef unsigned int unsigned_int;
       typedef unsigned long unsigned_long;])

    AC_CHECK_TYPES([ptrdiff_t])
    gl_cv_type_ptrdiff_t=$ac_cv_type_ptrdiff_t
    AC_REQUIRE([AC_TYPE_SIZE_T])
    gl_cv_type_size_t=yes
    gl_STDINT_BITSIZEOF([ptrdiff_t size_t], [#include <stddef.h>])
    gl_CHECK_TYPE_SAME([ptrdiff_t], [long], [#include <stddef.h>])
    gl_CHECK_TYPE_SAME([size_t], [unsigned long], [#include <stddef.h>])

    AC_CHECK_TYPES([sig_atomic_t], , , [#include <signal.h>])
    gl_cv_type_sig_atomic_t=$ac_cv_type_sig_atomic_t
    gl_STDINT_BITSIZEOF([sig_atomic_t], [#include <signal.h>])
    gl_CHECK_TYPES_SIGNED([sig_atomic_t], [#include <signal.h>])
    if test $HAVE_SIGNED_SIG_ATOMIC_T = 1; then
      gl_CHECK_TYPE_SAME([sig_atomic_t], [long], [#include <signal.h>])
    else
      gl_CHECK_TYPE_SAME([sig_atomic_t], [unsigned long], [#include <signal.h>])
    fi

    AC_REQUIRE([gt_TYPE_WCHAR_T])
    gl_cv_type_wchar_t=$gt_cv_c_wchar_t
    gl_STDINT_BITSIZEOF([wchar_t], [#include <stddef.h>])
    gl_CHECK_TYPES_SIGNED([wchar_t], [#include <stddef.h>])
    if test $HAVE_SIGNED_WCHAR_T = 1; then
      gl_CHECK_TYPE_SAME([wchar_t], [long], [#include <stddef.h>])
    else
      gl_CHECK_TYPE_SAME([wchar_t], [unsigned long], [#include <stddef.h>])
    fi

    dnl For wint_t we need <wchar.h>.
    dnl Tru64 with Desktop Toolkit C has a bug: <stdio.h> must be included
    dnl before <wchar.h>.
    dnl BSD/OS 4.1 has a bug: <stdio.h> and <time.h> must be included before
    dnl <wchar.h>.
    AC_CHECK_TYPES([wint_t], , , [#include <stdio.h>
#include <time.h>
#include <wchar.h>
])
    gl_cv_type_wint_t=$ac_cv_type_wint_t
    gl_STDINT_BITSIZEOF([wint_t], [#include <stdio.h>
#include <time.h>
#include <wchar.h>
])
    gl_CHECK_TYPES_SIGNED([wint_t], [#include <stdio.h>
#include <time.h>
#include <wchar.h>
])
    if test $HAVE_SIGNED_WINT_T = 1; then
      gl_CHECK_TYPE_SAME([wint_t], [long], [#include <stdio.h>
#include <time.h>
#include <wchar.h>
])
    else
      gl_CHECK_TYPE_SAME([wint_t], [unsigned long], [#include <stdio.h>
#include <time.h>
#include <wchar.h>
])
    fi

    STDINT_H='stdint.h'
  fi
  AC_SUBST(STDINT_H)
])

dnl Set gl_cv_header_stdint_h to yes and define HAVE_STDINT_H if
dnl <stdint.h> exists and doesn't clash with <sys/types.h>.
AC_DEFUN([gl_HEADER_STDINT_H],
[
  dnl Check for <stdint.h> that doesn't clash with <sys/types.h>.
  AC_CACHE_CHECK([for stdint.h], gl_cv_header_stdint_h, [
    AC_TRY_COMPILE([
#include <sys/types.h>
#include <stdint.h>],
      [], gl_cv_header_stdint_h=yes, gl_cv_header_stdint_h=no)
  ])
  if test $gl_cv_header_stdint_h = yes; then
    AC_DEFINE_UNQUOTED(HAVE_STDINT_H, 1,
      [Define if <stdint.h> exists and doesn't clash with <sys/types.h>.])
  fi
])

dnl Set gl_cv_header_inttypes_h to yes and define HAVE_INTTYPES_H if
dnl <inttypes.h> exists and doesn't clash with <sys/types.h>.
AC_DEFUN([gl_HEADER_INTTYPES_H],
[
  dnl Check for <inttypes.h> that doesn't clash with <sys/types.h>.
  dnl On IRIX 5.3, <inttypes.h> conflicts with <sys/types.h>.
  AC_CACHE_CHECK([for inttypes.h], gl_cv_header_inttypes_h, [
    AC_TRY_COMPILE([
#include <sys/types.h>
#include <inttypes.h>],
      [], gl_cv_header_inttypes_h=yes, gl_cv_header_inttypes_h=no)
  ])
  if test $gl_cv_header_inttypes_h = yes; then
    AC_DEFINE_UNQUOTED(HAVE_INTTYPES_H, 1,
      [Define if <inttypes.h> exists and doesn't clash with <sys/types.h>.])
  fi
])

dnl gl_STDINT_CHECK_TYPES(TYPES, INCLUDES, CACHE_VAR_PREFIX, MACRO_SUFFIX, DESCRIPTION_SUFFIX)
dnl Check each of the given types, whether they are defined in the given
dnl include files.
AC_DEFUN([gl_STDINT_CHECK_TYPES],
[
  dnl Use a shell loop, to avoid bloating configure, and
  dnl - extra AH_TEMPLATE calls, so that autoheader knows what to put into
  dnl   config.h.in,
  dnl - extra AC_SUBST calls, so that the right substitutions are made.
  AC_FOREACH([gltype], [$1],
    [AH_TEMPLATE([HAVE_]translit(gltype,[abcdefghijklmnopqrstuvwxyz],[ABCDEFGHIJKLMNOPQRSTUVWXYZ])[$4],
       [Define to 1 if the type ']gltype[' is already defined$5.])])
  for gltype in $1 ; do
    AC_MSG_CHECKING([for $gltype])
    AC_COMPILE_IFELSE([
      AC_LANG_PROGRAM([$2
/* Test if the type exists.  */
$gltype x = 17;
      ])],
      result=yes, result=no)
    eval $3${gltype}=\$result
    AC_MSG_RESULT($result)
    GLTYPE=`echo "$gltype" | tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ`
    if test $result = yes; then
      AC_DEFINE_UNQUOTED([HAVE_${GLTYPE}$4], 1)
      eval HAVE_${GLTYPE}$4=1
    else
      eval HAVE_${GLTYPE}$4=0
    fi
  done
  AC_FOREACH([gltype], [$1],
    [AC_SUBST([HAVE_]translit(gltype,[abcdefghijklmnopqrstuvwxyz],[ABCDEFGHIJKLMNOPQRSTUVWXYZ])[$4])])
])

dnl gl_STDINT_MISSING_BOUND(TYPE_BOUND, DEFAULT, DESCRIPTION)
dnl assumes an otherwise complete <stdint.h> and defines TYPE_BOUND if
dnl <stdint.h> doesn't define it.
AC_DEFUN([gl_STDINT_MISSING_BOUND],
[
  AC_CACHE_CHECK([for $1], [gl_cv_stdint_$1],
    [AC_EGREP_CPP([found it], [#include <stdint.h>
#ifdef $1
found it
#endif
       ], [gl_cv_stdint_$1=yes], [gl_cv_stdint_$1="$2"])])
  if test "$gl_cv_stdint_$1" != yes; then
    AC_DEFINE_UNQUOTED([$1], [$2], [$3])
  fi
])

dnl gl_STDINT_MISSING_BOUNDS(BOUNDS, INCLUDES)
dnl assumes an otherwise complete <stdint.h> and defines each element of BOUNDS
dnl if <stdint.h> doesn't define it.
dnl Use this for types whose signedness is determined by the first letter
dnl ('u' or not).
AC_DEFUN([gl_STDINT_MISSING_BOUNDS],
[
  dnl Use a shell loop, to avoid bloating configure, and
  dnl - extra AH_TEMPLATE calls, so that autoheader knows what to put into
  dnl   config.h.in.
  AC_FOREACH([bound], [$1],
    [AH_TEMPLATE(bound, [Define if <stdint.h> doesn't define it.])])
changequote(,)dnl
  sed_unsigned='s,^\(U*\).*,\1,'
  sed_limitkind='s,^.*\(_[^_]*\)$,\1,'
changequote([,])dnl
  for bound in $1; do
    type=`echo $bound | sed -e 's,_MAX,_t,' -e 's,_MIN,_t,' | tr ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz`
    unsigned=`echo $bound | sed -e "$sed_unsigned" | tr U u`
    width=`echo $bound | sed -e 's,^U*INT,,' -e 's,_MIN,,' -e 's,_MAX,,'`
    limitkind=`echo $bound | sed -e "$sed_limitkind"`
    AC_CACHE_CHECK([for $bound], [gl_cv_stdint_$bound],
      [AC_EGREP_CPP([found it], [#include <stdint.h>
#ifdef $bound
found it
#endif
         ], [eval gl_cv_stdint_$bound=yes],
         [result=
          case $width in
            *8) widthlist="8 16 32 64" ;;
            *16) widthlist="16 32 64" ;;
            *32 | PTR | MAX | PTRDIFF) widthlist="32 64" ;;
            *64) widthlist="64" ;;
          esac
          for w in $widthlist; do
            if test -z "$result"; then
              AC_COMPILE_IFELSE([[$2
#include <stdint.h>
int verify[2 * (sizeof ($type) == sizeof (${unsigned}int${w}_t)) - 1];
                ]], [result=`echo "$unsigned" | tr u U`INT${w}${limitkind}])
            else
              break
            fi
          done
          if test -z "$result"; then
            result=no
          fi
          eval gl_cv_stdint_$bound=\$result
         ])])
    eval result=\$gl_cv_stdint_$bound
    if test "$result" != yes && test "$result" != no; then
      AC_DEFINE_UNQUOTED([$bound], [$result],
        [Define if <stdint.h> doesn't define it.])
    fi
  done
])

dnl gl_STDINT_MISSING_BOUNDS2(BOUNDS, INCLUDES)
dnl assumes an otherwise complete <stdint.h> and defines each element of BOUNDS
dnl if <stdint.h> doesn't define it.
dnl Use this for types whose signedness is a priori unknown.
AC_DEFUN([gl_STDINT_MISSING_BOUNDS2],
[
  dnl Use a shell loop, to avoid bloating configure, and
  dnl - extra AH_TEMPLATE calls, so that autoheader knows what to put into
  dnl   config.h.in.
  AC_FOREACH([bound], [$1],
    [AH_TEMPLATE(bound, [Define if <stdint.h> doesn't define it.])])
changequote(,)dnl
  sed_limitkind='s,^.*\(_[^_]*\)$,\1,'
changequote([,])dnl
  for bound in $1; do
    type=`echo $bound | sed -e 's,_MAX,_t,' -e 's,_MIN,_t,' | tr ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz`
    limitkind=`echo $bound | sed -e "$sed_limitkind"`
    AC_CACHE_CHECK([for $bound], [gl_cv_stdint_$bound],
      [AC_EGREP_CPP([found it], [#include <stdint.h>
#ifdef $bound
found it
#endif
         ], [eval gl_cv_stdint_$bound=yes],
         [result=
          AC_COMPILE_IFELSE([[$2
            int verify[2 * (($type) -1 >= ($type) 0) - 1];
            ]],
            [eval gl_cv_${type}_signed=no],
            [eval gl_cv_${type}_signed=yes])
          if eval test \$gl_cv_${type}_signed = yes; then
            for w in 8 16 32 64; do
              if test -z "$result"; then
                AC_COMPILE_IFELSE([[$2
#include <stdint.h>
int verify[2 * (sizeof ($type) == sizeof (int${w}_t)) - 1];
                  ]], [result=INT${w}${limitkind}])
              else
                break
              fi
            done
          else
            if test ${limitkind} = _MIN; then
              result=0
            else
              for w in 8 16 32 64; do
                if test -z "$result"; then
                  AC_COMPILE_IFELSE([[$2
#include <stdint.h>
int verify[2 * (sizeof ($type) == sizeof (uint${w}_t)) - 1];
                    ]], [result=UINT${w}${limitkind}])
                else
                  break
                fi
              done
            fi
          fi
          if test -z "$result"; then
            result=no
          fi
          eval gl_cv_stdint_$bound=\$result
         ])])
    eval result=\$gl_cv_stdint_$bound
    if test "$result" != yes && test "$result" != no; then
      AC_DEFINE_UNQUOTED([$bound], [$result],
        [Define if <stdint.h> doesn't define it.])
    fi
  done
])

dnl gl_STDINT_BITSIZEOF(TYPES, INCLUDES)
dnl Determine the size of each of the given types in bits.
AC_DEFUN([gl_STDINT_BITSIZEOF],
[
  dnl Use a shell loop, to avoid bloating configure, and
  dnl - extra AH_TEMPLATE calls, so that autoheader knows what to put into
  dnl   config.h.in,
  dnl - extra AC_SUBST calls, so that the right substitutions are made.
  AC_FOREACH([gltype], [$1],
    [AH_TEMPLATE([BITSIZEOF_]translit(gltype,[abcdefghijklmnopqrstuvwxyz ],[ABCDEFGHIJKLMNOPQRSTUVWXYZ_]),
       [Define to the number of bits in type ']gltype['.])])
  for gltype in $1 ; do
    if eval test \$gl_cv_type_${gltype} = yes; then
      AC_CACHE_CHECK([for bit size of $gltype], [gl_cv_bitsizeof_${gltype}],
        [_AC_COMPUTE_INT([sizeof ($gltype) * CHAR_BIT], result,
           [$2
#include <limits.h>], result=unknown)
         eval gl_cv_bitsizeof_${gltype}=\$result
        ])
      eval result=\$gl_cv_bitsizeof_${gltype}
    else
      dnl Use a nonempty default, because some compilers, such as IRIX 5 cc,
      dnl do a syntax check even on unused #if conditions and give an error
      dnl on valid C code like this:
      dnl   #if 0
      dnl   # if  > 32
      dnl   # endif
      dnl   #endif
      result=0
    fi
    GLTYPE=`echo "$gltype" | tr 'abcdefghijklmnopqrstuvwxyz ' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ_'`
    AC_DEFINE_UNQUOTED([BITSIZEOF_${GLTYPE}], [$result])
    eval BITSIZEOF_${GLTYPE}=\$result
  done
  AC_FOREACH([gltype], [$1],
    [AC_SUBST([BITSIZEOF_]translit(gltype,[abcdefghijklmnopqrstuvwxyz ],[ABCDEFGHIJKLMNOPQRSTUVWXYZ_]))])
])

dnl gl_CHECK_TYPES_SIGNED(TYPES, INCLUDES)
dnl Determine the signedness of each of the given types.
dnl Define HAVE_SIGNED_TYPE if type is signed.
AC_DEFUN([gl_CHECK_TYPES_SIGNED],
[
  dnl Use a shell loop, to avoid bloating configure, and
  dnl - extra AH_TEMPLATE calls, so that autoheader knows what to put into
  dnl   config.h.in,
  dnl - extra AC_SUBST calls, so that the right substitutions are made.
  AC_FOREACH([gltype], [$1],
    [AH_TEMPLATE([HAVE_SIGNED_]translit(gltype,[abcdefghijklmnopqrstuvwxyz ],[ABCDEFGHIJKLMNOPQRSTUVWXYZ_]),
       [Define to 1 if ']gltype[' is a signed integer type.])])
  for gltype in $1 ; do
    AC_CACHE_CHECK([whether $gltype is signed], [gl_cv_type_${gltype}_signed],
      [AC_COMPILE_IFELSE(
         [AC_LANG_PROGRAM([[$2
            int verify[2 * (($1) -1 < ($1) 0) - 1];
            ]])],
         result=yes, result=no)
       eval gl_cv_type_${gltype}_signed=\$result
      ])
    eval result=\$gl_cv_type_${gltype}_signed
    GLTYPE=`echo $gltype | tr 'abcdefghijklmnopqrstuvwxyz ' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ_'`
    if test "$result" = yes; then
      AC_DEFINE_UNQUOTED([HAVE_SIGNED_${GLTYPE}], 1)
      eval HAVE_SIGNED_${GLTYPE}=1
    else
      eval HAVE_SIGNED_${GLTYPE}=0
    fi
  done
  AC_FOREACH([gltype], [$1],
    [AC_SUBST([HAVE_SIGNED_]translit(gltype,[abcdefghijklmnopqrstuvwxyz ],[ABCDEFGHIJKLMNOPQRSTUVWXYZ_]))])
])

dnl gl_CHECK_TYPE_SAME(TYPE, KNOWNTYPE, INCLUDES)
dnl Determines whether two types are the same.
AC_DEFUN([gl_CHECK_TYPE_SAME],
[
  AC_TRY_COMPILE([$3
    extern $1 foo;
    extern $2 foo;], [],
    [SAME_TYPE_]AS_TR_CPP([$1])[_]AS_TR_CPP([$2])[=1],
    [SAME_TYPE_]AS_TR_CPP([$1])[_]AS_TR_CPP([$2])[=0])
  AC_SUBST([SAME_TYPE_]AS_TR_CPP([$1])[_]AS_TR_CPP([$2]))
])

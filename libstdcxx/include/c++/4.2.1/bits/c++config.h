// Predefined symbols and macros -*- C++ -*-

// Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
// 2006, 2007
// Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

/** @file c++config.h
 *  This is an internal header file, included by other library headers.
 *  You should not attempt to use it directly.
 */

#ifndef _CXXCONFIG
#define _CXXCONFIG 1

// Pick up any OS-specific definitions.
#include <bits/os_defines.h>

// Pick up any CPU-specific definitions.
#include <bits/cpu_defines.h>

// The current version of the C++ library in compressed ISO date format.
#define __GLIBCXX__ 20070719 

// Macros for visibility.
# define _GLIBCXX_HAVE_ATTRIBUTE_VISIBILITY 1

#if _GLIBCXX_HAVE_ATTRIBUTE_VISIBILITY
#define _GLIBCXX_VISIBILITY(V) __attribute__ ((__visibility__ (#V)))
#else
#define _GLIBCXX_VISIBILITY(V) 
#endif

// Macros for controlling various namespace association schemes and modes.
#ifdef _GLIBCXX_DEBUG
# define _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG 1
#endif

# define _GLIBCXX_NAMESPACE_ASSOCIATION_VERSION 0 

#ifndef _GLIBCXX_VISIBILITY_DEFAULT
#define _GLIBCXX_VISIBILITY_DEFAULT _GLIBCXX_VISIBILITY(default)
#endif

// Macros for namespace scope.
// _GLIBCXX_BEGIN_NAMESPACE
// _GLIBCXX_END_NAMESPACE
// _GLIBCXX_BEGIN_NESTED_NAMESPACE
// _GLIBCXX_END_NESTED_NAMESPACE
#if _GLIBCXX_NAMESPACE_ASSOCIATION_VERSION
# define _GLIBCXX_BEGIN_NESTED_NAMESPACE(X, Y)  namespace X { namespace Y _GLIBCXX_VISIBILITY_DEFAULT {
# define _GLIBCXX_END_NESTED_NAMESPACE } }
# define _GLIBCXX_BEGIN_NAMESPACE(X) _GLIBCXX_BEGIN_NESTED_NAMESPACE(X, _6)
# define _GLIBCXX_END_NAMESPACE _GLIBCXX_END_NESTED_NAMESPACE
#else
# define _GLIBCXX_BEGIN_NAMESPACE(X) namespace X _GLIBCXX_VISIBILITY_DEFAULT { 
# define _GLIBCXX_END_NAMESPACE } 
# if _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG
#  define _GLIBCXX_BEGIN_NESTED_NAMESPACE(X, Y) namespace X { namespace Y _GLIBCXX_VISIBILITY_DEFAULT {
#  define _GLIBCXX_END_NESTED_NAMESPACE  } }
# else
#  define _GLIBCXX_BEGIN_NESTED_NAMESPACE(X, Y) _GLIBCXX_BEGIN_NAMESPACE(X)
#  define _GLIBCXX_END_NESTED_NAMESPACE _GLIBCXX_END_NAMESPACE
# endif
#endif

// Namespace associations for versioning mode.
#if _GLIBCXX_NAMESPACE_ASSOCIATION_VERSION
namespace std
{
  namespace _6 { }
  using namespace _6 __attribute__ ((strong));
}

// In addition, other supported namespace configurations.
namespace __gnu_cxx 
{ 
  namespace _6 { }
  using namespace _6 __attribute__ ((strong));
}

namespace std
{
  namespace tr1 
  { 
    namespace _6 { }
    using namespace _6 __attribute__ ((strong));
  }
}
#endif

// Namespace associations for debug mode.
#if _GLIBCXX_NAMESPACE_ASSOCIATION_DEBUG
namespace std
{ 
  namespace __norm { }
  namespace __debug { }
  using namespace __debug __attribute__ ((strong)); 
}

namespace __gnu_cxx
{
  namespace __norm { }
  namespace __debug { }
  using namespace __debug __attribute__ ((strong)); 
}

# define _GLIBCXX_STD __norm
# define _GLIBCXX_EXT __norm
# define _GLIBCXX_EXTERN_TEMPLATE 0
# if __NO_INLINE__ && !__GXX_WEAK__
#  warning debug mode without inlining may fail due to lack of weak symbols
# endif
#else
#if _GLIBCXX_NAMESPACE_ASSOCIATION_VERSION
# define _GLIBCXX_STD _6
# define _GLIBCXX_EXT _6
#else
# define _GLIBCXX_STD std
# define _GLIBCXX_EXT __gnu_cxx
#endif
#endif

/* Define if compatibility should be provided for -mlong-double-64. */
#undef _GLIBCXX_LONG_DOUBLE_COMPAT

// XXX GLIBCXX_ABI Deprecated
// Namespace associations for long double 128 mode.
_GLIBCXX_BEGIN_NAMESPACE(std)
#if defined _GLIBCXX_LONG_DOUBLE_COMPAT && defined __LONG_DOUBLE_128__
# define _GLIBCXX_LDBL_NAMESPACE __gnu_cxx_ldbl128::
# define _GLIBCXX_BEGIN_LDBL_NAMESPACE namespace __gnu_cxx_ldbl128 {
# define _GLIBCXX_END_LDBL_NAMESPACE }
  namespace __gnu_cxx_ldbl128 { }
  using namespace __gnu_cxx_ldbl128 __attribute__((__strong__));
#else
# define _GLIBCXX_LDBL_NAMESPACE
# define _GLIBCXX_BEGIN_LDBL_NAMESPACE
# define _GLIBCXX_END_LDBL_NAMESPACE
#endif
_GLIBCXX_END_NAMESPACE


// Allow use of "export template." This is currently not a feature
// that g++ supports.
// #define _GLIBCXX_EXPORT_TEMPLATE 1

// Allow use of the GNU syntax extension, "extern template." This
// extension is fully documented in the g++ manual, but in a nutshell,
// it inhibits all implicit instantiations and is used throughout the
// library to avoid multiple weak definitions for required types that
// are already explicitly instantiated in the library binary. This
// substantially reduces the binary size of resulting executables.
#ifndef _GLIBCXX_EXTERN_TEMPLATE
# define _GLIBCXX_EXTERN_TEMPLATE 1
#endif


// Certain function definitions that are meant to be overridable from
// user code are decorated with this macro.  For some targets, this
// macro causes these definitions to be weak.
#ifndef _GLIBCXX_WEAK_DEFINITION
# define _GLIBCXX_WEAK_DEFINITION
#endif

// The remainder of the prewritten config is automatic; all the
// user hooks are listed above.

// Create a boolean flag to be used to determine if --fast-math is set.
#ifdef __FAST_MATH__
# define _GLIBCXX_FAST_MATH 1
#else
# define _GLIBCXX_FAST_MATH 0
#endif

// This marks string literals in header files to be extracted for eventual
// translation.  It is primarily used for messages in thrown exceptions; see
// src/functexcept.cc.  We use __N because the more traditional _N is used
// for something else under certain OSes (see BADNAMES).
#define __N(msgid)     (msgid)

// For example, <windows.h> is known to #define min and max as macros...
#undef min
#undef max

// End of prewritten config; the discovered settings follow.
/* config.h.  Generated by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if you have the `acosf' function. */
#define _GLIBCXX_HAVE_ACOSF 1

/* Define to 1 if you have the `acosl' function. */
#define _GLIBCXX_HAVE_ACOSL 1

/* Define to 1 if you have the `asinf' function. */
#define _GLIBCXX_HAVE_ASINF 1

/* Define to 1 if you have the `asinl' function. */
#define _GLIBCXX_HAVE_ASINL 1

/* Define to 1 if you have the `atan2f' function. */
#define _GLIBCXX_HAVE_ATAN2F 1

/* Define to 1 if you have the `atan2l' function. */
#define _GLIBCXX_HAVE_ATAN2L 1

/* Define to 1 if you have the `atanf' function. */
#define _GLIBCXX_HAVE_ATANF 1

/* Define to 1 if you have the `atanl' function. */
#define _GLIBCXX_HAVE_ATANL 1

/* Define to 1 if you have the `ceilf' function. */
#define _GLIBCXX_HAVE_CEILF 1

/* Define to 1 if you have the `ceill' function. */
#define _GLIBCXX_HAVE_CEILL 1

/* Define to 1 if you have the <complex.h> header file. */
#define _GLIBCXX_HAVE_COMPLEX_H 1

/* Define to 1 if you have the `copysign' function. */
#define _GLIBCXX_HAVE_COPYSIGN 1

/* Define to 1 if you have the `copysignf' function. */
#define _GLIBCXX_HAVE_COPYSIGNF 1

/* Define to 1 if you have the `copysignl' function. */
#define _GLIBCXX_HAVE_COPYSIGNL 1

/* Define to 1 if you have the `cosf' function. */
#define _GLIBCXX_HAVE_COSF 1

/* Define to 1 if you have the `coshf' function. */
#define _GLIBCXX_HAVE_COSHF 1

/* Define to 1 if you have the `coshl' function. */
#define _GLIBCXX_HAVE_COSHL 1

/* Define to 1 if you have the `cosl' function. */
#define _GLIBCXX_HAVE_COSL 1

/* Define to 1 if you have the <endian.h> header file. */
/* #undef _GLIBCXX_HAVE_ENDIAN_H */

/* Define to 1 if you have the `expf' function. */
#define _GLIBCXX_HAVE_EXPF 1

/* Define to 1 if you have the `expl' function. */
#define _GLIBCXX_HAVE_EXPL 1

/* Define to 1 if you have the `fabsf' function. */
#define _GLIBCXX_HAVE_FABSF 1

/* Define to 1 if you have the `fabsl' function. */
#define _GLIBCXX_HAVE_FABSL 1

/* Define to 1 if you have the <fenv.h> header file. */
#define _GLIBCXX_HAVE_FENV_H 1

/* Define to 1 if you have the `finite' function. */
#define _GLIBCXX_HAVE_FINITE 1

/* Define to 1 if you have the `finitef' function. */
/* #undef _GLIBCXX_HAVE_FINITEF */

/* Define to 1 if you have the `finitel' function. */
/* #undef _GLIBCXX_HAVE_FINITEL */

/* Define to 1 if you have the <float.h> header file. */
#define _GLIBCXX_HAVE_FLOAT_H 1

/* Define to 1 if you have the `floorf' function. */
#define _GLIBCXX_HAVE_FLOORF 1

/* Define to 1 if you have the `floorl' function. */
#define _GLIBCXX_HAVE_FLOORL 1

/* Define to 1 if you have the `fmodf' function. */
#define _GLIBCXX_HAVE_FMODF 1

/* Define to 1 if you have the `fmodl' function. */
#define _GLIBCXX_HAVE_FMODL 1

/* Define to 1 if you have the `fpclass' function. */
/* #undef _GLIBCXX_HAVE_FPCLASS */

/* Define to 1 if you have the <fp.h> header file. */
/* #undef _GLIBCXX_HAVE_FP_H */

/* Define to 1 if you have the `frexpf' function. */
#define _GLIBCXX_HAVE_FREXPF 1

/* Define to 1 if you have the `frexpl' function. */
#define _GLIBCXX_HAVE_FREXPL 1

/* Define to 1 if you have the <gconv.h> header file. */
/* #undef _GLIBCXX_HAVE_GCONV_H */

/* Define if _Unwind_GetIPInfo is available. */
#define _GLIBCXX_HAVE_GETIPINFO 1

/* Define to 1 if you have the `getpagesize' function. */
#define _GLIBCXX_HAVE_GETPAGESIZE 1

/* Define if gthr-default.h exists (meaning that threading support is
   enabled). */
#define _GLIBCXX_HAVE_GTHR_DEFAULT 1

/* Define to 1 if you have the `hypot' function. */
#define _GLIBCXX_HAVE_HYPOT 1

/* Define to 1 if you have the `hypotf' function. */
#define _GLIBCXX_HAVE_HYPOTF 1

/* Define to 1 if you have the `hypotl' function. */
#define _GLIBCXX_HAVE_HYPOTL 1

/* Define to 1 if you have the `iconv' function. */
#define _GLIBCXX_HAVE_ICONV 1

/* Define to 1 if you have the `iconv_close' function. */
#define _GLIBCXX_HAVE_ICONV_CLOSE 1

/* Define to 1 if you have the `iconv_open' function. */
#define _GLIBCXX_HAVE_ICONV_OPEN 1

/* Define to 1 if you have the <ieeefp.h> header file. */
/* #undef _GLIBCXX_HAVE_IEEEFP_H */

/* Define if int64_t is available in <stdint.h>. */
#define _GLIBCXX_HAVE_INT64_T 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define _GLIBCXX_HAVE_INTTYPES_H 1

/* Define to 1 if you have the `isinf' function. */
#define _GLIBCXX_HAVE_ISINF 1

/* Define to 1 if you have the `isinff' function. */
/* #undef _GLIBCXX_HAVE_ISINFF */

/* Define to 1 if you have the `isinfl' function. */
/* #undef _GLIBCXX_HAVE_ISINFL */

/* Define to 1 if you have the `isnan' function. */
#define _GLIBCXX_HAVE_ISNAN 1

/* Define to 1 if you have the `isnanf' function. */
/* #undef _GLIBCXX_HAVE_ISNANF */

/* Define to 1 if you have the `isnanl' function. */
/* #undef _GLIBCXX_HAVE_ISNANL */

/* Defined if iswblank exists. */
#define _GLIBCXX_HAVE_ISWBLANK 1

/* Define if LC_MESSAGES is available in <locale.h>. */
#define _GLIBCXX_HAVE_LC_MESSAGES 1

/* Define to 1 if you have the `ldexpf' function. */
#define _GLIBCXX_HAVE_LDEXPF 1

/* Define to 1 if you have the `ldexpl' function. */
#define _GLIBCXX_HAVE_LDEXPL 1

/* Define to 1 if you have the <libintl.h> header file. */
/* #undef _GLIBCXX_HAVE_LIBINTL_H */

/* Define to 1 if you have the `m' library (-lm). */
#define _GLIBCXX_HAVE_LIBM 1

/* Only used in build directory testsuite_hooks.h. */
#define _GLIBCXX_HAVE_LIMIT_AS 1

/* Only used in build directory testsuite_hooks.h. */
#define _GLIBCXX_HAVE_LIMIT_DATA 1

/* Only used in build directory testsuite_hooks.h. */
#define _GLIBCXX_HAVE_LIMIT_FSIZE 1

/* Only used in build directory testsuite_hooks.h. */
#define _GLIBCXX_HAVE_LIMIT_RSS 1

/* Only used in build directory testsuite_hooks.h. */
#define _GLIBCXX_HAVE_LIMIT_VMEM 0

/* Define to 1 if you have the <locale.h> header file. */
#define _GLIBCXX_HAVE_LOCALE_H 1

/* Define to 1 if you have the `log10f' function. */
#define _GLIBCXX_HAVE_LOG10F 1

/* Define to 1 if you have the `log10l' function. */
#define _GLIBCXX_HAVE_LOG10L 1

/* Define to 1 if you have the `logf' function. */
#define _GLIBCXX_HAVE_LOGF 1

/* Define to 1 if you have the `logl' function. */
#define _GLIBCXX_HAVE_LOGL 1

/* Define to 1 if you have the <machine/endian.h> header file. */
#define _GLIBCXX_HAVE_MACHINE_ENDIAN_H 1

/* Define to 1 if you have the <machine/param.h> header file. */
#define _GLIBCXX_HAVE_MACHINE_PARAM_H 1

/* Define if mbstate_t exists in wchar.h. */
#define _GLIBCXX_HAVE_MBSTATE_T 1

/* Define to 1 if you have the <memory.h> header file. */
#define _GLIBCXX_HAVE_MEMORY_H 1

/* Define to 1 if you have a working `mmap' system call. */
#define _GLIBCXX_HAVE_MMAP 1

/* Define to 1 if you have the `modf' function. */
#define _GLIBCXX_HAVE_MODF 1

/* Define to 1 if you have the `modff' function. */
#define _GLIBCXX_HAVE_MODFF 1

/* Define to 1 if you have the `modfl' function. */
#define _GLIBCXX_HAVE_MODFL 1

/* Define to 1 if you have the <nan.h> header file. */
/* #undef _GLIBCXX_HAVE_NAN_H */

/* Define to 1 if you have the `nl_langinfo' function. */
#define _GLIBCXX_HAVE_NL_LANGINFO 1

/* Define if poll is available in <poll.h>. */
#define _GLIBCXX_HAVE_POLL 1

/* Define to 1 if you have the `powf' function. */
#define _GLIBCXX_HAVE_POWF 1

/* Define to 1 if you have the `powl' function. */
#define _GLIBCXX_HAVE_POWL 1

/* Define to 1 if you have the `qfpclass' function. */
/* #undef _GLIBCXX_HAVE_QFPCLASS */

/* Define to 1 if you have the `setenv' function. */
#define _GLIBCXX_HAVE_SETENV 1

/* Define if sigsetjmp is available. */
#define _GLIBCXX_HAVE_SIGSETJMP 1

/* Define to 1 if you have the `sincos' function. */
/* #undef _GLIBCXX_HAVE_SINCOS */

/* Define to 1 if you have the `sincosf' function. */
/* #undef _GLIBCXX_HAVE_SINCOSF */

/* Define to 1 if you have the `sincosl' function. */
/* #undef _GLIBCXX_HAVE_SINCOSL */

/* Define to 1 if you have the `sinf' function. */
#define _GLIBCXX_HAVE_SINF 1

/* Define to 1 if you have the `sinhf' function. */
#define _GLIBCXX_HAVE_SINHF 1

/* Define to 1 if you have the `sinhl' function. */
#define _GLIBCXX_HAVE_SINHL 1

/* Define to 1 if you have the `sinl' function. */
#define _GLIBCXX_HAVE_SINL 1

/* Define to 1 if you have the `sqrtf' function. */
#define _GLIBCXX_HAVE_SQRTF 1

/* Define to 1 if you have the `sqrtl' function. */
#define _GLIBCXX_HAVE_SQRTL 1

/* Define to 1 if you have the <stdbool.h> header file. */
#define _GLIBCXX_HAVE_STDBOOL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define _GLIBCXX_HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define _GLIBCXX_HAVE_STDLIB_H 1

/* Define if strerror_l is available in <string.h>. */
/* #undef _GLIBCXX_HAVE_STRERROR_L */

/* Define if strerror_r is available in <string.h>. */
#define _GLIBCXX_HAVE_STRERROR_R 1

/* Define to 1 if you have the <strings.h> header file. */
#define _GLIBCXX_HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define _GLIBCXX_HAVE_STRING_H 1

/* Define to 1 if you have the `strtof' function. */
#define _GLIBCXX_HAVE_STRTOF 1

/* Define to 1 if you have the `strtold' function. */
#define _GLIBCXX_HAVE_STRTOLD 1

/* Define if strxfrm_l is available in <string.h>. */
/* #undef _GLIBCXX_HAVE_STRXFRM_L */

/* Define to 1 if you have the <sys/filio.h> header file. */
#define _GLIBCXX_HAVE_SYS_FILIO_H 1

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#define _GLIBCXX_HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/ipc.h> header file. */
#define _GLIBCXX_HAVE_SYS_IPC_H 1

/* Define to 1 if you have the <sys/isa_defs.h> header file. */
/* #undef _GLIBCXX_HAVE_SYS_ISA_DEFS_H */

/* Define to 1 if you have the <sys/machine.h> header file. */
/* #undef _GLIBCXX_HAVE_SYS_MACHINE_H */

/* Define to 1 if you have the <sys/param.h> header file. */
/* #undef _GLIBCXX_HAVE_SYS_PARAM_H */

/* Define to 1 if you have the <sys/resource.h> header file. */
#define _GLIBCXX_HAVE_SYS_RESOURCE_H 1

/* Define to 1 if you have the <sys/sem.h> header file. */
#define _GLIBCXX_HAVE_SYS_SEM_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define _GLIBCXX_HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define _GLIBCXX_HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define _GLIBCXX_HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/uio.h> header file. */
#define _GLIBCXX_HAVE_SYS_UIO_H 1

/* Define if S_IFREG is available in <sys/stat.h>. */
/* #undef _GLIBCXX_HAVE_S_IFREG */

/* Define if S_IFREG is available in <sys/stat.h>. */
#define _GLIBCXX_HAVE_S_ISREG 1

/* Define to 1 if you have the `tanf' function. */
#define _GLIBCXX_HAVE_TANF 1

/* Define to 1 if you have the `tanhf' function. */
#define _GLIBCXX_HAVE_TANHF 1

/* Define to 1 if you have the `tanhl' function. */
#define _GLIBCXX_HAVE_TANHL 1

/* Define to 1 if you have the `tanl' function. */
#define _GLIBCXX_HAVE_TANL 1

/* Define to 1 if the target supports thread-local storage. */
/* #undef _GLIBCXX_HAVE_TLS */

/* Define to 1 if you have the <unistd.h> header file. */
#define _GLIBCXX_HAVE_UNISTD_H 1

/* Defined if vfwscanf exists. */
#define _GLIBCXX_HAVE_VFWSCANF 1

/* Defined if vswscanf exists. */
#define _GLIBCXX_HAVE_VSWSCANF 1

/* Defined if vwscanf exists. */
#define _GLIBCXX_HAVE_VWSCANF 1

/* Define to 1 if you have the <wchar.h> header file. */
#define _GLIBCXX_HAVE_WCHAR_H 1

/* Defined if wcstof exists. */
#define _GLIBCXX_HAVE_WCSTOF 1

/* Define to 1 if you have the <wctype.h> header file. */
#define _GLIBCXX_HAVE_WCTYPE_H 1

/* Define if writev is available in <sys/uio.h>. */
#define _GLIBCXX_HAVE_WRITEV 1

/* Define to 1 if you have the `_acosf' function. */
/* #undef _GLIBCXX_HAVE__ACOSF */

/* Define to 1 if you have the `_acosl' function. */
/* #undef _GLIBCXX_HAVE__ACOSL */

/* Define to 1 if you have the `_asinf' function. */
/* #undef _GLIBCXX_HAVE__ASINF */

/* Define to 1 if you have the `_asinl' function. */
/* #undef _GLIBCXX_HAVE__ASINL */

/* Define to 1 if you have the `_atan2f' function. */
/* #undef _GLIBCXX_HAVE__ATAN2F */

/* Define to 1 if you have the `_atan2l' function. */
/* #undef _GLIBCXX_HAVE__ATAN2L */

/* Define to 1 if you have the `_atanf' function. */
/* #undef _GLIBCXX_HAVE__ATANF */

/* Define to 1 if you have the `_atanl' function. */
/* #undef _GLIBCXX_HAVE__ATANL */

/* Define to 1 if you have the `_ceilf' function. */
/* #undef _GLIBCXX_HAVE__CEILF */

/* Define to 1 if you have the `_ceill' function. */
/* #undef _GLIBCXX_HAVE__CEILL */

/* Define to 1 if you have the `_copysign' function. */
/* #undef _GLIBCXX_HAVE__COPYSIGN */

/* Define to 1 if you have the `_copysignl' function. */
/* #undef _GLIBCXX_HAVE__COPYSIGNL */

/* Define to 1 if you have the `_cosf' function. */
/* #undef _GLIBCXX_HAVE__COSF */

/* Define to 1 if you have the `_coshf' function. */
/* #undef _GLIBCXX_HAVE__COSHF */

/* Define to 1 if you have the `_coshl' function. */
/* #undef _GLIBCXX_HAVE__COSHL */

/* Define to 1 if you have the `_cosl' function. */
/* #undef _GLIBCXX_HAVE__COSL */

/* Define to 1 if you have the `_expf' function. */
/* #undef _GLIBCXX_HAVE__EXPF */

/* Define to 1 if you have the `_expl' function. */
/* #undef _GLIBCXX_HAVE__EXPL */

/* Define to 1 if you have the `_fabsf' function. */
/* #undef _GLIBCXX_HAVE__FABSF */

/* Define to 1 if you have the `_fabsl' function. */
/* #undef _GLIBCXX_HAVE__FABSL */

/* Define to 1 if you have the `_finite' function. */
/* #undef _GLIBCXX_HAVE__FINITE */

/* Define to 1 if you have the `_finitef' function. */
/* #undef _GLIBCXX_HAVE__FINITEF */

/* Define to 1 if you have the `_finitel' function. */
/* #undef _GLIBCXX_HAVE__FINITEL */

/* Define to 1 if you have the `_floorf' function. */
/* #undef _GLIBCXX_HAVE__FLOORF */

/* Define to 1 if you have the `_floorl' function. */
/* #undef _GLIBCXX_HAVE__FLOORL */

/* Define to 1 if you have the `_fmodf' function. */
/* #undef _GLIBCXX_HAVE__FMODF */

/* Define to 1 if you have the `_fmodl' function. */
/* #undef _GLIBCXX_HAVE__FMODL */

/* Define to 1 if you have the `_fpclass' function. */
/* #undef _GLIBCXX_HAVE__FPCLASS */

/* Define to 1 if you have the `_frexpf' function. */
/* #undef _GLIBCXX_HAVE__FREXPF */

/* Define to 1 if you have the `_frexpl' function. */
/* #undef _GLIBCXX_HAVE__FREXPL */

/* Define to 1 if you have the `_hypot' function. */
/* #undef _GLIBCXX_HAVE__HYPOT */

/* Define to 1 if you have the `_hypotf' function. */
/* #undef _GLIBCXX_HAVE__HYPOTF */

/* Define to 1 if you have the `_hypotl' function. */
/* #undef _GLIBCXX_HAVE__HYPOTL */

/* Define to 1 if you have the `_isinf' function. */
/* #undef _GLIBCXX_HAVE__ISINF */

/* Define to 1 if you have the `_isinff' function. */
/* #undef _GLIBCXX_HAVE__ISINFF */

/* Define to 1 if you have the `_isinfl' function. */
/* #undef _GLIBCXX_HAVE__ISINFL */

/* Define to 1 if you have the `_isnan' function. */
/* #undef _GLIBCXX_HAVE__ISNAN */

/* Define to 1 if you have the `_isnanf' function. */
/* #undef _GLIBCXX_HAVE__ISNANF */

/* Define to 1 if you have the `_isnanl' function. */
/* #undef _GLIBCXX_HAVE__ISNANL */

/* Define to 1 if you have the `_ldexpf' function. */
/* #undef _GLIBCXX_HAVE__LDEXPF */

/* Define to 1 if you have the `_ldexpl' function. */
/* #undef _GLIBCXX_HAVE__LDEXPL */

/* Define to 1 if you have the `_log10f' function. */
/* #undef _GLIBCXX_HAVE__LOG10F */

/* Define to 1 if you have the `_log10l' function. */
/* #undef _GLIBCXX_HAVE__LOG10L */

/* Define to 1 if you have the `_logf' function. */
/* #undef _GLIBCXX_HAVE__LOGF */

/* Define to 1 if you have the `_logl' function. */
/* #undef _GLIBCXX_HAVE__LOGL */

/* Define to 1 if you have the `_modf' function. */
/* #undef _GLIBCXX_HAVE__MODF */

/* Define to 1 if you have the `_modff' function. */
/* #undef _GLIBCXX_HAVE__MODFF */

/* Define to 1 if you have the `_modfl' function. */
/* #undef _GLIBCXX_HAVE__MODFL */

/* Define to 1 if you have the `_powf' function. */
/* #undef _GLIBCXX_HAVE__POWF */

/* Define to 1 if you have the `_powl' function. */
/* #undef _GLIBCXX_HAVE__POWL */

/* Define to 1 if you have the `_qfpclass' function. */
/* #undef _GLIBCXX_HAVE__QFPCLASS */

/* Define to 1 if you have the `_sincos' function. */
/* #undef _GLIBCXX_HAVE__SINCOS */

/* Define to 1 if you have the `_sincosf' function. */
/* #undef _GLIBCXX_HAVE__SINCOSF */

/* Define to 1 if you have the `_sincosl' function. */
/* #undef _GLIBCXX_HAVE__SINCOSL */

/* Define to 1 if you have the `_sinf' function. */
/* #undef _GLIBCXX_HAVE__SINF */

/* Define to 1 if you have the `_sinhf' function. */
/* #undef _GLIBCXX_HAVE__SINHF */

/* Define to 1 if you have the `_sinhl' function. */
/* #undef _GLIBCXX_HAVE__SINHL */

/* Define to 1 if you have the `_sinl' function. */
/* #undef _GLIBCXX_HAVE__SINL */

/* Define to 1 if you have the `_sqrtf' function. */
/* #undef _GLIBCXX_HAVE__SQRTF */

/* Define to 1 if you have the `_sqrtl' function. */
/* #undef _GLIBCXX_HAVE__SQRTL */

/* Define to 1 if you have the `_tanf' function. */
/* #undef _GLIBCXX_HAVE__TANF */

/* Define to 1 if you have the `_tanhf' function. */
/* #undef _GLIBCXX_HAVE__TANHF */

/* Define to 1 if you have the `_tanhl' function. */
/* #undef _GLIBCXX_HAVE__TANHL */

/* Define to 1 if you have the `_tanl' function. */
/* #undef _GLIBCXX_HAVE__TANL */

/* Define if the compiler/host combination has __builtin_abs. */
#define _GLIBCXX_HAVE___BUILTIN_ABS 1

/* Define if the compiler/host combination has __builtin_cos. */
#define _GLIBCXX_HAVE___BUILTIN_COS 1

/* Define if the compiler/host combination has __builtin_cosf. */
#define _GLIBCXX_HAVE___BUILTIN_COSF 1

/* Define if the compiler/host combination has __builtin_cosl. */
#define _GLIBCXX_HAVE___BUILTIN_COSL 1

/* Define if the compiler/host combination has __builtin_fabs. */
#define _GLIBCXX_HAVE___BUILTIN_FABS 1

/* Define if the compiler/host combination has __builtin_fabsf. */
#define _GLIBCXX_HAVE___BUILTIN_FABSF 1

/* Define if the compiler/host combination has __builtin_fabsl. */
#define _GLIBCXX_HAVE___BUILTIN_FABSL 1

/* Define if the compiler/host combination has __builtin_labs. */
#define _GLIBCXX_HAVE___BUILTIN_LABS 1

/* Define if the compiler/host combination has __builtin_sin. */
#define _GLIBCXX_HAVE___BUILTIN_SIN 1

/* Define if the compiler/host combination has __builtin_sinf. */
#define _GLIBCXX_HAVE___BUILTIN_SINF 1

/* Define if the compiler/host combination has __builtin_sinl. */
#define _GLIBCXX_HAVE___BUILTIN_SINL 1

/* Define if the compiler/host combination has __builtin_sqrt. */
#define _GLIBCXX_HAVE___BUILTIN_SQRT 1

/* Define if the compiler/host combination has __builtin_sqrtf. */
#define _GLIBCXX_HAVE___BUILTIN_SQRTF 1

/* Define if the compiler/host combination has __builtin_sqrtl. */
#define _GLIBCXX_HAVE___BUILTIN_SQRTL 1

/* Define to 1 if you have the `__signbit' function. */
#if __ppc__
	#define _GLIBCXX_HAVE___SIGNBIT 1
#endif
/* #undef _GLIBCXX_HAVE___SIGNBIT */

/* Define to 1 if you have the `__signbitf' function. */
#define _GLIBCXX_HAVE___SIGNBITF 1

/* Define to 1 if you have the `__signbitl' function. */
#define _GLIBCXX_HAVE___SIGNBITL 1

/* Name of package */
/* #undef _GLIBCXX_PACKAGE */

/* Define to the address where bug reports for this package should be sent. */
#define _GLIBCXX_PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define _GLIBCXX_PACKAGE_NAME "package-unused"

/* Define to the full name and version of this package. */
#define _GLIBCXX_PACKAGE_STRING "package-unused version-unused"

/* Define to the one symbol short name of this package. */
#define _GLIBCXX_PACKAGE_TARNAME "libstdc++"

/* Define to the version of this package. */
#define _GLIBCXX_PACKAGE__GLIBCXX_VERSION "version-unused"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
/* #undef _GLIBCXX_VERSION */

/* Define if builtin atomic operations are supported on this host. */
#define _GLIBCXX_ATOMIC_BUILTINS 1

/* Define to use concept checking code from the boost libraries. */
/* #undef _GLIBCXX_CONCEPT_CHECKS */

/* Define if a fully dynamic basic_string is wanted. */
/* #undef _GLIBCXX_FULLY_DYNAMIC_STRING */

/* Define to 1 if a full hosted library is built, or 0 if freestanding. */
#define _GLIBCXX_HOSTED 1

/* Define if compatibility should be provided for -mlong-double-64. */

/* Define if ptrdiff_t is int. */
#if !__LP64__
	#define _GLIBCXX_PTRDIFF_T_IS_INT 1
#endif

/* Define if using setrlimit to set resource limits during "make check" */
#define _GLIBCXX_RES_LIMITS 1

/* Define if size_t is unsigned int. */
/* #undef _GLIBCXX_SIZE_T_IS_UINT */

/* Define if the compiler is configured for setjmp/longjmp exceptions. */
/* #undef _GLIBCXX_SJLJ_EXCEPTIONS */

/* Define to use symbol versioning in the shared library. */
#define _GLIBCXX_SYMVER 1

/* Define to use darwin versioning in the shared library. */
#define _GLIBCXX_SYMVER_DARWIN 1

/* Define to use GNU versioning in the shared library. */
/* #undef _GLIBCXX_SYMVER_GNU */

/* Define to use GNU namespace versioning in the shared library. */
/* #undef _GLIBCXX_SYMVER_GNU_NAMESPACE */

/* Define if C99 functions or macros from <wchar.h>, <math.h>, <complex.h>,
   <stdio.h>, and <stdlib.h> can be used or exposed. */
#define _GLIBCXX_USE_C99 1

/* Define if C99 functions in <complex.h> should be used in <complex>. Using
   compiler builtins for these functions requires corresponding C99 library
   functions to be present. */
#define _GLIBCXX_USE_C99_COMPLEX 1

/* Define if C99 functions in <complex.h> should be used in <tr1/complex>.
   Using compiler builtins for these functions requires corresponding C99
   library functions to be present. */
#define _GLIBCXX_USE_C99_COMPLEX_TR1 1

/* Define if C99 functions in <ctype.h> should be imported in <tr1/cctype> in
   namespace std::tr1. */
#define _GLIBCXX_USE_C99_CTYPE_TR1 1

/* Define if C99 functions in <fenv.h> should be imported in <tr1/cfenv> in
   namespace std::tr1. */
#define _GLIBCXX_USE_C99_FENV_TR1 1

/* Define if C99 functions in <inttypes.h> should be imported in
   <tr1/cinttypes> in namespace std::tr1. */
#define _GLIBCXX_USE_C99_INTTYPES_TR1 1

/* Define if C99 functions or macros in <math.h> should be imported in <cmath>
   in namespace std. */
#define _GLIBCXX_USE_C99_MATH 1

/* Define if C99 functions or macros in <math.h> should be imported in
   <tr1/cmath> in namespace std::tr1. */
#define _GLIBCXX_USE_C99_MATH_TR1 1

/* Define if C99 types in <stdint.h> should be imported in <tr1/cstdint> in
   namespace std::tr1. */
#define _GLIBCXX_USE_C99_STDINT_TR1 1

/* Define if iconv and related functions exist and are usable. */
#define _GLIBCXX_USE_ICONV 1

/* Define if LFS support is available. */
/* #undef _GLIBCXX_USE_LFS */

/* Define if code specialized for long long should be used. */
#define _GLIBCXX_USE_LONG_LONG 1

/* Define if NLS translations are to be used. */
/* #undef _GLIBCXX_USE_NLS */

/* Define if dev/random and dev/urandom are available for the random_device of
   TR1 (Chapter 5.1). */
#if __i386__
	#define _GLIBCXX_USE_RANDOM_TR1 1
#endif

/* Define if code specialized for wchar_t should be used. */
#define _GLIBCXX_USE_WCHAR_T 1

#if defined (_GLIBCXX_HAVE__ACOSF) && ! defined (_GLIBCXX_HAVE_ACOSF)
# define _GLIBCXX_HAVE_ACOSF 1
# define acosf _acosf
#endif

#if defined (_GLIBCXX_HAVE__ACOSL) && ! defined (_GLIBCXX_HAVE_ACOSL)
# define _GLIBCXX_HAVE_ACOSL 1
# define acosl _acosl
#endif

#if defined (_GLIBCXX_HAVE__ASINF) && ! defined (_GLIBCXX_HAVE_ASINF)
# define _GLIBCXX_HAVE_ASINF 1
# define asinf _asinf
#endif

#if defined (_GLIBCXX_HAVE__ASINL) && ! defined (_GLIBCXX_HAVE_ASINL)
# define _GLIBCXX_HAVE_ASINL 1
# define asinl _asinl
#endif

#if defined (_GLIBCXX_HAVE__ATAN2F) && ! defined (_GLIBCXX_HAVE_ATAN2F)
# define _GLIBCXX_HAVE_ATAN2F 1
# define atan2f _atan2f
#endif

#if defined (_GLIBCXX_HAVE__ATAN2L) && ! defined (_GLIBCXX_HAVE_ATAN2L)
# define _GLIBCXX_HAVE_ATAN2L 1
# define atan2l _atan2l
#endif

#if defined (_GLIBCXX_HAVE__ATANF) && ! defined (_GLIBCXX_HAVE_ATANF)
# define _GLIBCXX_HAVE_ATANF 1
# define atanf _atanf
#endif

#if defined (_GLIBCXX_HAVE__ATANL) && ! defined (_GLIBCXX_HAVE_ATANL)
# define _GLIBCXX_HAVE_ATANL 1
# define atanl _atanl
#endif

#if defined (_GLIBCXX_HAVE__CEILF) && ! defined (_GLIBCXX_HAVE_CEILF)
# define _GLIBCXX_HAVE_CEILF 1
# define ceilf _ceilf
#endif

#if defined (_GLIBCXX_HAVE__CEILL) && ! defined (_GLIBCXX_HAVE_CEILL)
# define _GLIBCXX_HAVE_CEILL 1
# define ceill _ceill
#endif

#if defined (_GLIBCXX_HAVE__COPYSIGN) && ! defined (_GLIBCXX_HAVE_COPYSIGN)
# define _GLIBCXX_HAVE_COPYSIGN 1
# define copysign _copysign
#endif

#if defined (_GLIBCXX_HAVE__COPYSIGNL) && ! defined (_GLIBCXX_HAVE_COPYSIGNL)
# define _GLIBCXX_HAVE_COPYSIGNL 1
# define copysignl _copysignl
#endif

#if defined (_GLIBCXX_HAVE__COSF) && ! defined (_GLIBCXX_HAVE_COSF)
# define _GLIBCXX_HAVE_COSF 1
# define cosf _cosf
#endif

#if defined (_GLIBCXX_HAVE__COSHF) && ! defined (_GLIBCXX_HAVE_COSHF)
# define _GLIBCXX_HAVE_COSHF 1
# define coshf _coshf
#endif

#if defined (_GLIBCXX_HAVE__COSHL) && ! defined (_GLIBCXX_HAVE_COSHL)
# define _GLIBCXX_HAVE_COSHL 1
# define coshl _coshl
#endif

#if defined (_GLIBCXX_HAVE__COSL) && ! defined (_GLIBCXX_HAVE_COSL)
# define _GLIBCXX_HAVE_COSL 1
# define cosl _cosl
#endif

#if defined (_GLIBCXX_HAVE__EXPF) && ! defined (_GLIBCXX_HAVE_EXPF)
# define _GLIBCXX_HAVE_EXPF 1
# define expf _expf
#endif

#if defined (_GLIBCXX_HAVE__EXPL) && ! defined (_GLIBCXX_HAVE_EXPL)
# define _GLIBCXX_HAVE_EXPL 1
# define expl _expl
#endif

#if defined (_GLIBCXX_HAVE__FABSF) && ! defined (_GLIBCXX_HAVE_FABSF)
# define _GLIBCXX_HAVE_FABSF 1
# define fabsf _fabsf
#endif

#if defined (_GLIBCXX_HAVE__FABSL) && ! defined (_GLIBCXX_HAVE_FABSL)
# define _GLIBCXX_HAVE_FABSL 1
# define fabsl _fabsl
#endif

#if defined (_GLIBCXX_HAVE__FINITE) && ! defined (_GLIBCXX_HAVE_FINITE)
# define _GLIBCXX_HAVE_FINITE 1
# define finite _finite
#endif

#if defined (_GLIBCXX_HAVE__FINITEF) && ! defined (_GLIBCXX_HAVE_FINITEF)
# define _GLIBCXX_HAVE_FINITEF 1
# define finitef _finitef
#endif

#if defined (_GLIBCXX_HAVE__FINITEL) && ! defined (_GLIBCXX_HAVE_FINITEL)
# define _GLIBCXX_HAVE_FINITEL 1
# define finitel _finitel
#endif

#if defined (_GLIBCXX_HAVE__FLOORF) && ! defined (_GLIBCXX_HAVE_FLOORF)
# define _GLIBCXX_HAVE_FLOORF 1
# define floorf _floorf
#endif

#if defined (_GLIBCXX_HAVE__FLOORL) && ! defined (_GLIBCXX_HAVE_FLOORL)
# define _GLIBCXX_HAVE_FLOORL 1
# define floorl _floorl
#endif

#if defined (_GLIBCXX_HAVE__FMODF) && ! defined (_GLIBCXX_HAVE_FMODF)
# define _GLIBCXX_HAVE_FMODF 1
# define fmodf _fmodf
#endif

#if defined (_GLIBCXX_HAVE__FMODL) && ! defined (_GLIBCXX_HAVE_FMODL)
# define _GLIBCXX_HAVE_FMODL 1
# define fmodl _fmodl
#endif

#if defined (_GLIBCXX_HAVE__FPCLASS) && ! defined (_GLIBCXX_HAVE_FPCLASS)
# define _GLIBCXX_HAVE_FPCLASS 1
# define fpclass _fpclass
#endif

#if defined (_GLIBCXX_HAVE__FREXPF) && ! defined (_GLIBCXX_HAVE_FREXPF)
# define _GLIBCXX_HAVE_FREXPF 1
# define frexpf _frexpf
#endif

#if defined (_GLIBCXX_HAVE__FREXPL) && ! defined (_GLIBCXX_HAVE_FREXPL)
# define _GLIBCXX_HAVE_FREXPL 1
# define frexpl _frexpl
#endif

#if defined (_GLIBCXX_HAVE__HYPOT) && ! defined (_GLIBCXX_HAVE_HYPOT)
# define _GLIBCXX_HAVE_HYPOT 1
# define hypot _hypot
#endif

#if defined (_GLIBCXX_HAVE__HYPOTF) && ! defined (_GLIBCXX_HAVE_HYPOTF)
# define _GLIBCXX_HAVE_HYPOTF 1
# define hypotf _hypotf
#endif

#if defined (_GLIBCXX_HAVE__HYPOTL) && ! defined (_GLIBCXX_HAVE_HYPOTL)
# define _GLIBCXX_HAVE_HYPOTL 1
# define hypotl _hypotl
#endif

#if defined (_GLIBCXX_HAVE__ISINF) && ! defined (_GLIBCXX_HAVE_ISINF)
# define _GLIBCXX_HAVE_ISINF 1
# define isinf _isinf
#endif

#if defined (_GLIBCXX_HAVE__ISINFF) && ! defined (_GLIBCXX_HAVE_ISINFF)
# define _GLIBCXX_HAVE_ISINFF 1
# define isinff _isinff
#endif

#if defined (_GLIBCXX_HAVE__ISINFL) && ! defined (_GLIBCXX_HAVE_ISINFL)
# define _GLIBCXX_HAVE_ISINFL 1
# define isinfl _isinfl
#endif

#if defined (_GLIBCXX_HAVE__ISNAN) && ! defined (_GLIBCXX_HAVE_ISNAN)
# define _GLIBCXX_HAVE_ISNAN 1
# define isnan _isnan
#endif

#if defined (_GLIBCXX_HAVE__ISNANF) && ! defined (_GLIBCXX_HAVE_ISNANF)
# define _GLIBCXX_HAVE_ISNANF 1
# define isnanf _isnanf
#endif

#if defined (_GLIBCXX_HAVE__ISNANL) && ! defined (_GLIBCXX_HAVE_ISNANL)
# define _GLIBCXX_HAVE_ISNANL 1
# define isnanl _isnanl
#endif

#if defined (_GLIBCXX_HAVE__LDEXPF) && ! defined (_GLIBCXX_HAVE_LDEXPF)
# define _GLIBCXX_HAVE_LDEXPF 1
# define ldexpf _ldexpf
#endif

#if defined (_GLIBCXX_HAVE__LDEXPL) && ! defined (_GLIBCXX_HAVE_LDEXPL)
# define _GLIBCXX_HAVE_LDEXPL 1
# define ldexpl _ldexpl
#endif

#if defined (_GLIBCXX_HAVE__LOG10F) && ! defined (_GLIBCXX_HAVE_LOG10F)
# define _GLIBCXX_HAVE_LOG10F 1
# define log10f _log10f
#endif

#if defined (_GLIBCXX_HAVE__LOG10L) && ! defined (_GLIBCXX_HAVE_LOG10L)
# define _GLIBCXX_HAVE_LOG10L 1
# define log10l _log10l
#endif

#if defined (_GLIBCXX_HAVE__LOGF) && ! defined (_GLIBCXX_HAVE_LOGF)
# define _GLIBCXX_HAVE_LOGF 1
# define logf _logf
#endif

#if defined (_GLIBCXX_HAVE__LOGL) && ! defined (_GLIBCXX_HAVE_LOGL)
# define _GLIBCXX_HAVE_LOGL 1
# define logl _logl
#endif

#if defined (_GLIBCXX_HAVE__MODF) && ! defined (_GLIBCXX_HAVE_MODF)
# define _GLIBCXX_HAVE_MODF 1
# define modf _modf
#endif

#if defined (_GLIBCXX_HAVE__MODFF) && ! defined (_GLIBCXX_HAVE_MODFF)
# define _GLIBCXX_HAVE_MODFF 1
# define modff _modff
#endif

#if defined (_GLIBCXX_HAVE__MODFL) && ! defined (_GLIBCXX_HAVE_MODFL)
# define _GLIBCXX_HAVE_MODFL 1
# define modfl _modfl
#endif

#if defined (_GLIBCXX_HAVE__POWF) && ! defined (_GLIBCXX_HAVE_POWF)
# define _GLIBCXX_HAVE_POWF 1
# define powf _powf
#endif

#if defined (_GLIBCXX_HAVE__POWL) && ! defined (_GLIBCXX_HAVE_POWL)
# define _GLIBCXX_HAVE_POWL 1
# define powl _powl
#endif

#if defined (_GLIBCXX_HAVE__QFPCLASS) && ! defined (_GLIBCXX_HAVE_QFPCLASS)
# define _GLIBCXX_HAVE_QFPCLASS 1
# define qfpclass _qfpclass
#endif

#if defined (_GLIBCXX_HAVE__SINCOS) && ! defined (_GLIBCXX_HAVE_SINCOS)
# define _GLIBCXX_HAVE_SINCOS 1
# define sincos _sincos
#endif

#if defined (_GLIBCXX_HAVE__SINCOSF) && ! defined (_GLIBCXX_HAVE_SINCOSF)
# define _GLIBCXX_HAVE_SINCOSF 1
# define sincosf _sincosf
#endif

#if defined (_GLIBCXX_HAVE__SINCOSL) && ! defined (_GLIBCXX_HAVE_SINCOSL)
# define _GLIBCXX_HAVE_SINCOSL 1
# define sincosl _sincosl
#endif

#if defined (_GLIBCXX_HAVE__SINF) && ! defined (_GLIBCXX_HAVE_SINF)
# define _GLIBCXX_HAVE_SINF 1
# define sinf _sinf
#endif

#if defined (_GLIBCXX_HAVE__SINHF) && ! defined (_GLIBCXX_HAVE_SINHF)
# define _GLIBCXX_HAVE_SINHF 1
# define sinhf _sinhf
#endif

#if defined (_GLIBCXX_HAVE__SINHL) && ! defined (_GLIBCXX_HAVE_SINHL)
# define _GLIBCXX_HAVE_SINHL 1
# define sinhl _sinhl
#endif

#if defined (_GLIBCXX_HAVE__SINL) && ! defined (_GLIBCXX_HAVE_SINL)
# define _GLIBCXX_HAVE_SINL 1
# define sinl _sinl
#endif

#if defined (_GLIBCXX_HAVE__SQRTF) && ! defined (_GLIBCXX_HAVE_SQRTF)
# define _GLIBCXX_HAVE_SQRTF 1
# define sqrtf _sqrtf
#endif

#if defined (_GLIBCXX_HAVE__SQRTL) && ! defined (_GLIBCXX_HAVE_SQRTL)
# define _GLIBCXX_HAVE_SQRTL 1
# define sqrtl _sqrtl
#endif

#if defined (_GLIBCXX_HAVE__STRTOF) && ! defined (_GLIBCXX_HAVE_STRTOF)
# define _GLIBCXX_HAVE_STRTOF 1
# define strtof _strtof
#endif

#if defined (_GLIBCXX_HAVE__STRTOLD) && ! defined (_GLIBCXX_HAVE_STRTOLD)
# define _GLIBCXX_HAVE_STRTOLD 1
# define strtold _strtold
#endif

#if defined (_GLIBCXX_HAVE__TANF) && ! defined (_GLIBCXX_HAVE_TANF)
# define _GLIBCXX_HAVE_TANF 1
# define tanf _tanf
#endif

#if defined (_GLIBCXX_HAVE__TANHF) && ! defined (_GLIBCXX_HAVE_TANHF)
# define _GLIBCXX_HAVE_TANHF 1
# define tanhf _tanhf
#endif

#if defined (_GLIBCXX_HAVE__TANHL) && ! defined (_GLIBCXX_HAVE_TANHL)
# define _GLIBCXX_HAVE_TANHL 1
# define tanhl _tanhl
#endif

#if defined (_GLIBCXX_HAVE__TANL) && ! defined (_GLIBCXX_HAVE_TANL)
# define _GLIBCXX_HAVE_TANL 1
# define tanl _tanl
#endif

#endif // _CXXCONFIG_

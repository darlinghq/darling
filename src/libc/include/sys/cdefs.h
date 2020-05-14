/*
 * Copyright (c) 2006 - 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/***********************************************************************
 * Not to be installed in /usr/include
 ***********************************************************************/

#ifndef _LIBC_SYS_CDEFS_H_
#define _LIBC_SYS_CDEFS_H_

/*
 * Now include the real sys/cdefs.h.  The checks in libc-features.h will assure
 * that those macros are consistent with the current build environment.
 */
#include_next <sys/cdefs.h>
#ifndef _LIBC_NO_FEATURE_VERIFICATION
#if defined(__arm64__) || defined(__arm__) || defined(__i386__) || defined(__x86_64__)
#  include "libc-features.h"
#else
#  error "Unknown architecture."
#endif
#endif /* _LIBC_NO_FEATURE_VERIFICATION */

/*
 * symbol suffixes used for symbol versioning
 */
#if defined(VARIANT_LEGACY)
#  define LIBC_SUF_UNIX03		/* nothing */
#  define LIBC_SUF_64_BIT_INO_T		/* nothing */
#  define LIBC_SUF_NON_CANCELABLE	/* nothing */
#  define LIBC_SUF_1050			/* nothing */
#else /* !VARIANT_LEGACY */
#  if __DARWIN_ONLY_UNIX_CONFORMANCE
#    define LIBC_SUF_UNIX03		/* nothing */
#  else /* !__DARWIN_ONLY_UNIX_CONFORMANCE */
#    define LIBC_SUF_UNIX03		"$UNIX2003"
#  endif /* __DARWIN_ONLY_UNIX_CONFORMANCE */

#  if defined(VARIANT_INODE32)
#    define LIBC_SUF_64_BIT_INO_T	/* nothing */
#  else /* !VARIANT_INODE32 */
#    if __DARWIN_ONLY_64_BIT_INO_T
#      define LIBC_SUF_64_BIT_INO_T	/* nothing */
#    else /* !__DARWIN_ONLY_64_BIT_INO_T */
#      define LIBC_SUF_64_BIT_INO_T	"$INODE64"
#    endif /* __DARWIN_ONLY_64_BIT_INO_T */
#  endif /* VARIANT_INODE32 */

#  if defined(VARIANT_CANCELABLE)
#    define LIBC_SUF_NON_CANCELABLE	/* nothing */
#  else /* !LIBC_NON_CANCELABLE */
#    define LIBC_SUF_NON_CANCELABLE	"$NOCANCEL"
#  endif /* LIBC_NON_CANCELABLE */

#  if defined(VARIANT_PRE1050)
#    define LIBC_SUF_1050		/* nothing */
#  else /* !VARIANT_PRE1050 */
#    if __DARWIN_ONLY_VERS_1050
#      define LIBC_SUF_1050		/* nothing */
#    else /* !__DARWIN_ONLY_VERS_1050 */
#      define LIBC_SUF_1050		"$1050"
#    endif /* __DARWIN_ONLY_VERS_1050 */
#  endif /* VARIANT_PRE1050 */

#endif /* LIBC_UNIX03 */

#define LIBC_SUF_EXTSN			"$DARWIN_EXTSN"

/*
 * symbol versioning macros
 */
#define LIBC_ALIAS(sym)		__asm("_" __STRING(sym) LIBC_SUF_UNIX03)
#define LIBC_ALIAS_C(sym)	__asm("_" __STRING(sym) LIBC_SUF_NON_CANCELABLE LIBC_SUF_UNIX03)
#define LIBC_ALIAS_I(sym)	__asm("_" __STRING(sym) LIBC_SUF_64_BIT_INO_T LIBC_SUF_UNIX03)
#define LIBC_INODE64(sym)	__asm("_" __STRING(sym) LIBC_SUF_64_BIT_INO_T)

#define LIBC_1050(sym)		__asm("_" __STRING(sym) LIBC_SUF_1050)
#define LIBC_1050ALIAS(sym)	__asm("_" __STRING(sym) LIBC_SUF_1050 LIBC_SUF_UNIX03)
#define LIBC_1050ALIAS_C(sym)	__asm("_" __STRING(sym) LIBC_SUF_1050 LIBC_SUF_NON_CANCELABLE LIBC_SUF_UNIX03)
#define LIBC_1050ALIAS_I(sym)	__asm("_" __STRING(sym) LIBC_SUF_1050 LIBC_SUF_64_BIT_INO_T LIBC_SUF_UNIX03)
#define LIBC_1050INODE64(sym)	__asm("_" __STRING(sym) LIBC_SUF_1050 LIBC_SUF_64_BIT_INO_T)

#define LIBC_EXTSN(sym)		__asm("_" __STRING(sym) LIBC_SUF_EXTSN)
#define LIBC_EXTSN_C(sym)	__asm("_" __STRING(sym) LIBC_SUF_EXTSN LIBC_SUF_NON_CANCELABLE)

extern int pthread_key_init_np(int, void (*)(void *));

#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
/* Simulator keys are offset by 200 */
#define	__LIBC_PTHREAD_KEY(x)		(210 + (x))
#else
#define	__LIBC_PTHREAD_KEY(x)		(10 + (x))
#endif

/*
 * Libc pthread key assignments
 */
#define __LIBC_PTHREAD_KEY_XLOCALE	__LIBC_PTHREAD_KEY(0)
//#define __LIBC_PTHREAD_KEY_RESERVED_WIN64	__LIBC_PTHREAD_KEY(1)
#define __LIBC_PTHREAD_KEY_LOCALTIME	__LIBC_PTHREAD_KEY(2)
#define __LIBC_PTHREAD_KEY_GMTIME	__LIBC_PTHREAD_KEY(3)
#define __LIBC_PTHREAD_KEY_GDTOA_BIGINT	__LIBC_PTHREAD_KEY(4)
#define __LIBC_PTHREAD_KEY_PARSEFLOAT	__LIBC_PTHREAD_KEY(5)
#define __LIBC_PTHREAD_KEY_TTYNAME	__LIBC_PTHREAD_KEY(6)

#endif /* _LIBC_SYS_CDEFS_H_ */

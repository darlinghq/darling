/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*
 * Copyright (c) 2003, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki desktop OS
 *
 *
 */

/**
 * \file
 * Default definitions of C compiler quirk work-arounds.
 * \author Adam Dunkels <adam@dunkels.com>
 *
 * This file is used for making use of extra functionality of some C
 * compilers used for Contiki, and defining work-arounds for various
 * quirks and problems with some other C compilers.
 */

#ifndef CC_H_
#define CC_H_

#include "contiki-conf.h"

/**
 * Configure if the C compiler supports the "register" keyword for
 * function arguments.
 */
#if CC_CONF_REGISTER_ARGS
#define CC_REGISTER_ARG register
#else /* CC_CONF_REGISTER_ARGS */
#define CC_REGISTER_ARG
#endif /* CC_CONF_REGISTER_ARGS */

/**
 * Configure if the C compiler supports the arguments for function
 * pointers.
 */
#if CC_CONF_FUNCTION_POINTER_ARGS
#define CC_FUNCTION_POINTER_ARGS 1
#else /* CC_CONF_FUNCTION_POINTER_ARGS */
#define CC_FUNCTION_POINTER_ARGS 0
#endif /* CC_CONF_FUNCTION_POINTER_ARGS */

/**
 * Configure if the C compiler supports fastcall function
 * declarations.
 */
#ifdef CC_CONF_FASTCALL
#define CC_FASTCALL CC_CONF_FASTCALL
#else /* CC_CONF_FASTCALL */
#define CC_FASTCALL
#endif /* CC_CONF_FASTCALL */

/**
 * Configure if the C compiler have problems with const function pointers
 */
#ifdef CC_CONF_CONST_FUNCTION_BUG
#define CC_CONST_FUNCTION
#else /* CC_CONF_FASTCALL */
#define CC_CONST_FUNCTION const
#endif /* CC_CONF_FASTCALL */

/**
 * Configure work-around for unsigned char bugs with sdcc.
 */
#if CC_CONF_UNSIGNED_CHAR_BUGS
#define CC_UNSIGNED_CHAR_BUGS 1
#else /* CC_CONF_UNSIGNED_CHAR_BUGS */
#define CC_UNSIGNED_CHAR_BUGS 0
#endif /* CC_CONF_UNSIGNED_CHAR_BUGS */

/**
 * Configure if C compiler supports double hash marks in C macros.
 */
#if CC_CONF_DOUBLE_HASH
#define CC_DOUBLE_HASH 1
#else /* CC_CONF_DOUBLE_HASH */
#define CC_DOUBLE_HASH 0
#endif /* CC_CONF_DOUBLE_HASH */

#ifdef CC_CONF_INLINE
#define CC_INLINE CC_CONF_INLINE
#else /* CC_CONF_INLINE */
#define CC_INLINE
#endif /* CC_CONF_INLINE */

/**
 * Configure if the C compiler supports the assignment of struct value.
 */
#ifdef CC_CONF_ASSIGN_AGGREGATE
#define CC_ASSIGN_AGGREGATE(dest, src)  CC_CONF_ASSIGN_AGGREGATE(dest, src)
#else /* CC_CONF_ASSIGN_AGGREGATE */
#define CC_ASSIGN_AGGREGATE(dest, src)  *dest = *src
#endif /* CC_CONF_ASSIGN_AGGREGATE */

#if CC_CONF_NO_VA_ARGS
#define CC_NO_VA_ARGS CC_CONF_VA_ARGS
#endif

#ifndef NULL
#define NULL 0
#endif /* NULL */

#ifndef MAX
#define MAX(n, m)   (((n) < (m)) ? (m) : (n))
#endif

#ifndef MIN
#define MIN(n, m)   (((n) < (m)) ? (n) : (m))
#endif

#ifndef ABS
#define ABS(n)      (((n) < 0) ? -(n) : (n))
#endif


#define CC_CONCAT2(s1, s2) s1##s2
/**
 * A C preprocessing macro for concatenating two preprocessor tokens.
 *
 * We need use two macros (CC_CONCAT and CC_CONCAT2) in order to allow
 * concatenation of two \#defined macros.
 */
#define CC_CONCAT(s1, s2) CC_CONCAT2(s1, s2)
#define CC_CONCAT_EXT_2(s1, s2) CC_CONCAT2(s1, s2)

/**
 * A C preprocessing macro for concatenating three preprocessor tokens.
 */
#define CC_CONCAT3(s1, s2, s3) s1##s2##s3
#define CC_CONCAT_EXT_3(s1, s2, s3) CC_CONCAT3(s1, s2, s3)

#endif /* CC_H_ */

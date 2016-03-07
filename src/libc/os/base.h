/* Copyright (c) 2012 Apple Inc. All rights reserved.
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

#ifndef __OS_BASE_H__
#define __OS_BASE_H__

#if __GNUC__
#define os_fastpath(x) ((__typeof__(x))(uintptr_t)__builtin_expect((uintptr_t)(x), ~0l))
#define os_slowpath(x) ((__typeof__(x))(uintptr_t)__builtin_expect((uintptr_t)(x), 0l))
#define os_constant(x) __builtin_constant_p((x))
#define os_hardware_trap() __asm__ __volatile__ (""); __builtin_trap()

#define __OS_COMPILETIME_ASSERT__(e) __extension__({ \
	char __compile_time_assert__[(e) ? 1 : -1];	\
	(void)__compile_time_assert__; \
})

#define __OS_CONST __attribute__((__const__))
#define __OS_PRINTFLIKE(x,y) __attribute__((__format__(printf,x,y)))
#else /* __GNUC__ */
#define os_fastpath(x) (x)
#define os_slowpath(x) (x)
#define os_constant(x) ((long)0)
#define os_hardware_trap() abort()

#define __OS_COMPILETIME_ASSERT__(e) (e)

#define __OS_CONST
#define __OS_PRINTFLIKE(x,y)
#endif /* __GNUC__ */

#endif /* __OS_BASE_H__ */

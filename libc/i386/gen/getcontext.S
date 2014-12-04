/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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

#if defined(__i386__)

#include <architecture/i386/asm_help.h>

TEXT
LABEL(_getcontext)
	subl  $28, %esp
	movl  32(%esp), %eax
	movl  %eax,   (%esp)
	movl  %esp,  4(%esp)
	CALL_EXTERN(_getmcontext)
	movl  %eax, %ecx
	addl  $28, %esp 
	movl  %ebx, 16(%ecx)
	movl  %edi, 28(%ecx)
	movl  %esi, 32(%ecx)
	movl  %ebp, 36(%ecx)
	movl  (%esp), %eax
	movl  %eax, 52(%ecx)
	movl  %esp, %eax
	addl  $4, %eax
	movl  %eax, 40(%ecx)
	pushf
	popl  %eax
	movl  %eax, 48(%ecx)
	xorl  %eax, %eax
	movl  %eax, 12(%ecx)
	ret

#endif /* __i386__ */

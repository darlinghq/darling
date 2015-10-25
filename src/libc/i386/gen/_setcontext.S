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
LABEL(__setcontext)
	subl  $28, %esp
	movl  32(%esp), %eax
	movl  4(%eax), %eax
	movl  %eax, (%esp)		/* load up signal mask */
	CALL_EXTERN(_sigsetmask)	/* set signal mask */
	addl  $28, %esp
	movl  4(%esp), %ecx
	movl  28(%ecx), %ecx
	movl  16(%ecx), %ebx
	movl  28(%ecx), %edi
	movl  32(%ecx), %esi
	movl  36(%ecx), %ebp
	movl  40(%ecx), %esp
	pushl 48(%ecx)
	popfl
	movl  12(%ecx), %eax
	jmp   *52(%ecx)

#endif /* __i386__ */

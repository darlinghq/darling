/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
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
 
 

#ifdef __i386__

    .align 2
    .globl _start
    .private_extern _start
_start:
	nop		# <rdar://problem/10753356> backtraces of LC_MAIN binaries don't end in "start"
Lstart:
	movl	%eax,(%esp)	    # pass result from main() to exit() 
	call	_exit
	hlt
	
#endif /* __i386__ */


#if __x86_64__

    .align 2
    .globl _start
    .private_extern _start
_start:
	nop		# <rdar://problem/10753356> backtraces of LC_MAIN binaries don't end in "start"
Lstart:
	movl	%eax,%edi	    # pass result from main() to exit() 
	call	_exit
	hlt

#endif


#if __arm__
 
    .align 2
	.code	16  
    .globl _start
    .private_extern _start
	.thumb_func	_start
_start:
	nop					// <rdar://problem/10753356> backtraces of LC_MAIN binaries don't end in "start"
Lstart:
	bl	_exit			// result in r0 already in param reg r0
	trap
 
#endif /* __arm__ */


#if __arm64__
 
    .align 2
    .globl _start
    .private_extern _start
_start:
	nop
Lstart:
	bl	_exit			// result in x0 already in param reg x0
	brk	#3

#endif /* __arm64__ */

	.subsections_via_symbols
	

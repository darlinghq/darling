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

#include "pthread_machdep.h"

.text
.align 2, 0x90
.globl _start_wqthread
_start_wqthread:
// This routine is never called directly by user code, jumped from kernel
        push   %ebp
        mov    %esp,%ebp
        sub    $28,%esp		// align the stack
        mov    %edi,16(%esp)    //arg5
        mov    %edx,12(%esp)    //arg4
        mov    %ecx,8(%esp)             //arg3
        mov    %ebx,4(%esp)             //arg2
        mov    %eax,(%esp)              //arg1
        call   __pthread_wqthread
		leave
        ret


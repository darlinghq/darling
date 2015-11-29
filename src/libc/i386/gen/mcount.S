/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
#import <architecture/i386/asm_help.h>

.text
        .globl mcount
mcount:
        pushl   %ebp            // setup mcount's frame
        movl    %esp,%ebp

        movl    (%ebp),%eax     // load the frame pointer of mcount's caller
        movl    4(%eax),%eax    // load mcount's caller's return address
        pushl   4(%ebp)         // push the selfpc parameter for moncount()
        pushl   %eax            // push the frompc parameter for moncount()
        CALL_EXTERN(_moncount)  // call moncount()

        movl    %ebp,%esp       // tear down mcount's frame
        popl    %ebp
        ret

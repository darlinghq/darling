/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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

#include <arm/arch.h>
#if defined _ARM_ARCH_7 && !defined VARIANT_DYLD

/**********************************************************************
 * Cortex-A8 implementation                                           *
 **********************************************************************/

// Cortex-A8 implementations of memset( ) and bzero( ).  Main loop is 64-byte
// NEON stores, unless the buffer length is > 1k.  Beyond that point, there is
// little to no speed advantage with NEON (and a slight regression in some
// measured cases), so we switch to the GPRs.
//
// The crossover point should be reevaluated for future architectures.
//
// -- Stephen Canon, August 2009

.text
.syntax unified
.code 16

// void bzero(void * destination,
//            size_t length);
//
// zeros out a buffer length bytes long, beginning at the address destination.
.thumb_func ___bzero$VARIANT$CortexA8
.globl ___bzero$VARIANT$CortexA8
.thumb_func _bzero$VARIANT$CortexA8
.globl _bzero$VARIANT$CortexA8
.align 2
___bzero$VARIANT$CortexA8:
_bzero$VARIANT$CortexA8:
    mov     r2,     r1              // match the API to memset(dest, 0, length)
    eor     r1,     r1              // and fall through into memset

// void *memset(void * destination,
//              int value, size_t n);
//
// writes value converted to an unsigned char to n successive bytes, beginning
// at destination.

// Notes on register usage:
// 
// Throughout this function, registers have nearly constant usage; the pattern
// is:
//
//     r0 holds the original destination pointer, unmodified.  This value
//        must be returned by the routine, so it is easiest to just leave it
//        in place.
//     r1 holds the value that is being copied into the buffer, in some stage
//        of splattedness.  The low byte is guaranteed to always have the value
//        but the higher bytes may or may not contain copies of it.
//     r2 holds the length minus some offset, where the offset is always the
//        number of bytes that the current loop stores per iteration.
//     r3-r6,r8,r10,r11 are used with stmia, and will only ever contain splatted
//        copies of the value to be stored.
//     ip holds a pointer to the lowest byte in the array that has not yet been
//        set to hold value.
//     q0 and q1 hold splatted copies of the value in the vector path, and are
//        otherwise unused.

.thumb_func _memset$VARIANT$CortexA8
.globl _memset$VARIANT$CortexA8
.align 2
_memset$VARIANT$CortexA8:
    mov       ip,      r0           // copy destination pointer.
    subs      r2,           #0x8    // if length - 8 is negative (i.e. length
    and       r1,           #0xff   // is less than 8), jump to cleanup path.
    blt       L_scalarCleanup       // 
    
    tst       ip,           #0x7    // if the destination is doubleword
    beq       L_vectorCopy          // aligned, jump to fast path.
    
0:  strb      r1,     [ip], #1      // store one byte at a time until
    sub       r2,           #1      // destination pointer is 8 byte aligned.
    tst       ip,           #7      //
    bne       0b                    //
    
    cmp       r2,           #0x0    // if length - 8 is negative,
    blt       L_scalarCleanup       // jump to the cleanup code

L_vectorCopy:
    vdup.8    q0,      r1           // splat the byte to be stored across
    subs      r2,           #0x38   // q0 and q1, and check if length - 64
    vmov      q1,      q0           // is negative; if so, jump to the
    blt       L_vectorCleanup       // cleanup code.
    
    tst       ip,           #0x38   // if the destination is cacheline
    beq       L_cachelineAligned    // aligned, jump to the fast path.

0:  vst1.64  {d0},    [ip, :64]!    // store one double word at a time until
    sub       r2,           #8      // the destination is 64-byte aligned
    tst       ip,           #0x38   // 
    bne       0b
    
    cmp       r2,           #0x0    // if length - 64 is negative,
    blt       L_vectorCleanup       // jump to the cleanup code

L_cachelineAligned:
    cmp       r2,           #0x3c0  // if length > 1024
    bge       L_useSTMIA            // we use stmia instead

.align 4                            // main loop
0:  vst1.64  {q0,q1}, [ip, :256]!   // store 32 bytes
    subs      r2,           #0x40   // decrement length by 64
    vst1.64  {q0,q1}, [ip, :256]!   // store 32 bytes
    bge       0b                    // if length - 64 >= 0, continue
    
L_vectorCleanup:
    adds      r2,           #0x38   // if (length - 8) < 0, goto scalar cleanup
    blt       L_scalarCleanup       //
    
0:  subs      r2,           #8      // store one double word at a time until
    vst1.64  {d0},    [ip, :64]!    // (length - 8) < 0.
    bge       0b
    
L_scalarCleanup:
    adds      r2,           #8      // restore length
    beq       1f                    // early out if zero.
    
0:  strb      r1,     [ip], #1      // store one byte at a time until length
    subs      r2,           #1      // is zero.
    bne       0b                    //
1:  bx        lr                    // return.

//  STMIA loop for large buffers
//
//  For stores larger than 1024 bytes, we use STMIA because we can't get enough
//  of a speedup from NEON to offset the higher power draw of the NEON unit.
//
//  This crossover should be reevaluated on future architectures.
//
//  We avoid using r7 and r9 even though it's not strictly necessary.

L_useSTMIA:
    push     {r4,r5,r6,r8,r10,r11}
    orr       r1,      r1,  r1, lsl #8
    orr       r1,      r1,  r1, lsl #16
    mov       r3,      r1
    mov       r4,      r1
    mov       r5,      r1
    mov       r6,      r1
    mov       r8,      r1
    mov       r10,     r1
    mov       r11,     r1
.align 4
0:  stmia     ip!,  {r1,r3,r4,r5,r6,r8,r10,r11}
    subs      r2,           #0x40
    stmia     ip!,  {r1,r3,r4,r5,r6,r8,r10,r11}
    bge       0b
    pop      {r4,r5,r6,r8,r10,r11}
    b         L_vectorCleanup

#endif // defined _ARM_ARCH_7 && !defined VARIANT_DYLD


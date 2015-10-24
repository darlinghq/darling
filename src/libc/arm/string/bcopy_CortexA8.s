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

/*****************************************************************************
 * Cortex-A8 implementation                                                  *
 *****************************************************************************/
 
// Cortex-A8 implementations of memcpy( ), memmove( ) and bcopy( ).
//
// Our tests have shown that NEON is always a performance win for memcpy( ).
// However, for the specific case of copies from a warm source to a cold
// destination when the buffer size is between 1k and 32k, it is not enough
// of a performance win to offset the increased power footprint, resulting
// in an energy usage regression.  Thus, we detect that particular case, and
// pass those copies through the ARM core registers.  All other copies larger
// than 8 bytes are handled on NEON.
//
// Stephen Canon, August 2009

.text
.code 16
.syntax unified

// void bcopy(const void * source,
//            void * destination,
//            size_t length);
//
// void *memmove(void * destination,
//               const void * source,
//               size_t n);
//
// void *memcpy(void * restrict destination,
//              const void * restrict source,
//              size_t n);
//
// all copy n successive bytes from source to destination. memmove and memcpy
// returns destination, whereas bcopy has no return value. copying takes place
// as if it were through a temporary buffer -- after return destination contains
// exactly the bytes from source, even if the buffers overlap.

.thumb_func _bcopy$VARIANT$CortexA8
.thumb_func _memmove$VARIANT$CortexA8
.thumb_func _memcpy$VARIANT$CortexA8
.globl _bcopy$VARIANT$CortexA8
.globl _memmove$VARIANT$CortexA8
.globl _memcpy$VARIANT$CortexA8

#define SAVE_REGISTERS {r4,r5,r6,r8,r10,r11}
#define COPY_REGISTERS {r3,r4,r5,r6,r8,r9,r10,r11}
               
/*****************************************************************************
 *  entry points                                                             *
 *****************************************************************************/

.align 2
_bcopy$VARIANT$CortexA8:

// bcopy has the first and second arguments in the opposite order as the C
// library functions memmove and memcpy.  If bcopy is called, we swap these
// two arguments and then fall into memmove.

    mov         r3,     r0
    mov         r0,     r1
    mov         r1,     r3

.align 2
_memmove$VARIANT$CortexA8:
_memcpy$VARIANT$CortexA8:

// At entry to memmove/memcpy, registers contain the following values:
//
//  r0  pointer to the first byte of the destination buffer
//  r1  pointer to the first byte of the source buffer
//  r2  number of bytes to copy
//
// Our preference is to use a (faster and easier to understand) front-to-back
// copy of the buffer.  However, memmove requires that copies take place as
// though through a temporary buffer.  This means that if the buffers overlap,
// it may be necessary to copy the buffer in reverse order.
//
// To properly detect such overlap, we begin by computing the offset between
// the source and destination pointers.  If the offset happens to be zero,
// then there is no work to be done, so we can early out.

    subs    r3,     r0, r1
    it      eq
    bxeq    lr

// r3 now contains the offset between the buffers, (destination - source).  If
// 0 < offset < length, then the high-addressed bits of the source alias the
// low addressed bytes of the destination.  Thus, if we were to perform the
// copy in ascending address order, we would overwrite the high-addressed
// source bytes before we had a chance to copy them, and the data would be lost.
//
// Thus, we can use the front-to-back copy only if offset is negative or
// greater than the length.  This is the case precisely if offset compares
// unsigned higher than length.

    cmp     r3,     r2
    bhs     L_copyFrontToBack
                             
/*****************************************************************************
 *  back to front copy                                                       *
 *****************************************************************************/

// Here we have fallen through into the back-to-front copy.  We preserve the
// original destination pointer in r0 because it is the return value for the
// routine, and update the other registers as follows:
//
//  r1  one byte beyond the end of the destination buffer
//  r2  number of bytes to copy
//  ip  one byte beyond the end of the destination buffer

    mov      ip,    r0
    add      r1,    r2
    add      ip,    r2
    
// Subtract 8 from the buffer length; if this is negative, then we will use
// only single-byte copies, and we jump directly to a scalar copy loop.

    subs     r2,        $8
    blt      L_scalarReverseCopy
    
// If the destination pointer is 8-byte aligned we can use 8-byte NEON copies
// to move the data.
    
    tst      ip,        $7
    beq      L_vectorReverseCopy
    
// Otherwise, we copy a single byte at a time, in order of descending memory
// address, until the destination is 8 byte aligned.  Within this loop,
// registers are used as follows:
//
//  r0  original destination pointer
//  r1  pointer to one byte past the next element to be copied
//  r2  (bytes remaining to be copied) - 8
//  r3  temporary to hold the byte that is being copied
//  ip  pointer one byte past the destination of the next byte to be copied
//
//  byte that will be copied in this iteration
//                            |  byte that was copied in the previous iteration                           
//  Source buffer:            v   v                                
//  ------------------------+---+---+-------------------------
//  bytes still to copy ... |   |   | ... bytes already copied
//  ------------------------+---+---+-------------------------
//                                ^
//                               r1 holds the address of this byte

0:  ldrb     r3,   [r1, $-1]!
    sub      r2,        $1
    strb     r3,   [ip, $-1]!
    tst      ip,        $7
    bne      0b
    
// At this point, the destination pointer is 8 byte aligned.  Check again that
// there are at least 8 bytes remaining to copy by comparing the remaining
// length minus 8 to zero.  If fewer than 8 bytes remain, jump to the cleanup
// path.
    
    cmp      r2,    $0
    blt      L_scalarReverseCopy
                                    
/*****************************************************************************
 *  destination is 8 byte aligned                                            *
 *****************************************************************************/

L_vectorReverseCopy:

// At this point, registers contain the following values:
//
//  r0  original destination pointer
//  r1  pointer to one byte past the next element to be copied
//  r2  (bytes remaining to copy) - 8
//  ip  pointer one byte past the destination of the next byte to be copied
//
// Furthermore, it is known that ip is 8 byte aligned, and that r2 is positive.
// NEON has really excellent alignment handling in hardware, so we would like
// to use that to handle cases where the source is not similarly aligned to the
// destination (it supports even single-byte misalignment at speed).  However,
// on some SoC designs, not all of the DMA busses support such access.  Thus,
// we must unfortunately use a software workaround in those cases.
//
// Fortunately, 4-byte aligned loads are supported even on the DMA busses, so
// we only need to handle the different possible source alignments modulo 4.
// Here we have a dispatch table to jump to the correct copy implementation
// for the given source alignment.
//
// The tbh instruction loads the address offset of the correct implementation
// from the data table that immediately follows it and adds it to the pc to 
// jump to the correct branch.

    ands     r3,    r1, $3
    tbh     [pc, r3, lsl $1]
0:  
.short (L_reverseAligned0-0b)/2
.short (L_reverseAligned1-0b)/2
.short (L_reverseAligned2-0b)/2
.short (L_reverseAligned3-0b)/2

/*****************************************************************************
 *  source is also at least word aligned                                     *
 *****************************************************************************/
    
L_reverseAligned0:

// Subtract 56 from r2, so that it contains the number of bytes remaining to
// copy minus 64.  If this result is negative, then we jump into a loop that
// copies 8 bytes at a time.

    subs     r2,        $0x38
    blt      L_reverseVectorCleanup
    
// Check if the destination pointer is 64-byte aligned.  If so, jump to a loop
// that copies whole cachelines.

    tst      ip,        $0x38
    beq      L_reverseCachelineAligned
    
// Otherwise, we copy a 8 bytes at a time, in order of descending memory
// address, until the destination is 64 byte aligned.  Within this loop,
// registers are used as follows:
//
//  r0  original destination pointer
//  r1  pointer to one byte past the next element to be copied
//  r2  (bytes remaining to be copied) - 64
//  ip  pointer one byte past the destination of the next byte to be copied
//  d0  temporary storage for copy
//
//  bytes that will be copied after this iteration
//        |         8 byte block that will be copied in this iteration                            
//        v         v
//  --------------+-------------------------------+---------------------
//                | 0   1   2   3   4   5   6   7 | bytes already copied
//  --------------+-------------------------------+---------------------
//                                                  ^
//                                                  r1 points here
    
0:  sub      r1,        $8
    vld1.32 {d0},  [r1]
    sub      ip,        $8
    sub      r2,        $8
    tst      ip,        $0x38
    vst1.64 {d0},  [ip,:64]
    bne      0b
    
// At this point, the destination pointer is 64 byte aligned.  Check again that
// there are at least 64 bytes remaining to copy by comparing the remaining
// length minus 64 to zero.  If fewer than 64 bytes remain, skip over the main
// copy loop.

    cmp      r2,        $0
    blt      L_reverseVectorCleanup
    
/*****************************************************************************
 *  destination is cacheline aligned                                         *
 *****************************************************************************/

L_reverseCachelineAligned:

// In the special case that we are copying a buffer of between 1k and 32k bytes
// we do not use a NEON copy for the main loop.  This is because if we happen
// to be doing a copy from a source in cache to a destination that is not in
// cache, this will result in an increase in energy usage.  In all other cases,
// NEON gives superior energy conservation.

    sub      r3,    r2, $0x3c0
    cmp      r3,        $0x7c00
    blo      L_useSTMDB
    
// Pre-decrement the source (r1) and destination (ip) pointers so that they
// point to the first byte of the trailing 32-byte window of each buffer.
// Additionally, load the address increment of -32 into r3.

    sub      r1,        $32
    sub      ip,        $32
    mov      r3,        $-32
    
// The destination pointer is known to be 64-byte aligned, so we can use the
// maximal alignment hint (:256) for our vector stores.  Detect if the source
// is also at least 32-byte aligned and jump to a loop that uses maximal
// alignment hints for the loads as well if possible.
    
    tst      r1,        $0x1f
    beq      L_reverseSourceAligned
    
// This loop copies 64 bytes per iteration, from a 4-byte aligned source to a
// 64-byte aligned destination, in order of descending memory address.  Within
// this loop, registers are used as follows:
//
//  r0      original destination pointer (unmodified)
//  r1      pointer to the next 32-byte block to load
//  r2      (number of bytes remaining to copy) - 64
//  r3      address increment of -32.
//  ip      pointer to which the next 32-byte block is to be stored
//  q0-q3   temporary registers used for copies
//
// Note that the loop is arrange in such a way that a single cleanup store is
// necessary after the final loop iteration.  This occurs at label (1), and is
// shared between the unaligned and aligned loops.
    
    vld1.32 {q2,q3}, [r1],      r3
    vld1.32 {q0,q1}, [r1],      r3
    subs     r2,         $64
    vst1.64 {q2,q3}, [ip,:256], r3
    blt      1f
.align 3
0:  vld1.32 {q2,q3}, [r1],      r3
    vst1.64 {q0,q1}, [ip,:256], r3
    vld1.32 {q0,q1}, [r1],      r3
    subs     r2,         $64
    vst1.64 {q2,q3}, [ip,:256], r3 
    bge      0b
    b        1f
    
L_reverseSourceAligned:

// This loop is identical to the immediately preceeding loop, except that it
// uses the additional alignment hint that the source pointer (r1) is 32-byte
// aligned.  The two loops share cleanup code for the final iteration.

    vld1.64 {q2,q3}, [r1,:256], r3
    vld1.64 {q0,q1}, [r1,:256], r3
    subs     r2,         $64
    vst1.64 {q2,q3}, [ip,:256], r3
    blt      1f
.align 3
0:  vld1.64 {q2,q3}, [r1,:256], r3
    vst1.64 {q0,q1}, [ip,:256], r3
    vld1.64 {q0,q1}, [r1,:256], r3
    subs     r2,         $64
    vst1.64 {q2,q3}, [ip,:256], r3
    bge      0b
    
// Final vector store for both of the above loops.

1:  vst1.64 {q0,q1}, [ip,:256], r3

// Adjust the source and destination pointers so that they once again point to
// the last byte that we used (which is one byte higher than the address that
// we will use next for any required cleanup).

    add      r1,         $32
    add      ip,         $32
    
L_reverseVectorCleanup:

// Add 56 to r2, so that it contains the number of bytes remaing to copy minus
// 8.  A comparison of this value with zero tells us if any more whole 8-byte
// blocks need to be copied.

    adds     r2,    r2, $0x38
    blt      L_scalarReverseCopy

// This loop copies 8 bytes at a time in order of descending memory address,
// until fewer than 8 bytes remain to be copied.  Within this loop, registers
// are used as follows:
//
//  r0  original destination pointer
//  r1  pointer to one byte past the next element to be copied
//  r2  (bytes remaining to be copied) - 64
//  ip  pointer one byte past the destination of the next byte to be copied
//  d0  temporary storage for copy

0:  sub      r1,        $8
    vld1.32 {d0},  [r1]
    sub      ip,        $8
    subs     r2,        $8
    vst1.64 {d0},  [ip,:64]
    bge      0b

/*****************************************************************************
 *  sub-doubleword cleanup copies                                            *
 *****************************************************************************/

L_scalarReverseCopy:

// Add 8 to r2, so that it contains the number of bytes remaining to copy, and
// return to the calling routine if zero bytes remain.

    adds     r2,        $8
    it       eq
    bxeq     lr

// Copy one byte at a time in descending address order until we reach the front
// of the buffer.  Within this loop, registers are used as follows:
//
//  r0  original destination pointer
//  r1  pointer to one byte past the next element to be copied
//  r2  (bytes remaining to be copied) - 8
//  r3  temporary to hold the byte that is being copied
//  ip  pointer one byte past the destination of the next byte to be copied
         
0:  ldrb     r3,   [r1, $-1]!
    subs     r2,        $1
    strb     r3,   [ip, $-1]!
    bne      0b
    bx       lr
         
/*****************************************************************************
 *  STMDB loop for 1k-32k buffers                                            *
 *****************************************************************************/

// This loop copies 64 bytes each iteration in order of descending memory
// address, using the GPRs instead of NEON.
//
//  r0  original destination pointer
//  r1  pointer to one byte past the next element to be copied
//  r2  (bytes remaining to be copied) - 64
//  r3-6,r8-11 (COPY_REGISTERS) temporary registers used for moving data
//  ip  pointer to one byte past the next location to store to

L_useSTMDB:
    push    SAVE_REGISTERS
.align 3
0:  ldmdb   r1!,    COPY_REGISTERS
    subs    r2,     r2,  $64
    stmdb   ip!,    COPY_REGISTERS
    ldmdb   r1!,    COPY_REGISTERS
    pld     [r1, $-64]
    stmdb   ip!,    COPY_REGISTERS
    bge     0b
    pop     SAVE_REGISTERS
    b       L_reverseVectorCleanup
    
/*****************************************************************************
 *  Misaligned reverse vld1 loop                                             *
 *****************************************************************************/

// Software alignment fixup to handle source and dest that are relatively
// misaligned mod 4 bytes.  
//
// The basic idea is to use 4-byte aligned loads to load 8 bytes per iteration,
// which we combine with the 8 bytes loaded in the previous iteration to get a
// 16 byte field; the next 8 bytes to be stored to the destination buffer are
// somewhere in that field, and we get them using the VEXT instruction:
//      
//     |  8 bytes from this iteration  |  8 bytes from last iteration  |
//     +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
//     | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | a | b | c | d | e | f |
//     +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
//         ^8 bytes to store this iteration^           |
//                                                   could be a page boundary 
//
// We need to be a little bit careful, however.  Because the loads only have 4
// byte alignment, the very first load could slop over into a page that is not
// mapped readable.  In order to prevent this scenario, we copy eight bytes
// using byte-by-byte before beginning the main loop.
//
// At the beginning of each iteration through this loop, registers are used
// as follows:
//
//  r0  original destination pointer
//  r1  pointer to the next block of 8 bytes to load
//  r2  (bytes remaining to copy) - 8
//  ip  pointer to the next block of 8 bytes to store
//  d0  next 8 bytes to store
//  d2  8 bytes loaded in the previous iteration
//  d3  8 bytes loaded two iterations ago

#define RCOPY_UNALIGNED(offset)      \
0:  ldrb      r3,     [r1,$-1]!     ;\
    strb      r3,     [ip,$-1]!     ;\
    subs      r2,         $1        ;\
    blt       L_scalarReverseCopy   ;\
    tst       ip,         $7        ;\
    bne       0b                    ;\
    bic       r1,         $3        ;\
    sub       r1,         $8        ;\
    sub       ip,         $8        ;\
    mov       r3,         $-8       ;\
    vld1.32  {d2,d3}, [r1], r3      ;\
    subs      r2,         $8        ;\
    blt       1f                    ;\
0:  vext.8    d0,  d2, d3, $(offset);\
    vmov      d3,      d2           ;\
    vld1.32  {d2},    [r1], r3      ;\
    subs      r2,          $8       ;\
    vst1.64  {d0},    [ip, :64], r3 ;\
    bge       0b                    ;\
1:  vext.8    d0,  d2, d3, $(offset);\
    add       r1,          $8       ;\
    vst1.64  {d0},    [ip, :64]     ;\
2:  add       r1,          $(offset);\
    b         L_scalarReverseCopy

L_reverseAligned1:
    RCOPY_UNALIGNED(1)
L_reverseAligned2:
    RCOPY_UNALIGNED(2)
L_reverseAligned3:
    RCOPY_UNALIGNED(3)

/*****************************************************************************
 *  front to back copy                                                       *
 *****************************************************************************/

L_copyFrontToBack:

// Here the pointers are laid out such that we can use our preferred
// front-to-back copy.  We preserve original destination pointer in r0 because
// it is the return value for the routine, and copy it to ip to use in this
// routine.

    mov      ip,    r0
    
// Subtract 8 from the buffer length; if this is negative, then we will use
// only single-byte copies, and we jump directly to a scalar copy loop.
    
    subs     r2,        $8
    blt      L_scalarCopy
    
// If the destination pointer is 8-byte aligned we can use 8-byte NEON copies
// to move the data.

    tst      ip,        $7
    beq      L_vectorCopy
    
// Otherwise, we copy a single byte at a time, in order of ascending memory
// address, until the destination is 8 byte aligned.  Within this loop,
// registers are used as follows:
//
//  r0  original destination pointer
//  r1  pointer to the next byte to copy
//  r2  (bytes remaining to be copied) - 8
//  r3  temporary to hold the byte that is being copied
//  ip  pointer to the next byte to store to

0:  ldrb     r3,  [r1], $1
    sub      r2,        $1
    strb     r3,  [ip], $1
    tst      ip,        $7
    bne      0b
    
// At this point, the destination pointer is 8 byte aligned.  Check again that
// there are at least 8 bytes remaining to copy by comparing the remaining
// length minus 8 to zero.  If fewer than 8 bytes remain, jump to the cleanup
// path.

    cmp      r2,        $0
    blt      L_scalarCopy
    
/*****************************************************************************
 *  destination is doubleword aligned                                        *
 *****************************************************************************/

L_vectorCopy:

// At this point, registers contain the following values:
//
//  r0  original destination pointer
//  r1  pointer to the next element to be copied
//  r2  (bytes remaining to copy) - 8
//  ip  pointer to the destination of the next byte to be copied
//
// Furthermore, it is known that ip is 8 byte aligned, and that r2 is positive.
// NEON has really excellent alignment handling in hardware, so we would like
// to use that to handle cases where the source is not similarly aligned to the
// destination (it supports even single-byte misalignment at speed).  However,
// on some SoC designs, not all of the DMA busses support such access.  Thus,
// we must unfortunately use a software workaround in those cases.
//
// Fortunately, 4-byte aligned loads are supported even on the DMA busses, so
// we only need to handle the different possible source alignments modulo 4.
// Here we have a dispatch table to jump to the correct copy implementation
// for the given source alignment.
//
// The tbh instruction loads the address offset of the correct implementation
// from the data table that immediately follows it and adds it to the pc to 
// jump to the correct branch.

    ands     r3,    r1, $3
    bic      r1,        $3
    tbh     [pc, r3, lsl $1]
0:  
.short (L_sourceAligned0-0b)/2
.short (L_sourceAligned1-0b)/2
.short (L_sourceAligned2-0b)/2
.short (L_sourceAligned3-0b)/2

/*****************************************************************************
 *  source is also at least word aligned                                     *
 *****************************************************************************/
    
L_sourceAligned0:

// Subtract 56 from r2, so that it contains the number of bytes remaining to
// copy minus 64.  If this result is negative, then we jump into a loop that
// copies 8 bytes at a time.

    subs     r2,        $0x38
    blt      L_vectorCleanup
    
// Check if the destination pointer is 64-byte aligned.  If so, jump to a loop
// that copies whole cachelines.

    tst      ip,        $0x38
    beq      L_cachelineAligned
        
// Otherwise, we copy a 8 bytes at a time, in order of ascending memory
// address, until the destination is 64 byte aligned.  Within this loop,
// registers are used as follows:
//
//  r0  original destination pointer
//  r1  pointer to the next element to be copied
//  r2  (bytes remaining to be copied) - 64
//  ip  pointer to the destination of the next byte to be copied
//  d0  temporary storage for copy

0:  vld1.32 {d0},  [r1]!
    sub      r2,        $8
    vst1.64 {d0},  [ip,:64]!
    tst      ip,        $0x38
    bne      0b
    
// At this point, the destination pointer is 64 byte aligned.  Check again that
// there are at least 64 bytes remaining to copy by comparing the remaining
// length minus 64 to zero.  If fewer than 64 bytes remain, skip over the main
// copy loop.

    cmp      r2,        $0
    blt      L_vectorCleanup
    
/*****************************************************************************
 *  destination is cacheline aligned                                         *
 *****************************************************************************/

// In the special case that we are copying a buffer of between 1k and 32k bytes
// we do not use a NEON copy for the main loop.  This is because if we happen
// to be doing a copy from a source in cache to a destination that is not in
// cache, this will result in an increase in energy usage.  In all other cases,
// NEON gives superior energy conservation.

L_cachelineAligned:
    sub      r3,    r2, $0x3c0
    cmp      r3,        $0x7c00
    blo      L_useSTMIA
    
// The destination pointer is known to be 64-byte aligned, so we can use the
// maximal alignment hint (:256) for our vector stores.  Detect if the source
// is also at least 32-byte aligned and jump to a loop that uses maximal
// alignment hints for the loads as well if possible.

    tst      r1,        $0x1f
    beq      L_sourceAligned32
    
// This loop copies 64 bytes per iteration, from a 4-byte aligned source to a
// 64-byte aligned destination, in order of ascending memory address.  Within
// this loop, registers are used as follows:
//
//  r0      original destination pointer (unmodified)
//  r1      pointer to the next 32-byte block to load
//  r2      (number of bytes remaining to copy) - 64
//  ip      pointer to which the next 32-byte block is to be stored
//  q0-q3   temporary registers used for copies
//
// Note that the loop is arrange in such a way that a single cleanup store is
// necessary after the final loop iteration.  This occurs at label (1), and is
// shared between the unaligned and aligned loops.

    vld1.32 {q2,q3}, [r1]!
    vld1.32 {q0,q1}, [r1]!
    subs     r2,         $64
    vst1.64 {q2,q3}, [ip,:256]!
    blt      1f
.align 3
0:  vld1.32 {q2,q3}, [r1]!
    vst1.64 {q0,q1}, [ip,:256]!
    vld1.32 {q0,q1}, [r1]!
    subs     r2,         $64
    vst1.64 {q2,q3}, [ip,:256]!
    bge      0b
    b        1f
    
L_sourceAligned32:

// This loop is identical to the immediately preceeding loop, except that it
// uses the additional alignment hint that the source pointer (r1) is 32-byte
// aligned.  The two loops share cleanup code for the final iteration.

    vld1.64 {q2,q3}, [r1,:256]!
    vld1.64 {q0,q1}, [r1,:256]!
    subs     r2,         $64
    vst1.64 {q2,q3}, [ip,:256]!
    blt      1f
.align 3
0:  vld1.64 {q2,q3}, [r1,:256]!
    vst1.64 {q0,q1}, [ip,:256]!
    vld1.64 {q0,q1}, [r1,:256]!
    subs     r2,         $64
    vst1.64 {q2,q3}, [ip,:256]!
    bge      0b
    
// Final vector store for both of the above loops.

1:  vst1.64 {q0,q1}, [ip,:256]!

L_vectorCleanup:

// Add 56 to r2, so that it contains the number of bytes remaing to copy minus
// 8.  A comparison of this value with zero tells us if any more whole 8-byte
// blocks need to be copied.

    adds     r2,        $0x38
    blt      L_scalarCopy

// This loop copies 8 bytes at a time in order of descending memory address,
// until fewer than 8 bytes remain to be copied.  Within this loop, registers
// are used as follows:
//
//  r0  original destination pointer
//  r1  pointer to the next element to be copied
//  r2  (bytes remaining to be copied) - 64
//  ip  pointer to the destination of the next byte to be copied
//  d0  temporary storage for copy
    
0:  vld1.32 {d0},   [r1]!
    subs     r2,        $8
    vst1.64 {d0},   [ip,:64]!
    bge      0b

/*****************************************************************************
 *  sub-doubleword cleanup copies                                            *
 *****************************************************************************/

L_scalarCopy:

// Add 8 to r2, so that it contains the number of bytes remaining to copy, and
// return to the calling routine if zero bytes remain.

    adds     r2,        $8
    it       eq
    bxeq     lr
         
// Copy one byte at a time in descending address order until we reach the front
// of the buffer.  Within this loop, registers are used as follows:
//
//  r0  original destination pointer
//  r1  pointer to one byte past the next element to be copied
//  r2  (bytes remaining to be copied) - 8
//  r3  temporary to hold the byte that is being copied
//  ip  pointer one byte past the destination of the next byte to be copied

0:  ldrb     r3,    [r1], $1
    strb     r3,    [ip], $1
    subs     r2,          $1
    bne      0b
    bx       lr
    
/*****************************************************************************
 *  STMIA loop for 1k-32k buffers                                            *
 *****************************************************************************/
    
// This loop copies 64 bytes each iteration in order of ascending memory
// address, using the GPRs instead of NEON.
//
//  r0  original destination pointer
//  r1  pointer to the next element to be copied
//  r2  (bytes remaining to be copied) - 64
//  r3-6,r8-11 (COPY_REGISTERS) temporary registers used for moving data
//  ip  pointer to the next location to store to

L_useSTMIA:
    push     SAVE_REGISTERS
.align 3
0:  ldmia   r1!,    COPY_REGISTERS
    subs    r2,     r2,  $64
    stmia   ip!,    COPY_REGISTERS
    ldmia   r1!,    COPY_REGISTERS
    pld     [r1, $64]
    stmia   ip!,    COPY_REGISTERS
    bge     0b
    pop     SAVE_REGISTERS
    b       L_vectorCleanup
    
/*****************************************************************************
 *  Misaligned forward vld1 loop                                             *
 *****************************************************************************/

// Software alignment fixup to handle source and dest that are relatively
// misaligned mod 4 bytes.  
//
// The basic idea is to use 4-byte aligned loads to load 8 bytes per iteration,
// which we combine with the 8 bytes loaded in the previous iteration to get a
// 16 byte field; the next 8 bytes to be stored to the destination buffer are
// somewhere in that field, and we get them using the VEXT instruction:
//      
//     |  8 bytes from last iteration  |  8 bytes from this iteration  |
//     +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
//     | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | a | b | c | d | e | f |
//     +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
//         ^8 bytes to store this iteration^           |
//                                                   could be a page boundary 
//
// We need to be a little bit careful, however.  Because the loads only have 4
// byte alignment, if we used this approach all the way to the end of the
// buffer, the very last 8 byte load might slop over onto a new page by 4
// bytes, and that new page might not be mapped into our process.  Thus, we
// terminate this copy loop when fewer than 12 bytes remain to be copied,
// instead of the more natural-seeming termination condition of "8 bytes
// remaining" (the illustration above shows the worst case and demonstrates
// why 12 is a sufficiently safe condition).
//
// At the beginning of each iteration through this loop, registers are used
// as follows:
//
//  r0  original destination pointer
//  r1  pointer to the next block of 8 bytes to load
//  r2  (bytes remaining to copy) - 12
//  ip  pointer to the next block of 8 bytes to store
//  d0  next 8 bytes to store
//  d2  8 bytes loaded in the previous iteration
//  d3  8 bytes loaded two iterations ago

#define COPY_UNALIGNED(offset)       \
    subs      r2,          $4       ;\
    blt       2f                    ;\
    vld1.32  {d2,d3}, [r1]!         ;\
    subs      r2,          $8       ;\
    blt       1f                    ;\
0:  vext.8    d0,  d2, d3, $(offset);\
    vmov      d2,      d3           ;\
    vld1.32  {d3},    [r1]!         ;\
    subs      r2,          $8       ;\
    vst1.64  {d0},    [ip, :64]!    ;\
    bge       0b                    ;\
1:  vext.8    d0,  d2, d3, $(offset);\
    sub       r1,          $8       ;\
    vst1.64  {d0},    [ip, :64]!    ;\
2:  add       r1,          $(offset);\
    add       r2,          $4       ;\
    b         L_scalarCopy

L_sourceAligned1:
    COPY_UNALIGNED(1)
L_sourceAligned2:
    COPY_UNALIGNED(2)
L_sourceAligned3:
    COPY_UNALIGNED(3)

#endif // defined _ARM_ARCH_7 && !defined VARIANT_DYLD

/*
 * Copyright (c) 2010 Apple, Inc. All rights reserved.
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
 
//  char * strchr(const char *s, int c);
//
//  The strchr( ) function locates the first occurence of c (converted to a
//  char) in the string s.  If c is '\0', the function locates the terminating
//  '\0' byte.

.text
.syntax unified
.code 32
.globl _strchr

.align 3
_strchr:
//  Setup a frame and convert c to a char.
    push   {r7,lr}
    mov     r7,         sp
    and     r1,     r1, #0xff

//  Basic character-by-character search for c; on each pass through the loop
//  we load a byte from s and increment s.  We then check if this byte is equal
//  to either c or to '\0'; if not, we continue the loop.
0:  ldrb    r2,    [r0],#1
    cmp     r2,     r1
    tstne   r2,     r2
    bne     0b

//  Either we have found a match, or we have reached the end of the string (or
//  both, if c == '\0').  Because we incremented s after loading the last
//  character, we need to decrement the pointer by one to get a pointer to
//  either the matching or terminating character. 
    sub     r0,     r0, #1
    
//  Now compare the last character we loaded to the character that we're
//  searching for.  If they don't match, the character was not found in the
//  string, so return NULL.
    cmp     r2,     r1
    movne   r0,     #0
    pop    {r7,pc}

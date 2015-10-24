/*
 * Copyright (c) 2010, Apple Inc. All rights reserved.
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

//  char * strstr(const char *s1, const char *s2);
//
//  If s2 is empty, s1 is returned.
//  If s2 does not occur in s1, NULL is returned.
//  Otherwise, a pointer to the first character of the first occurrence of s2
//  in s1 is returned.
//
//  We use a hand-tuned version of the naive quadratic time algorithm; we have
//  experimented with more sophisticated algorithms, however they have been
//  found wanting.  The increased startup cost combines with the frequency of
//  calls to strstr with small strings to swamp the gains from improved
//  asymptotic complexity.

#define CLEAR_FRAME_AND_RETURN \
    pop    {r4,r5,r6,r7,pc}

.text
.syntax unified
.code 32
.globl _strstr
.align 2
_strstr:
    push   {r4,r5,r6,r7,lr}
    add     r7,     sp, #12
    
//  Throughout this function, I will refer to the string in which we are
//  searching as "string" and the string for which we are searching as "word".
//  Using s1 and s2 is too confusing.  Thus, we want to return a pointer to
//  the first occurrence of "word" in "string".
    mov     r5,     r1
    mov     r4,     r0

//  We begin by calling strlen to find the length of "word".  We also handle
//  two special cases here: we early-out if word is an empty string (length
//  zero), and we call strchr if word is only a single character.
    mov     r0,     r5
    bl      _strlen
    subs    r0,     r0, #1
    ble     L_tinyWord
    
//  Load the first character of word
    ldrb    ip,    [r5]

L_lookForFirstCharacter:
//  Load the first character from string.  If it is equal to the first
//  character of word, or is a zero byte, then we do more processing;
//  otherwise, proceed to the next character.
    ldrb    r1,    [r4],#1
    cmp     r1,     ip
    tstne   r1,     r1
    bne     L_lookForFirstCharacter
    
//  The byte that we loaded from string either matched the first character
//  of word, or was a zero byte.  If it was a zero byte, then we have reached
//  the end of string without finding a match of word.  Otherwise, we fall
//  into a loop that checks additional characters to see if we have a match.
    tst     r1,     r1
    beq     L_noMatch
    
//  We have found a match for the first character of word; we want to read
//  characters from this point on to see if we have a match for the entirety
//  of word.  We want to be sure to preserve the state of the outside loop,
//  however:
//
//      r0: length(word) - 1
//      r4: pointer to the next character in string
//      r5: pointer to the first character in word
//      ip: first character in word
//
//  The registers r1-r3, r6, and lr are available as scratch.  We set them up
//  for the inner loop as follows:
//
//      r1: remaining length to be matched
//      r2: pointer to next character of string to match
//      r3: pointer to next character of word to match
//      r6: current character from string
//      lr: current character from word    
    mov     r2,     r4
    add     r3,     r5, #1
    mov     r1,     r0
    
L_checkMatch:
//  Load the next byte from both the candidate match and from word.  If they
//  are not equal, jump back to the outer loop.  If they are equal, decrement
//  the length, and continue the inner loop if we have not yet found a
//  complete match.
//
//  We don't need to worry about looking for null bytes in this loop; we know
//  that we won't load a null byte from word, because we computed it's length
//  earlier, and are using that as a termination condition.  If we hit a null
//  byte in string, the comparison will fail (because the corresponding byte
//  in word is non-null), and we will return to the outer loop, where the
//  null byte will be detected and handled properly.
    ldrb    r6,    [r2],#1
    ldrb    lr,    [r3],#1
    cmp     r6,     lr
    bne     L_lookForFirstCharacter
    subs    r1,     r1, #1
    bne     L_checkMatch

//  We have exhausted the length of word without finding a mismatched character
//  so we have found a match.  Return a pointer to the beginning of the match
//  string.  (This pointer is r4 - 1 because r4 was auto-incremented when we
//  loaded the first character).
    sub     r0,     r4, #1
    CLEAR_FRAME_AND_RETURN

L_noMatch:
//  No match was found; return NULL.
    mov     r0,     #0
    CLEAR_FRAME_AND_RETURN

L_tinyWord:
//  "word" is either empty or a single character.  If it is a single character,
//  translate strstr into a call to the (more efficient) strchr.
    blt     L_emptyWord
    ldrb    r1,    [r5]
    mov     r0,     r4
    bl      _strchr
    CLEAR_FRAME_AND_RETURN
    
L_emptyWord:
//  "word" is empty; return "string".
    movlt   r0,     r4
    CLEAR_FRAME_AND_RETURN
    
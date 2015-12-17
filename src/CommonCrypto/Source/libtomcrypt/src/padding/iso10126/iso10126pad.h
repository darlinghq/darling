/*
 * Copyright (c) 2010 Apple Inc. All Rights Reserved.
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

#include <stdint.h>
#include <stddef.h>

/*
 *  iso10126pad.h
 *  MacTomCrypt
 *
 * iso10126 padding mode functions.
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Fill out the padding for a buffer.  The blocksize and starting points are used to determine how much needs
 * to be padded.  If startpoint is blocksize+1 then a full new buffer is added.  Blocksize cannot be greater
 * than 256.
 */
 
int
iso10126_pad(char *buff, size_t blocksize, size_t startpoint);

/*
 * Given the last buffer containing the pad, how many bytes is the original text?
 * returns -1 on error.
 */
 
int
iso10126_unpadlen(char *buff, size_t blocksize);

/*
 * Maximum space needed for padding.  For iso10126 this is blocksize*2
 */
 
int
iso10126_len(size_t blocksize);

#ifdef __cplusplus
}
#endif


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
/*
 * Copyright (c) 1992 NeXT Computer, Inc.
 *
 * Natural alignment of shorts and longs (for i386)
 *
 * HISTORY
 *
 * 2 Sept 1992 Brian Raymor at NeXT
 *      Moved over to architecture.
 * 18 August 1992 Jack Greenfield at NeXT
 *	Created.
 */

#ifndef _ARCH_I386_ALIGNMENT_H_
#define _ARCH_I386_ALIGNMENT_H_

/*
 * NOP
 */
__inline__ static unsigned short
get_align_short(void *ivalue)
{
    return *((unsigned short *) ivalue);
}

__inline__ static unsigned short 
put_align_short(unsigned short ivalue, void *ovalue)
{
    return *((unsigned short *) ovalue) = ivalue;
}

/*
 * NOP
 */
__inline__ static unsigned long
get_align_long(void *ivalue)
{
    return *((unsigned long *) ivalue);
}

__inline__ static unsigned long 
put_align_long(unsigned long ivalue, void *ovalue)
{
    return *((unsigned long *) ovalue) = ivalue;
}

#endif	/* _ARCH_I386_ALIGNMENT_H_ */

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

#include <mach/kern_return.h>
#include <mach/mach_time.h>
#include <stdint.h>

extern void spin_lock(int *);
extern void spin_unlock(int *);

/* deprecated function stub */
kern_return_t
MKGetTimeBaseInfo(
    uint32_t *minAbsoluteTimeDelta,
    uint32_t *theAbsoluteTimeToNanosecondNumerator,
    uint32_t *theAbsoluteTimeToNanosecondDenominator,
    uint32_t *theProcessorToAbsoluteTimeNumerator,
    uint32_t *theProcessorToAbsoluteTimeDenominator
) {
    static struct mach_timebase_info mti = {0};
    static int MKGetTimeBaseInfo_spin_lock = 0;

    if(mti.numer == 0) {
	kern_return_t err;
	spin_lock(&MKGetTimeBaseInfo_spin_lock);
	err = mach_timebase_info(&mti);
	spin_unlock(&MKGetTimeBaseInfo_spin_lock);
	if(err != KERN_SUCCESS)
	    return err;
    }
    if(theAbsoluteTimeToNanosecondNumerator)
	*theAbsoluteTimeToNanosecondNumerator = mti.numer;
    if(theAbsoluteTimeToNanosecondDenominator)
	*theAbsoluteTimeToNanosecondDenominator = mti.denom;
    if(minAbsoluteTimeDelta)
	*minAbsoluteTimeDelta = 1;
    if(theProcessorToAbsoluteTimeNumerator)
	*theProcessorToAbsoluteTimeNumerator = 1;
    if(theProcessorToAbsoluteTimeDenominator)
	*theProcessorToAbsoluteTimeDenominator = 1;
    return KERN_SUCCESS;
}

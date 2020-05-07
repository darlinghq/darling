/*
 * Copyright (c) 1998-2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
#ifndef _IOBSD_H
#define _IOBSD_H

/*
 * bsd-related registry properties
 */

#define kIOBSDKey      "IOBSD"     // (BSD subsystem resource)
#define kIOBSDNameKey  "BSD Name"  // (an OSString)
#define kIOBSDNamesKey "BSD Names" // (an OSDictionary of OSString's, for links)
#define kIOBSDMajorKey "BSD Major" // (an OSNumber)
#define kIOBSDMinorKey "BSD Minor" // (an OSNumber)
#define kIOBSDUnitKey  "BSD Unit"  // (an OSNumber)


#ifdef KERNEL_PRIVATE

#include <stdint.h>
#include <kern/task.h>

#ifdef __cplusplus
extern "C" {
#endif

struct IOPolledFileIOVars;
struct mount;

enum{
	kIOMountChangeMount      = 0x00000101,
	kIOMountChangeUnmount    = 0x00000102,
	kIOMountChangeWillResize = 0x00000201,
	kIOMountChangeDidResize  = 0x00000202,
};
extern void IOBSDMountChange(struct mount * mp, uint32_t op);
extern boolean_t IOTaskHasEntitlement(task_t task, const char * entitlement);

typedef enum {
	kIOPolledCoreFileModeNotInitialized,
	kIOPolledCoreFileModeDisabled,
	kIOPolledCoreFileModeClosed,
	kIOPolledCoreFileModeStackshot,
	kIOPolledCoreFileModeCoredump,
} IOPolledCoreFileMode_t;

extern struct IOPolledFileIOVars * gIOPolledCoreFileVars;
extern kern_return_t gIOPolledCoreFileOpenRet;
extern IOPolledCoreFileMode_t gIOPolledCoreFileMode;

#ifdef __cplusplus
}
#endif

#endif /* XNU_KERNEL_PRIVATE */

#endif /* !_IOBSD_H */

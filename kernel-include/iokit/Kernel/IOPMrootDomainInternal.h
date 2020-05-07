/*
 * Copyright (c) 2006-2007 Apple Inc. All rights reserved.
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

/* DO NOT MODIFY THIS FILE
 *
 * This file is a partial mirror of
 *    AppleIntelCPUPowerManagement/pmioctl.h
 * Changes may only be made to the original, pmioctl.h.
 * This file must be updated only when pmioctl.h changes.
 */

/*
 * Defines the IOCTLs for dealing with the CPU power management KEXT.
 */
#ifndef _IOPMROOTDOMAINIOCTLS_H_
#define _IOPMROOTDOMAINIOCTLS_H_

#include <sys/ioccom.h>
#include <i386/pmCPU.h>

#define PMIOCGETVARIDINFO       _IOW('P', 25, uint64_t)
#define PMIOCGETVARNAMEINFO     _IOW('P', 26, uint64_t)
#define PMIOCSETVARINFO         _IOW('P', 27, uint64_t)

/*
 * Data structures used by IOCTLs
 */
#pragma pack(4)

#define PMVARNAMELEN    16

typedef enum{
	vUnknown            = 0,        /* Unknown type */
	vBool               = 1,        /* Boolean value */
	vInt                = 2,        /* signed integer value */
	vUInt               = 3,        /* Unsigned integer value */
	vChars              = 4,        /* 8 characters */
	vInvalid            = -1        /* invalid type */
} pmioctlVarType_t;

typedef struct pmioctlVaribleInfo {
	uint32_t            varID;      /* ID of variable */
	uint8_t             varName[PMVARNAMELEN + 1];
	pmioctlVarType_t    varType;    /* type of variable's value */
	uint64_t            varInitValue;/* variable's initial value */
	uint64_t            varCurValue;/* variable's current value */
} pmioctlVariableInfo_t;

#pragma pack()

#endif /* _IOPMROOTDOMAINIOCTLS_H_ */

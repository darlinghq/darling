/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
 
#ifndef _I386_VMX_ASM_H_
#define _I386_VMX_ASM_H_

#define VMX_FAIL_INVALID	-1
#define VMX_FAIL_VALID		-2
#define VMX_SUCCEED			0

/*
 * VMX Capability Registers (VCR)
 *
 */
#define VMX_VCR_VMCS_MEM_TYPE_BIT	50
#define VMX_VCR_VMCS_MEM_TYPE_MASK	0xF

#define VMX_VCR_VMCS_SIZE_BIT		32
#define VMX_VCR_VMCS_SIZE_MASK		0x01FFF
#define VMX_VCR_VMCS_REV_ID		0x00000000FFFFFFFFLL

#define VMX_VCR_ACT_HLT_BIT		6
#define VMX_VCR_ACT_HLT_MASK		0x1
#define VMX_VCR_ACT_SHUTDOWN_BIT	7
#define VMX_VCR_ACT_SHUTDOWN_MASK	0x1
#define VMX_VCR_ACT_SIPI_BIT		8
#define VMX_VCR_ACT_SIPI_MASK		0x1
#define VMX_VCR_ACT_CSTATE_BIT		9
#define VMX_VCR_ACT_CSTATE_MASK		0x1
#define VMX_VCR_CR3_TARGS_BIT		16
#define VMX_VCR_CR3_TARGS_MASK		0xFF
#define VMX_VCR_MAX_MSRS_BIT		25
#define VMX_VCR_MAX_MSRS_MASK		0x7
#define VMX_VCR_MSEG_ID_BIT		32
#define VMX_VCR_MSEG_ID_MASK		0xFFFFFFFF

#endif	/* _I386_VMX_H_ */

/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
/*
 * @OSF_COPYRIGHT@
 *
 */
#ifndef _I386_APIC_H_
#define _I386_APIC_H_

#define IOAPIC_START                    0xFEC00000
#define IOAPIC_SIZE                     0x00000020

#define IOAPIC_RSELECT                  0x00000000
#define IOAPIC_RWINDOW                  0x00000010
#define IOA_R_ID                        0x00
#define         IOA_R_ID_SHIFT          24
#define IOA_R_VERSION                   0x01
#define         IOA_R_VERSION_MASK      0xFF
#define         IOA_R_VERSION_ME_SHIFT  16
#define         IOA_R_VERSION_ME_MASK   0xFF
#define IOA_R_REDIRECTION               0x10
#define         IOA_R_R_VECTOR_MASK     0x000FF
#define         IOA_R_R_DM_MASK         0x00700
#define         IOA_R_R_DM_FIXED        0x00000
#define         IOA_R_R_DM_LOWEST       0x00100
#define         IOA_R_R_DM_NMI          0x00400
#define         IOA_R_R_DM_RESET        0x00500
#define         IOA_R_R_DM_EXTINT       0x00700
#define         IOA_R_R_DEST_LOGICAL    0x00800
#define         IOA_R_R_DS_PENDING      0x01000
#define         IOA_R_R_IP_PLRITY_LOW   0x02000
#define         IOA_R_R_TM_LEVEL        0x08000
#define         IOA_R_R_MASKED          0x10000

#endif /* _I386_APIC_H_ */

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
#ifdef KERNEL_PRIVATE
#ifndef _I386_HPET_H_
#define _I386_HPET_H_

/*
 * HPET kernel functions to support the HPET KEXT and the
 * power management KEXT.
 */


/*
 *	Memory mapped registers for the HPET
 */
typedef struct hpetReg {
	uint64_t        GCAP_ID;                /* General capabilities */
	uint64_t        rsv1;
	uint64_t        GEN_CONF;               /* General configuration */
	uint64_t        rsv2;
	uint64_t        GINTR_STA;              /* General Interrupt status */
	uint64_t        rsv3[25];
	uint64_t        MAIN_CNT;               /* Main counter */
	uint64_t        rsv4;
	uint64_t        TIM0_CONF;              /* Timer 0 config and cap */
#define                 TIM_CONF 0
#define                 Tn_INT_ENB_CNF 4
	uint64_t        TIM0_COMP;              /* Timer 0 comparator */
#define                 TIM_COMP 8
	uint64_t        rsv5[2];
	uint64_t        TIM1_CONF;              /* Timer 1 config and cap */
	uint64_t        TIM1_COMP;              /* Timer 1 comparator */
	uint64_t        rsv6[2];
	uint64_t        TIM2_CONF;              /* Timer 2 config and cap */
	uint64_t        TIM2_COMP;              /* Timer 2 comparator */
	uint64_t        rsv7[2];
} hpetReg;
typedef struct  hpetReg hpetReg_t;

typedef struct hpetTimer {
	uint64_t        Config;         /* Timer config and capabilities */
	uint64_t        Compare;        /* Timer comparitor */
} hpetTimer_t;

struct hpetInfo {
	uint64_t        hpetCvtt2n;
	uint64_t        hpetCvtn2t;
	uint64_t        tsc2hpet;
	uint64_t        hpet2tsc;
	uint64_t        bus2hpet;
	uint64_t        hpet2bus;
	uint32_t        rcbaArea;
	uint32_t        rcbaAreap;
};
typedef struct hpetInfo hpetInfo_t;

struct hpetRequest {
	uint32_t        flags;
	uint32_t        hpetOffset;
	uint32_t        hpetVector;
};
typedef struct hpetRequest hpetRequest_t;

#define HPET_REQFL_64BIT        0x00000001      /* Timer is 64 bits */

extern uint64_t hpetFemto;
extern uint64_t hpetFreq;
extern uint64_t hpetCvtt2n;
extern uint64_t hpetCvtn2t;
extern uint64_t tsc2hpet;
extern uint64_t hpet2tsc;
extern uint64_t bus2hpet;
extern uint64_t hpet2bus;

extern vm_offset_t rcbaArea;
extern uint32_t rcbaAreap;

extern void map_rcbaAread(void);
extern void hpet_init(void);

extern void hpet_save(void);
extern void hpet_restore(void);

#ifdef XNU_KERNEL_PRIVATE
extern int HPETInterrupt(void);
#endif

extern int hpet_register_callback(int (*hpet_reqst)(uint32_t apicid, void *arg, hpetRequest_t *hpet), void *arg);
extern int hpet_request(uint32_t cpu);

extern uint64_t rdHPET(void);
extern void hpet_get_info(hpetInfo_t *info);

#define hpetAddr        0xFED00000
#define hptcAE          0x80

#endif  /* _I386_HPET_H_ */

#endif  /* KERNEL_PRIVATE */

/*
 * Copyright (c) 2008 Apple Computer, Inc. All rights reserved.
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

#ifndef _I386_POSTCODE_H_
#define _I386_POSTCODE_H_

/*
 * Postcodes are no longer enabled by default in the DEBUG kernel
 * because platforms may not have builtin port 0x80 support.
 * To re-enable postcode outpout, uncomment the following define:
 */
//#define DEBUG_POSTCODE 1

/* Define this to delay about 1 sec after posting each code */
//#define POSTCODE_DELAY 1

/* The POSTCODE is port 0x80 */
#define POSTPORT 0x80

#define SPINCOUNT       300000000
#define CPU_PAUSE()     rep; nop

#if DEBUG_POSTCODE
/*
 * Macro to output byte value to postcode, destoying register al.
 * Additionally, if POSTCODE_DELAY, spin for about a second.
 */
#if POSTCODE_DELAY
#define POSTCODE_AL                     \
	outb    %al,$(POSTPORT);        \
	movl	$(SPINCOUNT), %eax;     \
1:                                      \
	CPU_PAUSE();                    \
	decl	%eax;                   \
	jne	1b
#define POSTCODE_AX                     \
	outw    %ax,$(POSTPORT);        \
	movl	$(SPINCOUNT), %eax;     \
1:                                      \
	CPU_PAUSE();                    \
	decl	%eax;                   \
	jne	1b
#else
#define POSTCODE_AL                     \
	outb    %al,$(POSTPORT)
#define POSTCODE_AX                     \
	outw    %ax,$(POSTPORT)
#endif /* POSTCODE_DELAY */

#define POSTCODE(XX)                    \
	mov	$(XX), %al;             \
	POSTCODE_AL

#define POSTCODE2(XXXX)                 \
	mov	$(XXXX), %ax;           \
	POSTCODE_AX

/* Output byte value to postcode, without destoying register eax */
#define POSTCODE_SAVE_EAX(XX)           \
	push	%eax;                   \
	POSTCODE(XX);                   \
	pop	%eax

/*
 * Display a 32-bit value to the post card - low byte to high byte
 * Entry: value in %ebx
 * Exit: %ebx preserved; %eax destroyed
 */
#define POSTCODE32_EBX                  \
	roll	$8, %ebx;               \
	movl	%ebx, %eax;             \
	POSTCODE_AL;                    \
                                        \
	roll	$8, %ebx;               \
	movl	%ebx, %eax;             \
	POSTCODE_AL;                    \
                                        \
	roll	$8, %ebx;               \
	movl	%ebx, %eax;             \
	POSTCODE_AL;                    \
                                        \
	roll	$8, %ebx;               \
	movl	%ebx, %eax;             \
	POSTCODE_AL

#else   /* DEBUG_POSTCODE */
#define POSTCODE_AL
#define POSTCODE_AX
#define POSTCODE(X)
#define POSTCODE2(X)
#define POSTCODE_SAVE_EAX(X)
#define POSTCODE32_EBX
#endif  /* DEBUG_POSTCODE */

/*
 * The following postcodes are defined for stages of early startup:
 */

#define PSTART_ENTRY                    0xFF
#define PSTART_REBASE                   0xFE
#define PSTART_BEFORE_PAGING            0xFE
#define PSTART_VSTART                   0xFD
#define VSTART_ENTRY                    0xFC
#define VSTART_IDT_INIT                 0xFB
#define VSTART_IDLE_PTS_INIT            0xFA
#define VSTART_PHYSMAP_INIT             0xF9
#define VSTART_DESC_ALIAS_INIT          0xF8
#define VSTART_SET_CR3                  0xF7
#define VSTART_CPU_DESC_INIT            0xF6
#define VSTART_CPU_MODE_INIT            0xF5
#define VSTART_EXIT                     0xF4
#define I386_INIT_ENTRY                 0xF3
#define CPU_INIT_D                      0xF2
#define PE_INIT_PLATFORM_D              0xF1

#define SLAVE_STARTPROG_ENTRY           0xEF
#define SLAVE_PSTART                    0xEE
#define I386_INIT_SLAVE                 0xED

#define PANIC_DOUBLE_FAULT              0xDF    /* Double Fault exception */
#define PANIC_MACHINE_CHECK             0xDC    /* Machine-Check */
#define MP_KDP_ENTER                    0xDB    /* Debugger Begin */
#define MP_KDP_EXIT                     0xDE    /* Debugger End */
#define PANIC_HLT                       0xD1    /* Die an early death */
#define BOOT_TRAP_HLT                   0xD0    /* D'oh! even earlier */

#define ACPI_WAKE_START_ENTRY           0xCF
#define ACPI_WAKE_PROT_ENTRY            0xCE
#define ACPI_WAKE_PAGED_ENTRY           0xCD

#define CPU_DESC_LOAD_ENTRY             0xBF
#define CPU_DESC_LOAD_GS_BASE           0xBE
#define CPU_DESC_LOAD_KERNEL_GS_BASE    0xBD
#define CPU_DESC_LOAD_GDT               0xBC
#define CPU_DESC_LOAD_IDT               0xBB
#define CPU_DESC_LOAD_LDT               0xBA
#define CPU_DESC_LOAD_TSS               0xB9
#define CPU_DESC_LOAD_EXIT              0xB7

#ifndef ASSEMBLER
inline static void
_postcode_delay(uint32_t        spincount)
{
	asm volatile ("1:			\n\t"
                      "  rep; nop;		\n\t"
                      "  decl %%eax;		\n\t"
                      "  jne 1b"
                      : : "a" (spincount));
}
inline static void
_postcode(uint8_t       xx)
{
	asm volatile ("outb %0, %1" : : "a" (xx), "N" (POSTPORT));
}
inline static void
_postcode2(uint16_t     xxxx)
{
	asm volatile ("outw %0, %1" : : "a" (xxxx), "N" (POSTPORT));
}
#if     DEBUG_POSTCODE
inline static void
postcode(uint8_t        xx)
{
	_postcode(xx);
#if     POSTCODE_DELAY
	_postcode_delay(SPINCOUNT);
#endif
}
inline static void
postcode2(uint8_t       xxxx)
{
	_postcode2(xxxx);
#if     POSTCODE_DELAY
	_postcode_delay(SPINCOUNT);
#endif
}
#else
#define postcode(xx) do {} while(0)
#define postcode2(xxxx) do {} while(0)
#endif
#endif

#endif /* _I386_POSTCODE_H_ */

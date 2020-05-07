/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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
#define BSWAP_32(x) \
	(((x) << 24) & 0xff000000) | \
	(((x) <<  8) & 0x00ff0000) | \
	(((x) >>  8) & 0x0000ff00) | \
	(((x) >> 24) & 0x000000ff)

#define BSWAP_32_OFFSET(x) \
	BSWAP_32(x + _COMM_PAGE_SIGS_OFFSET)

#define COMMPAGE_SIGS_BEGIN \
.const_data				; \
.align 2				; \
.private_extern _commpage_sigs_begin	; \
_commpage_sigs_begin:

#define COMMPAGE_SIGS_DONE \
.private_extern _commpage_sigs_end	; \
_commpage_sigs_end:			; \

#define COMMPAGE_SIG_START(x) \
.private_extern _commpage_sig ## x      ; \
_commpage_sig ## x ## :			; \
	.long BSWAP_32(0x14400000)	; \
	.long BSWAP_32(0x00000001)	; \
	.asciz # x                      ; \
	.align 2			; \
	.long BSWAP_32(0x14400000)

#define COMMPAGE_SIG_END(x) \
	.long BSWAP_32(0x4e800020)	; \
	.long BSWAP_32(0x14400000)	; \
	.long BSWAP_32(0x00000000)	; \
	.asciz # x			; \
	.align 2			; \
	.long BSWAP_32(0x14400000)

#define OBJCRTP_SIG_START(x) COMMPAGE_SIG_START(x)

#define OBJCRTP_SIG_END(x) \
	.long BSWAP_32(0x14400000)      ; \
	.long BSWAP_32(0x00000000)      ; \
	.asciz # x                      ; \
	.align 2                        ; \
	.long BSWAP_32(0x14400000)

#define OBJCRTP_SIG_CALL_SUBJECT(x) \
	.long BSWAP_32(0x14400002)      ; \
	.long BSWAP_32(0x00000000)      ; \
	.long BSWAP_32(0x00040000)      ; \
	.long BSWAP_32(0x00000000)      ; \
	.asciz # x                      ; \
	.align 2                        ; \
	.long BSWAP_32(0x14400002)

#define ARG(n) \
	((((n * 2) + 6) << 20) + 4)

#define COMMPAGE_SIG_ARG(n) \
	.long BSWAP_32(0x14400001)	; \
	.long BSWAP_32(ARG(n))		; \
	.long BSWAP_32(0x14400001)

#define COMMPAGE_SIG_CALL(x, n) \
	.long BSWAP_32(0x14400002)	; \
	.long BSWAP_32(n)		; \
	.long BSWAP_32(0x00000000)	; \
	.long BSWAP_32(0x00000000)	; \
	.asciz # x			; \
	.align 2			; \
	.long BSWAP_32(0x14400002)

#define COMMPAGE_SIG_CALL_VOID(x) \
	COMMPAGE_SIG_CALL(x, 0)

#define COMMPAGE_SIG_CALL_RET0(x) \
	COMMPAGE_SIG_CALL(x, ARG(0))

#define COMMPAGE_SIG_CALL_RET1(x) \
	COMMPAGE_SIG_CALL(x, ARG(1))

#define COMMPAGE_FAST_TRAP(x) \
	.long BSWAP_32(0x14400005) ; \
	.long BSWAP_32_OFFSET(x) ; \
	.long BSWAP_32(0x14400005)

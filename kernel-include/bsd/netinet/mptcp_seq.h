/*
 * Copyright (c) 2012-2017 Apple Inc. All rights reserved.
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

#ifndef _NETINET_MPTCP_SEQ_H_
#define _NETINET_MPTCP_SEQ_H_

/*
 * Use 64-bit modulo arithmetic for comparing
 * Data Sequence Numbers and Data ACKs. Implies
 * 2**63 space is available for sending data.
 */
#define MPTCP_SEQ_LT(a, b)      ((int64_t)((a) - (b)) < 0)
#define MPTCP_SEQ_LEQ(a, b)     ((int64_t)((a) - (b)) <= 0)
#define MPTCP_SEQ_GT(a, b)      ((int64_t)((a) - (b)) > 0)
#define MPTCP_SEQ_GEQ(a, b)     ((int64_t)((a) - (b)) >= 0)

#endif  /* _NETINET_MPTCP_SEQ_H_ */

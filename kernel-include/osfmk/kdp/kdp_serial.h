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
#ifndef _KDP_SERIAL_H_
#define _KDP_SERIAL_H_

/*
 * APIs for escaping a KDP UDP packet into a byte stream suitable
 * for a standard serial console
 */

enum {SERIALIZE_WAIT_START, SERIALIZE_READING};

/*
 * Take a buffer of specified length and output it with the given
 * function. Escapes special characters as needed
 */
void kdp_serialize_packet(unsigned char *, unsigned int, void (*func)(char));

/*
 * Add a new character to an internal buffer, and return that
 * buffer when a fully constructed packet has been identified.
 * Will track intermediate state using magic enums above
 */
unsigned char *kdp_unserialize_packet(unsigned char, unsigned int *);

#endif

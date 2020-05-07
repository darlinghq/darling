/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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
#ifndef _KEXT_PANIC_REPORT_H_
#define _KEXT_PANIC_REPORT_H_

#include <sys/cdefs.h>

__BEGIN_DECLS

/*******************************************************************************
* String-compaction tables for panic reports' kext listing.
*******************************************************************************/

typedef struct subs_entry_t {
	const char * substring;
	char         substitute;
} subs_entry_t;

/* Prefix substitution list. Common prefixes are replaced with a single
 * nonalphanumeric character at the beginning of the identifier.
 *
 * List should be in descending order of # components, and should then
 * be in descending frequency order.
 */
subs_entry_t kext_identifier_prefix_subs[] = {
	{ "com.apple.driver.", '>' },
	{ "com.apple.iokit.", '|' },
	{ "com.apple.security.", '$' },
	{ "com.apple.", '@' },

	{ (char *)NULL, '\0' }
};

/* Substring substitution list. Substrings are replaced with a '!' followed
 * by a single letter mapping to the original string.
 *
 * List should be in descending frequency order, and within
 * groups containing same prefix, in descending length order.
 */
subs_entry_t kext_identifier_substring_subs[] = {
	{ "AppleUSB", 'U' },
	{ "Apple", 'A' },
	{ "Family", 'F' },
	{ "Storage", 'S' },
	{ "Controller", 'C' },
	{ "Bluetooth", 'B' },
	{ "Intel", 'I' },

	{ (char *)NULL, '\0' }
};

__END_DECLS
#endif /* _KEXT_PANIC_REPORT_H_ */

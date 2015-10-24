/*
 * Copyright (c) 2005, 2009 Apple Inc. All rights reserved.
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

#ifndef _XLOCALE__TIME_H_
#define _XLOCALE__TIME_H_

__BEGIN_DECLS
//Begin-Libc
#ifndef LIBC_ALIAS_STRFTIME_L
//End-Libc
size_t	 strftime_l(char * __restrict, size_t, const char * __restrict,
		const struct tm * __restrict, locale_t)
		__DARWIN_ALIAS(strftime_l) __strftimelike(3);
//Begin-Libc
#else /* LIBC_ALIAS_STRFTIME_L */
size_t	 strftime_l(char * __restrict, size_t, const char * __restrict,
		const struct tm * __restrict, locale_t)
		LIBC_ALIAS(strftime_l) __strftimelike(3);
#endif /* !LIBC_ALIAS_STRFTIME_L */
//End-Libc
//Begin-Libc
#ifndef LIBC_ALIAS_STRPTIME_L
//End-Libc
char	*strptime_l(const char * __restrict, const char * __restrict,
		struct tm * __restrict, locale_t) 
		__DARWIN_ALIAS(strptime_l) __strftimelike(2);
//Begin-Libc
#else /* LIBC_ALIAS_STRPTIME_L */
char	*strptime_l(const char * __restrict, const char * __restrict,
		struct tm * __restrict, locale_t)
		LIBC_ALIAS(strptime_l) __strftimelike(2);
#endif /* !LIBC_ALIAS_STRPTIME_L */
//End-Libc
__END_DECLS

#endif /* _XLOCALE__TIME_H_ */

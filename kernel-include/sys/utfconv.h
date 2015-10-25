/*
 * Copyright (c) 2000-2005 Apple Computer, Inc. All rights reserved.
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

#ifndef _SYS_UTFCONV_H_
#define	_SYS_UTFCONV_H_

#include <sys/appleapiopts.h>
#include <sys/cdefs.h> 

#ifdef KERNEL
#ifdef __APPLE_API_UNSTABLE

/*
 * UTF-8 encode/decode flags
 */
#define	UTF_REVERSE_ENDIAN   0x0001   /* reverse UCS-2 byte order */
#define UTF_NO_NULL_TERM     0x0002   /* do not add null termination */
#define	UTF_DECOMPOSED       0x0004   /* generate fully decomposed UCS-2 */
#define	UTF_PRECOMPOSED      0x0008   /* generate precomposed UCS-2 */
#define UTF_ESCAPE_ILLEGAL   0x0010   /* escape illegal UTF-8 */
#define UTF_SFM_CONVERSIONS  0x0020   /* Use SFM mappings for illegal NTFS chars */

#define UTF_BIG_ENDIAN       \
        ((BYTE_ORDER == BIG_ENDIAN) ? 0 : UTF_REVERSE_ENDIAN)

#define UTF_LITTLE_ENDIAN    \
        ((BYTE_ORDER == LITTLE_ENDIAN) ? 0 : UTF_REVERSE_ENDIAN)

__BEGIN_DECLS


/*
 * unicode_combinable - Test for a combining unicode character.
 *
 * This function is similar to __CFUniCharIsNonBaseCharacter except
 * that it also includes Hangul Jamo characters.
 */

int unicode_combinable(u_int16_t character);

/*
 * Test for a precomposed character.
 * 
 * Similar to __CFUniCharIsDecomposableCharacter.
 */

int unicode_decomposeable(u_int16_t character);


/*
 * utf8_encodelen - Calculate the UTF-8 encoding length
 *
 * This function takes an Unicode input string, ucsp, of ucslen bytes
 * and calculates the size of the UTF-8 output in bytes (not including
 * a NULL termination byte). The string must reside in kernel memory.
 *
 * FLAGS
 *    UTF_REVERSE_ENDIAN:  Unicode byte order is opposite current runtime
 *
 *    UTF_BIG_ENDIAN:  Unicode byte order is always big endian
 *
 *    UTF_LITTLE_ENDIAN:  Unicode byte order is always little endian
 *
 *    UTF_DECOMPOSED:  assume fully decomposed output
 *
 * ERRORS
 *    None
 */
size_t
utf8_encodelen(const u_int16_t * ucsp, size_t ucslen, u_int16_t altslash,
               int flags);


/*
 * utf8_encodestr - Encodes a Unicode string into UTF-8
 *
 * This function takes an Unicode input string, ucsp, of ucslen bytes
 * and produces the UTF-8 output into a buffer of buflen bytes pointed
 * to by utf8p. The size of the output in bytes (not including a NULL
 * termination byte) is returned in utf8len. The UTF-8 string output
 * is NULL terminated. Both buffers must reside in kernel memory.
 *
 * If '/' chars are possible in the Unicode input then an alternate
 * (replacement) char must be provided in altslash.
 *
 * FLAGS
 *    UTF_REVERSE_ENDIAN:  Unicode byte order is opposite current runtime
 *
 *    UTF_BIG_ENDIAN:  Unicode byte order is always big endian
 *
 *    UTF_LITTLE_ENDIAN:  Unicode byte order is always little endian
 *
 *    UTF_NO_NULL_TERM:  do not add null termination to output string
 *
 *    UTF_DECOMPOSED:  generate fully decomposed output
 *
 * ERRORS
 *    ENAMETOOLONG:  output did not fit; only utf8len bytes were encoded
 *
 *    EINVAL:  illegal Unicode char encountered
 */
int
utf8_encodestr(const u_int16_t * ucsp, size_t ucslen, u_int8_t * utf8p,
               size_t * utf8len, size_t buflen, u_int16_t altslash, int flags);


/*
 * utf8_decodestr - Decodes a UTF-8 string into Unicode
 *
 * This function takes an UTF-8 input string, utf8p, of utf8len bytes
 * and produces the Unicode output into a buffer of buflen bytes pointed
 * to by ucsp. The size of the output in bytes (not including a NULL
 * termination byte) is returned in ucslen. Both buffers must reside
 * in kernel memory.
 *
 * If '/' chars are allowed in the Unicode output then an alternate
 * (replacement) char must be provided in altslash.
 *
 * FLAGS
 *    UTF_REV_ENDIAN:  Unicode byte order is opposite current runtime
 *
 *    UTF_BIG_ENDIAN:  Unicode byte order is always big endian
 *
 *    UTF_LITTLE_ENDIAN:  Unicode byte order is always little endian
 *
 *    UTF_DECOMPOSED:  generate fully decomposed output (NFD)
 *
 *    UTF_PRECOMPOSED:  generate precomposed output (NFC)
 *
 *    UTF_ESCAPE_ILLEGAL:  percent escape any illegal UTF-8 input
 *
 * ERRORS
 *    ENAMETOOLONG:  output did not fit; only ucslen bytes were decoded.
 *
 *    EINVAL:  illegal UTF-8 sequence encountered.
 */
int
utf8_decodestr(const u_int8_t* utf8p, size_t utf8len, u_int16_t* ucsp,
               size_t *ucslen, size_t buflen, u_int16_t altslash, int flags);


/*
 * utf8_normalizestr - Normalize a UTF-8 string (NFC or NFD)
 *
 * This function takes an UTF-8 input string, instr, of inlen bytes
 * and produces normalized UTF-8 output into a buffer of buflen bytes
 * pointed to by outstr. The size of the output in bytes (not including
 * a NULL termination byte) is returned in outlen. In-place conversions
 * are not supported (i.e. instr != outstr).  Both buffers must reside
 * in kernel memory.
 *
 * FLAGS
 *    UTF_DECOMPOSED:  output string will be fully decomposed (NFD)
 *
 *    UTF_PRECOMPOSED:  output string will be precomposed (NFC)
 *
 *    UTF_NO_NULL_TERM:  do not add null termination to output string
 *
 *    UTF_ESCAPE_ILLEGAL:  percent escape any illegal UTF-8 input
 *
 * ERRORS
 *    ENAMETOOLONG:  output did not fit or input exceeded MAXPATHLEN bytes
 *
 *    EINVAL:  illegal UTF-8 sequence encountered or invalid flags
 */
int
utf8_normalizestr(const u_int8_t* instr, size_t inlen, u_int8_t* outstr,
                  size_t *outlen, size_t buflen, int flags);


/*
 * utf8_validatestr - validates a UTF-8 string
 *
 * This function takes an UTF-8 input string, utf8p, of utf8len bytes
 * and determines if its valid UTF-8.  The string must reside in kernel
 * memory.
 *
 * ERRORS
 *    EINVAL:  illegal UTF-8 sequence encountered.
 */
int
utf8_validatestr(const u_int8_t* utf8p, size_t utf8len);


__END_DECLS

#endif /* __APPLE_API_UNSTABLE */
#endif /* KERNEL */

#endif /* !_SYS_UTFCONV_H_ */

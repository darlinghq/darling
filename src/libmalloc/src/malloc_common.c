/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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
#include "internal.h"

#pragma mark -
#pragma mark Utility Functions

// libplatform does not have strstr() and we don't want to add any new
// dependencies on libc, so we have to implement a version of strntr()
// here. Fortunately, as it's only used to look for boot arguments, it does not
// have to be efficient. We can also assume that the source string is
// nul-terminated. Eventually, we will move the function to a more central
// location and use it to replace other uses of strstr().
const char *
malloc_common_strstr(const char *src, const char *target, size_t target_len)
{
	const char *next = src;
	while (*next) {
		if (!strncmp(next, target, target_len)) {
			return next;
		}
		next++;
	}
	return NULL;
}

// Converts a string to a long. If a non-numeric value is found, the
// return value is whatever has been accumulated so far. end_ptr always points
// to the character that caused the conversion to stop. We can't use strtol()
// etc because that would add a new dependency on libc. Eventually, this
// function could be made generally available within the library and used to
// replace the existing calls to strtol(). Currenly only handles non-negative
// numbers and does not detect overflow.
long
malloc_common_convert_to_long(const char *ptr, const char **end_ptr)
{
	long value = 0;
	while (*ptr) {
		char c = *ptr;
		if (c < '0' || c > '9') {
			break;
		}
		value = value * 10 + (c - '0');
		ptr++;
	}
	*end_ptr = ptr;
	return value;
}

// Looks for a sequence of the form "key=value" in the string 'src' and
// returns the location of the first character of 'value', or NULL if not
// found. No spaces are permitted around the "=".
const char *
malloc_common_value_for_key(const char *src, const char *key)
{
	const char *ptr = src;
	size_t keylen = strlen(key);
	while ((ptr = malloc_common_strstr(ptr, key, keylen)) != NULL) {
		ptr += keylen;
		if (*ptr == '=') {
			return ptr + 1;
		}
	}
	return NULL;
}

// Looks for a sequence of the form "key=value" in the string 'src' and
// returns the location of the first character of 'value'. No spaces are
// permitted around the "=". The value is copied to 'bufp', up to the first
// whitespace or nul character and bounded by maxlen, and nul-terminated.
// Returns bufp if the key was found, NULL if not.
const char *
malloc_common_value_for_key_copy(const char *src, const char *key,
							   char *bufp, size_t maxlen)
{
	const char *ptr = malloc_common_value_for_key(src, key);
	if (ptr) {
		char *to = bufp;
		while (maxlen > 1) { // Always leave room for a '\0'
			char c = *ptr++;
			if (c == '\0' || c == ' ' || c == '\t' || c == '\n') {
				break;
			}
			*to++ = c;
			maxlen--;
		}
		*to = '\0';	// Always nul-terminate
		return bufp;
	}
	return NULL;
}



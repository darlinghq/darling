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

#pragma mark Utilities

/*
 * Factored out from _get_parse_boot_arg_value for unit testing purposes
 */
static bool
_parse_boot_arg_value(char *argsbuff, const char *which, char *where, size_t max)
{
	bool found = false;

	char *token = NULL;
	char *argsstr = argsbuff;
	static const char seps[] = { ' ', '\t', };
	while ((token = strsep(&argsstr, seps)) != NULL) {
		bool is_boolean = false;

		char *value = NULL;
		char *equals = strchr(token, '=');
		if (token[0] == '-') {
			/*
			 * Arguments whose names begins with "-" are booleans, so don't get
			 * key=value splitting.  Though I'd still discourage you from
			 * naming your option "-edge=case".
			 */
			is_boolean = true;
		} else if (equals) {
			equals[0] = '\0';
			value = &equals[1];
		} else {
			is_boolean = true;
		}

		if (strcmp(which, token) == 0) {
			/*
			 * Found it! Copy out the value as required.
			 */
			found = true;

			if (!where) {
				// Caller just wants to know whether the boot-arg exists.
			} else if (is_boolean || value == NULL) {
				strlcpy(where, "", max);
			} else {
				strlcpy(where, value, max);
			}

			break;
		}
	}

	return found;
}

/*
 * This is (very) loosely based on the implementation of
 * PE_parse_boot_argn() (or at least the parts where I was able to easily
 * decipher the policy).
 */
static bool
_get_boot_arg_value(const char *which, char *where, size_t max)
{
	bool found = false;
	__os_free char *argsbuff = NULL;
	size_t argsbuff_len = 0;
	errno_t error = sysctlbyname_get_data_np("kern.bootargs",
			(void **)&argsbuff, &argsbuff_len);

	if (!error) {
		found = _parse_boot_arg_value(argsbuff, which, where, max);
	}

	return found;
}

#pragma mark API
errno_t
sysctl_get_data_np(int mib[4], size_t mib_cnt, void **buff, size_t *buff_len)
{
	errno_t error = 0;
	int ret = 0;
	size_t needed = 0;
	void *mybuff = NULL;

	// We need to get the length of the parameter so we can allocate a buffer
	// that's large enough.
	ret = sysctl(mib, (unsigned int)mib_cnt, NULL, &needed, NULL, 0);
	if (ret) {
		error = errno;
		goto __out;
	}

	mybuff = malloc(needed);
	if (!mybuff) {
		error = errno;
		goto __out;
	}

	ret = sysctl(mib, (unsigned int)mib_cnt, mybuff, &needed, NULL, 0);
	if (ret) {
		// It's conceivable that some other process came along within this
		// window and modified the variable to be even larger than we'd
		// previously been told, but if that's the case, just give up.
		error = errno;
		goto __out;
	}

	*buff = mybuff;
	*buff_len = needed;

__out:
	if (error) {
		free(mybuff);
	}
	return error;
}

errno_t
sysctlbyname_get_data_np(const char *mibdesc, void **buff, size_t *buff_len)
{
	int ret = -1;
	int error = -1;
	int mib[4];
	size_t mib_cnt = countof(mib);

	ret = sysctlnametomib(mibdesc, mib, &mib_cnt);
	if (ret) {
		error = errno;
		goto __out;
	}

	error = sysctl_get_data_np(mib, mib_cnt, buff, buff_len);

__out:
	return error;
}

bool
os_parse_boot_arg_int(const char *which, int64_t *where)
{
	bool found = false;
	char buff[24] = {0};
	char *endptr = NULL;
	int64_t val = 0;

	found = _get_boot_arg_value(which, buff, sizeof(buff));
	if (!found || !where) {
		goto __out;
	}

	// A base of zero handles bases 8, 10, and 16.
	val = strtoll(buff, &endptr, 0);
	if (*endptr == 0) {
		*where = val;
	} else {
		// The boot-arg value was invalid, so say we didn't find it.
		found = false;
	}

__out:
	return found;
}

bool
os_parse_boot_arg_string(const char *which, char *where, size_t maxlen)
{
	return _get_boot_arg_value(which, where, maxlen);
}

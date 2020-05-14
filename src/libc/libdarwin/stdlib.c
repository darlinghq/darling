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

#pragma mark API
void
os_localtime_file(char buff[static 32])
{
	struct timeval tv;
	struct tm curtime;

	posix_assert_zero(gettimeofday(&tv, NULL));
	(void)localtime_r(&tv.tv_sec, &curtime);

	// Return a time representation that is ideal for use in filenames, so no
	// spaces or things that need to be escaped.
	snprintf(buff, 32, "%d-%02d-%02d_%02d.%02d.%02d.%06d",
			curtime.tm_year + 1900, curtime.tm_mon + 1, curtime.tm_mday,
			curtime.tm_hour, curtime.tm_min, curtime.tm_sec, tv.tv_usec);
}

// MurmurHash2 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
uint64_t
os_simple_hash_with_seed(const void *buff, size_t len, uint64_t seed)
{
#ifdef __LP64__
	// MurmurHash64A
	const uint64_t m = 0xc6a4a7935bd1e995;
	const int r = 47;
	uint64_t h = seed ^ (len * m);
	const uint64_t * data = (const uint64_t *)buff;
	const uint64_t * end = data + (len / 8);
	while(data != end) {
		uint64_t k = *data++;
		k *= m;
		k ^= k >> r;
		k *= m;
		h ^= k;
		h *= m;
	}
	const unsigned char * data2 = (const unsigned char *)data;
	switch(len & 7) {
	case 7: h ^= ((uint64_t)data2[6]) << 48;
	case 6: h ^= ((uint64_t)data2[5]) << 40;
	case 5: h ^= ((uint64_t)data2[4]) << 32;
	case 4: h ^= ((uint64_t)data2[3]) << 24;
	case 3: h ^= ((uint64_t)data2[2]) << 16;
	case 2: h ^= ((uint64_t)data2[1]) << 8;
	case 1: h ^= ((uint64_t)data2[0]);
			h *= m;
	};
	h ^= h >> r;
	h *= m;
	h ^= h >> r;
#else // __LP64__
	// MurmurHash64B
	const uint32_t m = 0x5bd1e995;
	const int r = 24;

	uint32_t h1 = (uint32_t)(seed) ^ len;
	uint32_t h2 = (uint32_t)(seed >> 32);

	const uint32_t * data = (const uint32_t *)buff;

#define MIX(k, h) \
	(k) *= m; (k) ^= (k) >> r; (k) *= m; (h) *= m; (h) ^= (k); len -= 4;

	while(len >= 8) {
		uint32_t k[2];
		memcpy(k, (const char*)data, sizeof(k));
		data += sizeof(k)/sizeof(k[0]);

		MIX(k[0], h1)
		MIX(k[1], h2)
	}

	if(len >= 4) {
		uint32_t k[1];
		memcpy(k, (const char *)data, sizeof(k));
		data += sizeof(k)/sizeof(k[0]);

		MIX(k[0], h1);
	}

#undef MIX

	switch(len) {
	case 3: h2 ^= ((unsigned char*)data)[2] << 16;
	case 2: h2 ^= ((unsigned char*)data)[1] << 8;
	case 1: h2 ^= ((unsigned char*)data)[0];
			h2 *= m;
	};

	h1 ^= h2 >> 18; h1 *= m;
	h2 ^= h1 >> 22; h2 *= m;
	h1 ^= h2 >> 17; h1 *= m;
	h2 ^= h1 >> 19; h2 *= m;

	uint64_t h = h1;

	h = (h << 32) | h2;
#endif // __LP64__
	return h;
}

uint64_t
os_simple_hash(const void *buff, size_t len)
{
	return os_simple_hash_with_seed(buff, len, 0);
}

uint64_t
os_simple_hash_string_with_seed(const char *string, uint64_t seed)
{
	size_t len = strlen(string);
	return os_simple_hash_with_seed(string, len, seed);
}

uint64_t
os_simple_hash_string(const char *string)
{
	return os_simple_hash_string_with_seed(string, 0);
}

errno_t
realpath_np(os_fd_t fd, char buff[static PATH_MAX])
{
	errno_t error = -1;
	int ret = -1;

	ret = fcntl(fd, F_GETPATH, buff);
	if (ret) {
		error = errno;
	} else {
		error = 0;
	}

	return error;
}

errno_t
memdup_np(void **_new, const void *src, size_t len)
{
	void *mynew = NULL;

	mynew = malloc(len);
	if (!mynew) {
		return errno;
	}

	memcpy(mynew, src, len);
	*_new = mynew;
	return 0;
}

void *
memdup2_np(const void *src, size_t len)
{
	void *_new = os_malloc(len);
	memcpy(_new, src, len);
	return _new;
}

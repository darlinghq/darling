/*
 * Copyright (c) 2003,2005,2006,2011,2012 Apple, Inc. All rights reserved.
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

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>

#define MAX_V4_ADDR_LEN 16
#define MAX_V6_ADDR_LEN 64

static const char *hexchars = "0123456789abcdef";

const char * inet_ntop6(const struct in6_addr *addr, char *dst, socklen_t size);
const char * inet_ntop4(const struct in_addr *addr, char *dst, socklen_t size);

const char *
inet_ntop(int af, const void *addr, char *buf, socklen_t len)
{
	if (af == AF_INET6) return inet_ntop6(addr, buf, len);
	if (af == AF_INET) return inet_ntop4(addr, buf, len);

	errno = EAFNOSUPPORT;
	return NULL;
}

const char *
inet_ntop6(const struct in6_addr *addr, char *dst, socklen_t size)
{
	char hexa[8][5], tmp[MAX_V6_ADDR_LEN];
	int zr[8];
	socklen_t len;
	int32_t i, j, k, skip;
	uint8_t x8, hx8;
	uint16_t x16;
	struct in_addr a4;

	if (addr == NULL)
	{
		errno = EAFNOSUPPORT;
		return NULL;
	}

	if (dst == NULL)
	{
		errno = ENOSPC;
		return NULL;
	}

	memset(tmp, 0, MAX_V6_ADDR_LEN);

	/*  check for mapped or compat addresses */
	i = IN6_IS_ADDR_V4MAPPED(addr);
	j = IN6_IS_ADDR_V4COMPAT(addr);
	if ((i != 0) || (j != 0))
	{
		const char *prefix;
		socklen_t prefix_len;
		if (i != 0) {
			prefix = "::ffff:";
			prefix_len = 7;
		} else {
			prefix = "::";
			prefix_len = 2;
		}
		a4.s_addr = addr->__u6_addr.__u6_addr32[3];
		inet_ntop4(&a4, tmp, sizeof(tmp));
		len = strlen(tmp) + 1;
		if (prefix_len + len > size)
		{
			errno = ENOSPC;
			return NULL;
		}

		memcpy(dst, prefix, prefix_len);
		memcpy(dst + prefix_len, tmp, len);
		return dst;
	}

	k = 0;
	for (i = 0; i < 16; i += 2)
	{
		j = 0;
		skip = 1;

		memset(hexa[k], 0, 5);

		x8 = addr->__u6_addr.__u6_addr8[i];

		hx8 = x8 >> 4;
		if (hx8 != 0)
		{
			skip = 0;
			hexa[k][j++] = hexchars[hx8];
		}

		hx8 = x8 & 0x0f;
		if ((skip == 0) || ((skip == 1) && (hx8 != 0)))
		{
			skip = 0;
			hexa[k][j++] = hexchars[hx8];
		}

		x8 = addr->__u6_addr.__u6_addr8[i + 1];

		hx8 = x8 >> 4;
		if ((skip == 0) || ((skip == 1) && (hx8 != 0)))
		{
			hexa[k][j++] = hexchars[hx8];
		}

		hx8 = x8 & 0x0f;
		hexa[k][j++] = hexchars[hx8];

		k++;
	}

	/* find runs of zeros for :: convention */
	j = 0;
	for (i = 7; i >= 0; i--)
	{
		zr[i] = j;
		x16 = addr->__u6_addr.__u6_addr16[i];
		if (x16 == 0) j++;
		else j = 0;
		zr[i] = j;
	}

	/* find longest run of zeros */
	k = -1;
	j = 0;
	for(i = 0; i < 8; i++)
	{
		if (zr[i] > j)
		{
			k = i;
			j = zr[i];
		}
	}

	for(i = 0; i < 8; i++)
	{
		if (i != k) zr[i] = 0;
	}

	len = 0;
	for (i = 0; i < 8; i++)
	{
		if (zr[i] != 0)
		{
			/* check for leading zero */
			if (i == 0) tmp[len++] = ':';
			tmp[len++] = ':';
			i += (zr[i] - 1);
			continue;
		}
		for (j = 0; hexa[i][j] != '\0'; j++) tmp[len++] = hexa[i][j];
		if (i != 7) tmp[len++] = ':';
	}

	/* trailing NULL */
	len++;

	if (len > size)
	{
		errno = ENOSPC;
		return NULL;
	}

	memcpy(dst, tmp, len);
	return dst;
}

const char *
inet_ntop4(const struct in_addr *addr, char *dst, socklen_t size)
{
	char tmp[MAX_V4_ADDR_LEN], *p;
	const u_int8_t *ap = (u_int8_t *)&addr->s_addr;
	int i, ql, len;

	if (addr == NULL)
	{
		errno = EAFNOSUPPORT;
		return NULL;
	}

	if (dst == NULL)
	{
		errno = ENOSPC;
		return NULL;
	}

	memset(tmp, 0, MAX_V4_ADDR_LEN);

	/* 3 dots, trailing nul */
	len = 4;

	p = tmp;

	for (i = 0; i < 4; i++, ap++)
	{
		snprintf(p, 4, "%d", *ap);
		ql = strlen(p);
		len += ql;
		p += ql;
		if (i < 3) *p++ = '.';
	}

	if (len > size)
	{
		errno = ENOSPC;
		return NULL;
	}

	memcpy(dst, tmp, len);
	return dst;
}

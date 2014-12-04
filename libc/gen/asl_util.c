/*
 * Copyright (c) 2006-2011 Apple Inc.  All rights reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License."
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * These routines needs to be separate from asl.c, so that dyld can build
 * and suck in these without the rest of asl.
 */

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

static uint8_t *b64charset = (uint8_t *)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

__private_extern__ const char *
_asl_escape(unsigned char c)
{
	switch(c)
	{
		case '\\':
			return "\\\\";
		case '[':
			return "\\[";
		case ']':
			return "\\]";
		case '\n':
			return "\\n";
	}
	return NULL;
}

static int
asl_is_utf8_char(const unsigned char *p, int *state, int *ctype)
{
	switch (*state)
	{
		case 0:
		{
			*ctype = 0;

			if (*p >= 0x80)
			{
				*state = 1;
				if ((*p >= 0xc2) && (*p <= 0xdf)) *ctype = 1;
				else if (*p == 0xe0) *ctype = 2;
				else if ((*p >= 0xe1) && (*p <= 0xef)) *ctype = 3;
				else if (*p == 0xf0) *ctype = 4;
				else if ((*p >= 0xf1) && (*p <= 0xf3)) *ctype = 5;
				else if (*p == 0xf4) *ctype = 6;
				else return 0;
			}

			break;
		}

		case 1:
		{
			switch (*ctype)
			{
				case 1:
				{
					if ((*p >= 0x80) && (*p <= 0xbf)) *state = 0;
					else return 0;
					break;
				}

				case 2:
				{
					if ((*p >= 0xa0) && (*p <= 0xbf)) *state = 2;
					else return 0;
					break;
				}

				case 3:
				{
					if ((*p >= 0x80) && (*p <= 0xbf)) *state = 2;
					else return 0;
					break;
				}

				case 4:
				{
					if ((*p >= 0x90) && (*p <= 0xbf)) *state = 2;
					else return 0;
					break;
				}

				case 5:
				{
					if ((*p >= 0x80) && (*p <= 0xbf)) *state = 2;
					else return 0;
					break;
				}

				case 6:
				{
					if ((*p >= 0x80) && (*p <= 0x8f)) *state = 2;
					else return 0;
					break;
				}

				default: return 0;
			}

			break;
		}

		case 2:
		{
			if ((*ctype >= 2) && (*ctype <= 3))
			{
				if ((*p >= 0x80) && (*p <= 0xbf)) *state = 0;
				else return 0;
			}
			else if ((*ctype >= 4) && (*ctype <= 6))
			{
				if ((*p >= 0x80) && (*p <= 0xbf)) *state = 3;
				else return 0;
			}
			else
			{
				return 0;
			}

			break;
		}

		case 3:
		{
			if ((*ctype >= 4) && (*ctype <= 6))
			{
				if ((*p >= 0x80) && (*p <= 0xbf)) *state = 0;
				else return 0;
			}
			else
			{
				return 0;
			}

			break;
		}

		default: return 0;
	}

	return 1;
}

__private_extern__ int
asl_is_utf8(const char *str)
{
	const unsigned char *p;
	int flag = 1;
	int state = 0;
	int ctype = 0;

	if (str == NULL) return flag;

	for (p = (const unsigned char *)str; (*p != '\0') && (flag == 1); p++)
	{
		flag = asl_is_utf8_char(p, &state, &ctype);
	}

	return flag;
}

__private_extern__ uint8_t *
asl_b64_encode(const uint8_t *buf, size_t len)
{
	uint8_t *out;
	uint8_t b;
	size_t i0, i1, i2, j, outlen;
	
	if (buf == NULL) return NULL;
	if (len == 0) return NULL;
	
	outlen = ((len + 2) / 3) * 4;
	out = (uint8_t *)malloc(outlen + 1);
	if (out == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}
	
	out[outlen] = 0;
	
	i0 = 0;
	i1 = 1;
	i2 = 2;
	j = 0;
	
	while (i2 < len)
	{
		b = buf[i0] >> 2;
		out[j++] = b64charset[b];
		
		b = ((buf[i0] & 0x03) << 4) | (buf[i1] >> 4);
		out[j++] = b64charset[b];
		
		b = ((buf[i1] & 0x0f) << 2) | ((buf[i2] & 0xc0) >> 6);
		out[j++] = b64charset[b];
		
		b = buf[i2] & 0x3f;
		out[j++] = b64charset[b];
		
		i0 += 3;
		i1 = i0 + 1;
		i2 = i1 + 1;
	}
	
	if (i0 < len)
	{
		b = buf[i0] >> 2;
		out[j++] = b64charset[b];
		
		b = (buf[i0] & 0x03) << 4;
		
		if (i1 < len) b |= (buf[i1] >> 4);
		out[j++] = b64charset[b];
		
		if (i1 >= len)
		{
			out[j++] = '=';
			out[j++] = '=';
			return out;
		}
		
		b = (buf[i1] & 0x0f) << 2;
		out[j++] = b64charset[b];
		out[j++] = '=';
	}
	
	return out;
}

/*
 * Copyright (c) 1999-2010 Apple Inc. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach/mach.h>
#include <servers/bootstrap.h>
#include <pthread.h>
#include <errno.h>
#include <notify.h>
#include <syslog.h>
#include <unistd.h>
#ifdef DEBUG
#include <asl.h>
#endif
#include <ils.h>

#define ILS_MAGIC_SIZE 8
#define ILS_MAGIC "ILSMAGIC"

/* GLOBAL */

static const uint32_t align_32[] = { 0, 1, 2, 0, 4, 0, 0, 0, 4 };
static const uint32_t align_64[] = { 0, 1, 2, 0, 4, 0, 0, 0, 8 };

static uint32_t
padsize(size_t curr, size_t item, const uint32_t *align)
{
	uint32_t na, diff;

	if (item > 8) item = 8;

	na = align[item];
	if (na == 0) return 0;

	diff = curr % na;
	if (diff == 0) return 0;

	return na - diff;
}

/*
 * Create a structure using in-line memory (i.e. all one blob).
 * This reduces malloc/free workload.
 *
 * Structure components may be strings, 1, 2, 4, or 8-byte values,  
 * lists of strings, or lists of 4, 8, or 16-byte values.
 *
 * Format keys:
 *	s	NUL terminated string
 *	1	1 byte value
 *	2	2 byte value
 *	4	4 byte value
 *	8	8 byte value
 *	S	128 byte value (_SS_MAXSIZE for a sockaddr)
 *	L	long (32 or 64 bits, depending on architecture)
 *	m	mach_port_t
 *	*	NULL-terminated list of strings
 *	a	NULL-terminated list of 4-byte values
 *	b	NULL-terminated list of 8-byte values
 *	c	NULL-terminated list of 16-byte values
 *	@	length (4 bytes) and buffer (requires two parameters)
 *
 */
void *
LI_ils_create(char *fmt, ...)
{
	va_list ap;
	char *arg, *f;
	char **list;
	void *hp, *dp, *lp, *ils;
	uint8_t u8;
	uint16_t u16;
	uint32_t u32, i, pad;
	uint64_t u64;
	unsigned long l;
	mach_port_t m;
	socket_data_t sdata;
	size_t memsize, hsize, csize, csizep1, csizep2, slen, largest;
	const uint32_t *align;

	if (fmt == NULL) return NULL;

	largest = 0;
	align = align_32;
	if (sizeof(char *) == 8) align = align_64;

	/* first pass: calculate size */
	memsize = ILS_MAGIC_SIZE;
	hsize = 0;

	va_start(ap, fmt);

	for (f = fmt; (*f) != '\0'; f++)
	{
		csize = 0;
		slen = 0;

		switch (*f)
		{
			case 's':
			{
				if (largest < sizeof(char *)) largest = sizeof(char *);

				csize = sizeof(char *) + padsize(hsize, sizeof(char *), align);
				arg = va_arg(ap, char *);
				if (arg != NULL) slen = strlen(arg) + 1;

				break;
			}

			case '1':
			{
				if (largest < 1) largest = 1;

				csize = 1;
				u8 = va_arg(ap, int);

				break;
			}

			case '2':
			{
				if (largest < 2) largest = 2;

				csize = 2 + padsize(hsize, 2, align);
				u16 = va_arg(ap, int);

				break;
			}

			case '4':
			{
				if (largest < 4) largest = 4;

				csize = 4 + padsize(hsize, 4, align);
				u32 = va_arg(ap, uint32_t);

				break;
			}

			case '8':
			{
				if (largest < 8) largest = 8;

				csize = 8 + padsize(hsize, 8, align);
				u64 = va_arg(ap, uint64_t);

				break;
			}

			case 'S':
			{
				if (largest < 128) largest = 128;

				/* 4-byte-align since a socket_data_t is really just a buffer */
				csize = 128 + padsize(hsize, 4, align);
				sdata = va_arg(ap, socket_data_t);

				break;
			}

			case 'L':
			{
				if (largest < sizeof(unsigned long)) largest = sizeof(unsigned long);

				csize = sizeof(unsigned long) + padsize(hsize, sizeof(unsigned long), align);
				l = va_arg(ap, unsigned long);

				break;
			}

			case 'm':
			{
				if (largest < sizeof(mach_port_t)) largest = sizeof(mach_port_t);

				csize = sizeof(mach_port_t) + padsize(hsize, sizeof(mach_port_t), align);
				m = va_arg(ap, mach_port_t);

				break;
			}

			case '*':
			{
				// NULL-terminated list of strings
				if (largest < sizeof(char *)) largest = sizeof(char *);

				csize = sizeof(char *) + padsize(hsize, sizeof(char *), align);
				list = va_arg(ap, char **);
				if (list != NULL)
				{
					for (i = 0; list[i] != NULL; i++)
					{
						slen += sizeof(char *);
						slen += (strlen(list[i]) + 1);
					}
				}

				// reserve space for the terminator
				slen += sizeof(char *);

				break;
			}

			case 'a':
			{
				/* NULL-terminated list of 4-byte values */
				if (largest < sizeof(char *)) largest = sizeof(char *);

				csize = sizeof(char *) + padsize(hsize, sizeof(char *), align);
				list = va_arg(ap, char **);
				if (list != NULL)
				{
					for (i = 0; list[i] != NULL; i++)
					{
						slen += sizeof(char *);
						slen += 4;
					}

					slen += sizeof(char *);
				}

				break;
			}

			case 'b':
			{
				/* NULL-terminated list of 8-byte values */
				if (largest < sizeof(char *)) largest = sizeof(char *);

				csize = sizeof(char *) + padsize(hsize, sizeof(char *), align);
				list = va_arg(ap, char **);
				if (list != NULL)
				{
					for (i = 0; list[i] != NULL; i++)
					{
						slen += sizeof(char *);
						slen += 8;
					}

					slen += sizeof(char *);
				}

				break;
			}

			case 'c':
			{
				/* NULL-terminated list of 16-byte values */
				if (largest < sizeof(char *)) largest = sizeof(char *);

				csize = sizeof(char *) + padsize(hsize, sizeof(char *), align);
				list = va_arg(ap, char **);
				if (list != NULL)
				{
					for (i = 0; list[i] != NULL; i++)
					{
						slen += sizeof(char *);
						slen += 16;
					}

					slen += sizeof(char *);
				}

				break;
			}

			case '@':
			{
				if (largest < 4) largest = 4;
				csizep1 = 4 + padsize(hsize, 4, align);
				slen = va_arg(ap, uint32_t);

				if (largest < sizeof(char *)) largest = sizeof(char *);
				csizep2 = sizeof(char *) + padsize(hsize + csizep1, sizeof(char *), align);
				arg = va_arg(ap, char *);

				csize = csizep1 + csizep2;

				break;
			}

			default: return NULL;
		}

		memsize += csize;
		memsize += slen;
		hsize += csize;
	}

	va_end(ap);

	pad = padsize(hsize, largest, align);
	memsize += pad;
	hsize += pad;

	ils = malloc(memsize);
	if (ils == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	/* insert magic cookie */
	dp = ils + hsize;
	memcpy(dp, ILS_MAGIC, ILS_MAGIC_SIZE);
	dp += ILS_MAGIC_SIZE;

	hp = ils;
	hsize = 0;

	/* second pass: copy data */
	va_start(ap, fmt);
	for (f = fmt; (*f) != '\0'; f++)
	{
		switch (*f)
		{
			case 's':
			{
				pad = padsize(hsize, sizeof(char *), align);
				if (pad != 0)
				{
					memset(hp, 0, pad);
					hp += pad;
					hsize += pad;
				}

				arg = va_arg(ap, char *);
				if (arg == NULL)
				{
					memset(hp, 0, sizeof(char *));
				}
				else
				{
					memcpy(hp, &dp, sizeof(char *));
					slen = strlen(arg) + 1;
					memcpy(dp, arg, slen);
					dp += slen;
				}

				hp += sizeof(char *);
				hsize += sizeof(char *);

				break;
			}

			case '1':
			{
				u8 = va_arg(ap, int);
				memcpy(hp, &u8, sizeof(uint8_t));
				hp += sizeof(uint8_t);

				break;
			}

			case '2':
			{
				pad = padsize(hsize, 2, align);
				if (pad != 0)
				{
					memset(hp, 0, pad);
					hp += pad;
					hsize += pad;
				}

				u16 = va_arg(ap, int);
				memcpy(hp, &u16, sizeof(uint16_t));

				hp += sizeof(uint16_t);
				hsize += sizeof(uint16_t);

				break;
			}

			case '4':
			{
				pad = padsize(hsize, 4, align);
				if (pad != 0)
				{
					memset(hp, 0, pad);
					hp += pad;
					hsize += pad;
				}

				u32 = va_arg(ap, uint32_t);
				memcpy(hp, &u32, sizeof(uint32_t));

				hp += sizeof(uint32_t);
				hsize += sizeof(uint32_t);

				break;
			}

			case '8':
			{
				pad = padsize(hsize, 8, align);
				if (pad != 0)
				{
					memset(hp, 0, pad);
					hp += pad;
					hsize += pad;
				}

				u64 = va_arg(ap, uint64_t);
				memcpy(hp, &u64, sizeof(uint64_t));

				hp += sizeof(uint64_t);
				hsize += sizeof(uint64_t);

				break;
			}

			case 'S':
			{
				pad = padsize(hsize, 4, align);
				if (pad != 0)
				{
					memset(hp, 0, pad);
					hp += pad;
					hsize += pad;
				}

				sdata = va_arg(ap, socket_data_t);
				memcpy(hp, (char *)(sdata.x), sizeof(socket_data_t));

				hp += sizeof(socket_data_t);
				hsize += sizeof(socket_data_t);

				break;
			}

			case 'L':
			{
				pad = padsize(hsize, sizeof(unsigned long), align);
				if (pad != 0)
				{
					memset(hp, 0, pad);
					hp += pad;
					hsize += pad;
				}

				l = va_arg(ap, unsigned long);
				memcpy(hp, &l, sizeof(unsigned long));

				hp += sizeof(unsigned long);
				hsize += sizeof(unsigned long);

				break;
			}

			case 'm':
			{
				pad = padsize(hsize, sizeof(mach_port_t), align);
				if (pad != 0)
				{
					memset(hp, 0, pad);
					hp += pad;
					hsize += pad;
				}

				m = va_arg(ap, mach_port_t);
				memcpy(hp, &m, sizeof(mach_port_t));

				hp += sizeof(mach_port_t);
				hsize += sizeof(mach_port_t);

				break;
			}

			case '*':
			{
				pad = padsize(hsize, sizeof(char *), align);
				if (pad != 0)
				{
					memset(hp, 0, pad);
					hp += pad;
					hsize += pad;
				}

				list = va_arg(ap, char **);

				memcpy(hp, &dp, sizeof(char *));

				for (i = 0; list && list[i] != NULL; i++);

				lp = dp;
				dp += ((i + 1) * sizeof(char *));

				for (i = 0; list && list[i] != NULL; i++)
				{
					memcpy(lp, &dp, sizeof(char *));
					lp += sizeof(char *);
					slen = strlen(list[i]) + 1;
					memcpy(dp, list[i], slen);
					dp += slen;
				}

				memset(lp, 0, sizeof(char *));

				hp += sizeof(char *);
				hsize += sizeof(char *);

				break;
			}

			case 'a':
			{
				pad = padsize(hsize, sizeof(char *), align);
				if (pad != 0)
				{
					memset(hp, 0, pad);
					hp += pad;
					hsize += pad;
				}

				list = va_arg(ap, char **);

				if (list == NULL)
				{
					memset(hp, 0, sizeof(char *));
				}
				else
				{
					memcpy(hp, &dp, sizeof(char *));

					for (i = 0; list[i] != NULL; i++);

					lp = dp;
					dp += ((i + 1) * sizeof(char *));

					for (i = 0; list[i] != NULL; i++)
					{
						memcpy(lp, &dp, sizeof(char *));
						lp += sizeof(char *);
						slen = 4;
						memcpy(dp, list[i], slen);
						dp += slen;
					}

					memset(lp, 0, sizeof(char *));
				}

				hp += sizeof(char *);
				hsize += sizeof(char *);

				break;
			}

			case 'b':
			{
				pad = padsize(hsize, sizeof(char *), align);
				if (pad != 0)
				{
					memset(hp, 0, pad);
					hp += pad;
					hsize += pad;
				}

				list = va_arg(ap, char **);

				if (list == NULL)
				{
					memset(hp, 0, sizeof(char *));
				}
				else
				{
					memcpy(hp, &dp, sizeof(char *));

					for (i = 0; list[i] != NULL; i++);

					lp = dp;
					dp += ((i + 1) * sizeof(char *));

					for (i = 0; list[i] != NULL; i++)
					{
						memcpy(lp, &dp, sizeof(char *));
						lp += sizeof(char *);
						slen = 8;
						memcpy(dp, list[i], slen);
						dp += slen;
					}

					memset(lp, 0, sizeof(char *));
				}

				hp += sizeof(char *);
				hsize += sizeof(char *);

				break;
			}

			case 'c':
			{
				pad = padsize(hsize, sizeof(char *), align);
				if (pad != 0)
				{
					memset(hp, 0, pad);
					hp += pad;
					hsize += pad;
				}

				list = va_arg(ap, char **);

				if (list == NULL)
				{
					memset(hp, 0, sizeof(char *));
				}
				else
				{
					memcpy(hp, &dp, sizeof(char *));

					for (i = 0; list[i] != NULL; i++);

					lp = dp;
					dp += ((i + 1) * sizeof(char *));

					for (i = 0; list[i] != NULL; i++)
					{
						memcpy(lp, &dp, sizeof(char *));
						lp += sizeof(char *);
						slen = 16;
						memcpy(dp, list[i], slen);
						dp += slen;
					}

					memset(lp, 0, sizeof(char *));
				}

				hp += sizeof(char *);
				hsize += sizeof(char *);

				break;
			}

			case '@':
			{
				pad = padsize(hsize, 4, align);
				if (pad != 0)
				{
					memset(hp, 0, pad);
					hp += pad;
					hsize += pad;
				}

				slen = va_arg(ap, uint32_t);
				memcpy(hp, &slen, sizeof(uint32_t));

				hp += sizeof(uint32_t);
				hsize += sizeof(uint32_t);

				pad = padsize(hsize, sizeof(char *), align);
				if (pad != 0)
				{
					memset(hp, 0, pad);
					hp += pad;
					hsize += pad;
				}

				arg = va_arg(ap, char *);
				if (arg == NULL)
				{
					memset(hp, 0, sizeof(char *));
				}
				else
				{
					memcpy(hp, &dp, sizeof(char *));
					memcpy(dp, arg, slen);
					dp += slen;
				}

				hp += sizeof(char *);
				hsize += sizeof(char *);
			}
		}
	}

	va_end(ap);

	pad = padsize(hsize, largest, align);
	if (pad > 0) memset(hp, 0, pad);

	return ils;
}

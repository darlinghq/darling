/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Posix rand_r function added May 1999 by Wes Peters <wes@softweyr.com>.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)rand.c	8.1 (Berkeley) 6/14/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdlib/rand.c,v 1.17 2007/12/11 20:39:32 ache Exp $");

#include "namespace.h"
#include <sys/time.h>          /* for sranddev() */
#include <sys/types.h>
#include <fcntl.h>             /* for sranddev() */
#include <stdlib.h>
#include <unistd.h>            /* for sranddev() */
#include "un-namespace.h"

#ifdef TEST
#include <stdio.h>
#endif /* TEST */

static int
do_rand(unsigned long *ctx)
{
#ifdef  USE_WEAK_SEEDING
/*
 * Historic implementation compatibility.
 * The random sequences do not vary much with the seed,
 * even with overflowing.
 */
	return ((*ctx = *ctx * 1103515245 + 12345) % ((u_long)RAND_MAX + 1));
#else   /* !USE_WEAK_SEEDING */
/*
 * Compute x = (7^5 * x) mod (2^31 - 1)
 * without overflowing 31 bits:
 *      (2^31 - 1) = 127773 * (7^5) + 2836
 * From "Random number generators: good ones are hard to find",
 * Park and Miller, Communications of the ACM, vol. 31, no. 10,
 * October 1988, p. 1195.
 */
	long hi, lo, x;

	/* Can't be initialized with 0, so use another value. */
	if (*ctx == 0)
		*ctx = 123459876;
	hi = *ctx / 127773;
	lo = *ctx % 127773;
	x = 16807 * lo - 2836 * hi;
	if (x < 0)
		x += 0x7fffffff;
	return ((*ctx = x) % ((u_long)RAND_MAX + 1));
#endif  /* !USE_WEAK_SEEDING */
}


int
rand_r(unsigned int *ctx)
{
	u_long val = (u_long) *ctx;
	int r = do_rand(&val);

	*ctx = (unsigned int) val;
	return (r);
}


static u_long next = 1;

int
rand()
{
	return (do_rand(&next));
}

void
srand(seed)
u_int seed;
{
	next = seed;
}


/*
 * sranddev:
 *
 * Many programs choose the seed value in a totally predictable manner.
 * This often causes problems.  We seed the generator using the much more
 * secure random(4) interface.
 */
void
sranddev()
{
	int fd, done;

	done = 0;
	fd = _open("/dev/random", O_RDONLY, 0);
	if (fd >= 0) {
		if (_read(fd, (void *) &next, sizeof(next)) == sizeof(next))
			done = 1;
		_close(fd);
	}

	if (!done) {
		struct timeval tv;
		unsigned long junk;

		gettimeofday(&tv, NULL);
		srand((getpid() << 16) ^ tv.tv_sec ^ tv.tv_usec ^ junk);
	}
}


#ifdef TEST

main()
{
    int i;
    unsigned myseed;

    printf("seeding rand with 0x19610910: \n");
    srand(0x19610910);

    printf("generating three pseudo-random numbers:\n");
    for (i = 0; i < 3; i++)
    {
	printf("next random number = %d\n", rand());
    }

    printf("generating the same sequence with rand_r:\n");
    myseed = 0x19610910;
    for (i = 0; i < 3; i++)
    {
	printf("next random number = %d\n", rand_r(&myseed));
    }

    return 0;
}

#endif /* TEST */


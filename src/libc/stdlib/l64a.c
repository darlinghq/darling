/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char *rcsid = "$OpenBSD: l64a.c,v 1.3 1997/08/17 22:58:34 millert Exp $";
#endif /* LIBC_SCCS and not lint */

#include <errno.h>
#include <stdlib.h>

char *
#ifdef __LP64__
l64a(v)
	long v;
#else /* !__LP64__ */
l64a(value)
	long value;
#endif /* __LP64__ */
{
	static char buf[8];
#ifdef __LP64__
	int value = v;
#endif /* __LP64__ */
	char *s = buf;
	int digit;
	int i;

#ifndef __LP64__
	if (value < 0) {
		errno = EINVAL;
		return(NULL);
	}
#endif /* __LP64__ */

	for (i = 0; value != 0 && i < 6; i++) {
		digit = value & 0x3f;

		if (digit < 2) 
			*s = digit + '.';
		else if (digit < 12)
			*s = digit + '0' - 2;
		else if (digit < 38)
			*s = digit + 'A' - 12;
		else
			*s = digit + 'a' - 38;

		value >>= 6;
		s++;
	}

	*s = '\0';

	return(buf);
}


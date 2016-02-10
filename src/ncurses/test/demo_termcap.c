/****************************************************************************
 * Copyright (c) 2005-2007,2008 Free Software Foundation, Inc.              *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice shall be included  *
 * in all copies or substantial portions of the Software.                   *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/*
 * Author: Thomas E. Dickey
 *
 * $Id: demo_termcap.c,v 1.7 2008/02/09 18:08:36 tom Exp $
 *
 * A simple demo of the termcap interface.
 */
#define USE_TINFO
#include <test.priv.h>

#if HAVE_TGETENT

#define isCapName(c) (isgraph(c) && strchr("^#=:\\", c) == 0)

static void
dumpit(char *cap)
{
    /*
     * One of the limitations of the termcap interface is that the library
     * cannot determine the size of the buffer passed via tgetstr(), nor the
     * amount of space remaining.  This demo simply reuses the whole buffer
     * for each call; a normal termcap application would try to use the buffer
     * to hold all of the strings extracted from the terminal entry.
     */
    char area[1024], *ap = area;
    char *str;
    int num;

    if ((str = tgetstr(cap, &ap)) != 0) {
	/*
	 * Note that the strings returned are mostly terminfo format, since
	 * ncurses does not convert except for a handful of special cases.
	 */
	printf("str %s = ", cap);
	while (*str != 0) {
	    int ch = UChar(*str++);
	    switch (ch) {
	    case '\177':
		fputs("^?", stdout);
		break;
	    case '\033':
		fputs("\\E", stdout);
		break;
	    case '\b':
		fputs("\\b", stdout);
		break;
	    case '\f':
		fputs("\\f", stdout);
		break;
	    case '\n':
		fputs("\\n", stdout);
		break;
	    case '\r':
		fputs("\\r", stdout);
		break;
	    case ' ':
		fputs("\\s", stdout);
		break;
	    case '\t':
		fputs("\\t", stdout);
		break;
	    case '^':
		fputs("\\^", stdout);
		break;
	    case ':':
		fputs("\\072", stdout);
		break;
	    case '\\':
		fputs("\\\\", stdout);
		break;
	    default:
		if (isgraph(ch))
		    fputc(ch, stdout);
		else if (ch < 32)
		    printf("^%c", ch + '@');
		else
		    printf("\\%03o", ch);
		break;
	    }
	}
	printf("\n");
    } else if ((num = tgetnum(cap)) >= 0) {
	printf("num %s = %d\n", cap, num);
    } else if ((num = tgetflag(cap)) > 0) {
	printf("flg %s\n", cap);
    }
    fflush(stdout);
}

static void
demo_termcap(char *name)
{
    char buffer[1024];

    printf("Terminal type %s\n", name);
    if (tgetent(buffer, name) >= 0) {
	char cap[3];
	int c1, c2;

	cap[2] = 0;
	for (c1 = 0; c1 < 256; ++c1) {
	    cap[0] = c1;
	    if (isCapName(c1)) {
		for (c2 = 0; c2 < 256; ++c2) {
		    cap[1] = c2;
		    if (isCapName(c2)) {
			dumpit(cap);
		    }
		}
	    }
	}
    }
}

int
main(int argc, char *argv[])
{
    int n;
    char *name;

    if (argc > 1) {
	for (n = 1; n < argc; ++n) {
	    demo_termcap(argv[n]);
	}
    } else if ((name = getenv("TERM")) != 0) {
	demo_termcap(name);
    } else {
	static char dumb[] = "dumb";
	demo_termcap(dumb);
    }

    ExitProgram(EXIT_SUCCESS);
}

#else
int
main(int argc GCC_UNUSED,
     char *argv[]GCC_UNUSED)
{
    printf("This program requires termcap\n");
    exit(EXIT_FAILURE);
}
#endif

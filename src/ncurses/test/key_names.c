/****************************************************************************
 * Copyright (c) 2007,2008 Free Software Foundation, Inc.                   *
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
 * $Id: key_names.c,v 1.4 2008/10/11 20:22:37 tom Exp $
 */

#include <test.priv.h>

#if USE_WIDEC_SUPPORT

static void
usage(void)
{
    fprintf(stderr, "Usage: key_names [-m] [-s]\n");
    ExitProgram(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int n;

    bool do_setup = FALSE;
    bool do_meta = FALSE;

    setlocale(LC_ALL, "");

    while ((n = getopt(argc, argv, "ms")) != -1) {
	switch (n) {
	case 'm':
	    do_meta = TRUE;
	    break;
	case 's':
	    do_setup = TRUE;
	    break;
	default:
	    usage();
	    /* NOTREACHED */
	}
    }

    if (do_setup) {
	/*
	 * Get the terminfo entry into memory, and tell ncurses that we want to
	 * use function keys.  That will make it add any user-defined keys that
	 * appear in the terminfo.
	 */
	newterm(getenv("TERM"), stderr, stdin);
	keypad(stdscr, TRUE);
	if (do_meta)
	    meta(stdscr, TRUE);
	endwin();
    }
    for (n = -1; n < KEY_MAX + 512; n++) {
	const char *result = key_name(n);
	if (result != 0)
	    printf("%d(%5o):%s\n", n, n, result);
    }
    ExitProgram(EXIT_SUCCESS);
}
#else
int
main(void)
{
    printf("This program requires the wide-ncurses library\n");
    ExitProgram(EXIT_FAILURE);
}
#endif

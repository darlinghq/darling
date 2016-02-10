/****************************************************************************
 * Copyright (c) 1998-2002,2006 Free Software Foundation, Inc.              *
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
/* gleaned from a web-search, shows a bug combining scanw and implicit scroll.
 * Date:  1997/03/17
 * From:  bayern@morpheus.cis.yale.edu
 *
 * $Id: testscanw.c,v 1.11 2006/04/01 19:08:03 tom Exp $
 */
#include <test.priv.h>

int
main(int argc, char *argv[])
{
    long badanswer = 1;
    long *response = &badanswer;

    setlocale(LC_ALL, "");

    initscr();
    scrollok(stdscr, TRUE);
    idlok(stdscr, TRUE);
    echo();

#if 0
    trace(TRACE_UPDATE | TRACE_CALLS);
#endif
    while (argc > 1) {
	if (isdigit(UChar(*argv[1])))
	    move(atoi(argv[1]), 0);
	else if (!strcmp(argv[1], "-k"))
	    keypad(stdscr, TRUE);
	argc--, argv++;
    }

    while (badanswer) {
	printw("Enter a number (0 to quit):\n");
	printw("--> ");
	scanw("%20ld", response);	/* yes, it's a pointer */
    }
    endwin();
    ExitProgram(EXIT_SUCCESS);
}

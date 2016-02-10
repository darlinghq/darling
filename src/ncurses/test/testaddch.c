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
/*
 * This is an example written by Alexander V. Lukyanov <lav@yars.free.net>,
 * to demonstrate an inconsistency between ncurses and SVr4 curses.
 *
 * $Id: testaddch.c,v 1.6 2006/04/01 19:08:03 tom Exp $
 */
#include <test.priv.h>

static void
attr_addstr(const char *s, chtype a)
{
    while (*s)
	addch(((unsigned char) (*s++)) | a);
}

int
main(
	int argc GCC_UNUSED,
	char *argv[]GCC_UNUSED)
{
    unsigned i;
    chtype back, set, attr;

    setlocale(LC_ALL, "");

    initscr();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_WHITE, COLOR_RED);
    init_pair(3, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(4, COLOR_BLACK, COLOR_GREEN);
    init_pair(5, COLOR_BLACK, COLOR_CYAN);
    init_pair(6, COLOR_BLACK, COLOR_YELLOW);
    init_pair(7, COLOR_BLACK, COLOR_WHITE);

    for (i = 0; i < 8; i++) {
	back = (i & 1) ? A_BOLD | 'B' : ' ';
	set = (i & 2) ? A_REVERSE : 0;
	attr = (i & 4) ? COLOR_PAIR(4) : 0;

	bkgdset(back);
	attrset(set);

	attr_addstr("Test string with spaces ->   <-\n", attr);
    }
    addch('\n');
    for (i = 0; i < 8; i++) {
	back = (i & 1) ? A_BOLD | 'B' | COLOR_PAIR(1) : ' ';
	set = (i & 2) ? A_REVERSE | COLOR_PAIR(2) : 0;
	attr = (i & 4) ? COLOR_PAIR(4) : 0;

	bkgdset(back);
	attrset(set);

	attr_addstr("Test string with spaces ->   <-\n", attr);
    }

    getch();
    endwin();
    ExitProgram(EXIT_SUCCESS);
}

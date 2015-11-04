/****************************************************************************
 * Copyright (c) 1998-2005,2006 Free Software Foundation, Inc.              *
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
 * This test was written by Alexander V. Lukyanov to demonstrate difference
 * between ncurses 4.1 and SVR4 curses
 *
 * $Id: firstlast.c,v 1.5 2006/04/01 19:03:18 tom Exp $
 */

#include <test.priv.h>

static void
fill(WINDOW *w, const char *str)
{
    const char *s;
    for (;;) {
	for (s = str; *s; s++) {
	    if (waddch(w, UChar(*s)) == ERR) {
		wmove(w, 0, 0);
		return;
	    }
	}
    }
}

int
main(int argc GCC_UNUSED,
     char *argv[]GCC_UNUSED)
{
    WINDOW *large, *small;
    initscr();
    noecho();

    large = newwin(20, 60, 2, 10);
    small = newwin(10, 30, 7, 25);

    /* test 1 - addch */
    fill(large, "LargeWindow");

    refresh();
    wrefresh(large);
    wrefresh(small);

    mvwaddstr(small, 5, 5, "   Test <place to change> String   ");
    wrefresh(small);
    getch();

    touchwin(large);
    wrefresh(large);

    mvwaddstr(small, 5, 5, "   Test <***************> String   ");
    wrefresh(small);

    /* DIFFERENCE! */
    getch();

    /* test 2: erase */
    erase();
    refresh();
    getch();

    /* test 3: clrtoeol */
    werase(small);
    wrefresh(small);
    touchwin(large);
    wrefresh(large);
    wmove(small, 5, 0);
    waddstr(small, " clrtoeol>");
    wclrtoeol(small);
    wrefresh(small);

    /* DIFFERENCE! */ ;
    getch();

    /* test 4: clrtobot */
    werase(small);
    wrefresh(small);
    touchwin(large);
    wrefresh(large);
    wmove(small, 5, 3);
    waddstr(small, " clrtobot>");
    wclrtobot(small);
    wrefresh(small);

    /* DIFFERENCE! */
    getch();

    endwin();

    ExitProgram(EXIT_SUCCESS);
}

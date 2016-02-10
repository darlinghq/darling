/****************************************************************************
 * Copyright (c) 2003-2006,2008 Free Software Foundation, Inc.              *
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
 * $Id: color_set.c,v 1.6 2008/02/10 00:18:01 tom Exp $
 */

#include <test.priv.h>

#if HAVE_COLOR_SET

#define SHOW(n) ((n) == ERR ? "ERR" : "OK")

int
main(int argc GCC_UNUSED, char *argv[]GCC_UNUSED)
{
    short f, b;
    int i;

    initscr();
    cbreak();
    noecho();

    if (has_colors()) {
	start_color();

	pair_content(0, &f, &b);
	printw("pair 0 contains (%d,%d)\n", f, b);
	getch();

	printw("Initializing pair 1 to red/black\n");
	init_pair(1, COLOR_RED, COLOR_BLACK);
	i = color_set(1, NULL);
	printw("RED/BLACK (%s)\n", SHOW(i));
	getch();

	printw("Initializing pair 2 to white/blue\n");
	init_pair(2, COLOR_WHITE, COLOR_BLUE);
	i = color_set(2, NULL);
	printw("WHITE/BLUE (%s)\n", SHOW(i));
	getch();

	printw("Resetting colors to pair 0\n");
	i = color_set(0, NULL);
	printw("Default Colors (%s)\n", SHOW(i));
	getch();

	printw("Resetting colors to pair 1\n");
	i = color_set(1, NULL);
	printw("RED/BLACK (%s)\n", SHOW(i));
	getch();

    } else {
	printw("This demo requires a color terminal");
	getch();
    }
    endwin();

    ExitProgram(EXIT_SUCCESS);
}
#else
int
main(void)
{
    printf("This program requires the curses color_set function\n");
    ExitProgram(EXIT_FAILURE);
}
#endif

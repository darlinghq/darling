/****************************************************************************
 * Copyright (c) 2003,2006 Free Software Foundation, Inc.                   *
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
 * $Id: background.c,v 1.3 2006/06/03 16:43:08 tom Exp $
 */

#include <test.priv.h>

int
main(int argc GCC_UNUSED, char *argv[]GCC_UNUSED)
{
    short f, b;

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
	bkgdset(' ' | COLOR_PAIR(1));
	printw("RED/BLACK\n");
	getch();

	printw("Initializing pair 2 to white/blue\n");
	init_pair(2, COLOR_WHITE, COLOR_BLUE);
	bkgdset(' ' | COLOR_PAIR(2));
	printw("WHITE/BLUE\n");
	getch();

	printw("Resetting colors to pair 0\n");
	bkgdset(' ' | COLOR_PAIR(0));
	printw("Default Colors\n");
	getch();

	printw("Resetting colors to pair 1\n");
	bkgdset(' ' | COLOR_PAIR(1));
	printw("RED/BLACK\n");
	getch();

	printw("Setting screen to pair 0\n");
	bkgd(' ' | COLOR_PAIR(0));
	getch();

	printw("Setting screen to pair 1\n");
	bkgd(' ' | COLOR_PAIR(1));
	getch();

	printw("Setting screen to pair 2\n");
	bkgd(' ' | COLOR_PAIR(2));
	getch();

	printw("Setting screen to pair 0\n");
	bkgd(' ' | COLOR_PAIR(0));
	getch();

    } else {
	printw("This demo requires a color terminal");
	getch();
    }
    endwin();

    ExitProgram(EXIT_SUCCESS);
}

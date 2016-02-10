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
 * $Id: firework.c,v 1.24 2006/05/20 15:36:08 tom Exp $
 */
#include <test.priv.h>

#include <time.h>

static int my_bg = COLOR_BLACK;

static void
cleanup(void)
{
    curs_set(1);
    endwin();
}

static RETSIGTYPE
onsig(int n GCC_UNUSED)
{
    cleanup();
    ExitProgram(EXIT_FAILURE);
}

static void
showit(void)
{
    int ch;
    napms(120);
    if ((ch = getch()) != ERR) {
#ifdef KEY_RESIZE
	if (ch == KEY_RESIZE) {
	    erase();
	} else
#endif
	if (ch == 'q') {
	    cleanup();
	    ExitProgram(EXIT_SUCCESS);
	} else if (ch == 's') {
	    nodelay(stdscr, FALSE);
	} else if (ch == ' ') {
	    nodelay(stdscr, TRUE);
	}
    }
}

static int
get_colour(chtype *bold)
{
    int attr;
    attr = (rand() % 16) + 1;

    *bold = A_NORMAL;
    if (attr > 8) {
	*bold = A_BOLD;
	attr &= 7;
    }
    return (attr);
}

static
void
explode(int row, int col)
{
    chtype bold;
    erase();
    mvprintw(row, col, "-");
    showit();

    init_pair(1, get_colour(&bold), my_bg);
    attrset(COLOR_PAIR(1) | bold);
    mvprintw(row - 1, col - 1, " - ");
    mvprintw(row + 0, col - 1, "-+-");
    mvprintw(row + 1, col - 1, " - ");
    showit();

    init_pair(1, get_colour(&bold), my_bg);
    attrset(COLOR_PAIR(1) | bold);
    mvprintw(row - 2, col - 2, " --- ");
    mvprintw(row - 1, col - 2, "-+++-");
    mvprintw(row + 0, col - 2, "-+#+-");
    mvprintw(row + 1, col - 2, "-+++-");
    mvprintw(row + 2, col - 2, " --- ");
    showit();

    init_pair(1, get_colour(&bold), my_bg);
    attrset(COLOR_PAIR(1) | bold);
    mvprintw(row - 2, col - 2, " +++ ");
    mvprintw(row - 1, col - 2, "++#++");
    mvprintw(row + 0, col - 2, "+# #+");
    mvprintw(row + 1, col - 2, "++#++");
    mvprintw(row + 2, col - 2, " +++ ");
    showit();

    init_pair(1, get_colour(&bold), my_bg);
    attrset(COLOR_PAIR(1) | bold);
    mvprintw(row - 2, col - 2, "  #  ");
    mvprintw(row - 1, col - 2, "## ##");
    mvprintw(row + 0, col - 2, "#   #");
    mvprintw(row + 1, col - 2, "## ##");
    mvprintw(row + 2, col - 2, "  #  ");
    showit();

    init_pair(1, get_colour(&bold), my_bg);
    attrset(COLOR_PAIR(1) | bold);
    mvprintw(row - 2, col - 2, " # # ");
    mvprintw(row - 1, col - 2, "#   #");
    mvprintw(row + 0, col - 2, "     ");
    mvprintw(row + 1, col - 2, "#   #");
    mvprintw(row + 2, col - 2, " # # ");
    showit();
}

int
main(
	int argc GCC_UNUSED,
	char *argv[]GCC_UNUSED)
{
    int start, end, row, diff, flag = 0, direction;
    unsigned seed;

    CATCHALL(onsig);

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    if (has_colors()) {
	start_color();
#if HAVE_USE_DEFAULT_COLORS
	if (use_default_colors() == OK)
	    my_bg = -1;
#endif
    }
    curs_set(0);

    seed = time((time_t *) 0);
    srand(seed);
    for (;;) {
	do {
	    start = rand() % (COLS - 3);
	    end = rand() % (COLS - 3);
	    start = (start < 2) ? 2 : start;
	    end = (end < 2) ? 2 : end;
	    direction = (start > end) ? -1 : 1;
	    diff = abs(start - end);
	} while (diff < 2 || diff >= LINES - 2);
	attrset(A_NORMAL);
	for (row = 0; row < diff; row++) {
	    mvprintw(LINES - row, start + (row * direction),
		     (direction < 0) ? "\\" : "/");
	    if (flag++) {
		showit();
		erase();
		flag = 0;
	    }
	}
	if (flag++) {
	    showit();
	    flag = 0;
	}
	seed = time((time_t *) 0);
	srand(seed);
	explode(LINES - row, start + (diff * direction));
	erase();
	showit();
    }
}

/****************************************************************************
 * Copyright (c) 1998-2004,2005 Free Software Foundation, Inc.              *
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
 * Test lower-right-hand corner access
 *
 * originally by Eric S. Raymond <esr@thyrsus.com>, written for animation
 * and resizing -T.Dickey
 *
 * This can't be part of the ncurses test-program, because ncurses rips off the
 * bottom line to do labels.
 *
 * $Id: lrtest.c,v 1.21 2005/12/31 16:51:53 tom Exp $
 */

#include <test.priv.h>

typedef struct {
    int y, x, mode, dir, inc;
    chtype value;
} MARK;

/*
 * Make a couple of markers go 'round the border to demonstrate that we can
 * really write to all positions properly.
 */
static void
show(MARK *m)
{
    mvaddch(m->y, m->x, m->value);
    if (m->mode == 0) {		/* along the x-direction */
	m->x += m->inc;
	if (m->x >= COLS) {
	    m->x = COLS - 1;
	    m->inc = -m->dir * m->inc;
	    m->y += m->inc;
	    m->mode = 1;
	} else if (m->x < 0) {
	    m->x = 0;
	    m->inc = -m->dir * m->inc;
	    m->y += m->inc;
	    m->mode = 1;
	}
    } else {			/* along the y-direction */
	m->y += m->inc;
	if (m->y >= LINES) {
	    m->y = LINES - 1;
	    m->inc = m->dir * m->inc;
	    m->x += m->inc;
	    m->mode = 0;
	} else if (m->y < 0) {
	    m->y = 0;
	    m->inc = m->dir * m->inc;
	    m->x += m->inc;
	    m->mode = 0;
	}
    }
}

int
main(
	int argc GCC_UNUSED,
	char *argv[]GCC_UNUSED)
{
    static MARK marks[] =
    {
	{0, 0, 0, -1, 1, '+' | A_BOLD},
	{0, 0, 1, 1, 2, 'X'},
	{0, 0, 1, -1, 3, 'Y'},
	{0, 8, 0, -1, 1, '+' | A_BOLD},
	{0, 9, 0, -1, 1, '+' | A_BOLD},
	{1, 0, 1, 1, 1, '*' | A_REVERSE},
	{2, 0, 1, 1, 1, '*' | A_REVERSE}
    };

    setlocale(LC_ALL, "");

    initscr();
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);
    curs_set(0);

#ifdef KEY_RESIZE
    keypad(stdscr, TRUE);
  restart:
#endif
    move(LINES / 2 - 1, 4);
    if (!(has_ic()
#if HAVE_SETUPTERM
    /* see PutCharLR() */
	  || auto_right_margin
	  || (enter_am_mode && exit_am_mode)
#endif
	)) {
	addstr("Your terminal lacks the capabilities needed to address the\n");
	move(LINES / 2, 4);
	addstr("lower-right-hand corner of the screen.\n");
    } else {
	addstr("This is a test of access to the lower right corner.\n");
	move(LINES / 2, 4);
	addstr("If the top of the box is missing, the test failed.\n");
	move(LINES / 2 + 1, 4);
	addstr("Please report this (with a copy of your terminfo entry).\n");
	move(LINES / 2 + 2, 4);
	addstr("to the ncurses maintainers, at bug-ncurses@gnu.org.\n");
    }

    for (;;) {
	int ch;
	unsigned n;

	box(stdscr, 0, 0);
	for (n = 0; n < SIZEOF(marks); n++) {
	    show(&marks[n]);
	}

	if ((ch = getch()) > 0) {
	    if (ch == 'q')
		break;
	    else if (ch == 's')
		nodelay(stdscr, FALSE);
	    else if (ch == ' ')
		nodelay(stdscr, TRUE);
#ifdef TRACE
	    else if (ch == 'T')
		trace(0);
	    else if (ch == 't')
		trace(TRACE_CALLS | TRACE_ICALLS | TRACE_UPDATE);
#endif
#ifdef KEY_RESIZE
	    else if (ch == KEY_RESIZE) {
		for (n = 0; n < SIZEOF(marks); n++) {
		    if (marks[n].mode == 0) {	/* moving along x-direction */
			if (marks[n].y)
			    marks[n].y = LINES - 1;
		    } else {
			if (marks[n].x)
			    marks[n].x = COLS - 1;
		    }
		}
		flash();
		erase();
		wrefresh(curscr);
		goto restart;
	    }
#endif
	}
	napms(50);
	refresh();
    }

    curs_set(1);
    endwin();
    ExitProgram(EXIT_SUCCESS);
}

/* lrtest.c ends here */

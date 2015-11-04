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
 * $Id: test_opaque.c,v 1.7 2008/07/05 23:18:40 tom Exp $
 *
 * Author: Thomas E Dickey
 *
 * Demonstrate the opaque functions from the curses library.

       WINDOW * wgetparent (const WINDOW *);
       bool is_cleared(const WINDOW *win);
       bool is_idcok(const WINDOW *win);
       bool is_idlok(const WINDOW *win);
       bool is_immedok(const WINDOW *win);
       bool is_keypad(const WINDOW *win);
       bool is_leaveok(const WINDOW *win);
       bool is_nodelay(const WINDOW *win);
       bool is_notimeout(const WINDOW *win);
       bool is_scrollok(const WINDOW *win);
       bool is_syncok(const WINDOW *win);
       int wgetscrreg (const WINDOW *, int *, int *);
 */

#include <test.priv.h>

#define BASE_Y 6
#define MAX_COLS 1024

#if defined(NCURSES_VERSION_PATCH) && (NCURSES_VERSION_PATCH >= 20080119) && NCURSES_EXT_FUNCS

static bool
Quit(int ch)
{
    return (ch == 'q' || ch == QUIT || ch == ESCAPE);
}

typedef bool(*BoolOpaque) (WINDOW *, int);

static bool
test_opaque_cleared(WINDOW *win, int mode)
{
    if (mode >= 0) {
	if (mode)
	    wclear(win);
    }
    return is_cleared(win);
}

static bool
test_opaque_idcok(WINDOW *win, int mode)
{
    if (mode >= 0) {
	idcok(win, mode);
    }
    return is_idcok(win);
}

static bool
test_opaque_idlok(WINDOW *win, int mode)
{
    if (mode >= 0) {
	idlok(win, mode);
    }
    return is_idlok(win);
}

static bool
test_opaque_immedok(WINDOW *win, int mode)
{
    if (mode >= 0) {
	immedok(win, mode);
    }
    return is_immedok(win);
}

static bool
test_opaque_keypad(WINDOW *win, int mode)
{
    if (mode >= 0) {
	keypad(win, mode);
    }
    return is_keypad(win);
}

static bool
test_opaque_leaveok(WINDOW *win, int mode)
{
    if (mode >= 0) {
	leaveok(win, mode);
    }
    return is_leaveok(win);
}

static bool
test_opaque_nodelay(WINDOW *win, int mode)
{
    if (mode >= 0) {
	nodelay(win, mode);
    }
    return is_nodelay(win);
}

static bool
test_opaque_notimeout(WINDOW *win, int mode)
{
    if (mode >= 0) {
	notimeout(win, mode);
    }
    return is_notimeout(win);
}

static bool
test_opaque_scrollok(WINDOW *win, int mode)
{
    if (mode >= 0) {
	scrollok(win, mode);
    }
    return is_scrollok(win);
}

static bool
test_opaque_syncok(WINDOW *win, int mode)
{
    if (mode >= 0) {
	syncok(win, mode);
    }
    return is_syncok(win);
}

static int
status_y(WINDOW *stswin, int cell)
{
    return (cell % getmaxy(stswin));
}

static int
status_x(WINDOW *stswin, int cell)
{
    return (15 * (cell / getmaxy(stswin)));
}

static void
to_keyword(WINDOW *stswin, int cell)
{
    wmove(stswin, status_y(stswin, cell), status_x(stswin, cell));
}

static void
to_result(WINDOW *stswin, int cell, bool before)
{
    int y = status_y(stswin, cell);
    int x = status_x(stswin, cell) + 11;
    if (!before)
	++x;
    wmove(stswin, y, x);
}

static void
show_keyword(WINDOW *stswin, int cell, int active, const char *name)
{
    to_keyword(stswin, cell);
    if (active == cell)
	wstandout(stswin);
    wprintw(stswin, "%s:", name);
    if (active == cell)
	wstandend(stswin);
}
/* *INDENT-OFF* */
static struct {
    const char *name;
    BoolOpaque func;
} bool_funcs[] = {
    { "cleared",   test_opaque_cleared },
    { "idcok",     test_opaque_idcok },
    { "idlok",     test_opaque_idlok },
    { "immedok",   test_opaque_immedok },
    { "keypad",    test_opaque_keypad },
    { "leaveok",   test_opaque_leaveok },
    { "nodelay",   test_opaque_nodelay },
    { "notimeout", test_opaque_notimeout },
    { "scrollok",  test_opaque_scrollok },
    { "syncok",    test_opaque_syncok }
};
/* *INDENT-ON* */

/*
 * Display and/or allow update for the properties accessed in the opaque
 * window.  Some may change state after refreshing the window, so we
 * distinguish between them using the 'before' parameter.
 */
static int
show_opaque(WINDOW *stswin, WINDOW *txtwin, bool before, int active)
{
    int n;
    int top, bottom;

    if (before) {
	werase(stswin);
    }
    for (n = 0; n < (int) SIZEOF(bool_funcs); ++n) {
	show_keyword(stswin, n, active, bool_funcs[n].name);

	to_result(stswin, n, before);
	wprintw(stswin, "%c", bool_funcs[n].func(txtwin, -1) ? 'T' : 'F');
    }

    show_keyword(stswin, n, active, "wgetparent");
    to_result(stswin, n, TRUE);
    wprintw(stswin, "%p", wgetparent(txtwin));

    ++n;
    show_keyword(stswin, n, active, "wgetscrreg");
    to_result(stswin, n, TRUE);
    if (wgetscrreg(txtwin, &top, &bottom) == OK)
	wprintw(stswin, "%d,%d", top, bottom);

    wnoutrefresh(stswin);
    return active;
}

static int
test_opaque(int level, char **argv, WINDOW *stswin)
{
    WINDOW *txtbox = 0;
    WINDOW *txtwin = 0;
    FILE *fp;
    int ch;
    int txt_x = 0, txt_y = 0;
    int base_y;
    bool in_status = FALSE;
    int active = 0;

    if (argv[level] == 0) {
	beep();
	return FALSE;
    }

    if (level > 1) {
	txtbox = newwin(LINES - BASE_Y, COLS - level, BASE_Y, level);
	box(txtbox, 0, 0);
	wnoutrefresh(txtbox);

	txtwin = derwin(txtbox,
			getmaxy(txtbox) - 2,
			getmaxx(txtbox) - 2,
			1, 1);
	base_y = 0;
    } else {
	txtwin = stdscr;
	base_y = BASE_Y;
    }

    keypad(txtwin, TRUE);	/* enable keyboard mapping */
    (void) cbreak();		/* take input chars one at a time, no wait for \n */
    (void) noecho();		/* don't echo input */

    txt_y = base_y;
    txt_x = 0;
    wmove(txtwin, txt_y, txt_x);

    if ((fp = fopen(argv[level], "r")) != 0) {
	while ((ch = fgetc(fp)) != EOF) {
	    if (waddch(txtwin, UChar(ch)) != OK) {
		break;
	    }
	}
	fclose(fp);
    } else {
	wprintw(txtwin, "Cannot open:\n%s", argv[1]);
    }

    for (;;) {
	if (in_status) {
	    to_keyword(stswin, active);

	    ch = wgetch(stswin);
	    show_opaque(stswin, txtwin, TRUE, active);
	    if (Quit(ch))
		break;

	    switch (ch) {
	    case '\t':
		in_status = FALSE;
		break;
	    case KEY_DOWN:
	    case 'j':
		if (active < (int) SIZEOF(bool_funcs) - 1)
		    active++;
		else
		    beep();
		break;
	    case KEY_UP:
	    case 'k':
		if (active > 0)
		    active--;
		else
		    beep();
		break;
	    case ' ':
		bool_funcs[active].func(txtwin,
					!bool_funcs[active].func(txtwin, -1));
		break;
	    default:
		beep();
		break;
	    }
	    show_opaque(stswin, txtwin, FALSE, in_status ? active : -1);
	} else {
	    ch = mvwgetch(txtwin, txt_y, txt_x);
	    show_opaque(stswin, txtwin, TRUE, -1);
	    if (Quit(ch))
		break;

	    switch (ch) {
	    case '\t':
		in_status = TRUE;
		break;
	    case KEY_DOWN:
	    case 'j':
		if (txt_y < getmaxy(txtwin) - 1)
		    txt_y++;
		else
		    beep();
		break;
	    case KEY_UP:
	    case 'k':
		if (txt_y > base_y)
		    txt_y--;
		else
		    beep();
		break;
	    case KEY_LEFT:
	    case 'h':
		if (txt_x > 0)
		    txt_x--;
		else
		    beep();
		break;
	    case KEY_RIGHT:
	    case 'l':
		if (txt_x < getmaxx(txtwin) - 1)
		    txt_x++;
		else
		    beep();
		break;
	    case 'w':
		test_opaque(level + 1, argv, stswin);
		if (txtbox != 0) {
		    touchwin(txtbox);
		    wnoutrefresh(txtbox);
		} else {
		    touchwin(txtwin);
		    wnoutrefresh(txtwin);
		}
		break;
	    default:
		beep();
		napms(100);
		break;
	    }

	    show_opaque(stswin, txtwin, FALSE, -1);
	}
    }
    if (level > 1) {
	delwin(txtwin);
	delwin(txtbox);
    }
    return TRUE;
}

static void
test_set_escdelay(void)
{
    set_escdelay((100 + ESCDELAY) / 2);
}

static void
test_set_tabsize(void)
{
    int y0, x0;
    int y, x;
    int save_tabsize = TABSIZE;

    (void) cbreak();		/* take input chars one at a time, no wait for \n */
    (void) noecho();		/* don't echo input */

    for (y = 0; y < LINES; ++y) {
	set_tabsize(y + 1);
	if (move(y, 0) == ERR)
	    break;
	for (x = 0; x < COLS;) {
	    addch('\t');
	    if (addch('*') == ERR) {
		break;
	    }
	    getyx(stdscr, y0, x0);
	    if (y0 != y || x0 == x) {
		break;
	    }
	}
    }
    getch();
    erase();

    set_tabsize(save_tabsize);
}

int
main(int argc, char *argv[])
{
    WINDOW *stsbox;
    WINDOW *stswin;

    setlocale(LC_ALL, "");

    if (argc < 2) {
	fprintf(stderr, "usage: %s file\n", argv[0]);
	return EXIT_FAILURE;
    }

    initscr();

    test_set_escdelay();
    test_set_tabsize();

    stsbox = derwin(stdscr, BASE_Y, COLS, 0, 0);
    box(stsbox, 0, 0);
    wnoutrefresh(stsbox);

    stswin = derwin(stsbox, BASE_Y - 2, COLS - 2, 1, 1);
    keypad(stswin, TRUE);

    test_opaque(1, argv, stswin);

    endwin();
    ExitProgram(EXIT_SUCCESS);
}
#else
int
main(void)
{
    printf("This program requires the ncurses library\n");
    ExitProgram(EXIT_FAILURE);
}
#endif

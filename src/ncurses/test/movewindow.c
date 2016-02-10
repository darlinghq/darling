/****************************************************************************
 * Copyright (c) 2006-2007,2008 Free Software Foundation, Inc.              *
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
 * $Id: movewindow.c,v 1.22 2008/04/12 22:01:41 tom Exp $
 *
 * Demonstrate move functions for windows and derived windows from the curses
 * library.
 *
 * Thomas Dickey - 2006/2/11
 */
/*
derwin
mvderwin
subwin
mvwin
 */

#include <test.priv.h>
#include <stdarg.h>

#ifdef HAVE_XCURSES
#undef derwin
#endif

#ifdef NCURSES_VERSION
#define CONST_FMT const
#else
#define CONST_FMT		/* nothing */
#endif

#undef LINE_MAX

#define LINE_MIN	2
#define LINE_MAX	(LINES - 2)
#define COL_MIN		2
#define COL_MAX		(COLS - 2)

typedef struct {
    int y, x;
} PAIR;

typedef struct {
    WINDOW *parent;		/* need this since WINDOW->_parent is not portable */
    WINDOW *child;		/* the actual value */
} FRAME;

static void head_line(CONST_FMT char *fmt,...) GCC_PRINTFLIKE(1, 2);
static void tail_line(CONST_FMT char *fmt,...) GCC_PRINTFLIKE(1, 2);

static unsigned num_windows;
static FRAME *all_windows;

static void
message(int lineno, CONST_FMT char *fmt, va_list argp)
{
    int y, x;

    getyx(stdscr, y, x);
    move(lineno, 0);
    clrtoeol();

#ifdef HAVE_XCURSES
    {
	char buffer[1024];
	vsprintf(buffer, fmt, argp);
	addstr(buffer);
    }
#else
    vwprintw(stdscr, fmt, argp);
#endif

    move(y, x);
    refresh();
}

static void
head_line(CONST_FMT char *fmt,...)
{
    va_list argp;

    va_start(argp, fmt);
    message(0, fmt, argp);
    va_end(argp);
}

static void
tail_line(CONST_FMT char *fmt,...)
{
    va_list argp;

    va_start(argp, fmt);
    message(LINES - 1, fmt, argp);
    va_end(argp);
}

/*
 * Arrow keys move cursor, return location at current on non-arrow key.
 */
static PAIR *
selectcell(WINDOW *parent, int uli, int ulj, int lri, int lrj)
{
    static PAIR res;		/* result cell */
    int si = lri - uli + 1;	/* depth of the select area */
    int sj = lrj - ulj + 1;	/* width of the select area */
    int i = 0, j = 0;		/* offsets into the select area */

    res.y = uli;
    res.x = ulj;
    for (;;) {
	tail_line("Upper left [%2d,%2d] Lower right [%2d,%2d] -> %d,%d",
		  uli, ulj,
		  lri, lrj,
		  uli + i, ulj + j);
	wmove(parent, uli + i, ulj + j);

	switch (wgetch(parent)) {
	case KEY_UP:
	    i += si - 1;
	    break;
	case KEY_DOWN:
	    i++;
	    break;
	case KEY_LEFT:
	    j += sj - 1;
	    break;
	case KEY_RIGHT:
	    j++;
	    break;
	case QUIT:
	case ESCAPE:
	    return ((PAIR *) 0);
#ifdef NCURSES_MOUSE_VERSION
	case KEY_MOUSE:
	    {
		MEVENT event;

		getmouse(&event);
		if (event.y > uli && event.x > ulj) {
		    i = event.y - uli;
		    j = event.x - ulj;
		} else {
		    beep();
		    break;
		}
	    }
	    /* FALLTHRU */
#endif
	default:
	    res.y = uli + i;
	    res.x = ulj + j;
	    return (&res);
	}
	i %= si;
	j %= sj;
    }
}

/*
 * Ask user for a window definition.
 */
static bool
getwindow(WINDOW *parent, PAIR * ul, PAIR * lr)
{
    int min_col = (parent == stdscr) ? COL_MIN : 0;
    int max_col = (parent == stdscr) ? COL_MAX : getmaxx(parent);
    int min_line = (parent == stdscr) ? LINE_MIN : 0;
    int max_line = (parent == stdscr) ? LINE_MAX : getmaxy(parent);
    PAIR *tmp;
    bool result = FALSE;

    head_line("Use arrows to move cursor, anything else to mark corner 1");
    if ((tmp = selectcell(parent, min_line, min_col, max_line, max_col)) != 0) {
	*ul = *tmp;
	mvwaddch(parent, ul->y, ul->x, '*');

	head_line("Use arrows to move cursor, anything else to mark corner 2");
	if ((tmp = selectcell(parent, ul->y, ul->x, max_line, max_col)) != 0) {
	    *lr = *tmp;
	    mvwaddch(parent, lr->y, lr->x, '*');
	    wmove(parent, lr->y, lr->x);
	    wsyncdown(parent);
	    wrefresh(parent);
	    result = (lr->y != ul->y && lr->x != ul->x);
	}
    }
    head_line("done");
    return result;
}

/*
 * Draw a box inside the given window.
 */
static void
box_inside(WINDOW *win)
{
    int y0, x0;
    int y1, x1;

    getyx(win, y0, x0);
    getmaxyx(win, y1, x1);

    mvwhline(win, 0, 0, ACS_HLINE, x1);
    mvwhline(win, y1 - 1, 0, ACS_HLINE, x1);

    mvwvline(win, 0, 0, ACS_VLINE, y1);
    mvwvline(win, 0, x1 - 1, ACS_VLINE, y1);

    mvwaddch(win, 0, 0, ACS_ULCORNER);
    mvwaddch(win, y1 - 1, 0, ACS_LLCORNER);
    mvwaddch(win, 0, x1 - 1, ACS_URCORNER);
    mvwaddch(win, y1 - 1, x1 - 1, ACS_LRCORNER);

    wsyncdown(win);
    wmove(win, y0, x0);
    wrefresh(win);
}

/*
 * Add a window to our list.
 */
static void
add_window(WINDOW *parent, WINDOW *child)
{
    static unsigned have = 0;
    unsigned need = ((num_windows + 1) | 31) + 1;

    keypad(child, TRUE);
    if (need > have) {
	all_windows = typeRealloc(FRAME, need, all_windows);
    }
    all_windows[num_windows].parent = parent;
    all_windows[num_windows].child = child;
    num_windows++;
}

static int
window2num(WINDOW *win)
{
    int n;
    int result = -1;
    for (n = 0; n < (int) num_windows; ++n) {
	if (win == all_windows[n].child) {
	    result = n;
	    break;
	}
    }
    return result;
}

static WINDOW *
parent_of(WINDOW *win)
{
    WINDOW *result = 0;
    int n = window2num(win);
    if (n >= 0)
	result = all_windows[n].parent;
    return result;
}

static void
repaint_one(WINDOW *win)
{
    touchwin(win);
    wnoutrefresh(win);
}

static void
refresh_all(WINDOW *win)
{
    unsigned n;

    for (n = 0; n < num_windows; ++n) {
	if (all_windows[n].child != win) {
	    repaint_one(all_windows[n].child);
	}
    }

    repaint_one(win);
    doupdate();
}

static WINDOW *
next_window(WINDOW *win)
{
    WINDOW *result = win;
    int n = window2num(win);

    if (n++ >= 0) {
	result = all_windows[n % num_windows].child;
	wmove(result, 0, 0);
	wrefresh(result);
    }
    return result;
}

static WINDOW *
prev_window(WINDOW *win)
{
    WINDOW *result = win;
    int n = window2num(win);

    if (n-- >= 0) {
	if (n < 0)
	    n = num_windows - 1;
	result = all_windows[n % num_windows].child;
	wmove(result, 0, 0);
	wrefresh(result);
    }
    return result;
}

static void
recur_move_window(WINDOW *parent, int dy, int dx)
{
    unsigned n;

    for (n = 0; n < num_windows; ++n) {
	if (all_windows[n].parent == parent) {
	    int y0, x0;

	    getbegyx(all_windows[n].child, y0, x0);
	    mvwin(all_windows[n].child, y0 + dy, x0 + dx);
	    recur_move_window(all_windows[n].child, dy, dx);
	}
    }
}

/*
 * test mvwin().
 */
static bool
move_window(WINDOW *win, bool recur)
{
    WINDOW *parent = parent_of(win);
    bool result = FALSE;

    if (parent != 0) {
	bool top = (parent == stdscr);
	int min_col = top ? COL_MIN : 0;
	int max_col = top ? COL_MAX : getmaxx(parent);
	int min_line = top ? LINE_MIN : 0;
	int max_line = top ? LINE_MAX : getmaxy(parent);
	PAIR *tmp;

	head_line("Select new position for %swindow", top ? "" : "sub");

	if ((tmp = selectcell(parent,
			      min_line, min_col,
			      max_line, max_col)) != 0) {
	    int y0, x0;
	    getbegyx(parent, y0, x0);
	    /*
	     * Note:  Moving a subwindow has the effect of moving a viewport
	     * around the screen.  The parent window retains the contents of
	     * the subwindow in the original location, but the viewport will
	     * show the contents (again) at the new location.  So it will look
	     * odd when testing.
	     */
	    if (mvwin(win, y0 + tmp->y, x0 + tmp->x) != ERR) {
		if (recur) {
		    recur_move_window(win, tmp->y, tmp->x);
		}
		refresh_all(win);
		doupdate();
		result = TRUE;
	    }
	}
    }
    return result;
}

/*
 * test mvderwin().
 */
static bool
move_subwin(WINDOW *win)
{
    WINDOW *parent = parent_of(win);
    bool result = FALSE;

    if (parent != 0) {
	bool top = (parent == stdscr);
	if (!top) {
	    int min_col = top ? COL_MIN : 0;
	    int max_col = top ? COL_MAX : getmaxx(parent);
	    int min_line = top ? LINE_MIN : 0;
	    int max_line = top ? LINE_MAX : getmaxy(parent);
	    PAIR *tmp;

	    head_line("Select new position for subwindow");

	    if ((tmp = selectcell(parent,
				  min_line, min_col,
				  max_line, max_col)) != 0) {
		int y0, x0;
		getbegyx(parent, y0, x0);
		if (mvderwin(win, y0 + tmp->y, x0 + tmp->x) != ERR) {
		    refresh_all(win);
		    doupdate();
		    result = TRUE;
		}
	    }
	}
    }
    return result;
}

static void
fill_window(WINDOW *win, chtype ch)
{
    int y, x;
    int y0, x0;
    int y1, x1;

    getyx(win, y0, x0);
    getmaxyx(win, y1, x1);
    for (y = 0; y < y1; ++y) {
	for (x = 0; x < x1; ++x) {
	    mvwaddch(win, y, x, ch);
	}
    }
    wsyncdown(win);
    wmove(win, y0, x0);
    wrefresh(win);
}

#define lines_of(ul,lr)	(lr.y - ul.y + 1)
#define cols_of(ul,lr)	(lr.x - ul.x + 1)
#define pair_of(ul)	ul.y, ul.x

static WINDOW *
create_my_window(WINDOW *current)
{
    PAIR ul, lr;
    WINDOW *result = 0;

    if (getwindow(stdscr, &ul, &lr)) {
	result = newwin(lines_of(ul, lr), cols_of(ul, lr), pair_of(ul));
	if (result != 0) {
	    fill_window(result, 'c');
	    add_window(stdscr, result);
	}
    }
    if (result == 0)
	result = current;
    return result;
}

static WINDOW *
create_my_derwin(WINDOW *parent)
{
    PAIR ul, lr;
    WINDOW *result = 0;

    if (getwindow(parent, &ul, &lr)) {
	result = derwin(parent, lines_of(ul, lr), cols_of(ul, lr), pair_of(ul));
	if (result != 0) {
	    fill_window(result, 'd');
	    add_window(parent, result);
	}
    }
    if (result == 0)
	result = parent;
    return result;
}

static WINDOW *
create_my_subwin(WINDOW *parent)
{
    PAIR ul, lr;
    WINDOW *result = 0;

    if (getwindow(parent, &ul, &lr)) {
	result = subwin(parent,
			lines_of(ul, lr),
			cols_of(ul, lr),
			ul.y + getbegy(parent),
			ul.x + getbegx(parent));
	if (result != 0) {
	    fill_window(result, 's');
	    add_window(parent, result);
	}
    }
    if (result == 0)
	result = parent;
    return result;
}

static void
show_help(WINDOW *current)
{
    /* *INDENT-OFF* */
    static struct {
	int	key;
	CONST_FMT char * msg;
    } help[] = {
	{ '?',		"Show this screen" },
	{ 'b',		"Draw a box inside the current window" },
	{ 'c',		"Create a new window" },
	{ 'd',		"Create a new derived window" },
	{ 'f',		"Fill the current window with the next character" },
	{ 'm',		"Move the current window" },
	{ 'M',		"Move the current window (and its children)" },
	{ 'q',		"Quit" },
	{ 's',		"Create a new subwindow" },
	{ 't',		"Move the current subwindow (moves content)" },
	{ CTRL('L'),	"Repaint all windows, doing current one last" },
	{ CTRL('N'),	"Cursor to next window" },
	{ CTRL('P'),	"Cursor to previous window" },
    };
    /* *INDENT-ON* */

    WINDOW *mywin = newwin(LINES, COLS, 0, 0);
    int row;

    for (row = 0; row < LINES - 2 && row < (int) SIZEOF(help); ++row) {
	wmove(mywin, row + 1, 1);
	wprintw(mywin, "%s", keyname(help[row].key));
	wmove(mywin, row + 1, 20);
	wprintw(mywin, "%s", help[row].msg);
    }
    box_inside(mywin);
    wmove(mywin, 1, 1);
    wgetch(mywin);
    delwin(mywin);
    refresh_all(current);
}

int
main(int argc GCC_UNUSED, char *argv[]GCC_UNUSED)
{
    WINDOW *current_win;
    int ch;
    bool done = FALSE;

    initscr();
    cbreak();
    noecho();
    nonl();
    intrflush(stdscr, FALSE);

    add_window(0, current_win = stdscr);

#ifdef NCURSES_MOUSE_VERSION
    (void) mousemask(BUTTON1_CLICKED, (mmask_t *) NULL);
#endif /* NCURSES_MOUSE_VERSION */

    while (!done && (ch = wgetch(current_win)) != ERR) {
	switch (ch) {
	case '?':
	    show_help(current_win);
	    break;
	case 'b':
	    box_inside(current_win);
	    break;
	case 'c':
	    current_win = create_my_window(current_win);
	    break;
	case 'd':
	    current_win = create_my_derwin(current_win);
	    break;
	case 'f':
	    fill_window(current_win, (chtype) wgetch(current_win));
	    break;
	case 'm':
	case 'M':
	    if (!move_window(current_win, (ch == 'M'))) {
		tail_line("error");
		continue;
	    }
	    break;
	case 'q':
	    done = TRUE;
	    break;
	case 's':
	    current_win = create_my_subwin(current_win);
	    break;
	case 't':
	    if (!move_subwin(current_win)) {
		tail_line("error");
		continue;
	    }
	    break;
	case CTRL('L'):
	    refresh_all(current_win);
	    break;
	case CTRL('N'):
	    current_win = next_window(current_win);
	    break;
	case CTRL('P'):
	    current_win = prev_window(current_win);
	    break;
#if 0
	    /* want to allow cursor to move around the current window too */
	    /* want to test the resizing of windows and subwindows too */
	    /* want to allow deleting a window also */
#endif
	default:
	    tail_line("unrecognized key (use '?' for help)");
	    beep();
	    continue;
	}
	tail_line("size [%d,%d] begin [%d,%d] parent [%d,%d]",
		  getmaxy(current_win),
		  getmaxx(current_win),
		  getbegy(current_win),
		  getbegx(current_win),
		  getpary(current_win),
		  getparx(current_win));
	wmove(current_win, 0, 0);
    }
    endwin();
    ExitProgram(EXIT_SUCCESS);
}

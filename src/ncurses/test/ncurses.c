/****************************************************************************
 * Copyright (c) 1998-2007,2008 Free Software Foundation, Inc.              *
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
/****************************************************************************

NAME
   ncurses.c --- ncurses library exerciser

SYNOPSIS
   ncurses

DESCRIPTION
   An interactive test module for the ncurses library.

AUTHOR
   Author: Eric S. Raymond <esr@snark.thyrsus.com> 1993
           Thomas E. Dickey (beginning revision 1.27 in 1996).

$Id: ncurses.c,v 1.329 2008/09/27 14:34:58 tom Exp $

***************************************************************************/

#include <test.priv.h>

#ifdef __hpux
#undef mvwdelch			/* HPUX 11.23 macro will not compile */
#endif

#if HAVE_GETTIMEOFDAY
#if HAVE_SYS_TIME_H && HAVE_SYS_TIME_SELECT
#include <sys/time.h>
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#endif

#if USE_LIBPANEL
#include <panel.h>
#endif

#if USE_LIBMENU
#include <menu.h>
#endif

#if USE_LIBFORM
#include <form.h>
#endif

#ifdef NCURSES_VERSION

#define NCURSES_CONST_PARAM const void

#ifdef TRACE
static unsigned save_trace = TRACE_ORDINARY | TRACE_ICALLS | TRACE_CALLS;
extern unsigned _nc_tracing;
#endif

#else

#define NCURSES_CONST_PARAM char

#define mmask_t chtype		/* not specified in XSI */

#ifndef ACS_S3
#ifdef CURSES_ACS_ARRAY
#define ACS_S3          (CURSES_ACS_ARRAY['p'])		/* scan line 3 */
#define ACS_S7          (CURSES_ACS_ARRAY['r'])		/* scan line 7 */
#define ACS_LEQUAL      (CURSES_ACS_ARRAY['y'])		/* less/equal */
#define ACS_GEQUAL      (CURSES_ACS_ARRAY['z'])		/* greater/equal */
#define ACS_PI          (CURSES_ACS_ARRAY['{'])		/* Pi */
#define ACS_NEQUAL      (CURSES_ACS_ARRAY['|'])		/* not equal */
#define ACS_STERLING    (CURSES_ACS_ARRAY['}'])		/* UK pound sign */
#else
#define ACS_S3          (A_ALTCHARSET + 'p')	/* scan line 3 */
#define ACS_S7          (A_ALTCHARSET + 'r')	/* scan line 7 */
#define ACS_LEQUAL      (A_ALTCHARSET + 'y')	/* less/equal */
#define ACS_GEQUAL      (A_ALTCHARSET + 'z')	/* greater/equal */
#define ACS_PI          (A_ALTCHARSET + '{')	/* Pi */
#define ACS_NEQUAL      (A_ALTCHARSET + '|')	/* not equal */
#define ACS_STERLING    (A_ALTCHARSET + '}')	/* UK pound sign */
#endif
#endif /* ACS_S3 */

#ifdef CURSES_WACS_ARRAY
#define WACS_S3         (&(CURSES_WACS_ARRAY['p']))	/* scan line 3 */
#define WACS_S7         (&(CURSES_WACS_ARRAY['r']))	/* scan line 7 */
#define WACS_LEQUAL     (&(CURSES_WACS_ARRAY['y']))	/* less/equal */
#define WACS_GEQUAL     (&(CURSES_WACS_ARRAY['z']))	/* greater/equal */
#define WACS_PI         (&(CURSES_WACS_ARRAY['{']))	/* Pi */
#define WACS_NEQUAL     (&(CURSES_WACS_ARRAY['|']))	/* not equal */
#define WACS_STERLING   (&(CURSES_WACS_ARRAY['}']))	/* UK pound sign */
#endif

#endif

#if HAVE_WCSRTOMBS
#define count_wchars(src, len, state)      wcsrtombs(0,   &src, len, state)
#define trans_wchars(dst, src, len, state) wcsrtombs(dst, &src, len, state)
#define reset_wchars(state) memset(&state, 0, sizeof(state))
#elif HAVE_WCSTOMBS && HAVE_MBTOWC && HAVE_MBLEN
#define count_wchars(src, len, state)      wcstombs(0,   src, len)
#define trans_wchars(dst, src, len, state) wcstombs(dst, src, len)
#define reset_wchars(state) mblen(NULL, 0), mbtowc(NULL, NULL, 0)
#define state_unused
#endif

#if HAVE_MBSRTOWCS
#define count_mbytes(src, len, state)      mbsrtowcs(0,   &src, len, state)
#define trans_mbytes(dst, src, len, state) mbsrtowcs(dst, &src, len, state)
#define reset_mbytes(state) memset(&state, 0, sizeof(state))
#elif HAVE_MBSTOWCS && HAVE_MBTOWC && HAVE_MBLEN
#define count_mbytes(src, len, state)      mbstowcs(0,   src, len)
#define trans_mbytes(dst, src, len, state) mbstowcs(dst, src, len)
#define reset_mbytes(state) mblen(NULL, 0), mbtowc(NULL, NULL, 0)
#define state_unused
#endif

#define ToggleAcs(temp,real) temp = ((temp == real) ? 0 : real)

#define P(string)	printw("%s\n", string)

#define BLANK		' '	/* this is the background character */

#undef max_colors
static int max_colors;		/* the actual number of colors we'll use */
static int min_colors;		/* the minimum color code */
static bool use_colors;		/* true if we use colors */

#undef max_pairs
static int max_pairs;		/* ...and the number of color pairs */

typedef struct {
    short red;
    short green;
    short blue;
} RGB_DATA;

static RGB_DATA *all_colors;

static void main_menu(bool);

/* The behavior of mvhline, mvvline for negative/zero length is unspecified,
 * though we can rely on negative x/y values to stop the macro.
 */
static void
do_h_line(int y, int x, chtype c, int to)
{
    if ((to) > (x))
	mvhline(y, x, c, (to) - (x));
}

static void
do_v_line(int y, int x, chtype c, int to)
{
    if ((to) > (y))
	mvvline(y, x, c, (to) - (y));
}

static void
Repaint(void)
{
    touchwin(stdscr);
    touchwin(curscr);
    wrefresh(curscr);
}

static bool
isQuit(int c)
{
    return ((c) == QUIT || (c) == ESCAPE);
}
#define case_QUIT	QUIT: case ESCAPE

/* Common function to allow ^T to toggle trace-mode in the middle of a test
 * so that trace-files can be made smaller.
 */
static int
wGetchar(WINDOW *win)
{
    int c;
#ifdef TRACE
    while ((c = wgetch(win)) == CTRL('T')) {
	if (_nc_tracing) {
	    save_trace = _nc_tracing;
	    Trace(("TOGGLE-TRACING OFF"));
	    _nc_tracing = 0;
	} else {
	    _nc_tracing = save_trace;
	}
	trace(_nc_tracing);
	if (_nc_tracing)
	    Trace(("TOGGLE-TRACING ON"));
    }
#else
    c = wgetch(win);
#endif
    return c;
}
#define Getchar() wGetchar(stdscr)

/* replaces wgetnstr(), since we want to be able to edit values */
static void
wGetstring(WINDOW *win, char *buffer, int limit)
{
    int y0, x0, x, ch;
    bool done = FALSE;

    echo();
    getyx(win, y0, x0);
    wattrset(win, A_REVERSE);

    x = (int) strlen(buffer);
    while (!done) {
	if (x > (int) strlen(buffer))
	    x = (int) strlen(buffer);
	wmove(win, y0, x0);
	wprintw(win, "%-*s", limit, buffer);
	wmove(win, y0, x0 + x);
	switch (ch = wGetchar(win)) {
	case '\n':
	case KEY_ENTER:
	    done = TRUE;
	    break;
	case CTRL('U'):
	    *buffer = '\0';
	    break;
	case '\b':
	case KEY_BACKSPACE:
	case KEY_DC:
	    if (x > 0) {
		int j;
		for (j = --x; (buffer[j] = buffer[j + 1]) != '\0'; ++j) {
		    ;
		}
	    } else {
		beep();
	    }
	    break;
	case KEY_LEFT:
	    if (x > 0) {
		--x;
	    } else {
		flash();
	    }
	    break;
	case KEY_RIGHT:
	    ++x;
	    break;
	default:
	    if (!isprint(ch) || ch >= KEY_MIN) {
		beep();
	    } else if ((int) strlen(buffer) < limit) {
		int j;
		for (j = (int) strlen(buffer) + 1; j > x; --j) {
		    buffer[j] = buffer[j - 1];
		}
		buffer[x++] = (char) ch;
	    } else {
		flash();
	    }
	}
    }

    wattroff(win, A_REVERSE);
    wmove(win, y0, x0);
    noecho();
}

#if USE_WIDEC_SUPPORT
static wchar_t
fullwidth_of(int ch)
{
    return (ch + 0xff10 - '0');
}

static void
make_fullwidth_text(wchar_t *target, const char *source)
{
    int ch;
    while ((ch = *source++) != 0) {
	*target++ = fullwidth_of(ch);
    }
    *target = 0;
}

static void
make_narrow_text(wchar_t *target, const char *source)
{
    int ch;
    while ((ch = *source++) != 0) {
	*target++ = ch;
    }
    *target = 0;
}

static void
make_fullwidth_digit(cchar_t *target, int digit)
{
    wchar_t source[2];

    source[0] = fullwidth_of(digit + '0');
    source[1] = 0;
    setcchar(target, source, A_NORMAL, 0, 0);
}

static int
wGet_wchar(WINDOW *win, wint_t *result)
{
    int c;
#ifdef TRACE
    while ((c = wget_wch(win, result)) == CTRL('T')) {
	if (_nc_tracing) {
	    save_trace = _nc_tracing;
	    Trace(("TOGGLE-TRACING OFF"));
	    _nc_tracing = 0;
	} else {
	    _nc_tracing = save_trace;
	}
	trace(_nc_tracing);
	if (_nc_tracing)
	    Trace(("TOGGLE-TRACING ON"));
    }
#else
    c = wget_wch(win, result);
#endif
    return c;
}
#define Get_wchar(result) wGet_wchar(stdscr, result)

/* replaces wgetn_wstr(), since we want to be able to edit values */
static void
wGet_wstring(WINDOW *win, wchar_t *buffer, int limit)
{
    int y0, x0, x;
    wint_t ch;
    bool done = FALSE;
    bool fkey = FALSE;

    echo();
    getyx(win, y0, x0);
    wattrset(win, A_REVERSE);

    x = (int) wcslen(buffer);
    while (!done) {
	if (x > (int) wcslen(buffer))
	    x = (int) wcslen(buffer);

	/* clear the "window' */
	wmove(win, y0, x0);
	wprintw(win, "%*s", limit, " ");

	/* write the existing buffer contents */
	wmove(win, y0, x0);
	waddnwstr(win, buffer, limit);

	/* positions the cursor past character 'x' */
	wmove(win, y0, x0);
	waddnwstr(win, buffer, x);

	switch (wGet_wchar(win, &ch)) {
	case KEY_CODE_YES:
	    fkey = TRUE;
	    switch (ch) {
	    case KEY_ENTER:
		ch = '\n';
		fkey = FALSE;
		break;
	    case KEY_BACKSPACE:
	    case KEY_DC:
		ch = '\b';
		fkey = FALSE;
		break;
	    case KEY_LEFT:
	    case KEY_RIGHT:
		break;
	    default:
		ch = (wint_t) -1;
		break;
	    }
	    break;
	case OK:
	    fkey = FALSE;
	    break;
	default:
	    ch = (wint_t) -1;
	    fkey = TRUE;
	    break;
	}

	switch (ch) {
	case '\n':
	    done = TRUE;
	    break;
	case CTRL('U'):
	    *buffer = '\0';
	    break;
	case '\b':
	    if (x > 0) {
		int j;
		for (j = --x; (buffer[j] = buffer[j + 1]) != '\0'; ++j) {
		    ;
		}
	    } else {
		beep();
	    }
	    break;
	case KEY_LEFT:
	    if (x > 0) {
		--x;
	    } else {
		beep();
	    }
	    break;
	case KEY_RIGHT:
	    ++x;
	    break;
	default:
	    if (fkey) {
		beep();
	    } else if ((int) wcslen(buffer) < limit) {
		int j;
		for (j = (int) wcslen(buffer) + 1; j > x; --j) {
		    buffer[j] = buffer[j - 1];
		}
		buffer[x++] = (wchar_t) ch;
	    } else {
		beep();
	    }
	}
    }

    wattroff(win, A_REVERSE);
    wmove(win, y0, x0);
    noecho();
}

#endif

static void
Pause(void)
{
    move(LINES - 1, 0);
    addstr("Press any key to continue... ");
    (void) Getchar();
}

static void
Cannot(const char *what)
{
    printw("\nThis %s terminal %s\n\n", getenv("TERM"), what);
    Pause();
}

static void
ShellOut(bool message)
{
    if (message)
	addstr("Shelling out...");
    def_prog_mode();
    endwin();
    system("sh");
    if (message)
	addstr("returned from shellout.\n");
    refresh();
}

#ifdef NCURSES_MOUSE_VERSION
/*
 * This function is the same as _tracemouse(), but we cannot count on that
 * being available in the non-debug library.
 */
static const char *
mouse_decode(MEVENT const *ep)
{
    static char buf[80 + (5 * 10) + (32 * 15)];

    (void) sprintf(buf, "id %2d  at (%2d, %2d, %2d) state %4lx = {",
		   ep->id, ep->x, ep->y, ep->z, (unsigned long) ep->bstate);

#define SHOW(m, s) if ((ep->bstate & m)==m) {strcat(buf,s); strcat(buf, ", ");}

    SHOW(BUTTON1_RELEASED, "release-1");
    SHOW(BUTTON1_PRESSED, "press-1");
    SHOW(BUTTON1_CLICKED, "click-1");
    SHOW(BUTTON1_DOUBLE_CLICKED, "doubleclick-1");
    SHOW(BUTTON1_TRIPLE_CLICKED, "tripleclick-1");
#if NCURSES_MOUSE_VERSION == 1
    SHOW(BUTTON1_RESERVED_EVENT, "reserved-1");
#endif

    SHOW(BUTTON2_RELEASED, "release-2");
    SHOW(BUTTON2_PRESSED, "press-2");
    SHOW(BUTTON2_CLICKED, "click-2");
    SHOW(BUTTON2_DOUBLE_CLICKED, "doubleclick-2");
    SHOW(BUTTON2_TRIPLE_CLICKED, "tripleclick-2");
#if NCURSES_MOUSE_VERSION == 1
    SHOW(BUTTON2_RESERVED_EVENT, "reserved-2");
#endif

    SHOW(BUTTON3_RELEASED, "release-3");
    SHOW(BUTTON3_PRESSED, "press-3");
    SHOW(BUTTON3_CLICKED, "click-3");
    SHOW(BUTTON3_DOUBLE_CLICKED, "doubleclick-3");
    SHOW(BUTTON3_TRIPLE_CLICKED, "tripleclick-3");
#if NCURSES_MOUSE_VERSION == 1
    SHOW(BUTTON3_RESERVED_EVENT, "reserved-3");
#endif

    SHOW(BUTTON4_RELEASED, "release-4");
    SHOW(BUTTON4_PRESSED, "press-4");
    SHOW(BUTTON4_CLICKED, "click-4");
    SHOW(BUTTON4_DOUBLE_CLICKED, "doubleclick-4");
    SHOW(BUTTON4_TRIPLE_CLICKED, "tripleclick-4");
#if NCURSES_MOUSE_VERSION == 1
    SHOW(BUTTON4_RESERVED_EVENT, "reserved-4");
#endif

#if NCURSES_MOUSE_VERSION == 2
    SHOW(BUTTON5_RELEASED, "release-5");
    SHOW(BUTTON5_PRESSED, "press-5");
    SHOW(BUTTON5_CLICKED, "click-5");
    SHOW(BUTTON5_DOUBLE_CLICKED, "doubleclick-5");
    SHOW(BUTTON5_TRIPLE_CLICKED, "tripleclick-5");
#endif

    SHOW(BUTTON_CTRL, "ctrl");
    SHOW(BUTTON_SHIFT, "shift");
    SHOW(BUTTON_ALT, "alt");
    SHOW(ALL_MOUSE_EVENTS, "all-events");
    SHOW(REPORT_MOUSE_POSITION, "position");

#undef SHOW

    if (buf[strlen(buf) - 1] == ' ')
	buf[strlen(buf) - 2] = '\0';
    (void) strcat(buf, "}");
    return (buf);
}
#endif /* NCURSES_MOUSE_VERSION */

/****************************************************************************
 *
 * Character input test
 *
 ****************************************************************************/

static void
setup_getch(WINDOW *win, bool flags[])
{
    keypad(win, flags['k']);	/* should be redundant, but for testing */
    meta(win, flags['m']);	/* force this to a known state */
    if (flags['e'])
	echo();
    else
	noecho();
}

static void
wgetch_help(WINDOW *win, bool flags[])
{
    static const char *help[] =
    {
	"e  -- toggle echo mode"
	,"g  -- triggers a getstr test"
	,"k  -- toggle keypad/literal mode"
	,"m  -- toggle meta (7-bit/8-bit) mode"
	,"^q -- quit"
	,"s  -- shell out\n"
	,"w  -- create a new window"
#ifdef SIGTSTP
	,"z  -- suspend this process"
#endif
    };
    int y, x;
    unsigned chk = ((SIZEOF(help) + 1) / 2);
    unsigned n;

    getyx(win, y, x);
    move(0, 0);
    printw("Type any key to see its %s value.  Also:\n",
	   flags['k'] ? "keypad" : "literal");
    for (n = 0; n < SIZEOF(help); ++n) {
	int row = 1 + (int) (n % chk);
	int col = (n >= chk) ? COLS / 2 : 0;
	int flg = ((strstr(help[n], "toggle") != 0)
		   && (flags[UChar(*help[n])] != FALSE));
	if (flg)
	    standout();
	mvprintw(row, col, "%s", help[n]);
	if (col == 0)
	    clrtoeol();
	if (flg)
	    standend();
    }
    wrefresh(stdscr);
    wmove(win, y, x);
}

static void
wgetch_wrap(WINDOW *win, int first_y)
{
    int last_y = getmaxy(win) - 1;
    int y = getcury(win) + 1;

    if (y >= last_y)
	y = first_y;
    wmove(win, y, 0);
    wclrtoeol(win);
}

#if defined(NCURSES_VERSION) && defined(KEY_RESIZE) && HAVE_WRESIZE
typedef struct {
    WINDOW *text;
    WINDOW *frame;
} WINSTACK;

static WINSTACK *winstack = 0;
static unsigned len_winstack = 0;

static void
forget_boxes(void)
{
    if (winstack != 0) {
	free(winstack);
    }
    winstack = 0;
    len_winstack = 0;
}

static void
remember_boxes(unsigned level, WINDOW *txt_win, WINDOW *box_win)
{
    unsigned need = (level + 1) * 2;

    assert(level < COLS);

    if (winstack == 0) {
	len_winstack = 20;
	winstack = typeMalloc(WINSTACK, len_winstack);
    } else if (need >= len_winstack) {
	len_winstack = need;
	winstack = typeRealloc(WINSTACK, len_winstack, winstack);
    }
    winstack[level].text = txt_win;
    winstack[level].frame = box_win;
}

#if USE_SOFTKEYS && (NCURSES_VERSION_PATCH < 20071229) && NCURSES_EXT_FUNCS
static void
slk_repaint(void)
{
    /* this chunk is now done in resize_term() */
    slk_touch();
    slk_clear();
    slk_noutrefresh();
}

#else
#define slk_repaint()		/* nothing */
#endif

/*
 * For wgetch_test(), we create pairs of windows - one for a box, one for text.
 * Resize both and paint the box in the parent.
 */
static void
resize_boxes(unsigned level, WINDOW *win)
{
    unsigned n;
    int base = 5;
    int high = LINES - base;
    int wide = COLS;

    touchwin(stdscr);
    wnoutrefresh(stdscr);

    slk_repaint();

    for (n = 0; n < level; ++n) {
	wresize(winstack[n].frame, high, wide);
	wresize(winstack[n].text, high - 2, wide - 2);
	high -= 2;
	wide -= 2;
	werase(winstack[n].text);
	box(winstack[n].frame, 0, 0);
	wnoutrefresh(winstack[n].frame);
	wprintw(winstack[n].text,
		"size %dx%d\n",
		getmaxy(winstack[n].text),
		getmaxx(winstack[n].text));
	wnoutrefresh(winstack[n].text);
	if (winstack[n].text == win)
	    break;
    }
    doupdate();
}
#else
#define forget_boxes()		/* nothing */
#define remember_boxes(level,text,frame)	/* nothing */
#endif

static void
wgetch_test(unsigned level, WINDOW *win, int delay)
{
    char buf[BUFSIZ];
    int first_y, first_x;
    int c;
    int incount = 0;
    bool flags[256];
    bool blocking = (delay < 0);

    memset(flags, FALSE, sizeof(flags));
    flags[UChar('k')] = (win == stdscr);

    setup_getch(win, flags);
    wtimeout(win, delay);
    getyx(win, first_y, first_x);

    wgetch_help(win, flags);
    wsetscrreg(win, first_y, getmaxy(win) - 1);
    scrollok(win, TRUE);

    for (;;) {
	while ((c = wGetchar(win)) == ERR) {
	    incount++;
	    if (blocking) {
		(void) wprintw(win, "%05d: input error", incount);
		break;
	    } else {
		(void) wprintw(win, "%05d: input timed out", incount);
	    }
	    wgetch_wrap(win, first_y);
	}
	if (c == ERR && blocking) {
	    wprintw(win, "ERR");
	    wgetch_wrap(win, first_y);
	} else if (isQuit(c)) {
	    break;
	} else if (c == 'e') {
	    flags[UChar('e')] = !flags[UChar('e')];
	    setup_getch(win, flags);
	    wgetch_help(win, flags);
	} else if (c == 'g') {
	    waddstr(win, "getstr test: ");
	    echo();
	    wgetnstr(win, buf, sizeof(buf) - 1);
	    noecho();
	    wprintw(win, "I saw %d characters:\n\t`%s'.", (int) strlen(buf), buf);
	    wclrtoeol(win);
	    wgetch_wrap(win, first_y);
	} else if (c == 'k') {
	    flags[UChar('k')] = !flags[UChar('k')];
	    setup_getch(win, flags);
	    wgetch_help(win, flags);
	} else if (c == 'm') {
	    flags[UChar('m')] = !flags[UChar('m')];
	    setup_getch(win, flags);
	    wgetch_help(win, flags);
	} else if (c == 's') {
	    ShellOut(TRUE);
	} else if (c == 'w') {
	    int high = getmaxy(win) - 1 - first_y + 1;
	    int wide = getmaxx(win) - first_x;
	    int old_y, old_x;
	    int new_y = first_y + getbegy(win);
	    int new_x = first_x + getbegx(win);

	    getyx(win, old_y, old_x);
	    if (high > 2 && wide > 2) {
		WINDOW *wb = newwin(high, wide, new_y, new_x);
		WINDOW *wi = newwin(high - 2, wide - 2, new_y + 1, new_x + 1);

		box(wb, 0, 0);
		wrefresh(wb);
		wmove(wi, 0, 0);
		remember_boxes(level, wi, wb);
		wgetch_test(level + 1, wi, delay);
		delwin(wi);
		delwin(wb);

		wgetch_help(win, flags);
		wmove(win, old_y, old_x);
		touchwin(win);
		wrefresh(win);
		doupdate();
	    }
#ifdef SIGTSTP
	} else if (c == 'z') {
	    kill(getpid(), SIGTSTP);
#endif
	} else {
	    wprintw(win, "Key pressed: %04o ", c);
#ifdef NCURSES_MOUSE_VERSION
	    if (c == KEY_MOUSE) {
		int y, x;
		MEVENT event;

		getmouse(&event);
		wprintw(win, "KEY_MOUSE, %s", mouse_decode(&event));
		getyx(win, y, x);
		move(event.y, event.x);
		addch('*');
		wmove(win, y, x);
	    } else
#endif /* NCURSES_MOUSE_VERSION */
	    if (c >= KEY_MIN) {
#if defined(NCURSES_VERSION) && defined(KEY_RESIZE) && HAVE_WRESIZE
		if (c == KEY_RESIZE) {
		    resize_boxes(level, win);
		}
#endif
		(void) waddstr(win, keyname(c));
	    } else if (c > 0x80) {
		unsigned c2 = (unsigned) (c & 0x7f);
		if (isprint(c2))
		    (void) wprintw(win, "M-%c", UChar(c2));
		else
		    (void) wprintw(win, "M-%s", unctrl(c2));
		waddstr(win, " (high-half character)");
	    } else {
		if (isprint(c))
		    (void) wprintw(win, "%c (ASCII printable character)", c);
		else
		    (void) wprintw(win, "%s (ASCII control character)",
				   unctrl(UChar(c)));
	    }
	    wgetch_wrap(win, first_y);
	}
    }

    wtimeout(win, -1);
}

static int
begin_getch_test(void)
{
    char buf[BUFSIZ];
    int delay;

    refresh();

#ifdef NCURSES_MOUSE_VERSION
    mousemask(ALL_MOUSE_EVENTS, (mmask_t *) 0);
#endif

    (void) printw("Delay in 10ths of a second (<CR> for blocking input)? ");
    echo();
    getnstr(buf, sizeof(buf) - 1);
    noecho();
    nonl();

    if (isdigit(UChar(buf[0]))) {
	delay = atoi(buf) * 100;
    } else {
	delay = -1;
    }
    raw();
    move(5, 0);
    return delay;
}

static void
finish_getch_test(void)
{
#ifdef NCURSES_MOUSE_VERSION
    mousemask(0, (mmask_t *) 0);
#endif
    erase();
    noraw();
    nl();
    endwin();
}

static void
getch_test(void)
{
    int delay = begin_getch_test();

    slk_restore();
    wgetch_test(0, stdscr, delay);
    forget_boxes();
    finish_getch_test();
}

#if USE_WIDEC_SUPPORT
/*
 * For wget_wch_test(), we create pairs of windows - one for a box, one for text.
 * Resize both and paint the box in the parent.
 */
#if defined(KEY_RESIZE) && HAVE_WRESIZE
static void
resize_wide_boxes(unsigned level, WINDOW *win)
{
    unsigned n;
    int base = 5;
    int high = LINES - base;
    int wide = COLS;

    touchwin(stdscr);
    wnoutrefresh(stdscr);

    slk_repaint();

    for (n = 0; n < level; ++n) {
	wresize(winstack[n].frame, high, wide);
	wresize(winstack[n].text, high - 2, wide - 2);
	high -= 2;
	wide -= 2;
	werase(winstack[n].text);
	box_set(winstack[n].frame, 0, 0);
	wnoutrefresh(winstack[n].frame);
	wprintw(winstack[n].text,
		"size %dx%d\n",
		getmaxy(winstack[n].text),
		getmaxx(winstack[n].text));
	wnoutrefresh(winstack[n].text);
	if (winstack[n].text == win)
	    break;
    }
    doupdate();
}
#endif /* KEY_RESIZE */

static char *
wcstos(const wchar_t *src)
{
    int need;
    char *result = 0;
    const wchar_t *tmp = src;
#ifndef state_unused
    mbstate_t state;
#endif

    reset_wchars(state);
    if ((need = (int) count_wchars(tmp, 0, &state)) > 0) {
	unsigned have = (unsigned) need;
	if ((result = typeCalloc(char, have + 1)) != 0) {
	    tmp = src;
	    if (trans_wchars(result, tmp, have, &state) != have) {
		free(result);
		result = 0;
	    }
	}
    }
    return result;
}

static void
wget_wch_test(unsigned level, WINDOW *win, int delay)
{
    wchar_t wchar_buf[BUFSIZ];
    wint_t wint_buf[BUFSIZ];
    int first_y, first_x;
    wint_t c;
    int incount = 0;
    bool flags[256];
    bool blocking = (delay < 0);
    int y, x, code;
    char *temp;

    memset(flags, FALSE, sizeof(flags));
    flags[UChar('k')] = (win == stdscr);

    setup_getch(win, flags);
    wtimeout(win, delay);
    getyx(win, first_y, first_x);

    wgetch_help(win, flags);
    wsetscrreg(win, first_y, getmaxy(win) - 1);
    scrollok(win, TRUE);

    for (;;) {
	while ((code = wGet_wchar(win, &c)) == ERR) {
	    incount++;
	    if (blocking) {
		(void) wprintw(win, "%05d: input error", incount);
		break;
	    } else {
		(void) wprintw(win, "%05d: input timed out", incount);
	    }
	    wgetch_wrap(win, first_y);
	}
	if (code == ERR && blocking) {
	    wprintw(win, "ERR");
	    wgetch_wrap(win, first_y);
	} else if (isQuit((int) c)) {
	    break;
	} else if (c == 'e') {
	    flags[UChar('e')] = !flags[UChar('e')];
	    setup_getch(win, flags);
	    wgetch_help(win, flags);
	} else if (c == 'g') {
	    waddstr(win, "getstr test: ");
	    echo();
	    code = wgetn_wstr(win, wint_buf, sizeof(wint_buf) - 1);
	    noecho();
	    if (code == ERR) {
		wprintw(win, "wgetn_wstr returns an error.");
	    } else {
		int n;
		for (n = 0; (wchar_buf[n] = (wchar_t) wint_buf[n]) != 0; ++n) {
		    ;
		}
		if ((temp = wcstos(wchar_buf)) != 0) {
		    wprintw(win, "I saw %d characters:\n\t`%s'.",
			    (int) wcslen(wchar_buf), temp);
		    free(temp);
		} else {
		    wprintw(win, "I saw %d characters (cannot convert).",
			    (int) wcslen(wchar_buf));
		}
	    }
	    wclrtoeol(win);
	    wgetch_wrap(win, first_y);
	} else if (c == 'k') {
	    flags[UChar('k')] = !flags[UChar('k')];
	    setup_getch(win, flags);
	    wgetch_help(win, flags);
	} else if (c == 'm') {
	    flags[UChar('m')] = !flags[UChar('m')];
	    setup_getch(win, flags);
	    wgetch_help(win, flags);
	} else if (c == 's') {
	    ShellOut(TRUE);
	} else if (c == 'w') {
	    int high = getmaxy(win) - 1 - first_y + 1;
	    int wide = getmaxx(win) - first_x;
	    int old_y, old_x;
	    int new_y = first_y + getbegy(win);
	    int new_x = first_x + getbegx(win);

	    getyx(win, old_y, old_x);
	    if (high > 2 && wide > 2) {
		WINDOW *wb = newwin(high, wide, new_y, new_x);
		WINDOW *wi = newwin(high - 2, wide - 2, new_y + 1, new_x + 1);

		box_set(wb, 0, 0);
		wrefresh(wb);
		wmove(wi, 0, 0);
		remember_boxes(level, wi, wb);
		wget_wch_test(level + 1, wi, delay);
		delwin(wi);
		delwin(wb);

		wgetch_help(win, flags);
		wmove(win, old_y, old_x);
		touchwin(win);
		wrefresh(win);
	    }
#ifdef SIGTSTP
	} else if (c == 'z') {
	    kill(getpid(), SIGTSTP);
#endif
	} else {
	    wprintw(win, "Key pressed: %04o ", (int) c);
#ifdef NCURSES_MOUSE_VERSION
	    if (c == KEY_MOUSE) {
		MEVENT event;

		getmouse(&event);
		wprintw(win, "KEY_MOUSE, %s", mouse_decode(&event));
		getyx(win, y, x);
		move(event.y, event.x);
		addch('*');
		wmove(win, y, x);
	    } else
#endif /* NCURSES_MOUSE_VERSION */
	    if (code == KEY_CODE_YES) {
#if defined(KEY_RESIZE) && HAVE_WRESIZE
		if (c == KEY_RESIZE) {
		    resize_wide_boxes(level, win);
		}
#endif
		(void) waddstr(win, key_name((wchar_t) c));
	    } else {
		if (c < 256 && iscntrl(c)) {
		    (void) wprintw(win, "%s (control character)", unctrl(c));
		} else {
		    wchar_t c2 = (wchar_t) c;
		    waddnwstr(win, &c2, 1);
		    (void) wprintw(win, " = %#x (printable character)", (unsigned) c);
		}
	    }
	    wgetch_wrap(win, first_y);
	}
    }

    wtimeout(win, -1);
}

static void
get_wch_test(void)
{
    int delay = begin_getch_test();

    slk_restore();
    wget_wch_test(0, stdscr, delay);
    forget_boxes();
    finish_getch_test();
}
#endif

/****************************************************************************
 *
 * Character attributes test
 *
 ****************************************************************************/

#if HAVE_SETUPTERM || HAVE_TGETENT
#define get_ncv() TIGETNUM("ncv","NC")
#define get_xmc() TIGETNUM("xmc","sg")
#else
#define get_ncv() -1
#define get_xmc() -1
#endif

#if !HAVE_TERMATTRS
static chtype
my_termattrs(void)
{
    static int first = TRUE;
    static chtype result = 0;

    if (first) {
#if !HAVE_TIGETSTR
	char buffer[4096];
	char parsed[4096];
	char *area_pointer = parsed;

	tgetent(buffer, getenv("TERM"));
#endif

	if (TIGETSTR("smso", "so"))
	    result |= A_STANDOUT;
	if (TIGETSTR("smul", "us"))
	    result |= A_UNDERLINE;
	if (TIGETSTR("rev", "mr"))
	    result |= A_REVERSE;
	if (TIGETSTR("blink", "mb"))
	    result |= A_BLINK;
	if (TIGETSTR("dim", "mh"))
	    result |= A_DIM;
	if (TIGETSTR("bold", "md"))
	    result |= A_BOLD;
	if (TIGETSTR("smacs", "ac"))
	    result |= A_ALTCHARSET;

	first = FALSE;
    }
    return result;
}
#define termattrs() my_termattrs()
#endif

#define MAX_ATTRSTRING 31
#define LEN_ATTRSTRING 26

static char attr_test_string[MAX_ATTRSTRING + 1];

static void
attr_legend(WINDOW *helpwin)
{
    int row = 1;
    int col = 1;

    mvwprintw(helpwin, row++, col,
	      "ESC to exit.");
    mvwprintw(helpwin, row++, col,
	      "^L repaints.");
    ++row;
    mvwprintw(helpwin, row++, col,
	      "Modify the test strings:");
    mvwprintw(helpwin, row++, col,
	      "  A digit sets gaps on each side of displayed attributes");
    mvwprintw(helpwin, row++, col,
	      "  </> shifts the text left/right. ");
    ++row;
    mvwprintw(helpwin, row++, col,
	      "Toggles:");
    if (use_colors) {
	mvwprintw(helpwin, row++, col,
		  "  f/F/b/F toggle foreground/background background color");
	mvwprintw(helpwin, row++, col,
		  "  t/T     toggle text/background color attribute");
    }
    mvwprintw(helpwin, row++, col,
	      "  a/A     toggle ACS (alternate character set) mapping");
    mvwprintw(helpwin, row++, col,
	      "  v/V     toggle video attribute to combine with each line");
}

static void
show_color_attr(int fg, int bg, int tx)
{
    if (use_colors) {
	printw("  Colors (fg %d, bg %d", fg, bg);
	if (tx >= 0)
	    printw(", text %d", tx);
	printw("),");
    }
}

static bool
cycle_color_attr(int ch, short *fg, short *bg, short *tx)
{
    bool error = FALSE;

    if (use_colors) {
	switch (ch) {
	case 'f':
	    *fg = (short) (*fg + 1);
	    break;
	case 'F':
	    *fg = (short) (*fg - 1);
	    break;
	case 'b':
	    *bg = (short) (*bg + 1);
	    break;
	case 'B':
	    *bg = (short) (*bg - 1);
	    break;
	case 't':
	    *tx = (short) (*tx + 1);
	    break;
	case 'T':
	    *tx = (short) (*tx - 1);
	    break;
	default:
	    beep();
	    error = TRUE;
	    break;
	}
	if (*fg >= COLORS)
	    *fg = (short) min_colors;
	if (*fg < min_colors)
	    *fg = (short) (COLORS - 1);
	if (*bg >= COLORS)
	    *bg = (short) min_colors;
	if (*bg < min_colors)
	    *bg = (short) (COLORS - 1);
	if (*tx >= COLORS)
	    *tx = -1;
	if (*tx < -1)
	    *tx = (short) (COLORS - 1);
    } else {
	beep();
	error = TRUE;
    }
    return error;
}

static void
adjust_attr_string(int adjust)
{
    int first = ((int) UChar(attr_test_string[0])) + adjust;
    int last = first + LEN_ATTRSTRING;

    if (first >= ' ' && last <= '~') {	/* 32..126 */
	int j, k;
	for (j = 0, k = first; j < MAX_ATTRSTRING && k <= last; ++j, ++k) {
	    attr_test_string[j] = (char) k;
	    if (((k + 1 - first) % 5) == 0) {
		if (++j >= MAX_ATTRSTRING)
		    break;
		attr_test_string[j] = ' ';
	    }
	}
	while (j < MAX_ATTRSTRING)
	    attr_test_string[j++] = ' ';
	attr_test_string[j] = '\0';
    } else {
	beep();
    }
}

static void
init_attr_string(void)
{
    attr_test_string[0] = 'a';
    adjust_attr_string(0);
}

static int
show_attr(int row, int skip, bool arrow, chtype attr, const char *name)
{
    int ncv = get_ncv();
    chtype test = attr & (chtype) (~A_ALTCHARSET);

    if (arrow)
	mvprintw(row, 5, "-->");
    mvprintw(row, 8, "%s mode:", name);
    mvprintw(row, 24, "|");
    if (skip)
	printw("%*s", skip, " ");
    /*
     * Just for testing, write text using the alternate character set one
     * character at a time (to pass its rendition directly), and use the
     * string operation for the other attributes.
     */
    if (attr & A_ALTCHARSET) {
	const char *s;
	chtype ch;

	for (s = attr_test_string; *s != '\0'; ++s) {
	    ch = UChar(*s);
	    addch(ch | attr);
	}
    } else {
	attrset(attr);
	addstr(attr_test_string);
	attroff(attr);
    }
    if (skip)
	printw("%*s", skip, " ");
    printw("|");
    if (test != A_NORMAL) {
	if (!(termattrs() & test)) {
	    printw(" (N/A)");
	} else {
	    if (ncv > 0 && (getbkgd(stdscr) & A_COLOR)) {
		static const chtype table[] =
		{
		    A_STANDOUT,
		    A_UNDERLINE,
		    A_REVERSE,
		    A_BLINK,
		    A_DIM,
		    A_BOLD,
#ifdef A_INVIS
		    A_INVIS,
#endif
		    A_PROTECT,
		    A_ALTCHARSET
		};
		unsigned n;
		bool found = FALSE;
		for (n = 0; n < SIZEOF(table); n++) {
		    if ((table[n] & attr) != 0
			&& ((1 << n) & ncv) != 0) {
			found = TRUE;
			break;
		    }
		}
		if (found)
		    printw(" (NCV)");
	    }
	    if ((termattrs() & test) != test)
		printw(" (Part)");
	}
    }
    return row + 2;
}
/* *INDENT-OFF* */
static const struct {
    chtype			attr;
    NCURSES_CONST char *	name;
} attrs_to_test[] = {
    { A_STANDOUT,	"STANDOUT" },
    { A_REVERSE,	"REVERSE" },
    { A_BOLD,		"BOLD" },
    { A_UNDERLINE,	"UNDERLINE" },
    { A_DIM,		"DIM" },
    { A_BLINK,		"BLINK" },
    { A_PROTECT,	"PROTECT" },
#ifdef A_INVIS
    { A_INVIS,		"INVISIBLE" },
#endif
    { A_NORMAL,		"NORMAL" },
};
/* *INDENT-ON* */

static bool
attr_getc(int *skip, short *fg, short *bg, short *tx, int *ac, unsigned *kc)
{
    bool result = TRUE;
    bool error = FALSE;
    WINDOW *helpwin;

    do {
	int ch = Getchar();

	error = FALSE;
	if (ch < 256 && isdigit(ch)) {
	    *skip = (ch - '0');
	} else {
	    switch (ch) {
	    case CTRL('L'):
		Repaint();
		break;
	    case '?':
		if ((helpwin = newwin(LINES - 1, COLS - 2, 0, 0)) != 0) {
		    box(helpwin, 0, 0);
		    attr_legend(helpwin);
		    wGetchar(helpwin);
		    delwin(helpwin);
		}
		break;
	    case 'a':
		*ac = 0;
		break;
	    case 'A':
		*ac = A_ALTCHARSET;
		break;
	    case 'v':
		if (*kc == 0)
		    *kc = SIZEOF(attrs_to_test) - 1;
		else
		    *kc -= 1;
		break;
	    case 'V':
		*kc += 1;
		if (*kc >= SIZEOF(attrs_to_test))
		    *kc = 0;
		break;
	    case '<':
		adjust_attr_string(-1);
		break;
	    case '>':
		adjust_attr_string(1);
		break;
	    case case_QUIT:
		result = FALSE;
		break;
	    default:
		error = cycle_color_attr(ch, fg, bg, tx);
		break;
	    }
	}
    } while (error);
    return result;
}

static void
attr_test(void)
/* test text attributes */
{
    int n;
    int skip = get_xmc();
    short fg = COLOR_BLACK;	/* color pair 0 is special */
    short bg = COLOR_BLACK;
    short tx = -1;
    int ac = 0;
    unsigned j, k;

    if (skip < 0)
	skip = 0;

    n = skip;			/* make it easy */
    k = SIZEOF(attrs_to_test) - 1;
    init_attr_string();

    do {
	int row = 2;
	chtype normal = A_NORMAL | BLANK;
	chtype extras = (chtype) ac;

	if (use_colors) {
	    short pair = (short) (fg != COLOR_BLACK || bg != COLOR_BLACK);
	    if (pair != 0) {
		pair = 1;
		if (init_pair(pair, fg, bg) == ERR) {
		    beep();
		} else {
		    normal |= COLOR_PAIR(pair);
		}
	    }
	    if (tx >= 0) {
		pair = 2;
		if (init_pair(pair, tx, bg) == ERR) {
		    beep();
		} else {
		    extras |= COLOR_PAIR(pair);
		}
	    }
	}
	bkgd(normal);
	bkgdset(normal);
	erase();

	box(stdscr, 0, 0);
	mvaddstr(0, 20, "Character attribute test display");

	for (j = 0; j < SIZEOF(attrs_to_test); ++j) {
	    bool arrow = (j == k);
	    row = show_attr(row, n, arrow,
			    extras |
			    attrs_to_test[j].attr |
			    attrs_to_test[k].attr,
			    attrs_to_test[j].name);
	}

	mvprintw(row, 8,
		 "This terminal does %shave the magic-cookie glitch",
		 get_xmc() > -1 ? "" : "not ");
	mvprintw(row + 1, 8, "Enter '?' for help.");
	show_color_attr(fg, bg, tx);
	printw("  ACS (%d)", ac != 0);

	refresh();
    } while (attr_getc(&n, &fg, &bg, &tx, &ac, &k));

    bkgdset(A_NORMAL | BLANK);
    erase();
    endwin();
}

#if USE_WIDEC_SUPPORT
static wchar_t wide_attr_test_string[MAX_ATTRSTRING + 1];

static void
wide_adjust_attr_string(int adjust)
{
    int first = ((int) UChar(wide_attr_test_string[0])) + adjust;
    int last = first + LEN_ATTRSTRING;

    if (first >= ' ' && last <= '~') {	/* 32..126 */
	int j, k;
	for (j = 0, k = first; j < MAX_ATTRSTRING && k <= last; ++j, ++k) {
	    wide_attr_test_string[j] = k;
	    if (((k + 1 - first) % 5) == 0) {
		if (++j >= MAX_ATTRSTRING)
		    break;
		wide_attr_test_string[j] = ' ';
	    }
	}
	while (j < MAX_ATTRSTRING)
	    wide_attr_test_string[j++] = ' ';
	wide_attr_test_string[j] = '\0';
    } else {
	beep();
    }
}

static void
wide_init_attr_string(void)
{
    wide_attr_test_string[0] = 'a';
    wide_adjust_attr_string(0);
}

static void
set_wide_background(short pair)
{
    cchar_t normal;
    wchar_t blank[2];

    blank[0] = ' ';
    blank[1] = 0;
    setcchar(&normal, blank, A_NORMAL, pair, 0);
    bkgrnd(&normal);
    bkgrndset(&normal);
}

static attr_t
get_wide_background(void)
{
    attr_t result = A_NORMAL;
    attr_t attr;
    cchar_t ch;
    short pair;
    wchar_t wch[10];

    if (getbkgrnd(&ch) != ERR) {
	if (getcchar(&ch, wch, &attr, &pair, 0) != ERR) {
	    result = attr;
	}
    }
    return result;
}

static int
wide_show_attr(int row, int skip, bool arrow, chtype attr, short pair, const char *name)
{
    int ncv = get_ncv();
    chtype test = attr & ~WA_ALTCHARSET;

    if (arrow)
	mvprintw(row, 5, "-->");
    mvprintw(row, 8, "%s mode:", name);
    mvprintw(row, 24, "|");
    if (skip)
	printw("%*s", skip, " ");

    /*
     * Just for testing, write text using the alternate character set one
     * character at a time (to pass its rendition directly), and use the
     * string operation for the other attributes.
     */
    if (attr & WA_ALTCHARSET) {
	const wchar_t *s;
	cchar_t ch;

	for (s = wide_attr_test_string; *s != L'\0'; ++s) {
	    wchar_t fill[2];
	    fill[0] = *s;
	    fill[1] = L'\0';
	    setcchar(&ch, fill, attr, pair, 0);
	    add_wch(&ch);
	}
    } else {
	attr_t old_attr;
	short old_pair;

	attr_get(&old_attr, &old_pair, 0);
	attr_set(attr, pair, 0);
	addwstr(wide_attr_test_string);
	attr_set(old_attr, old_pair, 0);
    }
    if (skip)
	printw("%*s", skip, " ");
    printw("|");
    if (test != A_NORMAL) {
	if (!(term_attrs() & test)) {
	    printw(" (N/A)");
	} else {
	    if (ncv > 0 && (get_wide_background() & A_COLOR)) {
		static const attr_t table[] =
		{
		    WA_STANDOUT,
		    WA_UNDERLINE,
		    WA_REVERSE,
		    WA_BLINK,
		    WA_DIM,
		    WA_BOLD,
		    WA_INVIS,
		    WA_PROTECT,
		    WA_ALTCHARSET
		};
		unsigned n;
		bool found = FALSE;
		for (n = 0; n < SIZEOF(table); n++) {
		    if ((table[n] & attr) != 0
			&& ((1 << n) & ncv) != 0) {
			found = TRUE;
			break;
		    }
		}
		if (found)
		    printw(" (NCV)");
	    }
	    if ((term_attrs() & test) != test)
		printw(" (Part)");
	}
    }
    return row + 2;
}

static bool
wide_attr_getc(int *skip, short *fg, short *bg, short *tx, int *ac, unsigned *kc)
{
    bool result = TRUE;
    bool error = FALSE;
    WINDOW *helpwin;

    do {
	int ch = Getchar();

	error = FALSE;
	if (ch < 256 && isdigit(ch)) {
	    *skip = (ch - '0');
	} else {
	    switch (ch) {
	    case CTRL('L'):
		Repaint();
		break;
	    case '?':
		if ((helpwin = newwin(LINES - 1, COLS - 2, 0, 0)) != 0) {
		    box_set(helpwin, 0, 0);
		    attr_legend(helpwin);
		    wGetchar(helpwin);
		    delwin(helpwin);
		}
		break;
	    case 'a':
		*ac = 0;
		break;
	    case 'A':
		*ac = A_ALTCHARSET;
		break;
	    case 'v':
		if (*kc == 0)
		    *kc = SIZEOF(attrs_to_test) - 1;
		else
		    *kc -= 1;
		break;
	    case 'V':
		*kc += 1;
		if (*kc >= SIZEOF(attrs_to_test))
		    *kc = 0;
		break;
	    case '<':
		wide_adjust_attr_string(-1);
		break;
	    case '>':
		wide_adjust_attr_string(1);
		break;
	    case case_QUIT:
		result = FALSE;
		break;
	    default:
		error = cycle_color_attr(ch, fg, bg, tx);
		break;
	    }
	}
    } while (error);
    return result;
}

static void
wide_attr_test(void)
/* test text attributes using wide-character calls */
{
    int n;
    int skip = get_xmc();
    short fg = COLOR_BLACK;	/* color pair 0 is special */
    short bg = COLOR_BLACK;
    short tx = -1;
    int ac = 0;
    unsigned j, k;

    if (skip < 0)
	skip = 0;

    n = skip;			/* make it easy */
    k = SIZEOF(attrs_to_test) - 1;
    wide_init_attr_string();

    do {
	int row = 2;
	short pair = 0;
	short extras = 0;

	if (use_colors) {
	    pair = (short) (fg != COLOR_BLACK || bg != COLOR_BLACK);
	    if (pair != 0) {
		pair = 1;
		if (init_pair(pair, fg, bg) == ERR) {
		    beep();
		}
	    }
	    extras = pair;
	    if (tx >= 0) {
		extras = 2;
		if (init_pair(extras, tx, bg) == ERR) {
		    beep();
		}
	    }
	}
	set_wide_background(pair);
	erase();

	box_set(stdscr, 0, 0);
	mvaddstr(0, 20, "Character attribute test display");

	for (j = 0; j < SIZEOF(attrs_to_test); ++j) {
	    row = wide_show_attr(row, n, j == k,
				 ac |
				 attrs_to_test[j].attr |
				 attrs_to_test[k].attr,
				 extras,
				 attrs_to_test[j].name);
	}

	mvprintw(row, 8,
		 "This terminal does %shave the magic-cookie glitch",
		 get_xmc() > -1 ? "" : "not ");
	mvprintw(row + 1, 8, "Enter '?' for help.");
	show_color_attr(fg, bg, tx);
	printw("  ACS (%d)", ac != 0);

	refresh();
    } while (wide_attr_getc(&n, &fg, &bg, &tx, &ac, &k));

    set_wide_background(0);
    erase();
    endwin();
}
#endif

/****************************************************************************
 *
 * Color support tests
 *
 ****************************************************************************/

static NCURSES_CONST char *the_color_names[] =
{
    "black",
    "red",
    "green",
    "yellow",
    "blue",
    "magenta",
    "cyan",
    "white",
    "BLACK",
    "RED",
    "GREEN",
    "YELLOW",
    "BLUE",
    "MAGENTA",
    "CYAN",
    "WHITE"
};

static void
show_color_name(int y, int x, int color, bool wide)
{
    if (move(y, x) != ERR) {
	char temp[80];
	int width = 8;

	if (wide) {
	    sprintf(temp, "%02d", color);
	    width = 4;
	} else if (color >= 8) {
	    sprintf(temp, "[%02d]", color);
	} else {
	    strcpy(temp, the_color_names[color]);
	}
	printw("%-*.*s", width, width, temp);
    }
}

static void
color_legend(WINDOW *helpwin, bool wide)
{
    int row = 1;
    int col = 1;

    mvwprintw(helpwin, row++, col,
	      "ESC to exit.");
    ++row;
    mvwprintw(helpwin, row++, col,
	      "Use up/down arrow to scroll through the display if it is");
    mvwprintw(helpwin, row++, col,
	      "longer than one screen. Control/N and Control/P can be used");
    mvwprintw(helpwin, row++, col,
	      "in place of up/down arrow.  Use pageup/pagedown to scroll a");
    mvwprintw(helpwin, row++, col,
	      "full screen; control/B and control/F can be used here.");
    ++row;
    mvwprintw(helpwin, row++, col,
	      "Toggles:");
    mvwprintw(helpwin, row++, col,
	      "  a/A     toggle altcharset off/on");
    mvwprintw(helpwin, row++, col,
	      "  b/B     toggle bold off/on");
    mvwprintw(helpwin, row++, col,
	      "  n/N     toggle text/number on/off");
    mvwprintw(helpwin, row++, col,
	      "  w/W     toggle width between 8/16 colors");
#if USE_WIDEC_SUPPORT
    if (wide) {
	mvwprintw(helpwin, row++, col,
		  "Wide characters:");
	mvwprintw(helpwin, row++, col,
		  "  x/X     toggle text between ASCII and wide-character");
    }
#else
    (void) wide;
#endif
}

#define set_color_test(name, value) if (name != value) { name = value; base_row = 0; }

/* generate a color test pattern */
static void
color_test(void)
{
    short i;
    int top = 0, width;
    int base_row = 0;
    int grid_top = top + 3;
    int page_size = (LINES - grid_top);
    int pairs_max = PAIR_NUMBER(A_COLOR) + 1;
    int row_limit;
    int per_row;
    char numbered[80];
    const char *hello;
    bool done = FALSE;
    bool opt_acsc = FALSE;
    bool opt_bold = FALSE;
    bool opt_wide = FALSE;
    bool opt_nums = FALSE;
    WINDOW *helpwin;

    if (pairs_max > COLOR_PAIRS)
	pairs_max = COLOR_PAIRS;

    while (!done) {
	int shown = 0;

	/* this assumes an 80-column line */
	if (opt_wide) {
	    width = 4;
	    hello = "Test";
	    per_row = (COLORS > 8) ? 16 : 8;
	} else {
	    width = 8;
	    hello = "Hello";
	    per_row = 8;
	}

	row_limit = (pairs_max + per_row - 1) / per_row;

	move(0, 0);
	(void) printw("There are %d color pairs and %d colors\n",
		      pairs_max, COLORS);

	clrtobot();
	(void) mvprintw(top + 1, 0,
			"%dx%d matrix of foreground/background colors, bold *%s*\n",
			row_limit,
			per_row,
			opt_bold ? "on" : "off");

	/* show color names/numbers across the top */
	for (i = 0; i < per_row; i++)
	    show_color_name(top + 2, (i + 1) * width, i, opt_wide);

	/* show a grid of colors, with color names/ numbers on the left */
	for (i = (short) (base_row * per_row); i < pairs_max; i++) {
	    int row = grid_top + (i / per_row) - base_row;
	    int col = (i % per_row + 1) * width;
	    short pair = i;

	    if (row >= 0 && move(row, col) != ERR) {
		short fg = (short) (i % COLORS);
		short bg = (short) (i / COLORS);

		init_pair(pair, fg, bg);
		attron((attr_t) COLOR_PAIR(pair));
		if (opt_acsc)
		    attron((attr_t) A_ALTCHARSET);
		if (opt_bold)
		    attron((attr_t) A_BOLD);

		if (opt_nums) {
		    sprintf(numbered, "{%02X}", i);
		    hello = numbered;
		}
		printw("%-*.*s", width, width, hello);
		attrset(A_NORMAL);

		if ((i % per_row) == 0 && (i % COLORS) == 0) {
		    show_color_name(row, 0, i / COLORS, opt_wide);
		}
		++shown;
	    } else if (shown) {
		break;
	    }
	}

	switch (wGetchar(stdscr)) {
	case 'a':
	    opt_acsc = FALSE;
	    break;
	case 'A':
	    opt_acsc = TRUE;
	    break;
	case 'b':
	    opt_bold = FALSE;
	    break;
	case 'B':
	    opt_bold = TRUE;
	    break;
	case 'n':
	    opt_nums = FALSE;
	    break;
	case 'N':
	    opt_nums = TRUE;
	    break;
	case case_QUIT:
	    done = TRUE;
	    continue;
	case 'w':
	    set_color_test(opt_wide, FALSE);
	    break;
	case 'W':
	    set_color_test(opt_wide, TRUE);
	    break;
	case CTRL('p'):
	case KEY_UP:
	    if (base_row <= 0) {
		beep();
	    } else {
		base_row -= 1;
	    }
	    break;
	case CTRL('n'):
	case KEY_DOWN:
	    if (base_row + page_size >= row_limit) {
		beep();
	    } else {
		base_row += 1;
	    }
	    break;
	case CTRL('b'):
	case KEY_PREVIOUS:
	case KEY_PPAGE:
	    if (base_row <= 0) {
		beep();
	    } else {
		base_row -= (page_size - 1);
		if (base_row < 0)
		    base_row = 0;
	    }
	    break;
	case CTRL('f'):
	case KEY_NEXT:
	case KEY_NPAGE:
	    if (base_row + page_size >= row_limit) {
		beep();
	    } else {
		base_row += page_size - 1;
		if (base_row + page_size >= row_limit) {
		    base_row = row_limit - page_size - 1;
		}
	    }
	    break;
	case '?':
	    if ((helpwin = newwin(LINES - 1, COLS - 2, 0, 0)) != 0) {
		box(helpwin, 0, 0);
		color_legend(helpwin, FALSE);
		wGetchar(helpwin);
		delwin(helpwin);
	    }
	    break;
	default:
	    beep();
	    continue;
	}
    }

    erase();
    endwin();
}

#if USE_WIDEC_SUPPORT
/* generate a color test pattern */
static void
wide_color_test(void)
{
    int c;
    int i;
    int top = 0, width;
    int base_row = 0;
    int grid_top = top + 3;
    int page_size = (LINES - grid_top);
    int pairs_max = COLOR_PAIRS;
    int row_limit;
    int per_row;
    char numbered[80];
    const char *hello;
    bool done = FALSE;
    bool opt_acsc = FALSE;
    bool opt_bold = FALSE;
    bool opt_wide = FALSE;
    bool opt_nums = FALSE;
    bool opt_xchr = FALSE;
    wchar_t buffer[10];
    WINDOW *helpwin;

    while (!done) {
	int shown = 0;

	/* this assumes an 80-column line */
	if (opt_wide) {
	    width = 4;
	    hello = "Test";
	    per_row = (COLORS > 8) ? 16 : 8;
	} else {
	    width = 8;
	    hello = "Hello";
	    per_row = 8;
	}
	if (opt_xchr) {
	    make_fullwidth_text(buffer, hello);
	    width *= 2;
	    per_row /= 2;
	} else {
	    make_narrow_text(buffer, hello);
	}

	row_limit = (pairs_max + per_row - 1) / per_row;

	move(0, 0);
	(void) printw("There are %d color pairs and %d colors\n",
		      pairs_max, COLORS);

	clrtobot();
	(void) mvprintw(top + 1, 0,
			"%dx%d matrix of foreground/background colors, bold *%s*\n",
			row_limit,
			per_row,
			opt_bold ? "on" : "off");

	/* show color names/numbers across the top */
	for (i = 0; i < per_row; i++)
	    show_color_name(top + 2, (i + 1) * width, i, opt_wide);

	/* show a grid of colors, with color names/ numbers on the left */
	for (i = (base_row * per_row); i < pairs_max; i++) {
	    int row = grid_top + (i / per_row) - base_row;
	    int col = (i % per_row + 1) * width;
	    short pair = (short) i;

	    if (row >= 0 && move(row, col) != ERR) {
		init_pair(pair, (short) (i % COLORS), (short) (i / COLORS));
		color_set(pair, NULL);
		if (opt_acsc)
		    attr_on((attr_t) A_ALTCHARSET, NULL);
		if (opt_bold)
		    attr_on((attr_t) A_BOLD, NULL);

		if (opt_nums) {
		    sprintf(numbered, "{%02X}", i);
		    if (opt_xchr) {
			make_fullwidth_text(buffer, numbered);
		    } else {
			make_narrow_text(buffer, numbered);
		    }
		}
		addnwstr(buffer, width);
		attr_set(A_NORMAL, 0, NULL);

		if ((i % per_row) == 0 && (i % COLORS) == 0) {
		    show_color_name(row, 0, i / COLORS, opt_wide);
		}
		++shown;
	    } else if (shown) {
		break;
	    }
	}

	switch (c = wGetchar(stdscr)) {
	case 'a':
	    opt_acsc = FALSE;
	    break;
	case 'A':
	    opt_acsc = TRUE;
	    break;
	case 'b':
	    opt_bold = FALSE;
	    break;
	case 'B':
	    opt_bold = TRUE;
	    break;
	case 'n':
	    opt_nums = FALSE;
	    break;
	case 'N':
	    opt_nums = TRUE;
	    break;
	case case_QUIT:
	    done = TRUE;
	    continue;
	case 'w':
	    set_color_test(opt_wide, FALSE);
	    break;
	case 'W':
	    set_color_test(opt_wide, TRUE);
	    break;
	case 'x':
	    opt_xchr = FALSE;
	    break;
	case 'X':
	    opt_xchr = TRUE;
	    break;
	case CTRL('p'):
	case KEY_UP:
	    if (base_row <= 0) {
		beep();
	    } else {
		base_row -= 1;
	    }
	    break;
	case CTRL('n'):
	case KEY_DOWN:
	    if (base_row + page_size >= row_limit) {
		beep();
	    } else {
		base_row += 1;
	    }
	    break;
	case CTRL('b'):
	case KEY_PREVIOUS:
	case KEY_PPAGE:
	    if (base_row <= 0) {
		beep();
	    } else {
		base_row -= (page_size - 1);
		if (base_row < 0)
		    base_row = 0;
	    }
	    break;
	case CTRL('f'):
	case KEY_NEXT:
	case KEY_NPAGE:
	    if (base_row + page_size >= row_limit) {
		beep();
	    } else {
		base_row += page_size - 1;
		if (base_row + page_size >= row_limit) {
		    base_row = row_limit - page_size - 1;
		}
	    }
	    break;
	case '?':
	    if ((helpwin = newwin(LINES - 1, COLS - 2, 0, 0)) != 0) {
		box(helpwin, 0, 0);
		color_legend(helpwin, TRUE);
		wGetchar(helpwin);
		delwin(helpwin);
	    }
	    break;
	default:
	    beep();
	    continue;
	}
    }

    erase();
    endwin();
}
#endif /* USE_WIDEC_SUPPORT */

static void
change_color(short current, int field, int value, int usebase)
{
    short red, green, blue;

    color_content(current, &red, &green, &blue);

    switch (field) {
    case 0:
	red = (short) (usebase ? (red + value) : value);
	break;
    case 1:
	green = (short) (usebase ? (green + value) : value);
	break;
    case 2:
	blue = (short) (usebase ? (blue + value) : value);
	break;
    }

    if (init_color(current, red, green, blue) == ERR)
	beep();
}

static void
init_all_colors(void)
{
    short c;

    for (c = 0; c < COLORS; ++c)
	init_color(c,
		   all_colors[c].red,
		   all_colors[c].green,
		   all_colors[c].blue);
}

#define scaled_rgb(n) ((255 * (n)) / 1000)

static void
color_edit(void)
/* display the color test pattern, without trying to edit colors */
{
    int i;
    int current = 0;
    int this_c = 0, value = 0, field = 0;
    int last_c;
    int top_color = 0;
    int page_size = (LINES - 6);

    init_all_colors();
    refresh();

    for (i = 0; i < max_colors; i++)
	init_pair((short) i, (short) COLOR_WHITE, (short) i);

    mvprintw(LINES - 2, 0, "Number: %d", value);

    do {
	short red, green, blue;

	attron(A_BOLD);
	mvaddstr(0, 20, "Color RGB Value Editing");
	attroff(A_BOLD);

	for (i = (short) top_color;
	     (i - top_color < page_size)
	     && (i < max_colors); i++) {
	    char numeric[80];

	    sprintf(numeric, "[%d]", i);
	    mvprintw(2 + i - top_color, 0, "%c %-8s:",
		     (i == current ? '>' : ' '),
		     (i < (int) SIZEOF(the_color_names)
		      ? the_color_names[i] : numeric));
	    attrset(COLOR_PAIR(i));
	    addstr("        ");
	    attrset(A_NORMAL);

	    color_content((short) i, &red, &green, &blue);
	    addstr("   R = ");
	    if (current == i && field == 0)
		attron(A_STANDOUT);
	    printw("%04d", red);
	    if (current == i && field == 0)
		attrset(A_NORMAL);
	    addstr(", G = ");
	    if (current == i && field == 1)
		attron(A_STANDOUT);
	    printw("%04d", green);
	    if (current == i && field == 1)
		attrset(A_NORMAL);
	    addstr(", B = ");
	    if (current == i && field == 2)
		attron(A_STANDOUT);
	    printw("%04d", blue);
	    if (current == i && field == 2)
		attrset(A_NORMAL);
	    attrset(A_NORMAL);
	    printw(" ( %3d %3d %3d )",
		   scaled_rgb(red),
		   scaled_rgb(green),
		   scaled_rgb(blue));
	}

	mvaddstr(LINES - 3, 0,
		 "Use up/down to select a color, left/right to change fields.");
	mvaddstr(LINES - 2, 0,
		 "Modify field by typing nnn=, nnn-, or nnn+.  ? for help.");

	move(2 + current - top_color, 0);

	last_c = this_c;
	this_c = Getchar();
	if (this_c < 256 && isdigit(this_c) && !isdigit(last_c))
	    value = 0;

	switch (this_c) {
	case CTRL('b'):
	case KEY_PPAGE:
	    if (current > 0)
		current -= (page_size - 1);
	    else
		beep();
	    break;

	case CTRL('f'):
	case KEY_NPAGE:
	    if (current < (max_colors - 1))
		current += (page_size - 1);
	    else
		beep();
	    break;

	case CTRL('p'):
	case KEY_UP:
	    current = (current == 0 ? (max_colors - 1) : current - 1);
	    break;

	case CTRL('n'):
	case KEY_DOWN:
	    current = (current == (max_colors - 1) ? 0 : current + 1);
	    break;

	case KEY_RIGHT:
	    field = (field == 2 ? 0 : field + 1);
	    break;

	case KEY_LEFT:
	    field = (field == 0 ? 2 : field - 1);
	    break;

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	    value = value * 10 + (this_c - '0');
	    break;

	case '+':
	    change_color((short) current, field, value, 1);
	    break;

	case '-':
	    change_color((short) current, field, -value, 1);
	    break;

	case '=':
	    change_color((short) current, field, value, 0);
	    break;

	case '?':
	    erase();
	    P("                      RGB Value Editing Help");
	    P("");
	    P("You are in the RGB value editor.  Use the arrow keys to select one of");
	    P("the fields in one of the RGB triples of the current colors; the one");
	    P("currently selected will be reverse-video highlighted.");
	    P("");
	    P("To change a field, enter the digits of the new value; they are echoed");
	    P("as entered.  Finish by typing `='.  The change will take effect instantly.");
	    P("To increment or decrement a value, use the same procedure, but finish");
	    P("with a `+' or `-'.");
	    P("");
	    P("Press 'm' to invoke the top-level menu with the current color settings.");
	    P("To quit, do ESC");

	    Pause();
	    erase();
	    break;

	case 'm':
	    endwin();
	    main_menu(FALSE);
	    refresh();
	    break;

	case case_QUIT:
	    break;

	default:
	    beep();
	    break;
	}

	if (current < 0)
	    current = 0;
	if (current >= max_colors)
	    current = max_colors - 1;
	if (current < top_color)
	    top_color = current;
	if (current - top_color >= page_size)
	    top_color = current - (page_size - 1);

	mvprintw(LINES - 1, 0, "Number: %d", value);
	clrtoeol();
    } while
	(!isQuit(this_c));

    erase();

    /*
     * ncurses does not reset each color individually when calling endwin().
     */
    init_all_colors();

    endwin();
}

/****************************************************************************
 *
 * Soft-key label test
 *
 ****************************************************************************/

#if USE_SOFTKEYS

#define SLK_HELP 17
#define SLK_WORK (SLK_HELP + 3)

static void
slk_help(void)
{
    static const char *table[] =
    {
	"Available commands are:"
	,""
	,"^L         -- repaint this message and activate soft keys"
	,"a/d        -- activate/disable soft keys"
	,"c          -- set centered format for labels"
	,"l          -- set left-justified format for labels"
	,"r          -- set right-justified format for labels"
	,"[12345678] -- set label; labels are numbered 1 through 8"
	,"e          -- erase stdscr (should not erase labels)"
	,"s          -- test scrolling of shortened screen"
#if HAVE_SLK_COLOR
	,"F/B        -- cycle through foreground/background colors"
#endif
	,"ESC        -- return to main menu"
	,""
	,"Note: if activating the soft keys causes your terminal to scroll up"
	,"one line, your terminal auto-scrolls when anything is written to the"
	,"last screen position.  The ncurses code does not yet handle this"
	,"gracefully."
    };
    unsigned j;

    move(2, 0);
    for (j = 0; j < SIZEOF(table); ++j) {
	P(table[j]);
    }
    refresh();
}

#if HAVE_SLK_COLOR
static void
call_slk_color(short fg, short bg)
{
    init_pair(1, bg, fg);
    slk_color(1);
    mvprintw(SLK_WORK, 0, "Colors %d/%d\n", fg, bg);
    clrtoeol();
    refresh();
}
#endif

static void
slk_test(void)
/* exercise the soft keys */
{
    int c, fmt = 1;
    char buf[9];
    char *s;
#if HAVE_SLK_COLOR
    short fg = COLOR_BLACK;
    short bg = COLOR_WHITE;
#endif

    c = CTRL('l');
#if HAVE_SLK_COLOR
    if (use_colors) {
	call_slk_color(fg, bg);
    }
#endif

    do {
	move(0, 0);
	switch (c) {
	case CTRL('l'):
	    erase();
	    attron(A_BOLD);
	    mvaddstr(0, 20, "Soft Key Exerciser");
	    attroff(A_BOLD);

	    slk_help();
	    /* fall through */

	case 'a':
	    slk_restore();
	    break;

	case 'e':
	    wclear(stdscr);
	    break;

	case 's':
	    mvprintw(SLK_WORK, 0, "Press Q to stop the scrolling-test: ");
	    while ((c = Getchar()) != 'Q' && (c != ERR))
		addch((chtype) c);
	    break;

	case 'd':
	    slk_clear();
	    break;

	case 'l':
	    fmt = 0;
	    break;

	case 'c':
	    fmt = 1;
	    break;

	case 'r':
	    fmt = 2;
	    break;

	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	    (void) mvaddstr(SLK_WORK, 0, "Please enter the label value: ");
	    strcpy(buf, "");
	    if ((s = slk_label(c - '0')) != 0) {
		strncpy(buf, s, 8);
	    }
	    wGetstring(stdscr, buf, 8);
	    slk_set((c - '0'), buf, fmt);
	    slk_refresh();
	    move(SLK_WORK, 0);
	    clrtobot();
	    break;

	case case_QUIT:
	    goto done;

#if HAVE_SLK_COLOR
	case 'F':
	    if (use_colors) {
		fg = (short) ((fg + 1) % COLORS);
		call_slk_color(fg, bg);
	    }
	    break;
	case 'B':
	    if (use_colors) {
		bg = (short) ((bg + 1) % COLORS);
		call_slk_color(fg, bg);
	    }
	    break;
#endif
#if defined(NCURSES_VERSION) && defined(KEY_RESIZE) && HAVE_WRESIZE
	case KEY_RESIZE:
	    wnoutrefresh(stdscr);
	    break;
#endif

	default:
	    beep();
	}
    } while (!isQuit(c = Getchar()));

  done:
    slk_clear();
    erase();
    endwin();
}

#if USE_WIDEC_SUPPORT
#define SLKLEN 8
static void
wide_slk_test(void)
/* exercise the soft keys */
{
    int c, fmt = 1;
    wchar_t buf[SLKLEN + 1];
    char *s;
    short fg = COLOR_BLACK;
    short bg = COLOR_WHITE;

    c = CTRL('l');
    if (use_colors) {
	call_slk_color(fg, bg);
    }
    do {
	move(0, 0);
	switch (c) {
	case CTRL('l'):
	    erase();
	    attr_on(WA_BOLD, NULL);
	    mvaddstr(0, 20, "Soft Key Exerciser");
	    attr_off(WA_BOLD, NULL);

	    slk_help();
	    /* fall through */

	case 'a':
	    slk_restore();
	    break;

	case 'e':
	    wclear(stdscr);
	    break;

	case 's':
	    mvprintw(SLK_WORK, 0, "Press Q to stop the scrolling-test: ");
	    while ((c = Getchar()) != 'Q' && (c != ERR))
		addch((chtype) c);
	    break;

	case 'd':
	    slk_clear();
	    break;

	case 'l':
	    fmt = 0;
	    break;

	case 'c':
	    fmt = 1;
	    break;

	case 'r':
	    fmt = 2;
	    break;

	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	    (void) mvaddstr(SLK_WORK, 0, "Please enter the label value: ");
	    *buf = 0;
	    if ((s = slk_label(c - '0')) != 0) {
		char *temp = strdup(s);
		size_t used = strlen(temp);
		size_t want = SLKLEN;
		size_t test;
#ifndef state_unused
		mbstate_t state;
#endif

		buf[0] = L'\0';
		while (want > 0 && used != 0) {
		    const char *base = s;
		    reset_mbytes(state);
		    test = count_mbytes(base, 0, &state);
		    if (test == (size_t) -1) {
			temp[--used] = 0;
		    } else if (test > want) {
			temp[--used] = 0;
		    } else {
			reset_mbytes(state);
			trans_mbytes(buf, base, want, &state);
			break;
		    }
		}
		free(temp);
	    }
	    wGet_wstring(stdscr, buf, SLKLEN);
	    slk_wset((c - '0'), buf, fmt);
	    slk_refresh();
	    move(SLK_WORK, 0);
	    clrtobot();
	    break;

	case case_QUIT:
	    goto done;

	case 'F':
	    if (use_colors) {
		fg = (short) ((fg + 1) % COLORS);
		call_slk_color(fg, bg);
	    }
	    break;
	case 'B':
	    if (use_colors) {
		bg = (short) ((bg + 1) % COLORS);
		call_slk_color(fg, bg);
	    }
	    break;
#if defined(NCURSES_VERSION) && defined(KEY_RESIZE) && HAVE_WRESIZE
	case KEY_RESIZE:
	    wnoutrefresh(stdscr);
	    break;
#endif
	default:
	    beep();
	}
    } while (!isQuit(c = Getchar()));

  done:
    slk_clear();
    erase();
    endwin();
}
#endif
#endif /* SLK_INIT */

/****************************************************************************
 *
 * Alternate character-set stuff
 *
 ****************************************************************************/
/* *INDENT-OFF* */
static struct {
    chtype attr;
    const char *name;
} attrs_to_cycle[] = {
    { A_NORMAL,		"normal" },
    { A_BOLD,		"bold" },
    { A_REVERSE,	"reverse" },
    { A_UNDERLINE,	"underline" },
};
/* *INDENT-ON* */

static bool
cycle_attr(int ch, unsigned *at_code, chtype *attr)
{
    bool result = TRUE;

    switch (ch) {
    case 'v':
	if ((*at_code += 1) >= SIZEOF(attrs_to_cycle))
	    *at_code = 0;
	break;
    case 'V':
	if (*at_code == 1)
	    *at_code = SIZEOF(attrs_to_cycle) - 1;
	else
	    *at_code -= 1;
	break;
    default:
	result = FALSE;
	break;
    }
    if (result)
	*attr = attrs_to_cycle[*at_code].attr;
    return result;
}

static bool
cycle_colors(int ch, int *fg, int *bg, short *pair)
{
    bool result = FALSE;

    if (use_colors) {
	result = TRUE;
	switch (ch) {
	case 'F':
	    if ((*fg -= 1) < 0)
		*fg = COLORS - 1;
	    break;
	case 'f':
	    if ((*fg += 1) >= COLORS)
		*fg = 0;
	    break;
	case 'B':
	    if ((*bg -= 1) < 0)
		*bg = COLORS - 1;
	    break;
	case 'b':
	    if ((*bg += 1) >= COLORS)
		*bg = 0;
	    break;
	default:
	    result = FALSE;
	    break;
	}
	if (result) {
	    *pair = (short) (*fg != COLOR_BLACK || *bg != COLOR_BLACK);
	    if (*pair != 0) {
		*pair = 1;
		if (init_pair(*pair, (short) *fg, (short) *bg) == ERR) {
		    result = FALSE;
		}
	    }
	}
    }
    return result;
}

/* ISO 6429:  codes 0x80 to 0x9f may be control characters that cause the
 * terminal to perform functions.  The remaining codes can be graphic.
 */
static void
show_upper_chars(unsigned first, int repeat, attr_t attr, short pair)
{
    bool C1 = (first == 128);
    unsigned code;
    unsigned last = first + 31;
    int reply;

    erase();
    attron(A_BOLD);
    mvprintw(0, 20, "Display of %s Character Codes %d to %d",
	     C1 ? "C1" : "GR", first, last);
    attroff(A_BOLD);
    refresh();

    for (code = first; code <= last; code++) {
	int count = repeat;
	int row = 2 + ((int) (code - first) % 16);
	int col = ((int) (code - first) / 16) * COLS / 2;
	char tmp[80];
	sprintf(tmp, "%3u (0x%x)", code, code);
	mvprintw(row, col, "%*s: ", COLS / 4, tmp);

	do {
	    if (C1)
		nodelay(stdscr, TRUE);
	    echochar(code | attr | COLOR_PAIR(pair));
	    if (C1) {
		/* (yes, this _is_ crude) */
		while ((reply = Getchar()) != ERR) {
		    addch(UChar(reply));
		    napms(10);
		}
		nodelay(stdscr, FALSE);
	    }
	} while (--count > 0);
    }
}

#define PC_COLS 4

static void
show_pc_chars(int repeat, attr_t attr, short pair)
{
    unsigned code;

    erase();
    attron(A_BOLD);
    mvprintw(0, 20, "Display of PC Character Codes");
    attroff(A_BOLD);
    refresh();

    for (code = 0; code < 16; ++code) {
	mvprintw(2, (int) code * PC_COLS + 8, "%X", code);
    }
    for (code = 0; code < 256; code++) {
	int count = repeat;
	int row = 3 + (int) (code / 16) + (code >= 128);
	int col = 8 + (int) (code % 16) * PC_COLS;
	if ((code % 16) == 0)
	    mvprintw(row, 0, "0x%02x:", code);
	move(row, col);
	do {
	    switch (code) {
	    case '\n':
	    case '\r':
	    case '\b':
	    case '\f':
	    case '\033':
	    case 0x9b:
		/*
		 * Skip the ones that do not work.
		 */
		break;
	    default:
		addch(code | A_ALTCHARSET | attr | COLOR_PAIR(pair));
		break;
	    }
	} while (--count > 0);
    }
}

static void
show_box_chars(int repeat, attr_t attr, short pair)
{
    (void) repeat;
    attr |= COLOR_PAIR(pair);

    erase();
    attron(A_BOLD);
    mvaddstr(0, 20, "Display of the ACS Line-Drawing Set");
    attroff(A_BOLD);
    refresh();
    box(stdscr, 0, 0);
    /* *INDENT-OFF* */
    mvhline(LINES / 2, 0,        ACS_HLINE | attr, COLS);
    mvvline(0,         COLS / 2, ACS_VLINE | attr, LINES);
    mvaddch(0,         COLS / 2, ACS_TTEE | attr);
    mvaddch(LINES / 2, COLS / 2, ACS_PLUS | attr);
    mvaddch(LINES - 1, COLS / 2, ACS_BTEE | attr);
    mvaddch(LINES / 2, 0,        ACS_LTEE | attr);
    mvaddch(LINES / 2, COLS - 1, ACS_RTEE | attr);
    /* *INDENT-ON* */

}

static int
show_1_acs(int n, int repeat, const char *name, chtype code)
{
    const int height = 16;
    int row = 2 + (n % height);
    int col = (n / height) * COLS / 2;

    mvprintw(row, col, "%*s : ", COLS / 4, name);
    do {
	addch(code);
    } while (--repeat > 0);
    return n + 1;
}

static void
show_acs_chars(int repeat, attr_t attr, short pair)
/* display the ACS character set */
{
    int n;

#define BOTH(name) #name, (name | attr | COLOR_PAIR(pair))

    erase();
    attron(A_BOLD);
    mvaddstr(0, 20, "Display of the ACS Character Set");
    attroff(A_BOLD);
    refresh();

    n = show_1_acs(0, repeat, BOTH(ACS_ULCORNER));
    n = show_1_acs(n, repeat, BOTH(ACS_URCORNER));
    n = show_1_acs(n, repeat, BOTH(ACS_LLCORNER));
    n = show_1_acs(n, repeat, BOTH(ACS_LRCORNER));

    n = show_1_acs(n, repeat, BOTH(ACS_LTEE));
    n = show_1_acs(n, repeat, BOTH(ACS_RTEE));
    n = show_1_acs(n, repeat, BOTH(ACS_TTEE));
    n = show_1_acs(n, repeat, BOTH(ACS_BTEE));

    n = show_1_acs(n, repeat, BOTH(ACS_HLINE));
    n = show_1_acs(n, repeat, BOTH(ACS_VLINE));

    /*
     * HPUX's ACS definitions are broken here.  Just give up.
     */
#if !(defined(__hpux) && !defined(NCURSES_VERSION))
    n = show_1_acs(n, repeat, BOTH(ACS_LARROW));
    n = show_1_acs(n, repeat, BOTH(ACS_RARROW));
    n = show_1_acs(n, repeat, BOTH(ACS_UARROW));
    n = show_1_acs(n, repeat, BOTH(ACS_DARROW));

    n = show_1_acs(n, repeat, BOTH(ACS_BLOCK));
    n = show_1_acs(n, repeat, BOTH(ACS_BOARD));
    n = show_1_acs(n, repeat, BOTH(ACS_LANTERN));
    n = show_1_acs(n, repeat, BOTH(ACS_BULLET));
    n = show_1_acs(n, repeat, BOTH(ACS_CKBOARD));
    n = show_1_acs(n, repeat, BOTH(ACS_DEGREE));
    n = show_1_acs(n, repeat, BOTH(ACS_DIAMOND));
    n = show_1_acs(n, repeat, BOTH(ACS_PLMINUS));
    n = show_1_acs(n, repeat, BOTH(ACS_PLUS));

    n = show_1_acs(n, repeat, BOTH(ACS_GEQUAL));
    n = show_1_acs(n, repeat, BOTH(ACS_NEQUAL));
    n = show_1_acs(n, repeat, BOTH(ACS_LEQUAL));

    n = show_1_acs(n, repeat, BOTH(ACS_STERLING));
    n = show_1_acs(n, repeat, BOTH(ACS_PI));
    n = show_1_acs(n, repeat, BOTH(ACS_S1));
    n = show_1_acs(n, repeat, BOTH(ACS_S3));
    n = show_1_acs(n, repeat, BOTH(ACS_S7));
    n = show_1_acs(n, repeat, BOTH(ACS_S9));
#endif
}

static void
acs_display(void)
{
    int c = 'a';
    char *term = getenv("TERM");
    const char *pch_kludge = ((term != 0 && strstr(term, "linux"))
			      ? "p=PC, "
			      : "");
    chtype attr = A_NORMAL;
    int digit = 0;
    int repeat = 1;
    int fg = COLOR_BLACK;
    int bg = COLOR_BLACK;
    unsigned at_code = 0;
    short pair = 0;
    void (*last_show_acs) (int, attr_t, short) = 0;

    do {
	switch (c) {
	case CTRL('L'):
	    Repaint();
	    break;
	case 'a':
	    ToggleAcs(last_show_acs, show_acs_chars);
	    break;
	case 'p':
	    if (*pch_kludge)
		ToggleAcs(last_show_acs, show_pc_chars);
	    else
		beep();
	    break;
	case 'x':
	    ToggleAcs(last_show_acs, show_box_chars);
	    break;
	case '0':
	case '1':
	case '2':
	case '3':
	    digit = (c - '0');
	    last_show_acs = 0;
	    break;
	case '-':
	    if (digit > 0) {
		--digit;
		last_show_acs = 0;
	    } else {
		beep();
	    }
	    break;
	case '+':
	    if (digit < 3) {
		++digit;
		last_show_acs = 0;
	    } else {
		beep();
	    }
	    break;
	case '>':
	    if (repeat < (COLS / 4))
		++repeat;
	    break;
	case '<':
	    if (repeat > 1)
		--repeat;
	    break;
	default:
	    if (cycle_attr(c, &at_code, &attr)
		|| cycle_colors(c, &fg, &bg, &pair)) {
		break;
	    } else {
		beep();
	    }
	    break;
	}
	if (last_show_acs != 0)
	    last_show_acs(repeat, attr, pair);
	else
	    show_upper_chars((unsigned) (digit * 32 + 128), repeat, attr, pair);

	mvprintw(LINES - 3, 0,
		 "Note: ANSI terminals may not display C1 characters.");
	mvprintw(LINES - 2, 0,
		 "Select: a=ACS, x=box, %s0=C1, 1-3,+/- non-ASCII, </> repeat, ESC=quit",
		 pch_kludge);
	if (use_colors) {
	    mvprintw(LINES - 1, 0,
		     "v/V, f/F, b/B cycle through video attributes (%s) and color %d/%d.",
		     attrs_to_cycle[at_code].name,
		     fg, bg);
	} else {
	    mvprintw(LINES - 1, 0,
		     "v/V cycles through video attributes (%s).",
		     attrs_to_cycle[at_code].name);
	}
	refresh();
    } while (!isQuit(c = Getchar()));

    Pause();
    erase();
    endwin();
}

#if USE_WIDEC_SUPPORT
static cchar_t *
merge_wide_attr(cchar_t *dst, const cchar_t *src, attr_t attr, short pair)
{
    int count = getcchar(src, NULL, NULL, NULL, 0);
    wchar_t *wch = 0;
    attr_t ignore_attr;
    short ignore_pair;

    *dst = *src;
    if (count > 0) {
	if ((wch = typeMalloc(wchar_t, (unsigned) count + 1)) != 0) {
	    if (getcchar(src, wch, &ignore_attr, &ignore_pair, 0) != ERR) {
		attr |= (ignore_attr & A_ALTCHARSET);
		setcchar(dst, wch, attr, pair, 0);
	    }
	    free(wch);
	}
    }
    return dst;
}

static void
show_upper_widechars(int first, int repeat, int space, attr_t attr, short pair)
{
    cchar_t temp;
    wchar_t code;
    int last = first + 31;

    erase();
    attron(A_BOLD);
    mvprintw(0, 20, "Display of Character Codes %d to %d", first, last);
    attroff(A_BOLD);

    for (code = first; (int) code <= last; code++) {
	int row = 2 + ((code - first) % 16);
	int col = ((code - first) / 16) * COLS / 2;
	wchar_t codes[10];
	char tmp[80];
	int count = repeat;
	int y, x;

	memset(&codes, 0, sizeof(codes));
	codes[0] = code;
	sprintf(tmp, "%3ld (0x%lx)", (long) code, (long) code);
	mvprintw(row, col, "%*s: ", COLS / 4, tmp);
	setcchar(&temp, codes, attr, pair, 0);
	do {
	    /*
	     * Give non-spacing characters something to combine with.  If we
	     * don't, they'll bunch up in a heap on the space after the ":".
	     * Mark them with reverse-video to make them simpler to find on
	     * the display.
	     */
	    if (wcwidth(code) == 0)
		addch(space | A_REVERSE);
	    /*
	     * This could use add_wch(), but is done for comparison with the
	     * normal 'f' test (and to make a test-case for echo_wchar()).
	     * The screen will flicker because the erase() at the top of the
	     * function is met by the builtin refresh() in echo_wchar().
	     */
	    echo_wchar(&temp);
	    /*
	     * The repeat-count may make text wrap - avoid that.
	     */
	    getyx(stdscr, y, x);
	    if (x >= col + (COLS / 2) - 2)
		break;
	} while (--count > 0);
    }
}

static int
show_1_wacs(int n, int repeat, const char *name, const cchar_t *code)
{
    const int height = 16;
    int row = 2 + (n % height);
    int col = (n / height) * COLS / 2;

    mvprintw(row, col, "%*s : ", COLS / 4, name);
    while (repeat-- >= 0) {
	add_wch(code);
    }
    return n + 1;
}

#define MERGE_ATTR(wch) merge_wide_attr(&temp, wch, attr, pair)

static void
show_wacs_chars(int repeat, attr_t attr, short pair)
/* display the wide-ACS character set */
{
    cchar_t temp;

    int n;

/*#define BOTH2(name) #name, &(name) */
#define BOTH2(name) #name, MERGE_ATTR(name)

    erase();
    attron(A_BOLD);
    mvaddstr(0, 20, "Display of the Wide-ACS Character Set");
    attroff(A_BOLD);
    refresh();

    n = show_1_wacs(0, repeat, BOTH2(WACS_ULCORNER));
    n = show_1_wacs(n, repeat, BOTH2(WACS_URCORNER));
    n = show_1_wacs(n, repeat, BOTH2(WACS_LLCORNER));
    n = show_1_wacs(n, repeat, BOTH2(WACS_LRCORNER));

    n = show_1_wacs(n, repeat, BOTH2(WACS_LTEE));
    n = show_1_wacs(n, repeat, BOTH2(WACS_RTEE));
    n = show_1_wacs(n, repeat, BOTH2(WACS_TTEE));
    n = show_1_wacs(n, repeat, BOTH2(WACS_BTEE));

    n = show_1_wacs(n, repeat, BOTH2(WACS_HLINE));
    n = show_1_wacs(n, repeat, BOTH2(WACS_VLINE));

    n = show_1_wacs(n, repeat, BOTH2(WACS_LARROW));
    n = show_1_wacs(n, repeat, BOTH2(WACS_RARROW));
    n = show_1_wacs(n, repeat, BOTH2(WACS_UARROW));
    n = show_1_wacs(n, repeat, BOTH2(WACS_DARROW));

    n = show_1_wacs(n, repeat, BOTH2(WACS_BLOCK));
    n = show_1_wacs(n, repeat, BOTH2(WACS_BOARD));
    n = show_1_wacs(n, repeat, BOTH2(WACS_LANTERN));
    n = show_1_wacs(n, repeat, BOTH2(WACS_BULLET));
    n = show_1_wacs(n, repeat, BOTH2(WACS_CKBOARD));
    n = show_1_wacs(n, repeat, BOTH2(WACS_DEGREE));
    n = show_1_wacs(n, repeat, BOTH2(WACS_DIAMOND));
    n = show_1_wacs(n, repeat, BOTH2(WACS_PLMINUS));
    n = show_1_wacs(n, repeat, BOTH2(WACS_PLUS));

#ifdef CURSES_WACS_ARRAY
    n = show_1_wacs(n, repeat, BOTH2(WACS_GEQUAL));
    n = show_1_wacs(n, repeat, BOTH2(WACS_NEQUAL));
    n = show_1_wacs(n, repeat, BOTH2(WACS_LEQUAL));

    n = show_1_wacs(n, repeat, BOTH2(WACS_STERLING));
    n = show_1_wacs(n, repeat, BOTH2(WACS_PI));
    n = show_1_wacs(n, repeat, BOTH2(WACS_S1));
    n = show_1_wacs(n, repeat, BOTH2(WACS_S3));
    n = show_1_wacs(n, repeat, BOTH2(WACS_S7));
    n = show_1_wacs(n, repeat, BOTH2(WACS_S9));
#endif
}

#undef MERGE_ATTR

#define MERGE_ATTR(wch) merge_wide_attr(&temp, wch, attr, pair)

static void
show_wbox_chars(int repeat, attr_t attr, short pair)
{
    cchar_t temp;

    (void) repeat;
    erase();
    attron(A_BOLD);
    mvaddstr(0, 20, "Display of the Wide-ACS Line-Drawing Set");
    attroff(A_BOLD);
    refresh();

    attr_set(attr, pair, 0);
    box_set(stdscr, 0, 0);
    attr_set(A_NORMAL, 0, 0);
    /* *INDENT-OFF* */
    mvhline_set(LINES / 2, 0,        MERGE_ATTR(WACS_HLINE), COLS);
    mvvline_set(0,         COLS / 2, MERGE_ATTR(WACS_VLINE), LINES);
    mvadd_wch(0,           COLS / 2, MERGE_ATTR(WACS_TTEE));
    mvadd_wch(LINES / 2,   COLS / 2, MERGE_ATTR(WACS_PLUS));
    mvadd_wch(LINES - 1,   COLS / 2, MERGE_ATTR(WACS_BTEE));
    mvadd_wch(LINES / 2,   0,        MERGE_ATTR(WACS_LTEE));
    mvadd_wch(LINES / 2,   COLS - 1, MERGE_ATTR(WACS_RTEE));
    /* *INDENT-ON* */

}

#undef MERGE_ATTR

static int
show_2_wacs(int n, const char *name, const char *code, attr_t attr, short pair)
{
    const int height = 16;
    int row = 2 + (n % height);
    int col = (n / height) * COLS / 2;
    char temp[80];

    mvprintw(row, col, "%*s : ", COLS / 4, name);
    attr_set(attr, pair, 0);
    addstr(strcpy(temp, code));
    attr_set(A_NORMAL, 0, 0);
    return n + 1;
}

#define SHOW_UTF8(n, name, code) show_2_wacs(n, name, code, attr, pair)

static void
show_utf8_chars(int repeat, attr_t attr, short pair)
{
    int n;

    (void) repeat;
    erase();
    attron(A_BOLD);
    mvaddstr(0, 20, "Display of the Wide-ACS Character Set");
    attroff(A_BOLD);
    refresh();
    /* *INDENT-OFF* */
    n = SHOW_UTF8(0, "WACS_ULCORNER",	"\342\224\214");
    n = SHOW_UTF8(n, "WACS_URCORNER",	"\342\224\220");
    n = SHOW_UTF8(n, "WACS_LLCORNER",	"\342\224\224");
    n = SHOW_UTF8(n, "WACS_LRCORNER",	"\342\224\230");

    n = SHOW_UTF8(n, "WACS_LTEE",	"\342\224\234");
    n = SHOW_UTF8(n, "WACS_RTEE",	"\342\224\244");
    n = SHOW_UTF8(n, "WACS_TTEE",	"\342\224\254");
    n = SHOW_UTF8(n, "WACS_BTEE",	"\342\224\264");

    n = SHOW_UTF8(n, "WACS_HLINE",	"\342\224\200");
    n = SHOW_UTF8(n, "WACS_VLINE",	"\342\224\202");

    n = SHOW_UTF8(n, "WACS_LARROW",	"\342\206\220");
    n = SHOW_UTF8(n, "WACS_RARROW",	"\342\206\222");
    n = SHOW_UTF8(n, "WACS_UARROW",	"\342\206\221");
    n = SHOW_UTF8(n, "WACS_DARROW",	"\342\206\223");

    n = SHOW_UTF8(n, "WACS_BLOCK",	"\342\226\256");
    n = SHOW_UTF8(n, "WACS_BOARD",	"\342\226\222");
    n = SHOW_UTF8(n, "WACS_LANTERN",	"\342\230\203");
    n = SHOW_UTF8(n, "WACS_BULLET",	"\302\267");
    n = SHOW_UTF8(n, "WACS_CKBOARD",	"\342\226\222");
    n = SHOW_UTF8(n, "WACS_DEGREE",	"\302\260");
    n = SHOW_UTF8(n, "WACS_DIAMOND",	"\342\227\206");
    n = SHOW_UTF8(n, "WACS_PLMINUS",	"\302\261");
    n = SHOW_UTF8(n, "WACS_PLUS",	"\342\224\274");
    n = SHOW_UTF8(n, "WACS_GEQUAL",	"\342\211\245");
    n = SHOW_UTF8(n, "WACS_NEQUAL",	"\342\211\240");
    n = SHOW_UTF8(n, "WACS_LEQUAL",	"\342\211\244");

    n = SHOW_UTF8(n, "WACS_STERLING",	"\302\243");
    n = SHOW_UTF8(n, "WACS_PI",		"\317\200");
    n = SHOW_UTF8(n, "WACS_S1",		"\342\216\272");
    n = SHOW_UTF8(n, "WACS_S3",		"\342\216\273");
    n = SHOW_UTF8(n, "WACS_S7",		"\342\216\274");
    n = SHOW_UTF8(n, "WACS_S9",		"\342\216\275");
    /* *INDENT-ON* */

}

/* display the wide-ACS character set */
static void
wide_acs_display(void)
{
    int c = 'a';
    int digit = 0;
    int repeat = 1;
    int space = ' ';
    chtype attr = A_NORMAL;
    int fg = COLOR_BLACK;
    int bg = COLOR_BLACK;
    unsigned at_code = 0;
    short pair = 0;
    void (*last_show_wacs) (int, attr_t, short) = 0;

    do {
	switch (c) {
	case CTRL('L'):
	    Repaint();
	    break;
	case 'a':
	    ToggleAcs(last_show_wacs, show_wacs_chars);
	    break;
	case 'x':
	    ToggleAcs(last_show_wacs, show_wbox_chars);
	    break;
	case 'u':
	    ToggleAcs(last_show_wacs, show_utf8_chars);
	    break;
	default:
	    if (c < 256 && isdigit(c)) {
		digit = (c - '0');
		last_show_wacs = 0;
	    } else if (c == '+') {
		++digit;
		last_show_wacs = 0;
	    } else if (c == '-' && digit > 0) {
		--digit;
		last_show_wacs = 0;
	    } else if (c == '>' && repeat < (COLS / 4)) {
		++repeat;
	    } else if (c == '<' && repeat > 1) {
		--repeat;
	    } else if (c == '_') {
		space = (space == ' ') ? '_' : ' ';
		last_show_wacs = 0;
	    } else if (cycle_attr(c, &at_code, &attr)
		       || cycle_colors(c, &fg, &bg, &pair)) {
		if (last_show_wacs != 0)
		    break;
	    } else {
		beep();
		break;
	    }
	    break;
	}
	if (last_show_wacs != 0)
	    last_show_wacs(repeat, attr, pair);
	else
	    show_upper_widechars(digit * 32 + 128, repeat, space, attr, pair);

	mvprintw(LINES - 3, 0,
		 "Select: a WACS, x box, u UTF-8, 0-9,+/- non-ASCII, </> repeat, ESC=quit");
	if (use_colors) {
	    mvprintw(LINES - 2, 0,
		     "v/V, f/F, b/B cycle through video attributes (%s) and color %d/%d.",
		     attrs_to_cycle[at_code].name,
		     fg, bg);
	} else {
	    mvprintw(LINES - 2, 0,
		     "v/V cycles through video attributes (%s).",
		     attrs_to_cycle[at_code].name);
	}
	refresh();
    } while (!isQuit(c = Getchar()));

    Pause();
    erase();
    endwin();
}

#endif

/*
 * Graphic-rendition test (adapted from vttest)
 */
static void
test_sgr_attributes(void)
{
    int pass;

    for (pass = 0; pass < 2; pass++) {
	chtype normal = ((pass == 0 ? A_NORMAL : A_REVERSE)) | BLANK;

	/* Use non-default colors if possible to exercise bce a little */
	if (use_colors) {
	    init_pair(1, COLOR_WHITE, COLOR_BLUE);
	    normal |= COLOR_PAIR(1);
	}
	bkgdset(normal);
	erase();
	mvprintw(1, 20, "Graphic rendition test pattern:");

	mvprintw(4, 1, "vanilla");

#define set_sgr(mask) bkgdset((normal^(mask)));
	set_sgr(A_BOLD);
	mvprintw(4, 40, "bold");

	set_sgr(A_UNDERLINE);
	mvprintw(6, 6, "underline");

	set_sgr(A_BOLD | A_UNDERLINE);
	mvprintw(6, 45, "bold underline");

	set_sgr(A_BLINK);
	mvprintw(8, 1, "blink");

	set_sgr(A_BLINK | A_BOLD);
	mvprintw(8, 40, "bold blink");

	set_sgr(A_UNDERLINE | A_BLINK);
	mvprintw(10, 6, "underline blink");

	set_sgr(A_BOLD | A_UNDERLINE | A_BLINK);
	mvprintw(10, 45, "bold underline blink");

	set_sgr(A_REVERSE);
	mvprintw(12, 1, "negative");

	set_sgr(A_BOLD | A_REVERSE);
	mvprintw(12, 40, "bold negative");

	set_sgr(A_UNDERLINE | A_REVERSE);
	mvprintw(14, 6, "underline negative");

	set_sgr(A_BOLD | A_UNDERLINE | A_REVERSE);
	mvprintw(14, 45, "bold underline negative");

	set_sgr(A_BLINK | A_REVERSE);
	mvprintw(16, 1, "blink negative");

	set_sgr(A_BOLD | A_BLINK | A_REVERSE);
	mvprintw(16, 40, "bold blink negative");

	set_sgr(A_UNDERLINE | A_BLINK | A_REVERSE);
	mvprintw(18, 6, "underline blink negative");

	set_sgr(A_BOLD | A_UNDERLINE | A_BLINK | A_REVERSE);
	mvprintw(18, 45, "bold underline blink negative");

	bkgdset(normal);
	mvprintw(LINES - 2, 1, "%s background. ", pass == 0 ? "Dark" :
		 "Light");
	clrtoeol();
	Pause();
    }

    bkgdset(A_NORMAL | BLANK);
    erase();
    endwin();
}

/****************************************************************************
 *
 * Windows and scrolling tester.
 *
 ****************************************************************************/

#define BOTLINES	4	/* number of line stolen from screen bottom */

typedef struct {
    int y, x;
} pair;

#define FRAME struct frame
FRAME
{
    FRAME *next, *last;
    bool do_scroll;
    bool do_keypad;
    WINDOW *wind;
};

#if defined(NCURSES_VERSION)
#if (NCURSES_VERSION_PATCH < 20070331) && NCURSES_EXT_FUNCS
#define is_keypad(win)   (win)->_use_keypad
#define is_scrollok(win) (win)->_scroll
#elif !defined(is_keypad)
#define is_keypad(win)   FALSE
#define is_scrollok(win) FALSE
#endif
#else
#define is_keypad(win)   FALSE
#define is_scrollok(win) FALSE
#endif

static WINDOW *
frame_win(FRAME * curp)
{
    return (curp != 0) ? curp->wind : stdscr;
}

/* We need to know if these flags are actually set, so don't look in FRAME.
 * These names are known to work with SVr4 curses as well as ncurses.  The
 * _use_keypad name does not work with Solaris 8.
 */
static bool
HaveKeypad(FRAME * curp)
{
    WINDOW *win = frame_win(curp);
    (void) win;
    return is_keypad(win);
}

static bool
HaveScroll(FRAME * curp)
{
    WINDOW *win = frame_win(curp);
    (void) win;
    return is_scrollok(win);
}

static void
newwin_legend(FRAME * curp)
{
    static const struct {
	const char *msg;
	int code;
    } legend[] = {
	{
	    "^C = create window", 0
	},
	{
	    "^N = next window", 0
	},
	{
	    "^P = previous window", 0
	},
	{
	    "^F = scroll forward", 0
	},
	{
	    "^B = scroll backward", 0
	},
	{
	    "^K = keypad(%s)", 1
	},
	{
	    "^S = scrollok(%s)", 2
	},
	{
	    "^W = save window to file", 0
	},
	{
	    "^R = restore window", 0
	},
#if HAVE_WRESIZE
	{
	    "^X = resize", 0
	},
#endif
	{
	    "^Q%s = exit", 3
	}
    };
    size_t n;
    int x;
    bool do_keypad = HaveKeypad(curp);
    bool do_scroll = HaveScroll(curp);
    char buf[BUFSIZ];

    move(LINES - 4, 0);
    for (n = 0; n < SIZEOF(legend); n++) {
	switch (legend[n].code) {
	default:
	    strcpy(buf, legend[n].msg);
	    break;
	case 1:
	    sprintf(buf, legend[n].msg, do_keypad ? "yes" : "no");
	    break;
	case 2:
	    sprintf(buf, legend[n].msg, do_scroll ? "yes" : "no");
	    break;
	case 3:
	    sprintf(buf, legend[n].msg, do_keypad ? "/ESC" : "");
	    break;
	}
	x = getcurx(stdscr);
	addstr((COLS < (x + 3 + (int) strlen(buf))) ? "\n" : (n ? ", " : ""));
	addstr(buf);
    }
    clrtoeol();
}

static void
transient(FRAME * curp, NCURSES_CONST char *msg)
{
    newwin_legend(curp);
    if (msg) {
	mvaddstr(LINES - 1, 0, msg);
	refresh();
	napms(1000);
    }

    move(LINES - 1, 0);
    printw("%s characters are echoed, window should %sscroll.",
	   HaveKeypad(curp) ? "Non-arrow" : "All other",
	   HaveScroll(curp) ? "" : "not ");
    clrtoeol();
}

static void
newwin_report(FRAME * curp)
/* report on the cursor's current position, then restore it */
{
    WINDOW *win = frame_win(curp);
    int y, x;

    if (win != stdscr)
	transient(curp, (char *) 0);
    getyx(win, y, x);
    move(LINES - 1, COLS - 17);
    printw("Y = %2d X = %2d", y, x);
    if (win != stdscr)
	refresh();
    else
	wmove(win, y, x);
}

static pair *
selectcell(int uli, int ulj, int lri, int lrj)
/* arrows keys move cursor, return location at current on non-arrow key */
{
    static pair res;		/* result cell */
    int si = lri - uli + 1;	/* depth of the select area */
    int sj = lrj - ulj + 1;	/* width of the select area */
    int i = 0, j = 0;		/* offsets into the select area */

    res.y = uli;
    res.x = ulj;
    for (;;) {
	move(uli + i, ulj + j);
	newwin_report((FRAME *) 0);

	switch (Getchar()) {
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
	case case_QUIT:
	    return ((pair *) 0);
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

static void
outerbox(pair ul, pair lr, bool onoff)
/* draw or erase a box *outside* the given pair of corners */
{
    mvaddch(ul.y - 1, lr.x - 1, onoff ? ACS_ULCORNER : ' ');
    mvaddch(ul.y - 1, lr.x + 1, onoff ? ACS_URCORNER : ' ');
    mvaddch(lr.y + 1, lr.x + 1, onoff ? ACS_LRCORNER : ' ');
    mvaddch(lr.y + 1, ul.x - 1, onoff ? ACS_LLCORNER : ' ');
    move(ul.y - 1, ul.x);
    hline(onoff ? ACS_HLINE : ' ', lr.x - ul.x + 1);
    move(ul.y, ul.x - 1);
    vline(onoff ? ACS_VLINE : ' ', lr.y - ul.y + 1);
    move(lr.y + 1, ul.x);
    hline(onoff ? ACS_HLINE : ' ', lr.x - ul.x + 1);
    move(ul.y, lr.x + 1);
    vline(onoff ? ACS_VLINE : ' ', lr.y - ul.y + 1);
}

static WINDOW *
getwindow(void)
/* Ask user for a window definition */
{
    WINDOW *rwindow;
    pair ul, lr, *tmp;

    move(0, 0);
    clrtoeol();
    addstr("Use arrows to move cursor, anything else to mark corner 1");
    refresh();
    if ((tmp = selectcell(2, 1, LINES - BOTLINES - 2, COLS - 2)) == (pair *) 0)
	return ((WINDOW *) 0);
    memcpy(&ul, tmp, sizeof(pair));
    mvaddch(ul.y - 1, ul.x - 1, ACS_ULCORNER);
    move(0, 0);
    clrtoeol();
    addstr("Use arrows to move cursor, anything else to mark corner 2");
    refresh();
    if ((tmp = selectcell(ul.y, ul.x, LINES - BOTLINES - 2, COLS - 2)) ==
	(pair *) 0)
	return ((WINDOW *) 0);
    memcpy(&lr, tmp, sizeof(pair));

    rwindow = subwin(stdscr, lr.y - ul.y + 1, lr.x - ul.x + 1, ul.y, ul.x);

    outerbox(ul, lr, TRUE);
    refresh();

    wrefresh(rwindow);

    move(0, 0);
    clrtoeol();
    return (rwindow);
}

static void
newwin_move(FRAME * curp, int dy, int dx)
{
    WINDOW *win = frame_win(curp);
    int cur_y, cur_x;
    int max_y, max_x;

    getyx(win, cur_y, cur_x);
    getmaxyx(win, max_y, max_x);
    if ((cur_x += dx) < 0)
	cur_x = 0;
    else if (cur_x >= max_x)
	cur_x = max_x - 1;
    if ((cur_y += dy) < 0)
	cur_y = 0;
    else if (cur_y >= max_y)
	cur_y = max_y - 1;
    wmove(win, cur_y, cur_x);
}

static FRAME *
delete_framed(FRAME * fp, bool showit)
{
    FRAME *np = 0;

    if (fp != 0) {
	fp->last->next = fp->next;
	fp->next->last = fp->last;

	if (showit) {
	    werase(fp->wind);
	    wrefresh(fp->wind);
	}
	delwin(fp->wind);

	np = (fp == fp->next) ? 0 : fp->next;
	free(fp);
    }
    return np;
}

static void
acs_and_scroll(void)
/* Demonstrate windows */
{
    int c;
    FRAME *current = (FRAME *) 0, *neww;
    WINDOW *usescr = stdscr;
#if HAVE_PUTWIN && HAVE_GETWIN
    FILE *fp;
#endif

#define DUMPFILE	"screendump"

#ifdef NCURSES_MOUSE_VERSION
    mousemask(BUTTON1_CLICKED, (mmask_t *) 0);
#endif
    c = CTRL('C');
    raw();
    do {
	transient((FRAME *) 0, (char *) 0);
	switch (c) {
	case CTRL('C'):
	    if ((neww = typeCalloc(FRAME, 1)) == 0) {
		goto breakout;
	    }
	    if ((neww->wind = getwindow()) == (WINDOW *) 0) {
		free(neww);
		goto breakout;
	    }

	    if (current == 0) {	/* First element,  */
		neww->next = neww;	/*   so point it at itself */
		neww->last = neww;
	    } else {
		neww->next = current->next;
		neww->last = current;
		neww->last->next = neww;
		neww->next->last = neww;
	    }
	    current = neww;
	    /* SVr4 curses sets the keypad on all newly-created windows to
	     * false.  Someone reported that PDCurses makes new windows inherit
	     * this flag.  Remove the following 'keypad()' call to test this
	     */
	    keypad(current->wind, TRUE);
	    current->do_keypad = HaveKeypad(current);
	    current->do_scroll = HaveScroll(current);
	    break;

	case CTRL('N'):	/* go to next window */
	    if (current)
		current = current->next;
	    break;

	case CTRL('P'):	/* go to previous window */
	    if (current)
		current = current->last;
	    break;

	case CTRL('F'):	/* scroll current window forward */
	    if (current)
		wscrl(frame_win(current), 1);
	    break;

	case CTRL('B'):	/* scroll current window backwards */
	    if (current)
		wscrl(frame_win(current), -1);
	    break;

	case CTRL('K'):	/* toggle keypad mode for current */
	    if (current) {
		current->do_keypad = !current->do_keypad;
		keypad(current->wind, current->do_keypad);
	    }
	    break;

	case CTRL('S'):
	    if (current) {
		current->do_scroll = !current->do_scroll;
		scrollok(current->wind, current->do_scroll);
	    }
	    break;

#if HAVE_PUTWIN && HAVE_GETWIN
	case CTRL('W'):	/* save and delete window */
	    if ((current != 0) && (current == current->next)) {
		transient(current, "Will not save/delete ONLY window");
		break;
	    } else if ((fp = fopen(DUMPFILE, "w")) == (FILE *) 0) {
		transient(current, "Can't open screen dump file");
	    } else {
		(void) putwin(frame_win(current), fp);
		(void) fclose(fp);

		current = delete_framed(current, TRUE);
	    }
	    break;

	case CTRL('R'):	/* restore window */
	    if ((fp = fopen(DUMPFILE, "r")) == (FILE *) 0) {
		transient(current, "Can't open screen dump file");
	    } else {
		if ((neww = typeCalloc(FRAME, 1)) != 0) {

		    neww->next = current ? current->next : 0;
		    neww->last = current;
		    neww->last->next = neww;
		    neww->next->last = neww;

		    neww->wind = getwin(fp);

		    wrefresh(neww->wind);
		}
		(void) fclose(fp);
	    }
	    break;
#endif

#if HAVE_WRESIZE
	case CTRL('X'):	/* resize window */
	    if (current) {
		pair *tmp, ul, lr;
		int i, mx, my;

		move(0, 0);
		clrtoeol();
		addstr("Use arrows to move cursor, anything else to mark new corner");
		refresh();

		getbegyx(current->wind, ul.y, ul.x);

		tmp = selectcell(ul.y, ul.x, LINES - BOTLINES - 2, COLS - 2);
		if (tmp == (pair *) 0) {
		    beep();
		    break;
		}

		getmaxyx(current->wind, lr.y, lr.x);
		lr.y += (ul.y - 1);
		lr.x += (ul.x - 1);
		outerbox(ul, lr, FALSE);
		wnoutrefresh(stdscr);

		/* strictly cosmetic hack for the test */
		getmaxyx(current->wind, my, mx);
		if (my > tmp->y - ul.y) {
		    getyx(current->wind, lr.y, lr.x);
		    wmove(current->wind, tmp->y - ul.y + 1, 0);
		    wclrtobot(current->wind);
		    wmove(current->wind, lr.y, lr.x);
		}
		if (mx > tmp->x - ul.x)
		    for (i = 0; i < my; i++) {
			wmove(current->wind, i, tmp->x - ul.x + 1);
			wclrtoeol(current->wind);
		    }
		wnoutrefresh(current->wind);

		memcpy(&lr, tmp, sizeof(pair));
		(void) wresize(current->wind, lr.y - ul.y + 0, lr.x - ul.x + 0);

		getbegyx(current->wind, ul.y, ul.x);
		getmaxyx(current->wind, lr.y, lr.x);
		lr.y += (ul.y - 1);
		lr.x += (ul.x - 1);
		outerbox(ul, lr, TRUE);
		wnoutrefresh(stdscr);

		wnoutrefresh(current->wind);
		move(0, 0);
		clrtoeol();
		doupdate();
	    }
	    break;
#endif /* HAVE_WRESIZE */

	case KEY_F(10):	/* undocumented --- use this to test area clears */
	    selectcell(0, 0, LINES - 1, COLS - 1);
	    clrtobot();
	    refresh();
	    break;

	case KEY_UP:
	    newwin_move(current, -1, 0);
	    break;
	case KEY_DOWN:
	    newwin_move(current, 1, 0);
	    break;
	case KEY_LEFT:
	    newwin_move(current, 0, -1);
	    break;
	case KEY_RIGHT:
	    newwin_move(current, 0, 1);
	    break;

	case KEY_BACKSPACE:
	    /* FALLTHROUGH */
	case KEY_DC:
	    {
		int y, x;
		getyx(frame_win(current), y, x);
		if (--x < 0) {
		    if (--y < 0)
			break;
		    x = getmaxx(frame_win(current)) - 1;
		}
		mvwdelch(frame_win(current), y, x);
	    }
	    break;

	case '\r':
	    c = '\n';
	    /* FALLTHROUGH */

	default:
	    if (current)
		waddch(current->wind, (chtype) c);
	    else
		beep();
	    break;
	}
	newwin_report(current);
	usescr = frame_win(current);
	wrefresh(usescr);
    } while
	(!isQuit(c = wGetchar(usescr))
	 && (c != ERR));

  breakout:
    while (current != 0)
	current = delete_framed(current, FALSE);

    scrollok(stdscr, TRUE);	/* reset to driver's default */
#ifdef NCURSES_MOUSE_VERSION
    mousemask(0, (mmask_t *) 0);
#endif
    noraw();
    erase();
    endwin();
}

/****************************************************************************
 *
 * Panels tester
 *
 ****************************************************************************/

#if USE_LIBPANEL
static int nap_msec = 1;

static NCURSES_CONST char *mod[] =
{
    "test ",
    "TEST ",
    "(**) ",
    "*()* ",
    "<--> ",
    "LAST "
};

/*+-------------------------------------------------------------------------
	wait_a_while(msec)
--------------------------------------------------------------------------*/
static void
wait_a_while(int msec GCC_UNUSED)
{
#if HAVE_NAPMS
    if (nap_msec == 1)
	wGetchar(stdscr);
    else
	napms(nap_msec);
#else
    if (nap_msec == 1)
	wGetchar(stdscr);
    else if (msec > 1000)
	sleep((unsigned) msec / 1000);
    else
	sleep(1);
#endif
}				/* end of wait_a_while */

/*+-------------------------------------------------------------------------
	saywhat(text)
--------------------------------------------------------------------------*/
static void
saywhat(NCURSES_CONST char *text)
{
    wmove(stdscr, LINES - 1, 0);
    wclrtoeol(stdscr);
    if (text != 0 && *text != '\0') {
	waddstr(stdscr, text);
	waddstr(stdscr, "; ");
    }
    waddstr(stdscr, "press any key to continue");
}				/* end of saywhat */

/*+-------------------------------------------------------------------------
	mkpanel(rows,cols,tly,tlx) - alloc a win and panel and associate them
--------------------------------------------------------------------------*/
static PANEL *
mkpanel(short color, int rows, int cols, int tly, int tlx)
{
    WINDOW *win;
    PANEL *pan = 0;

    if ((win = newwin(rows, cols, tly, tlx)) != 0) {
	if ((pan = new_panel(win)) == 0) {
	    delwin(win);
	} else if (use_colors) {
	    short fg = (short) ((color == COLOR_BLUE) ? COLOR_WHITE : COLOR_BLACK);
	    short bg = color;

	    init_pair(color, fg, bg);
	    wbkgdset(win, (chtype) (COLOR_PAIR(color) | ' '));
	} else {
	    wbkgdset(win, A_BOLD | ' ');
	}
    }
    return pan;
}				/* end of mkpanel */

/*+-------------------------------------------------------------------------
	rmpanel(pan)
--------------------------------------------------------------------------*/
static void
rmpanel(PANEL * pan)
{
    WINDOW *win = panel_window(pan);
    del_panel(pan);
    delwin(win);
}				/* end of rmpanel */

/*+-------------------------------------------------------------------------
	pflush()
--------------------------------------------------------------------------*/
static void
pflush(void)
{
    update_panels();
    doupdate();
}				/* end of pflush */

/*+-------------------------------------------------------------------------
	fill_panel(win)
--------------------------------------------------------------------------*/
static void
init_panel(void)
{
    register int y, x;

    for (y = 0; y < LINES - 1; y++) {
	for (x = 0; x < COLS; x++)
	    wprintw(stdscr, "%d", (y + x) % 10);
    }
}

static void
fill_panel(PANEL * pan)
{
    WINDOW *win = panel_window(pan);
    const char *userptr = (const char *) panel_userptr(pan);
    int num = (userptr && *userptr) ? userptr[1] : '?';
    int y, x;

    wmove(win, 1, 1);
    wprintw(win, "-pan%c-", num);
    wclrtoeol(win);
    box(win, 0, 0);
    for (y = 2; y < getmaxy(win) - 1; y++) {
	for (x = 1; x < getmaxx(win) - 1; x++) {
	    wmove(win, y, x);
	    waddch(win, UChar(num));
	}
    }
}

#if USE_WIDEC_SUPPORT
static void
init_wide_panel(void)
{
    int digit;
    cchar_t temp[10];

    for (digit = 0; digit < 10; ++digit)
	make_fullwidth_digit(&temp[digit], digit);

    do {
	int y, x;
	getyx(stdscr, y, x);
	digit = (y + x / 2) % 10;
    } while (add_wch(&temp[digit]) != ERR);
}

static void
fill_wide_panel(PANEL * pan)
{
    WINDOW *win = panel_window(pan);
    const char *userptr = (const char *) panel_userptr(pan);
    int num = (userptr && *userptr) ? userptr[1] : '?';
    int y, x;

    wmove(win, 1, 1);
    wprintw(win, "-pan%c-", num);
    wclrtoeol(win);
    box(win, 0, 0);
    for (y = 2; y < getmaxy(win) - 1; y++) {
	for (x = 1; x < getmaxx(win) - 1; x++) {
	    wmove(win, y, x);
	    waddch(win, UChar(num));
	}
    }
}
#endif

#define MAX_PANELS 5

static void
canned_panel(PANEL * px[MAX_PANELS + 1], NCURSES_CONST char *cmd)
{
    int which = cmd[1] - '0';

    saywhat(cmd);
    switch (*cmd) {
    case 'h':
	hide_panel(px[which]);
	break;
    case 's':
	show_panel(px[which]);
	break;
    case 't':
	top_panel(px[which]);
	break;
    case 'b':
	bottom_panel(px[which]);
	break;
    case 'd':
	rmpanel(px[which]);
	break;
    }
    pflush();
    wait_a_while(nap_msec);
}

static void
demo_panels(void (*InitPanel) (void), void (*FillPanel) (PANEL *))
{
    int count;
    int itmp;
    PANEL *px[MAX_PANELS + 1];

    scrollok(stdscr, FALSE);	/* we don't want stdscr to scroll! */
    refresh();

    InitPanel();
    for (count = 0; count < 5; count++) {
	px[1] = mkpanel(COLOR_RED,
			LINES / 2 - 2,
			COLS / 8 + 1,
			0,
			0);
	set_panel_userptr(px[1], (NCURSES_CONST void *) "p1");

	px[2] = mkpanel(COLOR_GREEN,
			LINES / 2 + 1,
			COLS / 7,
			LINES / 4,
			COLS / 10);
	set_panel_userptr(px[2], (NCURSES_CONST void *) "p2");

	px[3] = mkpanel(COLOR_YELLOW,
			LINES / 4,
			COLS / 10,
			LINES / 2,
			COLS / 9);
	set_panel_userptr(px[3], (NCURSES_CONST void *) "p3");

	px[4] = mkpanel(COLOR_BLUE,
			LINES / 2 - 2,
			COLS / 8,
			LINES / 2 - 2,
			COLS / 3);
	set_panel_userptr(px[4], (NCURSES_CONST void *) "p4");

	px[5] = mkpanel(COLOR_MAGENTA,
			LINES / 2 - 2,
			COLS / 8,
			LINES / 2,
			COLS / 2 - 2);
	set_panel_userptr(px[5], (NCURSES_CONST void *) "p5");

	FillPanel(px[1]);
	FillPanel(px[2]);
	FillPanel(px[3]);
	FillPanel(px[4]);
	FillPanel(px[5]);

	hide_panel(px[4]);
	hide_panel(px[5]);
	pflush();
	saywhat("");
	wait_a_while(nap_msec);

	saywhat("h3 s1 s2 s4 s5");
	move_panel(px[1], 0, 0);
	hide_panel(px[3]);
	show_panel(px[1]);
	show_panel(px[2]);
	show_panel(px[4]);
	show_panel(px[5]);
	pflush();
	wait_a_while(nap_msec);

	canned_panel(px, "s1");
	canned_panel(px, "s2");

	saywhat("m2");
	move_panel(px[2], LINES / 3 + 1, COLS / 8);
	pflush();
	wait_a_while(nap_msec);

	canned_panel(px, "s3");

	saywhat("m3");
	move_panel(px[3], LINES / 4 + 1, COLS / 15);
	pflush();
	wait_a_while(nap_msec);

	canned_panel(px, "b3");
	canned_panel(px, "s4");
	canned_panel(px, "s5");
	canned_panel(px, "t3");
	canned_panel(px, "t1");
	canned_panel(px, "t2");
	canned_panel(px, "t3");
	canned_panel(px, "t4");

	for (itmp = 0; itmp < 6; itmp++) {
	    WINDOW *w4 = panel_window(px[4]);
	    WINDOW *w5 = panel_window(px[5]);

	    saywhat("m4");
	    wmove(w4, LINES / 8, 1);
	    waddstr(w4, mod[itmp]);
	    move_panel(px[4], LINES / 6, itmp * (COLS / 8));
	    wmove(w5, LINES / 6, 1);
	    waddstr(w5, mod[itmp]);
	    pflush();
	    wait_a_while(nap_msec);

	    saywhat("m5");
	    wmove(w4, LINES / 6, 1);
	    waddstr(w4, mod[itmp]);
	    move_panel(px[5], LINES / 3 - 1, (itmp * 10) + 6);
	    wmove(w5, LINES / 8, 1);
	    waddstr(w5, mod[itmp]);
	    pflush();
	    wait_a_while(nap_msec);
	}

	saywhat("m4");
	move_panel(px[4], LINES / 6, itmp * (COLS / 8));
	pflush();
	wait_a_while(nap_msec);

	canned_panel(px, "t5");
	canned_panel(px, "t2");
	canned_panel(px, "t1");
	canned_panel(px, "d2");
	canned_panel(px, "h3");
	canned_panel(px, "d1");
	canned_panel(px, "d4");
	canned_panel(px, "d5");
	canned_panel(px, "d3");

	wait_a_while(nap_msec);
	if (nap_msec == 1)
	    break;
	nap_msec = 100L;
    }

    erase();
    endwin();
}
#endif /* USE_LIBPANEL */

/****************************************************************************
 *
 * Pad tester
 *
 ****************************************************************************/

#define GRIDSIZE	3

static bool pending_pan = FALSE;
static bool show_panner_legend = TRUE;

static int
panner_legend(int line)
{
    static const char *const legend[] =
    {
	"Use arrow keys (or U,D,L,R) to pan, ESC to quit, ! to shell-out.",
	"Use +,- (or j,k) to grow/shrink the panner vertically.",
	"Use <,> (or h,l) to grow/shrink the panner horizontally.",
	"Number repeats.  Toggle legend:? filler:a timer:t scrollmark:s."
    };
    int n = ((int) SIZEOF(legend) - (LINES - line));
    if (line < LINES && (n >= 0)) {
	move(line, 0);
	if (show_panner_legend)
	    printw("%s", legend[n]);
	clrtoeol();
	return show_panner_legend;
    }
    return FALSE;
}

static void
panner_h_cleanup(int from_y, int from_x, int to_x)
{
    if (!panner_legend(from_y))
	do_h_line(from_y, from_x, ' ', to_x);
}

static void
panner_v_cleanup(int from_y, int from_x, int to_y)
{
    if (!panner_legend(from_y))
	do_v_line(from_y, from_x, ' ', to_y);
}

static void
fill_pad(WINDOW *panpad, bool pan_lines)
{
    int y, x;
    unsigned gridcount = 0;

    wmove(panpad, 0, 0);
    for (y = 0; y < getmaxy(panpad); y++) {
	for (x = 0; x < getmaxx(panpad); x++) {
	    if (y % GRIDSIZE == 0 && x % GRIDSIZE == 0) {
		if (y == 0 && x == 0)
		    waddch(panpad, pan_lines ? ACS_ULCORNER : '+');
		else if (y == 0)
		    waddch(panpad, pan_lines ? ACS_TTEE : '+');
		else if (y == 0 || x == 0)
		    waddch(panpad, pan_lines ? ACS_LTEE : '+');
		else
		    waddch(panpad, (chtype) ((pan_lines ? 'a' : 'A') +
					     (int) (gridcount++ % 26)));
	    } else if (y % GRIDSIZE == 0)
		waddch(panpad, pan_lines ? ACS_HLINE : '-');
	    else if (x % GRIDSIZE == 0)
		waddch(panpad, pan_lines ? ACS_VLINE : '|');
	    else
		waddch(panpad, ' ');
	}
    }
}

static void
panner(WINDOW *pad,
       int top_x, int top_y, int porty, int portx,
       int (*pgetc) (WINDOW *))
{
#if HAVE_GETTIMEOFDAY
    struct timeval before, after;
    bool timing = TRUE;
#endif
    bool pan_lines = FALSE;
    bool scrollers = TRUE;
    int basex = 0;
    int basey = 0;
    int pxmax, pymax, lowend, highend, c;

    getmaxyx(pad, pymax, pxmax);
    scrollok(stdscr, FALSE);	/* we don't want stdscr to scroll! */

    c = KEY_REFRESH;
    do {
#ifdef NCURSES_VERSION
	/*
	 * During shell-out, the user may have resized the window.  Adjust
	 * the port size of the pad to accommodate this.  Ncurses automatically
	 * resizes all of the normal windows to fit on the new screen.
	 */
	if (top_x > COLS)
	    top_x = COLS;
	if (portx > COLS)
	    portx = COLS;
	if (top_y > LINES)
	    top_y = LINES;
	if (porty > LINES)
	    porty = LINES;
#endif
	switch (c) {
	case KEY_REFRESH:
	    erase();

	    /* FALLTHRU */
	case '?':
	    if (c == '?')
		show_panner_legend = !show_panner_legend;
	    panner_legend(LINES - 4);
	    panner_legend(LINES - 3);
	    panner_legend(LINES - 2);
	    panner_legend(LINES - 1);
	    break;
	case 'a':
	    pan_lines = !pan_lines;
	    fill_pad(pad, pan_lines);
	    pending_pan = FALSE;
	    break;

#if HAVE_GETTIMEOFDAY
	case 't':
	    timing = !timing;
	    if (!timing)
		panner_legend(LINES - 1);
	    break;
#endif
	case 's':
	    scrollers = !scrollers;
	    break;

	    /* Move the top-left corner of the pad, keeping the bottom-right
	     * corner fixed.
	     */
	case 'h':		/* increase-columns: move left edge to left */
	    if (top_x <= 0)
		beep();
	    else {
		panner_v_cleanup(top_y, top_x, porty);
		top_x--;
	    }
	    break;

	case 'j':		/* decrease-lines: move top-edge down */
	    if (top_y >= porty)
		beep();
	    else {
		panner_h_cleanup(top_y - 1, top_x - (top_x > 0), portx);
		top_y++;
	    }
	    break;

	case 'k':		/* increase-lines: move top-edge up */
	    if (top_y <= 0)
		beep();
	    else {
		top_y--;
		panner_h_cleanup(top_y, top_x, portx);
	    }
	    break;

	case 'l':		/* decrease-columns: move left-edge to right */
	    if (top_x >= portx)
		beep();
	    else {
		panner_v_cleanup(top_y - (top_y > 0), top_x - 1, porty);
		top_x++;
	    }
	    break;

	    /* Move the bottom-right corner of the pad, keeping the top-left
	     * corner fixed.
	     */
	case KEY_IC:		/* increase-columns: move right-edge to right */
	    if (portx >= pxmax || portx >= COLS)
		beep();
	    else {
		panner_v_cleanup(top_y - (top_y > 0), portx - 1, porty);
		++portx;
	    }
	    break;

	case KEY_IL:		/* increase-lines: move bottom-edge down */
	    if (porty >= pymax || porty >= LINES)
		beep();
	    else {
		panner_h_cleanup(porty - 1, top_x - (top_x > 0), portx);
		++porty;
	    }
	    break;

	case KEY_DC:		/* decrease-columns: move bottom edge up */
	    if (portx <= top_x)
		beep();
	    else {
		portx--;
		panner_v_cleanup(top_y - (top_y > 0), portx, porty);
	    }
	    break;

	case KEY_DL:		/* decrease-lines */
	    if (porty <= top_y)
		beep();
	    else {
		porty--;
		panner_h_cleanup(porty, top_x - (top_x > 0), portx);
	    }
	    break;

	case KEY_LEFT:		/* pan leftwards */
	    if (basex > 0)
		basex--;
	    else
		beep();
	    break;

	case KEY_RIGHT:	/* pan rightwards */
	    if (basex + portx - (pymax > porty) < pxmax)
		basex++;
	    else
		beep();
	    break;

	case KEY_UP:		/* pan upwards */
	    if (basey > 0)
		basey--;
	    else
		beep();
	    break;

	case KEY_DOWN:		/* pan downwards */
	    if (basey + porty - (pxmax > portx) < pymax)
		basey++;
	    else
		beep();
	    break;

	case 'H':
	case KEY_HOME:
	case KEY_FIND:
	    basey = 0;
	    break;

	case 'E':
	case KEY_END:
	case KEY_SELECT:
	    basey = pymax - porty;
	    if (basey < 0)
		basey = 0;
	    break;

	default:
	    beep();
	    break;
	}

	mvaddch(top_y - 1, top_x - 1, ACS_ULCORNER);
	do_v_line(top_y, top_x - 1, ACS_VLINE, porty);
	do_h_line(top_y - 1, top_x, ACS_HLINE, portx);

	if (scrollers && (pxmax > portx - 1)) {
	    int length = (portx - top_x - 1);
	    float ratio = ((float) length) / ((float) pxmax);

	    lowend = (int) ((float) top_x + ((float) basex * ratio));
	    highend = (int) ((float) top_x + ((float) (basex + length) * ratio));

	    do_h_line(porty - 1, top_x, ACS_HLINE, lowend);
	    if (highend < portx) {
		attron(A_REVERSE);
		do_h_line(porty - 1, lowend, ' ', highend + 1);
		attroff(A_REVERSE);
		do_h_line(porty - 1, highend + 1, ACS_HLINE, portx);
	    }
	} else
	    do_h_line(porty - 1, top_x, ACS_HLINE, portx);

	if (scrollers && (pymax > porty - 1)) {
	    int length = (porty - top_y - 1);
	    float ratio = ((float) length) / ((float) pymax);

	    lowend = (int) ((float) top_y + ((float) basey * ratio));
	    highend = (int) ((float) top_y + ((float) (basey + length) * ratio));

	    do_v_line(top_y, portx - 1, ACS_VLINE, lowend);
	    if (highend < porty) {
		attron(A_REVERSE);
		do_v_line(lowend, portx - 1, ' ', highend + 1);
		attroff(A_REVERSE);
		do_v_line(highend + 1, portx - 1, ACS_VLINE, porty);
	    }
	} else
	    do_v_line(top_y, portx - 1, ACS_VLINE, porty);

	mvaddch(top_y - 1, portx - 1, ACS_URCORNER);
	mvaddch(porty - 1, top_x - 1, ACS_LLCORNER);
	mvaddch(porty - 1, portx - 1, ACS_LRCORNER);

	if (!pending_pan) {
#if HAVE_GETTIMEOFDAY
	    gettimeofday(&before, 0);
#endif
	    wnoutrefresh(stdscr);

	    pnoutrefresh(pad,
			 basey, basex,
			 top_y, top_x,
			 porty - (pxmax > portx) - 1,
			 portx - (pymax > porty) - 1);

	    doupdate();
#if HAVE_GETTIMEOFDAY
	    if (timing) {
		double elapsed;
		gettimeofday(&after, 0);
		elapsed = (after.tv_sec + after.tv_usec / 1.0e6)
		    - (before.tv_sec + before.tv_usec / 1.0e6);
		move(LINES - 1, COLS - 12);
		printw("Secs: %2.03f", elapsed);
		refresh();
	    }
#endif
	}

    } while
	((c = pgetc(pad)) != KEY_EXIT);

    scrollok(stdscr, TRUE);	/* reset to driver's default */
}

static int
padgetch(WINDOW *win)
{
    static int count;
    static int last;
    int c;

    if ((pending_pan = (count > 0)) != FALSE) {
	count--;
	pending_pan = (count != 0);
    } else {
	for (;;) {
	    switch (c = wGetchar(win)) {
	    case '!':
		ShellOut(FALSE);
		/* FALLTHRU */
	    case CTRL('r'):
		endwin();
		refresh();
		c = KEY_REFRESH;
		break;
	    case CTRL('l'):
		c = KEY_REFRESH;
		break;
	    case 'U':
		c = KEY_UP;
		break;
	    case 'D':
		c = KEY_DOWN;
		break;
	    case 'R':
		c = KEY_RIGHT;
		break;
	    case 'L':
		c = KEY_LEFT;
		break;
	    case '+':
		c = KEY_IL;
		break;
	    case '-':
		c = KEY_DL;
		break;
	    case '>':
		c = KEY_IC;
		break;
	    case '<':
		c = KEY_DC;
		break;
	    case ERR:		/* FALLTHRU */
	    case case_QUIT:
		count = 0;
		c = KEY_EXIT;
		break;
	    default:
		if (c >= '0' && c <= '9') {
		    count = count * 10 + (c - '0');
		    continue;
		}
		break;
	    }
	    last = c;
	    break;
	}
	if (count > 0)
	    count--;
    }
    return (last);
}

#define PAD_HIGH 200
#define PAD_WIDE 200

static void
demo_pad(void)
/* Demonstrate pads. */
{
    WINDOW *panpad = newpad(PAD_HIGH, PAD_WIDE);

    if (panpad == 0) {
	Cannot("cannot create requested pad");
	return;
    }

    fill_pad(panpad, FALSE);

    panner_legend(LINES - 4);
    panner_legend(LINES - 3);
    panner_legend(LINES - 2);
    panner_legend(LINES - 1);

    keypad(panpad, TRUE);

    /* Make the pad (initially) narrow enough that a trace file won't wrap.
     * We'll still be able to widen it during a test, since that's required
     * for testing boundaries.
     */
    panner(panpad, 2, 2, LINES - 5, COLS - 15, padgetch);

    delwin(panpad);
    endwin();
    erase();
}

/****************************************************************************
 *
 * Tests from John Burnell's PDCurses tester
 *
 ****************************************************************************/

static void
Continue(WINDOW *win)
{
    noecho();
    wmove(win, 10, 1);
    mvwaddstr(win, 10, 1, " Press any key to continue");
    wrefresh(win);
    wGetchar(win);
}

static void
flushinp_test(WINDOW *win)
/* Input test, adapted from John Burnell's PDCurses tester */
{
    int w, h, bx, by, sw, sh, i;

    WINDOW *subWin;
    wclear(win);

    getmaxyx(win, h, w);
    getbegyx(win, by, bx);
    sw = w / 3;
    sh = h / 3;
    if ((subWin = subwin(win, sh, sw, by + h - sh - 2, bx + w - sw - 2)) == 0)
	return;

#ifdef A_COLOR
    if (use_colors) {
	init_pair(2, COLOR_CYAN, COLOR_BLUE);
	wbkgd(subWin, COLOR_PAIR(2) | ' ');
    }
#endif
    wattrset(subWin, A_BOLD);
    box(subWin, ACS_VLINE, ACS_HLINE);
    mvwaddstr(subWin, 2, 1, "This is a subwindow");
    wrefresh(win);

    /*
     * This used to set 'nocbreak()'.  However, Alexander Lukyanov says that
     * it only happened to "work" on SVr4 because that implementation does not
     * emulate nocbreak+noecho mode, whereas ncurses does.  To get the desired
     * test behavior, we're using 'cbreak()', which will allow a single
     * character to return without needing a newline. - T.Dickey 1997/10/11.
     */
    cbreak();
    mvwaddstr(win, 0, 1, "This is a test of the flushinp() call.");

    mvwaddstr(win, 2, 1, "Type random keys for 5 seconds.");
    mvwaddstr(win, 3, 1,
	      "These should be discarded (not echoed) after the subwindow goes away.");
    wrefresh(win);

    for (i = 0; i < 5; i++) {
	mvwprintw(subWin, 1, 1, "Time = %d", i);
	wrefresh(subWin);
	napms(1000);
	flushinp();
    }

    delwin(subWin);
    werase(win);
    flash();
    wrefresh(win);
    napms(1000);

    mvwaddstr(win, 2, 1,
	      "If you were still typing when the window timer expired,");
    mvwaddstr(win, 3, 1,
	      "or else you typed nothing at all while it was running,");
    mvwaddstr(win, 4, 1,
	      "test was invalid.  You'll see garbage or nothing at all. ");
    mvwaddstr(win, 6, 1, "Press a key");
    wmove(win, 9, 10);
    wrefresh(win);
    echo();
    wGetchar(win);
    flushinp();
    mvwaddstr(win, 12, 0,
	      "If you see any key other than what you typed, flushinp() is broken.");
    Continue(win);

    wmove(win, 9, 10);
    wdelch(win);
    wrefresh(win);
    wmove(win, 12, 0);
    clrtoeol();
    waddstr(win,
	    "What you typed should now have been deleted; if not, wdelch() failed.");
    Continue(win);

    cbreak();
}

/****************************************************************************
 *
 * Menu test
 *
 ****************************************************************************/

#if USE_LIBMENU

#define MENU_Y	8
#define MENU_X	8

static int
menu_virtualize(int c)
{
    if (c == '\n' || c == KEY_EXIT)
	return (MAX_COMMAND + 1);
    else if (c == 'u')
	return (REQ_SCR_ULINE);
    else if (c == 'd')
	return (REQ_SCR_DLINE);
    else if (c == 'b' || c == KEY_NPAGE)
	return (REQ_SCR_UPAGE);
    else if (c == 'f' || c == KEY_PPAGE)
	return (REQ_SCR_DPAGE);
    else if (c == 'n' || c == KEY_DOWN)
	return (REQ_NEXT_ITEM);
    else if (c == 'p' || c == KEY_UP)
	return (REQ_PREV_ITEM);
    else if (c == ' ')
	return (REQ_TOGGLE_ITEM);
    else {
	if (c != KEY_MOUSE)
	    beep();
	return (c);
    }
}

static CONST_MENUS char *animals[] =
{
    "Lions",
    "Tigers",
    "Bears",
    "(Oh my!)",
    "Newts",
    "Platypi",
    "Lemurs",
    "(Oh really?!)",
    "Leopards",
    "Panthers",
    "Pumas",
    "Lions, Tigers, Bears, (Oh my!), Newts, Platypi, Lemurs",
    "Lions, Tigers, Bears, (Oh my!), Newts, Platypi, Lemurs, Lions, Tigers, Bears, (Oh my!), Newts, Platypi, Lemurs",
    (char *) 0
};

static void
menu_test(void)
{
    MENU *m;
    ITEM *items[SIZEOF(animals)];
    ITEM **ip = items;
    CONST_MENUS char **ap;
    int mrows, mcols, c;
    WINDOW *menuwin;

#ifdef NCURSES_MOUSE_VERSION
    mousemask(ALL_MOUSE_EVENTS, (mmask_t *) 0);
#endif
    mvaddstr(0, 0, "This is the menu test:");
    mvaddstr(2, 0, "  Use up and down arrow to move the select bar.");
    mvaddstr(3, 0, "  'n' and 'p' act like arrows.");
    mvaddstr(4, 0,
	     "  'b' and 'f' scroll up/down (page), 'u' and 'd' (line).");
    mvaddstr(5, 0, "  Press return to exit.");
    refresh();

    for (ap = animals; *ap; ap++) {
	if ((*ip = new_item(*ap, "")) != 0)
	    ++ip;
    }
    *ip = (ITEM *) 0;

    m = new_menu(items);

    set_menu_format(m, (SIZEOF(animals) + 1) / 2, 1);
    scale_menu(m, &mrows, &mcols);

    menuwin = newwin(mrows + 2, mcols + 2, MENU_Y, MENU_X);
    set_menu_win(m, menuwin);
    keypad(menuwin, TRUE);
    box(menuwin, 0, 0);

    set_menu_sub(m, derwin(menuwin, mrows, mcols, 1, 1));

    post_menu(m);

    while ((c = menu_driver(m, menu_virtualize(wGetchar(menuwin)))) != E_UNKNOWN_COMMAND) {
	if (c == E_NOT_POSTED)
	    break;
	if (c == E_REQUEST_DENIED)
	    beep();
	continue;
    }

    (void) mvprintw(LINES - 2, 0,
		    "You chose: %s\n", item_name(current_item(m)));
    (void) addstr("Press any key to continue...");
    wGetchar(stdscr);

    unpost_menu(m);
    delwin(menuwin);

    free_menu(m);
    for (ip = items; *ip; ip++)
	free_item(*ip);
#ifdef NCURSES_MOUSE_VERSION
    mousemask(0, (mmask_t *) 0);
#endif
}

#ifdef TRACE
#define T_TBL(name) { #name, name }
static struct {
    const char *name;
    unsigned mask;
} t_tbl[] = {

    T_TBL(TRACE_DISABLE),
	T_TBL(TRACE_TIMES),
	T_TBL(TRACE_TPUTS),
	T_TBL(TRACE_UPDATE),
	T_TBL(TRACE_MOVE),
	T_TBL(TRACE_CHARPUT),
	T_TBL(TRACE_ORDINARY),
	T_TBL(TRACE_CALLS),
	T_TBL(TRACE_VIRTPUT),
	T_TBL(TRACE_IEVENT),
	T_TBL(TRACE_BITS),
	T_TBL(TRACE_ICALLS),
	T_TBL(TRACE_CCALLS),
	T_TBL(TRACE_DATABASE),
	T_TBL(TRACE_ATTRS),
	T_TBL(TRACE_MAXIMUM),
    {
	(char *) 0, 0
    }
};

static char *
tracetrace(unsigned tlevel)
{
    static char *buf;
    int n;

    if (buf == 0) {
	size_t need = 12;
	for (n = 0; t_tbl[n].name != 0; n++)
	    need += strlen(t_tbl[n].name) + 2;
	buf = typeMalloc(char, need);
    }
    sprintf(buf, "0x%02x = {", tlevel);
    if (tlevel == 0) {
	sprintf(buf + strlen(buf), "%s, ", t_tbl[0].name);
    } else {
	for (n = 1; t_tbl[n].name != 0; n++)
	    if ((tlevel & t_tbl[n].mask) == t_tbl[n].mask) {
		strcat(buf, t_tbl[n].name);
		strcat(buf, ", ");
	    }
    }
    if (buf[strlen(buf) - 2] == ',')
	buf[strlen(buf) - 2] = '\0';
    return (strcat(buf, "}"));
}

/* fake a dynamically reconfigurable menu using the 0th entry to deselect
 * the others
 */
static int
run_trace_menu(MENU * m)
{
    ITEM **items;
    ITEM *i, **p;

    for (;;) {
	bool changed = FALSE;
	switch (menu_driver(m, menu_virtualize(wGetchar(menu_win(m))))) {
	case E_UNKNOWN_COMMAND:
	    return FALSE;
	default:
	    items = menu_items(m);
	    i = current_item(m);
	    if (i == items[0]) {
		if (item_value(i)) {
		    for (p = items + 1; *p != 0; p++)
			if (item_value(*p)) {
			    set_item_value(*p, FALSE);
			    changed = TRUE;
			}
		}
	    } else {
		for (p = items + 1; *p != 0; p++)
		    if (item_value(*p)) {
			set_item_value(items[0], FALSE);
			changed = TRUE;
			break;
		    }
	    }
	    if (!changed)
		return TRUE;
	}
    }
}

static void
trace_set(void)
/* interactively set the trace level */
{
    MENU *m;
    ITEM *items[SIZEOF(t_tbl)];
    ITEM **ip = items;
    int mrows, mcols;
    unsigned newtrace;
    int n;
    WINDOW *menuwin;

    mvaddstr(0, 0, "Interactively set trace level:");
    mvaddstr(2, 0, "  Press space bar to toggle a selection.");
    mvaddstr(3, 0, "  Use up and down arrow to move the select bar.");
    mvaddstr(4, 0, "  Press return to set the trace level.");
    mvprintw(6, 0, "(Current trace level is %s)", tracetrace(_nc_tracing));

    refresh();

    for (n = 0; t_tbl[n].name != 0; n++) {
	if ((*ip = new_item(t_tbl[n].name, "")) != 0) {
	    ++ip;
	}
    }
    *ip = (ITEM *) 0;

    m = new_menu(items);

    set_menu_format(m, 0, 2);
    scale_menu(m, &mrows, &mcols);

    menu_opts_off(m, O_ONEVALUE);
    menuwin = newwin(mrows + 2, mcols + 2, MENU_Y, MENU_X);
    set_menu_win(m, menuwin);
    keypad(menuwin, TRUE);
    box(menuwin, 0, 0);

    set_menu_sub(m, derwin(menuwin, mrows, mcols, 1, 1));

    post_menu(m);

    for (ip = menu_items(m); *ip; ip++) {
	unsigned mask = t_tbl[item_index(*ip)].mask;
	if (mask == 0)
	    set_item_value(*ip, _nc_tracing == 0);
	else if ((mask & _nc_tracing) == mask)
	    set_item_value(*ip, TRUE);
    }

    while (run_trace_menu(m))
	continue;

    newtrace = 0;
    for (ip = menu_items(m); *ip; ip++)
	if (item_value(*ip))
	    newtrace |= t_tbl[item_index(*ip)].mask;
    trace(newtrace);
    Trace(("trace level interactively set to %s", tracetrace(_nc_tracing)));

    (void) mvprintw(LINES - 2, 0,
		    "Trace level is %s\n", tracetrace(_nc_tracing));
    (void) addstr("Press any key to continue...");
    wGetchar(stdscr);

    unpost_menu(m);
    delwin(menuwin);

    free_menu(m);
    for (ip = items; *ip; ip++)
	free_item(*ip);
}
#endif /* TRACE */
#endif /* USE_LIBMENU */

/****************************************************************************
 *
 * Forms test
 *
 ****************************************************************************/
#if USE_LIBFORM
static FIELD *
make_label(int frow, int fcol, NCURSES_CONST char *label)
{
    FIELD *f = new_field(1, (int) strlen(label), frow, fcol, 0, 0);

    if (f) {
	set_field_buffer(f, 0, label);
	set_field_opts(f, (int) (field_opts(f) & ~O_ACTIVE));
    }
    return (f);
}

static FIELD *
make_field(int frow, int fcol, int rows, int cols, bool secure)
{
    FIELD *f = new_field(rows, cols, frow, fcol, 0, secure ? 1 : 0);

    if (f) {
	set_field_back(f, A_UNDERLINE);
	set_field_userptr(f, (void *) 0);
    }
    return (f);
}

static void
display_form(FORM * f)
{
    WINDOW *w;
    int rows, cols;

    scale_form(f, &rows, &cols);

    if ((w = newwin(rows + 2, cols + 4, 0, 0)) != (WINDOW *) 0) {
	set_form_win(f, w);
	set_form_sub(f, derwin(w, rows, cols, 1, 2));
	box(w, 0, 0);
	keypad(w, TRUE);
    }

    if (post_form(f) != E_OK)
	wrefresh(w);
}

static void
erase_form(FORM * f)
{
    WINDOW *w = form_win(f);
    WINDOW *s = form_sub(f);

    unpost_form(f);
    werase(w);
    wrefresh(w);
    delwin(s);
    delwin(w);
}

static int
edit_secure(FIELD * me, int c)
{
    int rows, cols, frow, fcol, nrow, nbuf;

    if (field_info(me, &rows, &cols, &frow, &fcol, &nrow, &nbuf) == E_OK
	&& nbuf > 0) {
	char *source = field_buffer(me, 1);
	char temp[80];
	long len;

	strcpy(temp, source ? source : "");
	len = (long) (char *) field_userptr(me);
	if (c <= KEY_MAX) {
	    if (isgraph(c) && (len + 1) < (int) sizeof(temp)) {
		temp[len++] = (char) c;
		temp[len] = 0;
		set_field_buffer(me, 1, temp);
		c = '*';
	    } else {
		c = 0;
	    }
	} else {
	    switch (c) {
	    case REQ_BEG_FIELD:
	    case REQ_CLR_EOF:
	    case REQ_CLR_EOL:
	    case REQ_DEL_LINE:
	    case REQ_DEL_WORD:
	    case REQ_DOWN_CHAR:
	    case REQ_END_FIELD:
	    case REQ_INS_CHAR:
	    case REQ_INS_LINE:
	    case REQ_LEFT_CHAR:
	    case REQ_NEW_LINE:
	    case REQ_NEXT_WORD:
	    case REQ_PREV_WORD:
	    case REQ_RIGHT_CHAR:
	    case REQ_UP_CHAR:
		c = 0;		/* we don't want to do inline editing */
		break;
	    case REQ_CLR_FIELD:
		if (len) {
		    temp[0] = 0;
		    set_field_buffer(me, 1, temp);
		}
		break;
	    case REQ_DEL_CHAR:
	    case REQ_DEL_PREV:
		if (len) {
		    temp[--len] = 0;
		    set_field_buffer(me, 1, temp);
		}
		break;
	    }
	}
	set_field_userptr(me, (void *) len);
    }
    return c;
}

static int
form_virtualize(FORM * f, WINDOW *w)
{
    /* *INDENT-OFF* */
    static const struct {
	int code;
	int result;
    } lookup[] = {
	{ CTRL('A'),	REQ_NEXT_CHOICE },
	{ CTRL('B'),	REQ_PREV_WORD },
	{ CTRL('C'),	REQ_CLR_EOL },
	{ CTRL('D'),	REQ_DOWN_FIELD },
	{ CTRL('E'),	REQ_END_FIELD },
	{ CTRL('F'),	REQ_NEXT_PAGE },
	{ CTRL('G'),	REQ_DEL_WORD },
	{ CTRL('H'),	REQ_DEL_PREV },
	{ CTRL('I'),	REQ_INS_CHAR },
	{ CTRL('K'),	REQ_CLR_EOF },
	{ CTRL('L'),	REQ_LEFT_FIELD },
	{ CTRL('M'),	REQ_NEW_LINE },
	{ CTRL('N'),	REQ_NEXT_FIELD },
	{ CTRL('O'),	REQ_INS_LINE },
	{ CTRL('P'),	REQ_PREV_FIELD },
	{ CTRL('R'),	REQ_RIGHT_FIELD },
	{ CTRL('S'),	REQ_BEG_FIELD },
	{ CTRL('U'),	REQ_UP_FIELD },
	{ CTRL('V'),	REQ_DEL_CHAR },
	{ CTRL('W'),	REQ_NEXT_WORD },
	{ CTRL('X'),	REQ_CLR_FIELD },
	{ CTRL('Y'),	REQ_DEL_LINE },
	{ CTRL('Z'),	REQ_PREV_CHOICE },
	{ ESCAPE,	MAX_FORM_COMMAND + 1 },
	{ KEY_BACKSPACE, REQ_DEL_PREV },
	{ KEY_DOWN,	REQ_DOWN_CHAR },
	{ KEY_END,	REQ_LAST_FIELD },
	{ KEY_HOME,	REQ_FIRST_FIELD },
	{ KEY_LEFT,	REQ_LEFT_CHAR },
	{ KEY_LL,	REQ_LAST_FIELD },
	{ KEY_NEXT,	REQ_NEXT_FIELD },
	{ KEY_NPAGE,	REQ_NEXT_PAGE },
	{ KEY_PPAGE,	REQ_PREV_PAGE },
	{ KEY_PREVIOUS, REQ_PREV_FIELD },
	{ KEY_RIGHT,	REQ_RIGHT_CHAR },
	{ KEY_UP,	REQ_UP_CHAR },
	{ QUIT,		MAX_FORM_COMMAND + 1 }
    };
    /* *INDENT-ON* */

    static int mode = REQ_INS_MODE;
    int c = wGetchar(w);
    unsigned n;
    FIELD *me = current_field(f);
    bool current = TRUE;

    if (c == CTRL(']')) {
	if (mode == REQ_INS_MODE) {
	    mode = REQ_OVL_MODE;
	} else {
	    mode = REQ_INS_MODE;
	}
	c = mode;
    } else {
	for (n = 0; n < SIZEOF(lookup); n++) {
	    if (lookup[n].code == c) {
		c = lookup[n].result;
		break;
	    }
	}
    }
    mvprintw(0, COLS - 6, "(%s)", mode == REQ_INS_MODE ? "INS" : "OVL");

    /*
     * Force the field that the user is typing into to be in reverse video,
     * while the other fields are shown underlined.
     */
    switch (c) {
    case REQ_BEG_FIELD:
    case REQ_CLR_EOF:
    case REQ_CLR_EOL:
    case REQ_CLR_FIELD:
    case REQ_DEL_CHAR:
    case REQ_DEL_LINE:
    case REQ_DEL_PREV:
    case REQ_DEL_WORD:
    case REQ_END_FIELD:
    case REQ_INS_CHAR:
    case REQ_INS_LINE:
    case REQ_LEFT_CHAR:
    case REQ_LEFT_FIELD:
    case REQ_NEXT_WORD:
    case REQ_RIGHT_CHAR:
	current = TRUE;
	break;
    default:
	current = (c < KEY_MAX);
	break;
    }
    if (current) {
	c = edit_secure(me, c);
	set_field_back(me, A_REVERSE);
    } else {
	c = edit_secure(me, c);
	set_field_back(me, A_UNDERLINE);
    }
    return c;
}

static int
my_form_driver(FORM * form, int c)
{
    if (c == (MAX_FORM_COMMAND + 1)
	&& form_driver(form, REQ_VALIDATION) == E_OK)
	return (TRUE);
    else {
	beep();
	return (FALSE);
    }
}

#ifdef NCURSES_VERSION
#define FIELDCHECK_CB(func) bool func(FIELD * fld, const void * data GCC_UNUSED)
#define CHAR_CHECK_CB(func) bool func(int ch, const void *data GCC_UNUSED)
#else
#define FIELDCHECK_CB(func) int func(FIELD * fld, char * data GCC_UNUSED)
#define CHAR_CHECK_CB(func) int func(int ch, char *data GCC_UNUSED)
#endif

/*
 * Allow a middle initial, optionally with a '.' to end it.
 */
static
FIELDCHECK_CB(mi_field_check)
{
    char *s = field_buffer(fld, 0);
    int state = 0;
    int n;

    for (n = 0; s[n] != '\0'; ++n) {
	switch (state) {
	case 0:
	    if (s[n] == '.') {
		if (n != 1)
		    return FALSE;
		state = 2;
	    } else if (isspace(UChar(s[n]))) {
		state = 2;
	    }
	    break;
	case 2:
	    if (!isspace(UChar(s[n])))
		return FALSE;
	    break;
	}
    }

    /* force the form to display a leading capital */
    if (islower(UChar(s[0]))) {
	s[0] = (char) toupper(UChar(s[0]));
	set_field_buffer(fld, 0, s);
    }
    return TRUE;
}

static
CHAR_CHECK_CB(mi_char_check)
{
    return ((isalpha(ch) || ch == '.') ? TRUE : FALSE);
}

/*
 * Passwords should be at least 6 characters.
 */
static
FIELDCHECK_CB(pw_field_check)
{
    char *s = field_buffer(fld, 0);
    int n;

    for (n = 0; s[n] != '\0'; ++n) {
	if (isspace(UChar(s[n]))) {
	    if (n < 6)
		return FALSE;
	}
    }
    return TRUE;
}

static
CHAR_CHECK_CB(pw_char_check)
{
    return (isgraph(ch) ? TRUE : FALSE);
}

static void
demo_forms(void)
{
    WINDOW *w;
    FORM *form;
    FIELD *f[12], *secure;
    FIELDTYPE *fty_middle = new_fieldtype(mi_field_check, mi_char_check);
    FIELDTYPE *fty_passwd = new_fieldtype(pw_field_check, pw_char_check);
    int finished = 0, c;
    unsigned n = 0;

#ifdef NCURSES_MOUSE_VERSION
    mousemask(ALL_MOUSE_EVENTS, (mmask_t *) 0);
#endif

    move(18, 0);
    addstr("Defined edit/traversal keys:   ^Q/ESC- exit form\n");
    addstr("^N   -- go to next field       ^P  -- go to previous field\n");
    addstr("Home -- go to first field      End -- go to last field\n");
    addstr("^L   -- go to field to left    ^R  -- go to field to right\n");
    addstr("^U   -- move upward to field   ^D  -- move downward to field\n");
    addstr("^W   -- go to next word        ^B  -- go to previous word\n");
    addstr("^S   -- go to start of field   ^E  -- go to end of field\n");
    addstr("^H   -- delete previous char   ^Y  -- delete line\n");
    addstr("^G   -- delete current word    ^C  -- clear to end of line\n");
    addstr("^K   -- clear to end of field  ^X  -- clear field\n");
    addstr("Arrow keys move within a field as you would expect. ^] toggles overlay mode.");

    mvaddstr(4, 57, "Forms Entry Test");

    refresh();

    /* describe the form */
    memset(f, 0, sizeof(f));
    f[n++] = make_label(0, 15, "Sample Form");

    f[n++] = make_label(2, 0, "Last Name");
    f[n++] = make_field(3, 0, 1, 18, FALSE);
    set_field_type(f[n - 1], TYPE_ALPHA, 1);

    f[n++] = make_label(2, 20, "First Name");
    f[n++] = make_field(3, 20, 1, 12, FALSE);
    set_field_type(f[n - 1], TYPE_ALPHA, 1);

    f[n++] = make_label(2, 34, "Middle Name");
    f[n++] = make_field(3, 34, 1, 12, FALSE);
    set_field_type(f[n - 1], fty_middle);

    f[n++] = make_label(5, 0, "Comments");
    f[n++] = make_field(6, 0, 4, 46, FALSE);

    f[n++] = make_label(5, 20, "Password:");
    secure =
	f[n++] = make_field(5, 30, 1, 9, TRUE);
    set_field_type(f[n - 1], fty_passwd);
    f[n++] = (FIELD *) 0;

    if ((form = new_form(f)) != 0) {

	display_form(form);

	w = form_win(form);
	raw();
	nonl();			/* lets us read ^M's */
	while (!finished) {
	    switch (form_driver(form, c = form_virtualize(form, w))) {
	    case E_OK:
		mvaddstr(5, 57, field_buffer(secure, 1));
		clrtoeol();
		refresh();
		break;
	    case E_UNKNOWN_COMMAND:
		finished = my_form_driver(form, c);
		break;
	    default:
		beep();
		break;
	    }
	}

	erase_form(form);

	free_form(form);
    }
    for (c = 0; f[c] != 0; c++)
	free_field(f[c]);
    free_fieldtype(fty_middle);
    free_fieldtype(fty_passwd);
    noraw();
    nl();

#ifdef NCURSES_MOUSE_VERSION
    mousemask(ALL_MOUSE_EVENTS, (mmask_t *) 0);
#endif
}
#endif /* USE_LIBFORM */

/****************************************************************************
 *
 * Overlap test
 *
 ****************************************************************************/

static void
fillwin(WINDOW *win, char ch)
{
    int y, x;
    int y1, x1;

    getmaxyx(win, y1, x1);
    for (y = 0; y < y1; y++) {
	wmove(win, y, 0);
	for (x = 0; x < x1; x++)
	    waddch(win, UChar(ch));
    }
}

static void
crosswin(WINDOW *win, char ch)
{
    int y, x;
    int y1, x1;

    getmaxyx(win, y1, x1);
    for (y = 0; y < y1; y++) {
	for (x = 0; x < x1; x++)
	    if (((x > (x1 - 1) / 3) && (x <= (2 * (x1 - 1)) / 3))
		|| (((y > (y1 - 1) / 3) && (y <= (2 * (y1 - 1)) / 3)))) {
		wmove(win, y, x);
		waddch(win, UChar(ch));
	    }
    }
}

#define OVERLAP_FLAVORS 5

static void
overlap_helpitem(int state, int item, char *message)
{
    int row = (item / 2);
    int col = ((item % 2) ? COLS / 2 : 0);

    move(LINES - 6 + row, col);
    printw("%c%c = %s", state == row ? '>' : ' ', 'a' + item, message);
    clrtoeol();
}

static void
overlap_test_1_attr(WINDOW *win, int flavor, int col)
{
    short cpair = (short) (1 + (flavor * 2) + col);

    switch (flavor) {
    case 0:
	wattrset(win, A_NORMAL);
	break;
    case 1:
	wattrset(win, A_BOLD);
	break;
    case 2:
	init_pair(cpair, COLOR_BLUE, COLOR_WHITE);
	wattrset(win, COLOR_PAIR(cpair) | A_NORMAL);
	break;
    case 3:
	init_pair(cpair, COLOR_WHITE, COLOR_BLUE);
	wattrset(win, COLOR_PAIR(cpair) | A_BOLD);
	break;
    }
}

static void
overlap_test_2_attr(WINDOW *win, int flavor, int col)
{
    short cpair = (short) (9 + (flavor * 2) + col);

    switch (flavor) {
    case 0:
	/* no effect */
	break;
    case 1:
	/* no effect */
	break;
    case 2:
	init_pair(cpair, COLOR_RED, COLOR_GREEN);
	wbkgdset(win, ' ' | A_BLINK | COLOR_PAIR(cpair));
	break;
    case 3:
	wbkgdset(win, ' ' | A_NORMAL);
	break;
    }
}

static int
overlap_help(int state, int flavors[OVERLAP_FLAVORS])
{
    int row;
    int col;
    int item;
    const char *ths, *tht;
    char msg[80];

    if (state < 0)
	state += OVERLAP_FLAVORS;
    state = state % OVERLAP_FLAVORS;
    assert(state >= 0 && state < OVERLAP_FLAVORS);

    for (item = 0; item < (2 * OVERLAP_FLAVORS); ++item) {
	row = item / 2;
	col = item % 2;
	ths = col ? "B" : "A";
	tht = col ? "A" : "B";

	switch (row) {
	case 0:
	    flavors[row] = 0;
	    sprintf(msg, "refresh %s, then %s, then doupdate.", ths, tht);
	    break;
	case 1:
	    if (use_colors) {
		flavors[row] %= 4;
	    } else {
		flavors[row] %= 2;
	    }
	    overlap_test_1_attr(stdscr, flavors[row], col);
	    sprintf(msg, "fill window %s with letter %s.", ths, ths);
	    break;
	case 2:
	    if (use_colors) {
		flavors[row] %= 4;
	    } else {
		flavors[row] %= 2;
	    }
	    switch (flavors[row]) {
	    case 0:
		sprintf(msg, "cross pattern in window %s.", ths);
		break;
	    case 1:
		sprintf(msg, "draw box in window %s.", ths);
		break;
	    case 2:
		sprintf(msg, "set background of window %s.", ths);
		break;
	    case 3:
		sprintf(msg, "reset background of window %s.", ths);
		break;
	    }
	    break;
	case 3:
	    flavors[row] = 0;
	    sprintf(msg, "clear window %s.", ths);
	    break;
	case 4:
	    flavors[row] %= 4;
	    switch (flavors[row]) {
	    case 0:
		sprintf(msg, "overwrite %s onto %s.", ths, tht);
		break;
	    case 1:
		sprintf(msg, "copywin(FALSE) %s onto %s.", ths, tht);
		break;
	    case 2:
		sprintf(msg, "copywin(TRUE) %s onto %s.", ths, tht);
		break;
	    case 3:
		sprintf(msg, "overlay %s onto %s.", ths, tht);
		break;
	    }
	    break;
	}
	overlap_helpitem(state, item, msg);
	wattrset(stdscr, A_NORMAL);
	wbkgdset(stdscr, ' ' | A_NORMAL);
    }
    move(LINES - 1, 0);
    printw("^Q/ESC = terminate test.  Up/down/space select test variations (%d %d).",
	   state, flavors[state]);

    return state;
}

static void
overlap_test_0(WINDOW *a, WINDOW *b)
{
    touchwin(a);
    touchwin(b);
    wnoutrefresh(a);
    wnoutrefresh(b);
    doupdate();
}

static void
overlap_test_1(int flavor, int col, WINDOW *a, char fill)
{
    overlap_test_1_attr(a, flavor, col);
    fillwin(a, fill);
    wattrset(a, A_NORMAL);
}

static void
overlap_test_2(int flavor, int col, WINDOW *a, char fill)
{
    overlap_test_2_attr(a, flavor, col);
    switch (flavor) {
    case 0:
	crosswin(a, fill);
	break;
    case 1:
	box(a, 0, 0);
	break;
    case 2:
	/* done in overlap_test_2_attr */
	break;
    case 3:
	/* done in overlap_test_2_attr */
	break;
    }
}

static void
overlap_test_3(WINDOW *a)
{
    wclear(a);
    wmove(a, 0, 0);
}

static void
overlap_test_4(int flavor, WINDOW *a, WINDOW *b)
{
    switch (flavor) {
    case 0:
	overwrite(a, b);
	break;
    case 1:
	copywin(a, b, 0, 0, 0, 0, getmaxy(b), getmaxx(b), FALSE);
	break;
    case 2:
	copywin(a, b, 0, 0, 0, 0, getmaxy(b), getmaxx(b), TRUE);
	break;
    case 3:
	overlay(a, b);
	break;
    }
}

/* test effects of overlapping windows */
static void
overlap_test(void)
{
    int ch;
    int state, flavor[OVERLAP_FLAVORS];

    WINDOW *win1 = newwin(9, 20, 3, 3);
    WINDOW *win2 = newwin(9, 20, 9, 16);

    curs_set(0);
    raw();
    refresh();
    move(0, 0);
    printw("This test shows the behavior of wnoutrefresh() with respect to\n");
    printw("the shared region of two overlapping windows A and B.  The cross\n");
    printw("pattern in each window does not overlap the other.\n");

    memset(flavor, 0, sizeof(flavor));
    state = overlap_help(0, flavor);

    while (!isQuit(ch = Getchar()))
	switch (ch) {
	case 'a':		/* refresh window A first, then B */
	    overlap_test_0(win1, win2);
	    break;

	case 'b':		/* refresh window B first, then A */
	    overlap_test_0(win2, win1);
	    break;

	case 'c':		/* fill window A so it's visible */
	    overlap_test_1(flavor[1], 0, win1, 'A');
	    break;

	case 'd':		/* fill window B so it's visible */
	    overlap_test_1(flavor[1], 1, win2, 'B');
	    break;

	case 'e':		/* cross test pattern in window A */
	    overlap_test_2(flavor[2], 0, win1, 'A');
	    break;

	case 'f':		/* cross test pattern in window A */
	    overlap_test_2(flavor[2], 1, win2, 'B');
	    break;

	case 'g':		/* clear window A */
	    overlap_test_3(win1);
	    break;

	case 'h':		/* clear window B */
	    overlap_test_3(win2);
	    break;

	case 'i':		/* overwrite A onto B */
	    overlap_test_4(flavor[4], win1, win2);
	    break;

	case 'j':		/* overwrite B onto A */
	    overlap_test_4(flavor[4], win2, win1);
	    break;

	case CTRL('n'):
	case KEY_DOWN:
	    state = overlap_help(state + 1, flavor);
	    break;

	case CTRL('p'):
	case KEY_UP:
	    state = overlap_help(state - 1, flavor);
	    break;

	case ' ':
	    flavor[state] += 1;
	    state = overlap_help(state, flavor);
	    break;

	case '?':
	    state = overlap_help(state, flavor);
	    break;

	default:
	    beep();
	    break;
	}

    delwin(win2);
    delwin(win1);
    erase();
    curs_set(1);
    endwin();
}

/****************************************************************************
 *
 * Main sequence
 *
 ****************************************************************************/

static bool
do_single_test(const char c)
/* perform a single specified test */
{
    switch (c) {
    case 'a':
	getch_test();
	break;

#if USE_WIDEC_SUPPORT
    case 'A':
	get_wch_test();
	break;
#endif

    case 'b':
	attr_test();
	break;

#if USE_WIDEC_SUPPORT
    case 'B':
	wide_attr_test();
	break;
#endif

    case 'c':
	if (!use_colors)
	    Cannot("does not support color.");
	else
	    color_test();
	break;

#if USE_WIDEC_SUPPORT
    case 'C':
	if (!use_colors)
	    Cannot("does not support color.");
	else
	    wide_color_test();
	break;
#endif

    case 'd':
	if (!use_colors)
	    Cannot("does not support color.");
	else if (!can_change_color())
	    Cannot("has hardwired color values.");
	else
	    color_edit();
	break;

#if USE_SOFTKEYS
    case 'e':
	slk_test();
	break;
#endif

#if USE_WIDEC_SUPPORT
    case 'E':
	wide_slk_test();
	break;
#endif
    case 'f':
	acs_display();
	break;

#if USE_WIDEC_SUPPORT
    case 'F':
	wide_acs_display();
	break;
#endif

#if USE_LIBPANEL
    case 'o':
	demo_panels(init_panel, fill_panel);
	break;
#endif

#if USE_WIDEC_SUPPORT && USE_LIBPANEL
    case 'O':
	demo_panels(init_wide_panel, fill_wide_panel);
	break;
#endif

    case 'g':
	acs_and_scroll();
	break;

    case 'i':
	flushinp_test(stdscr);
	break;

    case 'k':
	test_sgr_attributes();
	break;

#if USE_LIBMENU
    case 'm':
	menu_test();
	break;
#endif

    case 'p':
	demo_pad();
	break;

#if USE_LIBFORM
    case 'r':
	demo_forms();
	break;
#endif

    case 's':
	overlap_test();
	break;

#if USE_LIBMENU && defined(TRACE)
    case 't':
	trace_set();
	break;
#endif

    case '?':
	break;

    default:
	return FALSE;
    }

    return TRUE;
}

static void
usage(void)
{
    static const char *const tbl[] =
    {
	"Usage: ncurses [options]"
	,""
	,"Options:"
#ifdef NCURSES_VERSION
	,"  -a f,b   set default-colors (assumed white-on-black)"
	,"  -d       use default-colors if terminal supports them"
#endif
#if USE_SOFTKEYS
	,"  -e fmt   specify format for soft-keys test (e)"
#endif
#if HAVE_RIPOFFLINE
	,"  -f       rip-off footer line (can repeat)"
	,"  -h       rip-off header line (can repeat)"
#endif
	,"  -m       do not use colors"
	,"  -p file  rgb values to use in 'd' rather than ncurses's builtin"
#if USE_LIBPANEL
	,"  -s msec  specify nominal time for panel-demo (default: 1, to hold)"
#endif
#ifdef TRACE
	,"  -t mask  specify default trace-level (may toggle with ^T)"
#endif
    };
    size_t n;
    for (n = 0; n < SIZEOF(tbl); n++)
	fprintf(stderr, "%s\n", tbl[n]);
    ExitProgram(EXIT_FAILURE);
}

static void
set_terminal_modes(void)
{
    noraw();
    cbreak();
    noecho();
    scrollok(stdscr, TRUE);
    idlok(stdscr, TRUE);
    keypad(stdscr, TRUE);
}

#ifdef SIGUSR1
static RETSIGTYPE
announce_sig(int sig)
{
    (void) fprintf(stderr, "Handled signal %d\r\n", sig);
}
#endif

#if HAVE_RIPOFFLINE
static int
rip_footer(WINDOW *win, int cols)
{
    wbkgd(win, A_REVERSE);
    werase(win);
    wmove(win, 0, 0);
    wprintw(win, "footer: window %p, %d columns", win, cols);
    wnoutrefresh(win);
    return OK;
}

static int
rip_header(WINDOW *win, int cols)
{
    wbkgd(win, A_REVERSE);
    werase(win);
    wmove(win, 0, 0);
    wprintw(win, "header: window %p, %d columns", win, cols);
    wnoutrefresh(win);
    return OK;
}
#endif /* HAVE_RIPOFFLINE */

static void
main_menu(bool top)
{
    char command;

    do {
	(void) puts("This is the ncurses main menu");
	(void) puts("a = keyboard and mouse input test");
#if USE_WIDEC_SUPPORT
	(void) puts("A = wide-character keyboard and mouse input test");
#endif
	(void) puts("b = character attribute test");
#if USE_WIDEC_SUPPORT
	(void) puts("B = wide-character attribute test");
#endif
	(void) puts("c = color test pattern");
#if USE_WIDEC_SUPPORT
	(void) puts("C = color test pattern using wide-character calls");
#endif
	if (top)
	    (void) puts("d = edit RGB color values");
#if USE_SOFTKEYS
	(void) puts("e = exercise soft keys");
#if USE_WIDEC_SUPPORT
	(void) puts("E = exercise soft keys using wide-characters");
#endif
#endif
	(void) puts("f = display ACS characters");
#if USE_WIDEC_SUPPORT
	(void) puts("F = display Wide-ACS characters");
#endif
	(void) puts("g = display windows and scrolling");
	(void) puts("i = test of flushinp()");
	(void) puts("k = display character attributes");
#if USE_LIBMENU
	(void) puts("m = menu code test");
#endif
#if USE_LIBPANEL
	(void) puts("o = exercise panels library");
#if USE_WIDEC_SUPPORT
	(void) puts("O = exercise panels with wide-characters");
#endif
#endif
	(void) puts("p = exercise pad features");
	(void) puts("q = quit");
#if USE_LIBFORM
	(void) puts("r = exercise forms code");
#endif
	(void) puts("s = overlapping-refresh test");
#if USE_LIBMENU && defined(TRACE)
	(void) puts("t = set trace level");
#endif
	(void) puts("? = repeat this command summary");

	(void) fputs("> ", stdout);
	(void) fflush(stdout);	/* necessary under SVr4 curses */

	/*
	 * This used to be an 'fgets()' call.  However (on Linux, at least)
	 * mixing stream I/O and 'read()' (used in the library) causes the
	 * input stream to be flushed when switching between the two.
	 */
	command = 0;
	for (;;) {
	    char ch = '\0';
	    if (read(fileno(stdin), &ch, 1) <= 0) {
		if (command == 0)
		    command = 'q';
		break;
	    } else if (command == 0 && !isspace(UChar(ch))) {
		command = ch;
	    } else if (ch == '\n' || ch == '\r') {
		if ((command == 'd') && !top) {
		    (void) fputs("Do not nest test-d\n", stdout);
		    command = 0;
		}
		if (command != 0)
		    break;
		(void) fputs("> ", stdout);
		(void) fflush(stdout);
	    }
	}

	if (do_single_test(command)) {
	    /*
	     * This may be overkill; it's intended to reset everything back
	     * to the initial terminal modes so that tests don't get in
	     * each other's way.
	     */
	    flushinp();
	    set_terminal_modes();
	    reset_prog_mode();
	    clear();
	    refresh();
	    endwin();
	    if (command == '?') {
		(void) puts("This is the ncurses capability tester.");
		(void)
		    puts("You may select a test from the main menu by typing the");
		(void)
		    puts("key letter of the choice (the letter to left of the =)");
		(void)
		    puts("at the > prompt.  Type `q' to exit.");
	    }
	    continue;
	}
    } while
	(command != 'q');
}

/*+-------------------------------------------------------------------------
	main(argc,argv)
--------------------------------------------------------------------------*/

#define okCOLOR(n) ((n) >= 0 && (n) < max_colors)
#define okRGB(n)   ((n) >= 0 && (n) <= 1000)

int
main(int argc, char *argv[])
{
    int c;
    int my_e_param = 1;
#ifdef NCURSES_VERSION
    int default_fg = COLOR_WHITE;
    int default_bg = COLOR_BLACK;
    bool assumed_colors = FALSE;
    bool default_colors = FALSE;
#endif
    char *palette_file = 0;
    bool monochrome = FALSE;

    setlocale(LC_ALL, "");

    while ((c = getopt(argc, argv, "a:de:fhmp:s:t:")) != -1) {
	switch (c) {
#ifdef NCURSES_VERSION
	case 'a':
	    assumed_colors = TRUE;
	    sscanf(optarg, "%d,%d", &default_fg, &default_bg);
	    break;
	case 'd':
	    default_colors = TRUE;
	    break;
#endif
	case 'e':
	    my_e_param = atoi(optarg);
#ifdef NCURSES_VERSION
	    if (my_e_param > 3)	/* allow extended layouts */
		usage();
#else
	    if (my_e_param > 1)
		usage();
#endif
	    break;
#if HAVE_RIPOFFLINE
	case 'f':
	    ripoffline(-1, rip_footer);
	    break;
	case 'h':
	    ripoffline(1, rip_header);
	    break;
#endif /* HAVE_RIPOFFLINE */
	case 'm':
	    monochrome = TRUE;
	    break;
	case 'p':
	    palette_file = optarg;
	    break;
#if USE_LIBPANEL
	case 's':
	    nap_msec = atol(optarg);
	    break;
#endif
#ifdef TRACE
	case 't':
	    save_trace = (unsigned) strtol(optarg, 0, 0);
	    break;
#endif
	default:
	    usage();
	}
    }

    /*
     * If there's no menus (unlikely for ncurses!), then we'll have to set
     * tracing on initially, just in case the user wants to test something that
     * doesn't involve wGetchar.
     */
#ifdef TRACE
    /* enable debugging */
#if !USE_LIBMENU
    trace(save_trace);
#else
    if (!isatty(fileno(stdin)))
	trace(save_trace);
#endif /* USE_LIBMENU */
#endif /* TRACE */

#if USE_SOFTKEYS
    /* tell it we're going to play with soft keys */
    slk_init(my_e_param);
#endif

#ifdef SIGUSR1
    /* set up null signal catcher so we can see what interrupts to getch do */
    signal(SIGUSR1, announce_sig);
#endif

    /* we must initialize the curses data structure only once */
    initscr();
    bkgdset(BLANK);

    /* tests, in general, will want these modes */
    use_colors = monochrome ? FALSE : has_colors();

    if (use_colors) {
	start_color();
#ifdef NCURSES_VERSION_PATCH
	max_colors = COLORS;	/* was > 16 ? 16 : COLORS */
#if HAVE_USE_DEFAULT_COLORS
	if (default_colors) {
	    use_default_colors();
	    min_colors = -1;
	}
#if NCURSES_VERSION_PATCH >= 20000708
	else if (assumed_colors)
	    assume_default_colors(default_fg, default_bg);
#endif
#endif
#else /* normal SVr4 curses */
	max_colors = COLORS;	/* was > 8 ? 8 : COLORS */
#endif
	max_pairs = COLOR_PAIRS;	/* was > 256 ? 256 : COLOR_PAIRS */

	if (can_change_color()) {
	    short cp;
	    all_colors = typeMalloc(RGB_DATA, (unsigned) max_colors);
	    for (cp = 0; cp < max_colors; ++cp) {
		color_content(cp,
			      &all_colors[cp].red,
			      &all_colors[cp].green,
			      &all_colors[cp].blue);
	    }
	    if (palette_file != 0) {
		FILE *fp = fopen(palette_file, "r");
		if (fp != 0) {
		    char buffer[BUFSIZ];
		    int red, green, blue;
		    int scale = 1000;
		    while (fgets(buffer, sizeof(buffer), fp) != 0) {
			if (sscanf(buffer, "scale:%d", &c) == 1) {
			    scale = c;
			} else if (sscanf(buffer, "%d:%d %d %d",
					  &c,
					  &red,
					  &green,
					  &blue) == 4
				   && okCOLOR(c)
				   && okRGB(red)
				   && okRGB(green)
				   && okRGB(blue)) {
			    all_colors[c].red = (short) ((red * 1000) / scale);
			    all_colors[c].green = (short) ((green * 1000) / scale);
			    all_colors[c].blue = (short) ((blue * 1000) / scale);
			}
		    }
		    fclose(fp);
		}
	    }
	}
    }
    set_terminal_modes();
    def_prog_mode();

    /*
     * Return to terminal mode, so we're guaranteed of being able to
     * select terminal commands even if the capabilities are wrong.
     */
    endwin();

#if HAVE_CURSES_VERSION
    (void) printf("Welcome to %s.  Press ? for help.\n", curses_version());
#elif defined(NCURSES_VERSION_MAJOR) && defined(NCURSES_VERSION_MINOR) && defined(NCURSES_VERSION_PATCH)
    (void) printf("Welcome to ncurses %d.%d.%d.  Press ? for help.\n",
		  NCURSES_VERSION_MAJOR,
		  NCURSES_VERSION_MINOR,
		  NCURSES_VERSION_PATCH);
#else
    (void) puts("Welcome to ncurses.  Press ? for help.");
#endif

    main_menu(TRUE);

    ExitProgram(EXIT_SUCCESS);
}

/* ncurses.c ends here */

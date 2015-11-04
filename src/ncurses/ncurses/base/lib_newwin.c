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
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1992,1995               *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 *     and: Thomas E. Dickey                        1996-on                 *
 ****************************************************************************/

/*
**	lib_newwin.c
**
**	The routines newwin(), subwin() and their dependent
**
*/

#include <curses.priv.h>
#include <stddef.h>

MODULE_ID("$Id: lib_newwin.c,v 1.52 2008/06/07 13:58:09 tom Exp $")

#define window_is(name) ((sp)->_##name == win)

#if USE_REENTRANT
#define remove_window(name) \
		sp->_##name = 0
#else
#define remove_window(name) \
		sp->_##name = 0; \
		if (win == name) \
		    name = 0
#endif

static void
remove_window_from_screen(WINDOW *win)
{
    SCREEN *sp;

    for (each_screen(sp)) {
	if (window_is(curscr)) {
	    remove_window(curscr);
	    break;
	} else if (window_is(stdscr)) {
	    remove_window(stdscr);
	    break;
	} else if (window_is(newscr)) {
	    remove_window(newscr);
	    break;
	}
    }
}

NCURSES_EXPORT(int)
_nc_freewin(WINDOW *win)
{
    WINDOWLIST *p, *q;
    int i;
    int result = ERR;

    T((T_CALLED("_nc_freewin(%p)"), win));

    if (win != 0) {
	if (_nc_try_global(curses) == 0) {
	    q = 0;
	    for (each_window(p)) {
		if (&(p->win) == win) {
		    remove_window_from_screen(win);
		    if (q == 0)
			_nc_windows = p->next;
		    else
			q->next = p->next;

		    if (!(win->_flags & _SUBWIN)) {
			for (i = 0; i <= win->_maxy; i++)
			    FreeIfNeeded(win->_line[i].text);
		    }
		    free(win->_line);
		    free(p);

		    result = OK;
		    T(("...deleted win=%p", win));
		    break;
		}
		q = p;
	    }
	    _nc_unlock_global(curses);
	}
    }
    returnCode(result);
}

NCURSES_EXPORT(WINDOW *)
newwin(int num_lines, int num_columns, int begy, int begx)
{
    WINDOW *win;
    NCURSES_CH_T *ptr;
    int i;

    T((T_CALLED("newwin(%d,%d,%d,%d)"), num_lines, num_columns, begy, begx));

    if (begy < 0 || begx < 0 || num_lines < 0 || num_columns < 0)
	returnWin(0);

    if (num_lines == 0)
	num_lines = SP->_lines_avail - begy;
    if (num_columns == 0)
	num_columns = screen_columns - begx;

    if ((win = _nc_makenew(num_lines, num_columns, begy, begx, 0)) == 0)
	returnWin(0);

    for (i = 0; i < num_lines; i++) {
	win->_line[i].text = typeCalloc(NCURSES_CH_T, (unsigned) num_columns);
	if (win->_line[i].text == 0) {
	    (void) _nc_freewin(win);
	    returnWin(0);
	}
	for (ptr = win->_line[i].text;
	     ptr < win->_line[i].text + num_columns;
	     ptr++)
	    SetChar(*ptr, BLANK_TEXT, BLANK_ATTR);
    }

    returnWin(win);
}

NCURSES_EXPORT(WINDOW *)
derwin(WINDOW *orig, int num_lines, int num_columns, int begy, int begx)
{
    WINDOW *win;
    int i;
    int flags = _SUBWIN;

    T((T_CALLED("derwin(%p,%d,%d,%d,%d)"), orig, num_lines, num_columns,
       begy, begx));

    /*
     * make sure window fits inside the original one
     */
    if (begy < 0 || begx < 0 || orig == 0 || num_lines < 0 || num_columns < 0)
	returnWin(0);
    if (begy + num_lines > orig->_maxy + 1
	|| begx + num_columns > orig->_maxx + 1)
	returnWin(0);

    if (num_lines == 0)
	num_lines = orig->_maxy + 1 - begy;

    if (num_columns == 0)
	num_columns = orig->_maxx + 1 - begx;

    if (orig->_flags & _ISPAD)
	flags |= _ISPAD;

    if ((win = _nc_makenew(num_lines, num_columns, orig->_begy + begy,
			   orig->_begx + begx, flags)) == 0)
	returnWin(0);

    win->_pary = begy;
    win->_parx = begx;
    WINDOW_ATTRS(win) = WINDOW_ATTRS(orig);
    win->_nc_bkgd = orig->_nc_bkgd;

    for (i = 0; i < num_lines; i++)
	win->_line[i].text = &orig->_line[begy++].text[begx];

    win->_parent = orig;

    returnWin(win);
}

NCURSES_EXPORT(WINDOW *)
subwin(WINDOW *w, int l, int c, int y, int x)
{
    T((T_CALLED("subwin(%p, %d, %d, %d, %d)"), w, l, c, y, x));
    T(("parent has begy = %ld, begx = %ld", (long) w->_begy, (long) w->_begx));

    returnWin(derwin(w, l, c, y - w->_begy, x - w->_begx));
}

static bool
dimension_limit(int value)
{
    NCURSES_SIZE_T test = value;
    return (test == value && value > 0);
}

NCURSES_EXPORT(WINDOW *)
_nc_makenew(int num_lines, int num_columns, int begy, int begx, int flags)
{
    int i;
    WINDOWLIST *wp;
    WINDOW *win;
    bool is_pad = (flags & _ISPAD);

    T((T_CALLED("_nc_makenew(%d,%d,%d,%d)"), num_lines, num_columns, begy, begx));

    if (SP == 0)
	returnWin(0);

    if (!dimension_limit(num_lines) || !dimension_limit(num_columns))
	returnWin(0);

    if ((wp = typeCalloc(WINDOWLIST, 1)) == 0)
	returnWin(0);

    win = &(wp->win);

    if ((win->_line = typeCalloc(struct ldat, ((unsigned) num_lines))) == 0) {
	free(win);
	returnWin(0);
    }

    _nc_lock_global(curses);

    win->_curx = 0;
    win->_cury = 0;
    win->_maxy = num_lines - 1;
    win->_maxx = num_columns - 1;
    win->_begy = begy;
    win->_begx = begx;
    win->_yoffset = SP->_topstolen;

    win->_flags = flags;
    WINDOW_ATTRS(win) = A_NORMAL;
    SetChar(win->_nc_bkgd, BLANK_TEXT, BLANK_ATTR);

    win->_clear = is_pad ? FALSE : (num_lines == screen_lines
				    && num_columns == screen_columns);
    win->_idlok = FALSE;
    win->_idcok = TRUE;
    win->_scroll = FALSE;
    win->_leaveok = FALSE;
    win->_use_keypad = FALSE;
    win->_delay = -1;
    win->_immed = FALSE;
    win->_sync = 0;
    win->_parx = -1;
    win->_pary = -1;
    win->_parent = 0;

    win->_regtop = 0;
    win->_regbottom = num_lines - 1;

    win->_pad._pad_y = -1;
    win->_pad._pad_x = -1;
    win->_pad._pad_top = -1;
    win->_pad._pad_bottom = -1;
    win->_pad._pad_left = -1;
    win->_pad._pad_right = -1;

    for (i = 0; i < num_lines; i++) {
	/*
	 * This used to do
	 *
	 * win->_line[i].firstchar = win->_line[i].lastchar = _NOCHANGE;
	 *
	 * which marks the whole window unchanged.  That's how
	 * SVr1 curses did it, but SVr4 curses marks the whole new
	 * window changed.
	 *
	 * With the old SVr1-like code, say you have stdscr full of
	 * characters, then create a new window with newwin(),
	 * then do a printw(win, "foo        ");, the trailing spaces are
	 * completely ignored by the following refreshes.  So, you
	 * get "foojunkjunk" on the screen instead of "foo        " as
	 * you actually intended.
	 *
	 * SVr4 doesn't do this.  Instead the spaces are actually written.
	 * So that's how we want ncurses to behave.
	 */
	win->_line[i].firstchar = 0;
	win->_line[i].lastchar = num_columns - 1;

	if_USE_SCROLL_HINTS(win->_line[i].oldindex = i);
    }

    if (!is_pad && (begx + num_columns == screen_columns)) {
	win->_flags |= _ENDLINE;

	if (begx == 0 && num_lines == screen_lines && begy == 0)
	    win->_flags |= _FULLWIN;

	if (begy + num_lines == screen_lines)
	    win->_flags |= _SCROLLWIN;
    }

    wp->next = _nc_windows;
    wp->screen = SP;
    _nc_windows = wp;

    T((T_CREATE("window %p"), win));

    _nc_unlock_global(curses);
    returnWin(win);
}

/*
 * wgetch() and other functions with a WINDOW* parameter may use a SCREEN*
 * internally, and it is useful to allow those to be invoked without switching
 * SCREEN's, e.g., for multi-threaded applications.
 */
NCURSES_EXPORT(SCREEN *)
_nc_screen_of(WINDOW *win)
{
    SCREEN *sp = 0;

    if (win != 0) {
	WINDOWLIST *wp = (WINDOWLIST *) win;
	sp = wp->screen;
    }
    return (sp);
}

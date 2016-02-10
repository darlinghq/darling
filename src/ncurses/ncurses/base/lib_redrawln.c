/****************************************************************************
 * Copyright (c) 1998-2006,2007 Free Software Foundation, Inc.              *
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
 *  Author: Thomas E. Dickey <dickey@clark.net> 1997                        *
 ****************************************************************************/

/*
 *	lib_redrawln.c
 *
 *	The routine wredrawln().
 *
 */

#include <curses.priv.h>

MODULE_ID("$Id: lib_redrawln.c,v 1.12 2007/10/13 20:08:56 tom Exp $")

NCURSES_EXPORT(int)
wredrawln(WINDOW *win, int beg, int num)
{
    int i;
    int end;
    size_t len;

    T((T_CALLED("wredrawln(%p,%d,%d)"), win, beg, num));

    if (win == 0)
	returnCode(ERR);

    if (beg < 0)
	beg = 0;

    if (touchline(win, beg, num) == ERR)
	returnCode(ERR);

    if (touchline(curscr, beg + win->_begy, num) == ERR)
	returnCode(ERR);

    end = beg + num;
    if (end > curscr->_maxy + 1)
	end = curscr->_maxy + 1;
    if (end > win->_maxy + 1)
	end = win->_maxy + 1;

    len = (win->_maxx + 1);
    if (len > (size_t) (curscr->_maxx + 1))
	len = (size_t) (curscr->_maxx + 1);
    len *= sizeof(curscr->_line[0].text[0]);

    for (i = beg; i < end; i++) {
	int crow = i + win->_begy;

	memset(curscr->_line[crow].text + win->_begx, 0, len);
	_nc_make_oldhash(crow);
    }

    returnCode(OK);
}

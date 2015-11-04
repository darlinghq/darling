/****************************************************************************
 * Copyright (c) 1998-2003,2005 Free Software Foundation, Inc.              *
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
**	lib_printw.c
**
**	The routines printw(), wprintw() and friends.
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_printw.c,v 1.18 2006/12/17 19:21:39 tom Exp $")

NCURSES_EXPORT(int)
printw(const char *fmt,...)
{
    va_list argp;
    int code;

#ifdef TRACE
    va_start(argp, fmt);
    T((T_CALLED("printw(%s%s)"),
       _nc_visbuf(fmt), _nc_varargs(fmt, argp)));
    va_end(argp);
#endif

    va_start(argp, fmt);
    code = vwprintw(stdscr, fmt, argp);
    va_end(argp);

    returnCode(code);
}

NCURSES_EXPORT(int)
wprintw(WINDOW *win, const char *fmt,...)
{
    va_list argp;
    int code;

#ifdef TRACE
    va_start(argp, fmt);
    T((T_CALLED("wprintw(%p,%s%s)"),
       win, _nc_visbuf(fmt), _nc_varargs(fmt, argp)));
    va_end(argp);
#endif

    va_start(argp, fmt);
    code = vwprintw(win, fmt, argp);
    va_end(argp);

    returnCode(code);
}

NCURSES_EXPORT(int)
mvprintw(int y, int x, const char *fmt,...)
{
    va_list argp;
    int code;

#ifdef TRACE
    va_start(argp, fmt);
    T((T_CALLED("mvprintw(%d,%d,%s%s)"),
       y, x, _nc_visbuf(fmt), _nc_varargs(fmt, argp)));
    va_end(argp);
#endif

    if ((code = move(y, x)) != ERR) {
	va_start(argp, fmt);
	code = vwprintw(stdscr, fmt, argp);
	va_end(argp);
    }
    returnCode(code);
}

NCURSES_EXPORT(int)
mvwprintw(WINDOW *win, int y, int x, const char *fmt,...)
{
    va_list argp;
    int code;

#ifdef TRACE
    va_start(argp, fmt);
    T((T_CALLED("mvwprintw(%d,%d,%p,%s%s)"),
       y, x, win, _nc_visbuf(fmt), _nc_varargs(fmt, argp)));
    va_end(argp);
#endif

    if ((code = wmove(win, y, x)) != ERR) {
	va_start(argp, fmt);
	code = vwprintw(win, fmt, argp);
	va_end(argp);
    }
    returnCode(code);
}

NCURSES_EXPORT(int)
vwprintw(WINDOW *win, const char *fmt, va_list argp)
{
    char *buf;
    int code = ERR;

    T((T_CALLED("vwprintw(%p,%s,va_list)"), win, _nc_visbuf(fmt)));

    if ((buf = _nc_printf_string(fmt, argp)) != 0) {
	code = waddstr(win, buf);
    }
    returnCode(code);
}

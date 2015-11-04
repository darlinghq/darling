/****************************************************************************
 * Copyright (c) 2004,2006 Free Software Foundation, Inc.                   *
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
**	lib_add_wch.c
**
**	The routine wadd_wch().
**
*/

#include <curses.priv.h>

MODULE_ID("$Id: lib_add_wch.c,v 1.6 2006/12/02 21:19:17 tom Exp $")

NCURSES_EXPORT(int)
wadd_wch(WINDOW *win, const cchar_t *wch)
{
    PUTC_DATA;
    int n;
    int code = ERR;

    TR(TRACE_VIRTPUT | TRACE_CCALLS, (T_CALLED("wadd_wch(%p, %s)"), win,
				      _tracech_t(wch)));

    if (win != 0) {
	PUTC_INIT;
	for (PUTC_i = 0; PUTC_i < CCHARW_MAX; ++PUTC_i) {
	    attr_t attrs = (wch->attr & A_ATTRIBUTES);

	    if ((PUTC_ch = wch->chars[PUTC_i]) == L'\0')
		break;
	    if ((PUTC_n = wcrtomb(PUTC_buf, PUTC_ch, &PUT_st)) <= 0) {
		code = ERR;
		if (is8bits(PUTC_ch))
		    code = waddch(win, UChar(PUTC_ch) | attrs);
		break;
	    }
	    for (n = 0; n < PUTC_n; n++) {
		if ((code = waddch(win, UChar(PUTC_buf[n]) | attrs)) == ERR) {
		    break;
		}
	    }
	    if (code == ERR)
		break;
	}
    }

    TR(TRACE_VIRTPUT | TRACE_CCALLS, (T_RETURN("%d"), code));
    return (code);
}

NCURSES_EXPORT(int)
wecho_wchar(WINDOW *win, const cchar_t *wch)
{
    PUTC_DATA;
    int n;
    int code = ERR;

    TR(TRACE_VIRTPUT | TRACE_CCALLS, (T_CALLED("wecho_wchar(%p, %s)"), win,
				      _tracech_t(wch)));

    if (win != 0) {
	PUTC_INIT;
	for (PUTC_i = 0; PUTC_i < CCHARW_MAX; ++PUTC_i) {
	    attr_t attrs = (wch->attr & A_ATTRIBUTES);

	    if ((PUTC_ch = wch->chars[PUTC_i]) == L'\0')
		break;
	    if ((PUTC_n = wcrtomb(PUTC_buf, PUTC_ch, &PUT_st)) <= 0) {
		code = ERR;
		if (is8bits(PUTC_ch))
		    code = waddch(win, UChar(PUTC_ch) | attrs);
		break;
	    }
	    for (n = 0; n < PUTC_n; n++) {
		if ((code = waddch(win, UChar(PUTC_buf[n]) | attrs)) == ERR) {
		    break;
		}
	    }
	    if (code == ERR)
		break;
	}
	wrefresh(win);
    }

    TR(TRACE_VIRTPUT | TRACE_CCALLS, (T_RETURN("%d"), code));
    return (code);
}

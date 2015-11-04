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
 *  Author: Thomas E. Dickey                                                *
 ****************************************************************************/

/*
 *	home_terminfo.c -- return the $HOME/.terminfo string, expanded
 */

#include <curses.priv.h>
#include <tic.h>

MODULE_ID("$Id: home_terminfo.c,v 1.11 2008/08/03 23:43:11 tom Exp $")

/* ncurses extension...fall back on user's private directory */

#define MyBuffer _nc_globals.home_terminfo

NCURSES_EXPORT(char *)
_nc_home_terminfo(void)
{
    char *result = 0;
#if USE_HOME_TERMINFO
    char *home;

    if (use_terminfo_vars()) {
	if (MyBuffer == 0) {
	    if ((home = getenv("HOME")) != 0) {
		unsigned want = (strlen(home) + sizeof(PRIVATE_INFO));
		MyBuffer = typeMalloc(char, want);
		if (MyBuffer == 0)
		    _nc_err_abort(MSG_NO_MEMORY);
		(void) sprintf(MyBuffer, PRIVATE_INFO, home);
	    }
	}
	result = MyBuffer;
    }
#endif
    return result;
}

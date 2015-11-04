/****************************************************************************
 * Copyright (c) 1998-2000,2003 Free Software Foundation, Inc.              *
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
 ****************************************************************************/

#include <curses.priv.h>

#include <term_entry.h>
#include <tic.h>

MODULE_ID("$Id: lib_ti.c,v 1.23 2003/05/24 21:10:28 tom Exp $")

NCURSES_EXPORT(int)
tigetflag(NCURSES_CONST char *str)
{
    unsigned i;

    T((T_CALLED("tigetflag(%s)"), str));

    if (cur_term != 0) {
	TERMTYPE *tp = &(cur_term->type);
	for_each_boolean(i, tp) {
	    const char *capname = ExtBoolname(tp, i, boolnames);
	    if (!strcmp(str, capname)) {
		/* setupterm forces invalid booleans to false */
		returnCode(tp->Booleans[i]);
	    }
	}
    }

    returnCode(ABSENT_BOOLEAN);
}

NCURSES_EXPORT(int)
tigetnum(NCURSES_CONST char *str)
{
    unsigned i;

    T((T_CALLED("tigetnum(%s)"), str));

    if (cur_term != 0) {
	TERMTYPE *tp = &(cur_term->type);
	for_each_number(i, tp) {
	    const char *capname = ExtNumname(tp, i, numnames);
	    if (!strcmp(str, capname)) {
		if (!VALID_NUMERIC(tp->Numbers[i]))
		    returnCode(ABSENT_NUMERIC);
		returnCode(tp->Numbers[i]);
	    }
	}
    }

    returnCode(CANCELLED_NUMERIC);	/* Solaris returns a -1 instead */
}

NCURSES_EXPORT(char *)
tigetstr(NCURSES_CONST char *str)
{
    unsigned i;

    T((T_CALLED("tigetstr(%s)"), str));

    if (cur_term != 0) {
	TERMTYPE *tp = &(cur_term->type);
	for_each_string(i, tp) {
	    const char *capname = ExtStrname(tp, i, strnames);
	    if (!strcmp(str, capname)) {
		/* setupterm forces cancelled strings to null */
		returnPtr(tp->Strings[i]);
	    }
	}
    }

    returnPtr(CANCELLED_STRING);
}

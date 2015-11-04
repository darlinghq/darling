/****************************************************************************
 * Copyright (c) 1998-2005,2006 Free Software Foundation, Inc.              *
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
 *  Author: Thomas E. Dickey                    1997-on                     *
 ****************************************************************************/

#include <curses.priv.h>

MODULE_ID("$Id: define_key.c,v 1.13 2006/12/30 23:23:31 tom Exp $")

NCURSES_EXPORT(int)
define_key(const char *str, int keycode)
{
    int code = ERR;

    T((T_CALLED("define_key(%s,%d)"), _nc_visbuf(str), keycode));
    if (SP == 0) {
	code = ERR;
    } else if (keycode > 0) {
	unsigned ukey = (unsigned) keycode;

	if (str != 0) {
	    define_key(str, 0);
	} else if (has_key(keycode)) {
	    while (_nc_remove_key(&(SP->_keytry), ukey))
		code = OK;
	}
	if (str != 0) {
	    if (key_defined(str) == 0) {
		if (_nc_add_to_try(&(SP->_keytry), str, ukey) == OK) {
		    code = OK;
		} else {
		    code = ERR;
		}
	    } else {
		code = ERR;
	    }
	}
    } else {
	while (_nc_remove_string(&(SP->_keytry), str))
	    code = OK;
    }
    returnCode(code);
}

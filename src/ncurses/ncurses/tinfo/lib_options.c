/****************************************************************************
 * Copyright (c) 1998-2006,2008 Free Software Foundation, Inc.              *
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
**	lib_options.c
**
**	The routines to handle option setting.
**
*/

#include <curses.priv.h>

#include <term.h>

MODULE_ID("$Id: lib_options.c,v 1.58 2008/08/16 21:20:48 Werner.Fink Exp $")

static int _nc_curs_set(SCREEN *, int);
static int _nc_meta(SCREEN *, bool);

NCURSES_EXPORT(int)
idlok(WINDOW *win, bool flag)
{
    T((T_CALLED("idlok(%p,%d)"), win, flag));

    if (win) {
	_nc_idlok = win->_idlok = (flag && (has_il() || change_scroll_region));
	returnCode(OK);
    } else
	returnCode(ERR);
}

NCURSES_EXPORT(void)
idcok(WINDOW *win, bool flag)
{
    T((T_CALLED("idcok(%p,%d)"), win, flag));

    if (win)
	_nc_idcok = win->_idcok = (flag && has_ic());

    returnVoid;
}

NCURSES_EXPORT(int)
halfdelay(int t)
{
    T((T_CALLED("halfdelay(%d)"), t));

    if (t < 1 || t > 255 || SP == 0)
	returnCode(ERR);

    cbreak();
    SP->_cbreak = t + 1;
    returnCode(OK);
}

NCURSES_EXPORT(int)
nodelay(WINDOW *win, bool flag)
{
    T((T_CALLED("nodelay(%p,%d)"), win, flag));

    if (win) {
	if (flag == TRUE)
	    win->_delay = 0;
	else
	    win->_delay = -1;
	returnCode(OK);
    } else
	returnCode(ERR);
}

NCURSES_EXPORT(int)
notimeout(WINDOW *win, bool f)
{
    T((T_CALLED("notimeout(%p,%d)"), win, f));

    if (win) {
	win->_notimeout = f;
	returnCode(OK);
    } else
	returnCode(ERR);
}

NCURSES_EXPORT(void)
wtimeout(WINDOW *win, int delay)
{
    T((T_CALLED("wtimeout(%p,%d)"), win, delay));

    if (win) {
	win->_delay = delay;
    }
    returnVoid;
}

NCURSES_EXPORT(int)
keypad(WINDOW *win, bool flag)
{
    T((T_CALLED("keypad(%p,%d)"), win, flag));

    if (win) {
	win->_use_keypad = flag;
	returnCode(_nc_keypad(SP, flag));
    } else
	returnCode(ERR);
}

NCURSES_EXPORT(int)
meta(WINDOW *win GCC_UNUSED, bool flag)
{
    int result;

    /* Ok, we stay relaxed and don't signal an error if win is NULL */
    T((T_CALLED("meta(%p,%d)"), win, flag));
    result = _nc_meta(SP, flag);
    returnCode(result);
}

/* curs_set() moved here to narrow the kernel interface */

NCURSES_EXPORT(int)
curs_set(int vis)
{
    int result;

    T((T_CALLED("curs_set(%d)"), vis));
    result = _nc_curs_set(SP, vis);
    returnCode(result);
}

NCURSES_EXPORT(int)
typeahead(int fd)
{
    T((T_CALLED("typeahead(%d)"), fd));
    if (SP != 0) {
	SP->_checkfd = fd;
	returnCode(OK);
    } else {
	returnCode(ERR);
    }
}

/*
**      has_key()
**
**      Return TRUE if the current terminal has the given key
**
*/

#if NCURSES_EXT_FUNCS
static int
has_key_internal(int keycode, TRIES * tp)
{
    if (tp == 0)
	return (FALSE);
    else if (tp->value == keycode)
	return (TRUE);
    else
	return (has_key_internal(keycode, tp->child)
		|| has_key_internal(keycode, tp->sibling));
}

NCURSES_EXPORT(int)
has_key(int keycode)
{
    T((T_CALLED("has_key(%d)"), keycode));
    returnCode(SP != 0 ? has_key_internal(keycode, SP->_keytry) : FALSE);
}
#endif /* NCURSES_EXT_FUNCS */

/*
 * Internal entrypoints use SCREEN* parameter to obtain capabilities rather
 * than cur_term.
 */
#undef CUR
#define CUR (sp->_term)->type.

static int
_nc_putp(const char *name GCC_UNUSED, const char *value)
{
    int rc = ERR;

    if (value) {
	TPUTS_TRACE(name);
	rc = putp(value);
    }
    return rc;
}

static int
_nc_putp_flush(const char *name, const char *value)
{
    int rc = _nc_putp(name, value);
    if (rc != ERR) {
	_nc_flush();
    }
    return rc;
}

/* Turn the keypad on/off
 *
 * Note:  we flush the output because changing this mode causes some terminals
 * to emit different escape sequences for cursor and keypad keys.  If we don't
 * flush, then the next wgetch may get the escape sequence that corresponds to
 * the terminal state _before_ switching modes.
 */
NCURSES_EXPORT(int)
_nc_keypad(SCREEN *sp, bool flag)
{
    int rc = ERR;

    if (sp != 0) {
#ifdef USE_PTHREADS
	/*
	 * We might have this situation in a multithreaded application that
	 * has wgetch() reading in more than one thread.  putp() and below
	 * may use SP explicitly.
	 */
	if (_nc_use_pthreads && sp != SP) {
	    SCREEN *save_sp;

	    /* cannot use use_screen(), since that is not in tinfo library */
	    _nc_lock_global(curses);
	    save_sp = SP;
	    _nc_set_screen(sp);
	    rc = _nc_keypad(sp, flag);
	    _nc_set_screen(save_sp);
	    _nc_unlock_global(curses);
	} else
#endif
	{
	    if (flag) {
		(void) _nc_putp_flush("keypad_xmit", keypad_xmit);
	    } else if (!flag && keypad_local) {
		(void) _nc_putp_flush("keypad_local", keypad_local);
	    }

	    if (flag && !sp->_tried) {
		_nc_init_keytry(sp);
		sp->_tried = TRUE;
	    }
	    sp->_keypad_on = flag;
	    rc = OK;
	}
    }
    return (rc);
}

static int
_nc_curs_set(SCREEN *sp, int vis)
{
    int result = ERR;

    T((T_CALLED("curs_set(%d)"), vis));
    if (sp != 0 && vis >= 0 && vis <= 2) {
	int cursor = sp->_cursor;

	if (vis == cursor) {
	    result = cursor;
	} else {
	    switch (vis) {
	    case 2:
		result = _nc_putp_flush("cursor_visible", cursor_visible);
		break;
	    case 1:
		result = _nc_putp_flush("cursor_normal", cursor_normal);
		break;
	    case 0:
		result = _nc_putp_flush("cursor_invisible", cursor_invisible);
		break;
	    }
	    if (result != ERR)
		result = (cursor == -1 ? 1 : cursor);
	    sp->_cursor = vis;
	}
    }
    returnCode(result);
}

static int
_nc_meta(SCREEN *sp, bool flag)
{
    int result = ERR;

    /* Ok, we stay relaxed and don't signal an error if win is NULL */

    if (SP != 0) {
	SP->_use_meta = flag;

	if (flag) {
	    _nc_putp("meta_on", meta_on);
	} else {
	    _nc_putp("meta_off", meta_off);
	}
	result = OK;
    }
    return result;
}

/****************************************************************************
 * Copyright (c) 1998-2002,2007 Free Software Foundation, Inc.              *
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
 *     and: Thomas E. Dickey 1998 on                                        *
 ****************************************************************************/

/*
 *	raw.c
 *
 *	Routines:
 *		raw()
 *		cbreak()
 *		noraw()
 *		nocbreak()
 *		qiflush()
 *		noqiflush()
 *		intrflush()
 *
 */

#include <curses.priv.h>
#include <term.h>		/* cur_term */

MODULE_ID("$Id: lib_raw.c,v 1.14 2007/09/29 21:50:22 tom Exp $")

#if SVR4_TERMIO && !defined(_POSIX_SOURCE)
#define _POSIX_SOURCE
#endif

#if HAVE_SYS_TERMIO_H
#include <sys/termio.h>		/* needed for ISC */
#endif

#ifdef __EMX__
#include <io.h>
#define _nc_setmode(mode) setmode(SP->_ifd, mode)
#else
#define _nc_setmode(mode)	/* nothing */
#endif

#define COOKED_INPUT	(IXON|BRKINT|PARMRK)

#ifdef TRACE
#define BEFORE(N)	if (USE_TRACEF(TRACE_BITS)) _nc_locked_tracef("%s before bits: %s", N, _nc_tracebits())
#define AFTER(N)	if (USE_TRACEF(TRACE_BITS)) _nc_locked_tracef("%s after bits: %s", N, _nc_tracebits())
#else
#define BEFORE(s)
#define AFTER(s)
#endif /* TRACE */

NCURSES_EXPORT(int)
raw(void)
{
    int result = ERR;

    T((T_CALLED("raw()")));

    if (SP != 0 && cur_term != 0) {
	TTY buf;

	BEFORE("raw");
	_nc_setmode(O_BINARY);

	buf = cur_term->Nttyb;
#ifdef TERMIOS
	buf.c_lflag &= ~(ICANON | ISIG | IEXTEN);
	buf.c_iflag &= ~(COOKED_INPUT);
	buf.c_cc[VMIN] = 1;
	buf.c_cc[VTIME] = 0;
#else
	buf.sg_flags |= RAW;
#endif
	if ((result = _nc_set_tty_mode(&buf)) == OK) {
	    SP->_raw = TRUE;
	    SP->_cbreak = 1;
	    cur_term->Nttyb = buf;
	}
	AFTER("raw");
    }
    returnCode(result);
}

NCURSES_EXPORT(int)
cbreak(void)
{
    int result = ERR;

    T((T_CALLED("cbreak()")));

    if (SP != 0 && cur_term != 0) {
	TTY buf;

	BEFORE("cbreak");
	_nc_setmode(O_BINARY);

	buf = cur_term->Nttyb;
#ifdef TERMIOS
	buf.c_lflag &= ~ICANON;
	buf.c_iflag &= ~ICRNL;
	buf.c_lflag |= ISIG;
	buf.c_cc[VMIN] = 1;
	buf.c_cc[VTIME] = 0;
#else
	buf.sg_flags |= CBREAK;
#endif
	if ((result = _nc_set_tty_mode(&buf)) == OK) {
	    SP->_cbreak = 1;
	    cur_term->Nttyb = buf;
	}
	AFTER("cbreak");
    }
    returnCode(result);
}

/*
 * Note:
 * this implementation may be wrong.  See the comment under intrflush().
 */
NCURSES_EXPORT(void)
qiflush(void)
{
    int result = ERR;

    T((T_CALLED("qiflush()")));

    if (cur_term != 0) {
	TTY buf;

	BEFORE("qiflush");
	buf = cur_term->Nttyb;
#ifdef TERMIOS
	buf.c_lflag &= ~(NOFLSH);
	result = _nc_set_tty_mode(&buf);
#else
	/* FIXME */
#endif
	if (result == OK)
	    cur_term->Nttyb = buf;
	AFTER("qiflush");
    }
    returnVoid;
}

NCURSES_EXPORT(int)
noraw(void)
{
    int result = ERR;

    T((T_CALLED("noraw()")));

    if (SP != 0 && cur_term != 0) {
	TTY buf;

	BEFORE("noraw");
	_nc_setmode(O_TEXT);

	buf = cur_term->Nttyb;
#ifdef TERMIOS
	buf.c_lflag |= ISIG | ICANON |
	    (cur_term->Ottyb.c_lflag & IEXTEN);
	buf.c_iflag |= COOKED_INPUT;
#else
	buf.sg_flags &= ~(RAW | CBREAK);
#endif
	if ((result = _nc_set_tty_mode(&buf)) == OK) {
	    SP->_raw = FALSE;
	    SP->_cbreak = 0;
	    cur_term->Nttyb = buf;
	}
	AFTER("noraw");
    }
    returnCode(result);
}

NCURSES_EXPORT(int)
nocbreak(void)
{
    int result = ERR;

    T((T_CALLED("nocbreak()")));

    if (SP != 0 && cur_term != 0) {
	TTY buf;

	BEFORE("nocbreak");
	_nc_setmode(O_TEXT);

	buf = cur_term->Nttyb;
#ifdef TERMIOS
	buf.c_lflag |= ICANON;
	buf.c_iflag |= ICRNL;
#else
	buf.sg_flags &= ~CBREAK;
#endif
	if ((result = _nc_set_tty_mode(&buf)) == OK) {
	    SP->_cbreak = 0;
	    cur_term->Nttyb = buf;
	}
	AFTER("nocbreak");
    }
    returnCode(result);
}

/*
 * Note:
 * this implementation may be wrong.  See the comment under intrflush().
 */
NCURSES_EXPORT(void)
noqiflush(void)
{
    int result = ERR;

    T((T_CALLED("noqiflush()")));

    if (cur_term != 0) {
	TTY buf;

	BEFORE("noqiflush");
	buf = cur_term->Nttyb;
#ifdef TERMIOS
	buf.c_lflag |= NOFLSH;
	result = _nc_set_tty_mode(&buf);
#else
	/* FIXME */
#endif
	if (result == OK) {
	    cur_term->Nttyb = buf;
	}
	AFTER("noqiflush");
    }
    returnVoid;
}

/*
 * This call does the same thing as the qiflush()/noqiflush() pair.  We know
 * for certain that SVr3 intrflush() tweaks the NOFLSH bit; on the other hand,
 * the match (in the SVr4 man pages) between the language describing NOFLSH in
 * termio(7) and the language describing qiflush()/noqiflush() in
 * curs_inopts(3x) is too exact to be coincidence.
 */
NCURSES_EXPORT(int)
intrflush(WINDOW *win GCC_UNUSED, bool flag)
{
    int result = ERR;

    T((T_CALLED("intrflush(%d)"), flag));

    if (cur_term != 0) {
	TTY buf;

	BEFORE("intrflush");
	buf = cur_term->Nttyb;
#ifdef TERMIOS
	if (flag)
	    buf.c_lflag &= ~(NOFLSH);
	else
	    buf.c_lflag |= (NOFLSH);
	result = _nc_set_tty_mode(&buf);
#else
	/* FIXME */
#endif
	if (result == OK) {
	    cur_term->Nttyb = buf;
	}
	AFTER("intrflush");
    }
    returnCode(result);
}

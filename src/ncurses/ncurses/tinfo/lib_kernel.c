/****************************************************************************
 * Copyright (c) 1998-2003,2004 Free Software Foundation, Inc.              *
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
 *     and: Thomas E. Dickey 2002                                           *
 ****************************************************************************/

/*
 *	lib_kernel.c
 *
 *	Misc. low-level routines:
 *		erasechar()
 *		killchar()
 *		flushinp()
 *
 * The baudrate() and delay_output() functions could logically live here,
 * but are in other modules to reduce the static-link size of programs
 * that use only these facilities.
 */

#include <curses.priv.h>
#include <term.h>		/* cur_term */

MODULE_ID("$Id: lib_kernel.c,v 1.24 2004/05/08 17:11:21 tom Exp $")

static int
_nc_vdisable(void)
{
    int value = -1;
#if defined(_POSIX_VDISABLE) && HAVE_UNISTD_H
    value = _POSIX_VDISABLE;
#endif
#if defined(_PC_VDISABLE)
    if (value == -1) {
	value = fpathconf(0, _PC_VDISABLE);
	if (value == -1) {
	    value = 0377;
	}
    }
#elif defined(VDISABLE)
    if (value == -1)
	value = VDISABLE;
#endif
    return value;
}

/*
 *	erasechar()
 *
 *	Return erase character as given in cur_term->Ottyb.
 *
 */

NCURSES_EXPORT(char)
erasechar(void)
{
    int result = ERR;
    T((T_CALLED("erasechar()")));

    if (cur_term != 0) {
#ifdef TERMIOS
	result = cur_term->Ottyb.c_cc[VERASE];
	if (result == _nc_vdisable())
	    result = ERR;
#else
	result = cur_term->Ottyb.sg_erase;
#endif
    }
    returnCode(result);
}

/*
 *	killchar()
 *
 *	Return kill character as given in cur_term->Ottyb.
 *
 */

NCURSES_EXPORT(char)
killchar(void)
{
    int result = ERR;
    T((T_CALLED("killchar()")));

    if (cur_term != 0) {
#ifdef TERMIOS
	result = cur_term->Ottyb.c_cc[VKILL];
	if (result == _nc_vdisable())
	    result = ERR;
#else
	result = cur_term->Ottyb.sg_kill;
#endif
    }
    returnCode(result);
}

/*
 *	flushinp()
 *
 *	Flush any input on cur_term->Filedes
 *
 */

NCURSES_EXPORT(int)
flushinp(void)
{
    T((T_CALLED("flushinp()")));

    if (cur_term != 0) {
#ifdef TERMIOS
	tcflush(cur_term->Filedes, TCIFLUSH);
#else
	errno = 0;
	do {
	    ioctl(cur_term->Filedes, TIOCFLUSH, 0);
	} while
	    (errno == EINTR);
#endif
	if (SP) {
	    SP->_fifohead = -1;
	    SP->_fifotail = 0;
	    SP->_fifopeek = 0;
	}
	returnCode(OK);
    }
    returnCode(ERR);
}

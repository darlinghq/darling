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
 *                                                                          *
 * some of the code in here was contributed by:                             *
 * Magnus Bengtsson, d6mbeng@dtek.chalmers.se (Nov'93)                      *
 * (but it has changed a lot)                                               *
 ****************************************************************************/

#define __INTERNAL_CAPS_VISIBLE
#include <curses.priv.h>

#include <termcap.h>
#include <tic.h>
#include <ctype.h>

#include <term_entry.h>

MODULE_ID("$Id: lib_termcap.c,v 1.63 2008/08/16 19:22:55 tom Exp $")

NCURSES_EXPORT_VAR(char *) UP = 0;
NCURSES_EXPORT_VAR(char *) BC = 0;

#define MyCache  _nc_globals.tgetent_cache
#define CacheInx _nc_globals.tgetent_index
#define CacheSeq _nc_globals.tgetent_sequence

#define FIX_SGR0 MyCache[CacheInx].fix_sgr0
#define LAST_TRM MyCache[CacheInx].last_term
#define LAST_BUF MyCache[CacheInx].last_bufp
#define LAST_USE MyCache[CacheInx].last_used
#define LAST_SEQ MyCache[CacheInx].sequence

/***************************************************************************
 *
 * tgetent(bufp, term)
 *
 * In termcap, this function reads in the entry for terminal `term' into the
 * buffer pointed to by bufp. It must be called before any of the functions
 * below are called.
 * In this terminfo emulation, tgetent() simply calls setupterm() (which
 * does a bit more than tgetent() in termcap does), and returns its return
 * value (1 if successful, 0 if no terminal with the given name could be
 * found, or -1 if no terminal descriptions have been installed on the
 * system).  The bufp argument is ignored.
 *
 ***************************************************************************/

NCURSES_EXPORT(int)
tgetent(char *bufp, const char *name)
{
    int errcode;
    int n;
    bool found_cache = FALSE;

    START_TRACE();
    T((T_CALLED("tgetent()")));

    _nc_setupterm((NCURSES_CONST char *) name, STDOUT_FILENO, &errcode, TRUE);

    /*
     * In general we cannot tell if the fixed sgr0 is still used by the
     * caller, but if tgetent() is called with the same buffer, that is
     * good enough, since the previous data would be invalidated by the
     * current call.
     *
     * bufp may be a null pointer, e.g., GNU termcap.  That allocates data,
     * which is good until the next tgetent() call.  The conventional termcap
     * is inconvenient because of the fixed buffer size, but because it uses
     * caller-supplied buffers, can have multiple terminal descriptions in
     * use at a given time.
     */
    for (n = 0; n < TGETENT_MAX; ++n) {
	bool same_result = (MyCache[n].last_used && MyCache[n].last_bufp == bufp);
	if (same_result) {
	    CacheInx = n;
	    if (FIX_SGR0 != 0) {
		FreeAndNull(FIX_SGR0);
	    }
	    /*
	     * Also free the terminfo data that we loaded (much bigger leak).
	     */
	    if (LAST_TRM != 0 && LAST_TRM != cur_term) {
		TERMINAL *trm = LAST_TRM;
		del_curterm(LAST_TRM);
		for (CacheInx = 0; CacheInx < TGETENT_MAX; ++CacheInx)
		    if (LAST_TRM == trm)
			LAST_TRM = 0;
		CacheInx = n;
	    }
	    found_cache = TRUE;
	    break;
	}
    }
    if (!found_cache) {
	int best = 0;

	for (CacheInx = 0; CacheInx < TGETENT_MAX; ++CacheInx) {
	    if (LAST_SEQ < MyCache[best].sequence) {
		best = CacheInx;
	    }
	}
	CacheInx = best;
    }
    LAST_TRM = cur_term;
    LAST_SEQ = ++CacheSeq;

    PC = 0;
    UP = 0;
    BC = 0;
    FIX_SGR0 = 0;		/* don't free it - application may still use */

    if (errcode == 1) {

	if (cursor_left)
	    if ((backspaces_with_bs = (char) !strcmp(cursor_left, "\b")) == 0)
		backspace_if_not_bs = cursor_left;

	/* we're required to export these */
	if (pad_char != NULL)
	    PC = pad_char[0];
	if (cursor_up != NULL)
	    UP = cursor_up;
	if (backspace_if_not_bs != NULL)
	    BC = backspace_if_not_bs;

	if ((FIX_SGR0 = _nc_trim_sgr0(&(cur_term->type))) != 0) {
	    if (!strcmp(FIX_SGR0, exit_attribute_mode)) {
		if (FIX_SGR0 != exit_attribute_mode) {
		    free(FIX_SGR0);
		}
		FIX_SGR0 = 0;
	    }
	}
	LAST_BUF = bufp;
	LAST_USE = TRUE;

	SetNoPadding(SP);
	(void) baudrate();	/* sets ospeed as a side-effect */

/* LINT_PREPRO
#if 0*/
#include <capdefaults.c>
/* LINT_PREPRO
#endif*/

    }
    returnCode(errcode);
}

/***************************************************************************
 *
 * tgetflag(str)
 *
 * Look up boolean termcap capability str and return its value (TRUE=1 if
 * present, FALSE=0 if not).
 *
 ***************************************************************************/

NCURSES_EXPORT(int)
tgetflag(NCURSES_CONST char *id)
{
    unsigned i;

    T((T_CALLED("tgetflag(%s)"), id));
    if (cur_term != 0) {
	TERMTYPE *tp = &(cur_term->type);
	for_each_boolean(i, tp) {
	    const char *capname = ExtBoolname(tp, i, boolcodes);
	    if (!strncmp(id, capname, 2)) {
		/* setupterm forces invalid booleans to false */
		returnCode(tp->Booleans[i]);
	    }
	}
    }
    returnCode(0);		/* Solaris does this */
}

/***************************************************************************
 *
 * tgetnum(str)
 *
 * Look up numeric termcap capability str and return its value, or -1 if
 * not given.
 *
 ***************************************************************************/

NCURSES_EXPORT(int)
tgetnum(NCURSES_CONST char *id)
{
    unsigned i;

    T((T_CALLED("tgetnum(%s)"), id));
    if (cur_term != 0) {
	TERMTYPE *tp = &(cur_term->type);
	for_each_number(i, tp) {
	    const char *capname = ExtNumname(tp, i, numcodes);
	    if (!strncmp(id, capname, 2)) {
		if (!VALID_NUMERIC(tp->Numbers[i]))
		    returnCode(ABSENT_NUMERIC);
		returnCode(tp->Numbers[i]);
	    }
	}
    }
    returnCode(ABSENT_NUMERIC);
}

/***************************************************************************
 *
 * tgetstr(str, area)
 *
 * Look up string termcap capability str and return a pointer to its value,
 * or NULL if not given.
 *
 ***************************************************************************/

NCURSES_EXPORT(char *)
tgetstr(NCURSES_CONST char *id, char **area)
{
    unsigned i;
    char *result = NULL;

    T((T_CALLED("tgetstr(%s,%p)"), id, area));
    if (cur_term != 0) {
	TERMTYPE *tp = &(cur_term->type);
	for_each_string(i, tp) {
	    const char *capname = ExtStrname(tp, i, strcodes);
	    if (!strncmp(id, capname, 2)) {
		result = tp->Strings[i];
		TR(TRACE_DATABASE, ("found match : %s", _nc_visbuf(result)));
		/* setupterm forces canceled strings to null */
		if (VALID_STRING(result)) {
		    if (result == exit_attribute_mode
			&& FIX_SGR0 != 0) {
			result = FIX_SGR0;
			TR(TRACE_DATABASE, ("altered to : %s", _nc_visbuf(result)));
		    }
		    if (area != 0
			&& *area != 0) {
			(void) strcpy(*area, result);
			result = *area;
			*area += strlen(*area) + 1;
		    }
		}
		break;
	    }
	}
    }
    returnPtr(result);
}

#if NO_LEAKS
NCURSES_EXPORT(void)
_nc_tgetent_leaks(void)
{
    for (CacheInx = 0; CacheInx < TGETENT_MAX; ++CacheInx) {
	FreeIfNeeded(FIX_SGR0);
	if (LAST_TRM != 0)
	    del_curterm(LAST_TRM);
    }
}
#endif

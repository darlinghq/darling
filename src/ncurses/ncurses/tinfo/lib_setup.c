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
 ****************************************************************************/

/*
 * Terminal setup routines common to termcap and terminfo:
 *
 *		use_env(bool)
 *		setupterm(char *, int, int *)
 */

#include <curses.priv.h>
#include <tic.h>		/* for MAX_NAME_SIZE */
#include <term_entry.h>

#if SVR4_TERMIO && !defined(_POSIX_SOURCE)
#define _POSIX_SOURCE
#endif

#if HAVE_LOCALE_H
#include <locale.h>
#endif

#include <term.h>		/* lines, columns, cur_term */

MODULE_ID("$Id: lib_setup.c,v 1.111 2008/08/03 22:42:33 tom Exp $")

/****************************************************************************
 *
 * Terminal size computation
 *
 ****************************************************************************/

#if HAVE_SIZECHANGE
# if !defined(sun) || !TERMIOS
#  if HAVE_SYS_IOCTL_H
#   include <sys/ioctl.h>
#  endif
# endif
#endif

#if NEED_PTEM_H
 /* On SCO, they neglected to define struct winsize in termios.h -- it's only
  * in termio.h and ptem.h (the former conflicts with other definitions).
  */
# include <sys/stream.h>
# include <sys/ptem.h>
#endif

#if HAVE_LANGINFO_CODESET
#include <langinfo.h>
#endif

/*
 * SCO defines TIOCGSIZE and the corresponding struct.  Other systems (SunOS,
 * Solaris, IRIX) define TIOCGWINSZ and struct winsize.
 */
#ifdef TIOCGSIZE
# define IOCTL_WINSIZE TIOCGSIZE
# define STRUCT_WINSIZE struct ttysize
# define WINSIZE_ROWS(n) (int)n.ts_lines
# define WINSIZE_COLS(n) (int)n.ts_cols
#else
# ifdef TIOCGWINSZ
#  define IOCTL_WINSIZE TIOCGWINSZ
#  define STRUCT_WINSIZE struct winsize
#  define WINSIZE_ROWS(n) (int)n.ws_row
#  define WINSIZE_COLS(n) (int)n.ws_col
# endif
#endif

/*
 * Reduce explicit use of "cur_term" global variable.
 */
#undef CUR
#define CUR termp->type.

/*
 * Wrap global variables in this module.
 */
#if USE_REENTRANT
NCURSES_EXPORT(char *)
NCURSES_PUBLIC_VAR(ttytype) (void)
{
    static char empty[] = "";
    return cur_term ? cur_term->type.term_names : empty;
}
NCURSES_EXPORT(int *)
_nc_ptr_Lines(void)
{
    return ptrLines();
}
NCURSES_EXPORT(int)
NCURSES_PUBLIC_VAR(LINES) (void)
{
    return *_nc_ptr_Lines();
}
NCURSES_EXPORT(int *)
_nc_ptr_Cols(void)
{
    return ptrCols();
}
NCURSES_EXPORT(int)
NCURSES_PUBLIC_VAR(COLS) (void)
{
    return *_nc_ptr_Cols();
}
NCURSES_EXPORT(int)
NCURSES_PUBLIC_VAR(TABSIZE) (void)
{
    return SP ? SP->_TABSIZE : 8;
}
#else
NCURSES_EXPORT_VAR(char) ttytype[NAMESIZE] = "";
NCURSES_EXPORT_VAR(int) LINES = 0;
NCURSES_EXPORT_VAR(int) COLS = 0;
NCURSES_EXPORT_VAR(int) TABSIZE = 0;
#endif

#if NCURSES_EXT_FUNCS
NCURSES_EXPORT(int)
set_tabsize(int value)
{
    int code = OK;
#if USE_REENTRANT
    if (SP) {
	SP->_TABSIZE = value;
    } else {
	code = ERR;
    }
#else
    TABSIZE = value;
#endif
    return code;
}
#endif

#if USE_SIGWINCH
/*
 * If we have a pending SIGWINCH, set the flag in each screen.
 */
NCURSES_EXPORT(int)
_nc_handle_sigwinch(SCREEN *sp)
{
    SCREEN *scan;

    if (_nc_globals.have_sigwinch) {
	_nc_globals.have_sigwinch = 0;

	for (each_screen(scan)) {
	    scan->_sig_winch = TRUE;
	}
    }

    return (sp ? sp->_sig_winch : 0);
}

#endif

NCURSES_EXPORT(void)
use_env(bool f)
{
    T((T_CALLED("use_env()")));
    _nc_prescreen.use_env = f;
    returnVoid;
}

NCURSES_EXPORT(void)
_nc_get_screensize(SCREEN *sp, int *linep, int *colp)
/* Obtain lines/columns values from the environment and/or terminfo entry */
{
    TERMINAL *termp = cur_term;
    int my_tabsize;

    /* figure out the size of the screen */
    T(("screen size: terminfo lines = %d columns = %d", lines, columns));

    if (!_nc_prescreen.use_env) {
	*linep = (int) lines;
	*colp = (int) columns;
    } else {			/* usually want to query LINES and COLUMNS from environment */
	int value;

	*linep = *colp = 0;

	/* first, look for environment variables */
	if ((value = _nc_getenv_num("LINES")) > 0) {
	    *linep = value;
	}
	if ((value = _nc_getenv_num("COLUMNS")) > 0) {
	    *colp = value;
	}
	T(("screen size: environment LINES = %d COLUMNS = %d", *linep, *colp));

#ifdef __EMX__
	if (*linep <= 0 || *colp <= 0) {
	    int screendata[2];
	    _scrsize(screendata);
	    *colp = screendata[0];
	    *linep = screendata[1];
	    T(("EMX screen size: environment LINES = %d COLUMNS = %d",
	       *linep, *colp));
	}
#endif
#if HAVE_SIZECHANGE
	/* if that didn't work, maybe we can try asking the OS */
	if (*linep <= 0 || *colp <= 0) {
	    if (isatty(cur_term->Filedes)) {
		STRUCT_WINSIZE size;

		errno = 0;
		do {
		    if (ioctl(cur_term->Filedes, IOCTL_WINSIZE, &size) < 0
			&& errno != EINTR)
			goto failure;
		} while
		    (errno == EINTR);

		/*
		 * Solaris lets users override either dimension with an
		 * environment variable.
		 */
		if (*linep <= 0)
		    *linep = (sp != 0 && sp->_filtered) ? 1 : WINSIZE_ROWS(size);
		if (*colp <= 0)
		    *colp = WINSIZE_COLS(size);
	    }
	    /* FALLTHRU */
	  failure:;
	}
#endif /* HAVE_SIZECHANGE */

	/* if we can't get dynamic info about the size, use static */
	if (*linep <= 0) {
	    *linep = (int) lines;
	}
	if (*colp <= 0) {
	    *colp = (int) columns;
	}

	/* the ultimate fallback, assume fixed 24x80 size */
	if (*linep <= 0) {
	    *linep = 24;
	}
	if (*colp <= 0) {
	    *colp = 80;
	}

	/*
	 * Put the derived values back in the screen-size caps, so
	 * tigetnum() and tgetnum() will do the right thing.
	 */
	lines = (short) (*linep);
	columns = (short) (*colp);
    }

    T(("screen size is %dx%d", *linep, *colp));

    if (VALID_NUMERIC(init_tabs))
	my_tabsize = (int) init_tabs;
    else
	my_tabsize = 8;

#if USE_REENTRANT
    if (sp != 0)
	sp->_TABSIZE = my_tabsize;
#else
    TABSIZE = my_tabsize;
#endif
    T(("TABSIZE = %d", TABSIZE));
}

#if USE_SIZECHANGE
NCURSES_EXPORT(void)
_nc_update_screensize(SCREEN *sp)
{
    TERMINAL *termp = cur_term;
    int old_lines = lines;
    int new_lines;
    int old_cols = columns;
    int new_cols;

    _nc_get_screensize(sp, &new_lines, &new_cols);

    /*
     * See is_term_resized() and resizeterm().
     * We're doing it this way because those functions belong to the upper
     * ncurses library, while this resides in the lower terminfo library.
     */
    if (sp != 0
	&& sp->_resize != 0) {
	if ((new_lines != old_lines) || (new_cols != old_cols))
	    sp->_resize(new_lines, new_cols);
	sp->_sig_winch = FALSE;
    }
}
#endif

/****************************************************************************
 *
 * Terminal setup
 *
 ****************************************************************************/

#define ret_error(code, fmt, arg)	if (errret) {\
					    *errret = code;\
					    returnCode(ERR);\
					} else {\
					    fprintf(stderr, fmt, arg);\
					    exit(EXIT_FAILURE);\
					}

#define ret_error0(code, msg)		if (errret) {\
					    *errret = code;\
					    returnCode(ERR);\
					} else {\
					    fprintf(stderr, msg);\
					    exit(EXIT_FAILURE);\
					}

#if USE_DATABASE || USE_TERMCAP
/*
 * Return 1 if entry found, 0 if not found, -1 if database not accessible,
 * just like tgetent().
 */
static int
grab_entry(const char *const tn, TERMTYPE *const tp)
{
    char filename[PATH_MAX];
    int status = _nc_read_entry(tn, filename, tp);

    /*
     * If we have an entry, force all of the cancelled strings to null
     * pointers so we don't have to test them in the rest of the library.
     * (The terminfo compiler bypasses this logic, since it must know if
     * a string is cancelled, for merging entries).
     */
    if (status == TGETENT_YES) {
	unsigned n;
	for_each_boolean(n, tp) {
	    if (!VALID_BOOLEAN(tp->Booleans[n]))
		tp->Booleans[n] = FALSE;
	}
	for_each_string(n, tp) {
	    if (tp->Strings[n] == CANCELLED_STRING)
		tp->Strings[n] = ABSENT_STRING;
	}
    }
    return (status);
}
#endif

/*
**	do_prototype()
**
**	Take the real command character out of the CC environment variable
**	and substitute it in for the prototype given in 'command_character'.
*/
static void
do_prototype(TERMINAL * termp)
{
    unsigned i;
    char CC;
    char proto;
    char *tmp;

    if ((tmp = getenv("CC")) != 0) {
	if ((CC = *tmp) != 0) {
	    proto = *command_character;

	    for_each_string(i, &(termp->type)) {
		for (tmp = termp->type.Strings[i]; *tmp; tmp++) {
		    if (*tmp == proto)
			*tmp = CC;
		}
	    }
	}
    }
}

/*
 * Find the locale which is in effect.
 */
NCURSES_EXPORT(char *)
_nc_get_locale(void)
{
    char *env;
#if HAVE_LOCALE_H
    /*
     * This is preferable to using getenv() since it ensures that we are using
     * the locale which was actually initialized by the application.
     */
    env = setlocale(LC_CTYPE, 0);
#else
    if (((env = getenv("LC_ALL")) != 0 && *env != '\0')
	|| ((env = getenv("LC_CTYPE")) != 0 && *env != '\0')
	|| ((env = getenv("LANG")) != 0 && *env != '\0')) {
	;
    }
#endif
    T(("_nc_get_locale %s", _nc_visbuf(env)));
    return env;
}

/*
 * Check if we are running in a UTF-8 locale.
 */
NCURSES_EXPORT(int)
_nc_unicode_locale(void)
{
    int result = 0;
#if HAVE_LANGINFO_CODESET
    char *env = nl_langinfo(CODESET);
    result = !strcmp(env, "UTF-8");
    T(("_nc_unicode_locale(%s) ->%d", env, result));
#else
    char *env = _nc_get_locale();
    if (env != 0) {
	if (strstr(env, ".UTF-8") != 0) {
	    result = 1;
	    T(("_nc_unicode_locale(%s) ->%d", env, result));
	}
    }
#endif
    return result;
}

#define CONTROL_N(s) ((s) != 0 && strstr(s, "\016") != 0)
#define CONTROL_O(s) ((s) != 0 && strstr(s, "\017") != 0)

/*
 * Check for known broken cases where a UTF-8 locale breaks the alternate
 * character set.
 */
NCURSES_EXPORT(int)
_nc_locale_breaks_acs(TERMINAL * termp)
{
    char *env;

    if ((env = getenv("NCURSES_NO_UTF8_ACS")) != 0) {
	return atoi(env);
    } else if ((env = getenv("TERM")) != 0) {
	if (strstr(env, "linux"))
	    return 1;		/* always broken */
	if (strstr(env, "screen") != 0
	    && ((env = getenv("TERMCAP")) != 0
		&& strstr(env, "screen") != 0)
	    && strstr(env, "hhII00") != 0) {
	    if (CONTROL_N(enter_alt_charset_mode) ||
		CONTROL_O(enter_alt_charset_mode) ||
		CONTROL_N(set_attributes) ||
		CONTROL_O(set_attributes))
		return 1;
	}
    }
    return 0;
}

/*
 * This entrypoint is called from tgetent() to allow a special case of reusing
 * the same TERMINAL data (see comment).
 */
NCURSES_EXPORT(int)
_nc_setupterm(NCURSES_CONST char *tname, int Filedes, int *errret, bool reuse)
{
    TERMINAL *termp;
    int status;

    START_TRACE();
    T((T_CALLED("setupterm(%s,%d,%p)"), _nc_visbuf(tname), Filedes, errret));

    if (tname == 0) {
	tname = getenv("TERM");
	if (tname == 0 || *tname == '\0') {
	    ret_error0(TGETENT_ERR, "TERM environment variable not set.\n");
	}
    }

    if (strlen(tname) > MAX_NAME_SIZE) {
	ret_error(TGETENT_ERR,
		  "TERM environment must be <= %d characters.\n",
		  MAX_NAME_SIZE);
    }

    T(("your terminal name is %s", tname));

    /*
     * Allow output redirection.  This is what SVr3 does.  If stdout is
     * directed to a file, screen updates go to standard error.
     */
    if (Filedes == STDOUT_FILENO && !isatty(Filedes))
	Filedes = STDERR_FILENO;

    /*
     * Check if we have already initialized to use this terminal.  If so, we
     * do not need to re-read the terminfo entry, or obtain TTY settings.
     *
     * This is an improvement on SVr4 curses.  If an application mixes curses
     * and termcap calls, it may call both initscr and tgetent.  This is not
     * really a good thing to do, but can happen if someone tries using ncurses
     * with the readline library.  The problem we are fixing is that when
     * tgetent calls setupterm, the resulting Ottyb struct in cur_term is
     * zeroed.  A subsequent call to endwin uses the zeroed terminal settings
     * rather than the ones saved in initscr.  So we check if cur_term appears
     * to contain terminal settings for the same output file as our current
     * call - and copy those terminal settings.  (SVr4 curses does not do this,
     * however applications that are working around the problem will still work
     * properly with this feature).
     */
    if (reuse
	&& (termp = cur_term) != 0
	&& termp->Filedes == Filedes
	&& termp->_termname != 0
	&& !strcmp(termp->_termname, tname)
	&& _nc_name_match(termp->type.term_names, tname, "|")) {
	T(("reusing existing terminal information and mode-settings"));
    } else {

	termp = typeCalloc(TERMINAL, 1);

	if (termp == 0) {
	    ret_error0(TGETENT_ERR,
		       "Not enough memory to create terminal structure.\n");
	}
#if USE_DATABASE || USE_TERMCAP
	status = grab_entry(tname, &termp->type);
#else
	status = TGETENT_NO;
#endif

	/* try fallback list if entry on disk */
	if (status != TGETENT_YES) {
	    const TERMTYPE *fallback = _nc_fallback(tname);

	    if (fallback) {
		termp->type = *fallback;
		status = TGETENT_YES;
	    }
	}

	if (status != TGETENT_YES) {
	    del_curterm(termp);
	    if (status == TGETENT_ERR) {
		ret_error0(status, "terminals database is inaccessible\n");
	    } else if (status == TGETENT_NO) {
		ret_error(status, "'%s': unknown terminal type.\n", tname);
	    }
	}
#if !USE_REENTRANT
	strncpy(ttytype, termp->type.term_names, NAMESIZE - 1);
	ttytype[NAMESIZE - 1] = '\0';
#endif

	termp->Filedes = Filedes;
	termp->_termname = strdup(tname);

	set_curterm(termp);

	if (command_character && getenv("CC"))
	    do_prototype(termp);

	/*
	 * If an application calls setupterm() rather than initscr() or
	 * newterm(), we will not have the def_prog_mode() call in
	 * _nc_setupscreen().  Do it now anyway, so we can initialize the
	 * baudrate.
	 */
	if (isatty(Filedes)) {
	    def_prog_mode();
	    baudrate();
	}
    }

    /*
     * We should always check the screensize, just in case.
     */
    _nc_get_screensize(SP, ptrLines(), ptrCols());

    if (errret)
	*errret = TGETENT_YES;

    if (generic_type) {
	ret_error(TGETENT_NO, "'%s': I need something more specific.\n", tname);
    }
    if (hard_copy) {
	ret_error(TGETENT_YES, "'%s': I can't handle hardcopy terminals.\n", tname);
    }
    returnCode(OK);
}

/*
 *	setupterm(termname, Filedes, errret)
 *
 *	Find and read the appropriate object file for the terminal
 *	Make cur_term point to the structure.
 */
NCURSES_EXPORT(int)
setupterm(NCURSES_CONST char *tname, int Filedes, int *errret)
{
    return _nc_setupterm(tname, Filedes, errret, FALSE);
}

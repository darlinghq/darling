/****************************************************************************
 * Copyright (c) 2006,2007 Free Software Foundation, Inc.                   *
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
 * Iterators for terminal databases.
 */

#include <curses.priv.h>

#include <tic.h>

MODULE_ID("$Id: db_iterator.c,v 1.6 2007/04/22 00:00:26 tom Exp $")

#define HaveTicDirectory _nc_globals.have_tic_directory
#define KeepTicDirectory _nc_globals.keep_tic_directory
#define TicDirectory     _nc_globals.tic_directory

/*
 * Record the "official" location of the terminfo directory, according to
 * the place where we're writing to, or the normal default, if not.
 */
NCURSES_EXPORT(const char *)
_nc_tic_dir(const char *path)
{
    if (!KeepTicDirectory) {
	if (path != 0) {
	    TicDirectory = path;
	    HaveTicDirectory = TRUE;
	} else if (!HaveTicDirectory && use_terminfo_vars()) {
	    char *envp;
	    if ((envp = getenv("TERMINFO")) != 0)
		return _nc_tic_dir(envp);
	}
    }
    return TicDirectory;
}

/*
 * Special fix to prevent the terminfo directory from being moved after tic
 * has chdir'd to it.  If we let it be changed, then if $TERMINFO has a
 * relative path, we'll lose track of the actual directory.
 */
NCURSES_EXPORT(void)
_nc_keep_tic_dir(const char *path)
{
    _nc_tic_dir(path);
    KeepTicDirectory = TRUE;
}

/*
 * Process the list of :-separated directories, looking for the terminal type.
 * We don't use strtok because it does not show us empty tokens.
 */
#define ThisDbList	_nc_globals.dbi_list
#define ThisDbSize	_nc_globals.dbi_size

/*
 * Cleanup.
 */
NCURSES_EXPORT(void)
_nc_last_db(void)
{
    if (ThisDbList != 0) {
	FreeAndNull(ThisDbList);
    }
    ThisDbSize = 0;
}

/* The TERMINFO_DIRS value, if defined by the configure script, begins with a
 * ":", which will be interpreted as TERMINFO.
 */
static const char *
next_list_item(const char *source, int *offset)
{
    if (source != 0) {
	FreeIfNeeded(ThisDbList);
	ThisDbList = strdup(source);
	ThisDbSize = strlen(source);
    }

    if (ThisDbList != 0 && ThisDbSize && *offset < ThisDbSize) {
	static char system_db[] = TERMINFO;
	char *result = ThisDbList + *offset;
	char *marker = strchr(result, NCURSES_PATHSEP);

	/*
	 * Put a null on the marker if a separator was found.  Set the offset
	 * to the next position after the marker so we can call this function
	 * again, using the data at the offset.
	 */
	if (marker == 0) {
	    *offset += strlen(result) + 1;
	    marker = result + *offset;
	} else {
	    *marker++ = 0;
	    *offset = marker - ThisDbList;
	}
	if (*result == 0 && result != (ThisDbList + ThisDbSize))
	    result = system_db;
	return result;
    }
    return 0;
}

#define NEXT_DBD(var, offset) next_list_item((*offset == 0) ? var : 0, offset)

/*
 * This is a simple iterator which allows the caller to step through the
 * possible locations for a terminfo directory.  ncurses uses this to find
 * terminfo files to read.
 */
NCURSES_EXPORT(const char *)
_nc_next_db(DBDIRS * state, int *offset)
{
    const char *result;
    char *envp;

    while (*state < dbdLAST) {
	DBDIRS next = (DBDIRS) ((int) (*state) + 1);

	result = 0;

	switch (*state) {
	case dbdTIC:
	    if (HaveTicDirectory)
		result = _nc_tic_dir(0);
	    break;
#if USE_DATABASE
	case dbdEnvOnce:
	    if (use_terminfo_vars()) {
		if ((envp = getenv("TERMINFO")) != 0)
		    result = _nc_tic_dir(envp);
	    }
	    break;
	case dbdHome:
	    if (use_terminfo_vars()) {
		result = _nc_home_terminfo();
	    }
	    break;
	case dbdEnvList:
	    if (use_terminfo_vars()) {
		if ((result = NEXT_DBD(getenv("TERMINFO_DIRS"), offset)) != 0)
		    next = *state;
	    }
	    break;
	case dbdCfgList:
#ifdef TERMINFO_DIRS
	    if ((result = NEXT_DBD(TERMINFO_DIRS, offset)) != 0)
		next = *state;
#endif
	    break;
	case dbdCfgOnce:
#ifndef TERMINFO_DIRS
	    result = TERMINFO;
#endif
	    break;
#endif /* USE_DATABASE */
#if USE_TERMCAP
	case dbdEnvOnce2:
	    if (use_terminfo_vars()) {
		if ((envp = getenv("TERMCAP")) != 0)
		    result = _nc_tic_dir(envp);
	    }
	    break;
	case dbdEnvList2:
	    if (use_terminfo_vars()) {
		if ((result = NEXT_DBD(getenv("TERMPATH"), offset)) != 0)
		    next = *state;
	    }
	    break;
	case dbdCfgList2:
	    if ((result = NEXT_DBD(TERMPATH, offset)) != 0)
		next = *state;
	    break;
#endif /* USE_TERMCAP */
	case dbdLAST:
	    break;
	}
	if (*state != next) {
	    *state = next;
	    *offset = 0;
	    _nc_last_db();
	}
	if (result != 0) {
	    return result;
	}
    }
    return 0;
}

NCURSES_EXPORT(void)
_nc_first_db(DBDIRS * state, int *offset)
{
    *state = dbdTIC;
    *offset = 0;
}

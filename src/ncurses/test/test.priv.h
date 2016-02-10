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
 *  Author: Thomas E. Dickey                    1996-on                     *
 ****************************************************************************/
/* $Id: test.priv.h,v 1.79 2008/10/04 21:53:41 tom Exp $ */

#ifndef __TEST_PRIV_H
#define __TEST_PRIV_H 1

#include <ncurses_cfg.h>

/*
 * Fix ifdef's that look for the form/menu/panel libraries, if we are building
 * with wide-character ncurses.
 */
#ifdef  HAVE_LIBFORMW
#define HAVE_LIBFORMW 1
#endif

#ifdef  HAVE_LIBMENUW
#define HAVE_LIBMENUW 1
#endif

#ifdef  HAVE_LIBPANELW
#define HAVE_LIBPANELW 1
#endif

/*
 * Fallback definitions to accommodate broken compilers.
 */
#ifndef HAVE_CURSES_VERSION
#define HAVE_CURSES_VERSION 0
#endif

#ifndef HAVE_CHGAT
#define HAVE_CHGAT 0
#endif

#ifndef HAVE_COLOR_SET
#define HAVE_COLOR_SET 0
#endif

#ifndef HAVE_FILTER
#define HAVE_FILTER 0
#endif

#ifndef HAVE_FORM_H
#define HAVE_FORM_H 0
#endif

#ifndef HAVE_GETBEGX
#define HAVE_GETBEGX 0
#endif

#ifndef HAVE_GETCURX
#define HAVE_GETCURX 0
#endif

#ifndef HAVE_GETMAXX
#define HAVE_GETMAXX 0
#endif

#ifndef HAVE_GETOPT_H
#define HAVE_GETOPT_H 0
#endif

#ifndef HAVE_GETPARX
#define HAVE_GETPARX 0
#endif

#ifndef HAVE_GETWIN
#define HAVE_GETWIN 0
#endif

#ifndef HAVE_LIBFORM
#define HAVE_LIBFORM 0
#endif

#ifndef HAVE_LIBMENU
#define HAVE_LIBMENU 0
#endif

#ifndef HAVE_LIBPANEL
#define HAVE_LIBPANEL 0
#endif

#ifndef HAVE_LOCALE_H
#define HAVE_LOCALE_H 0
#endif

#ifndef HAVE_MENU_H
#define HAVE_MENU_H 0
#endif

#ifndef HAVE_MVVLINE
#define HAVE_MVVLINE 0
#endif

#ifndef HAVE_MVWVLINE
#define HAVE_MVWVLINE 0
#endif

#ifndef HAVE_NAPMS
#define HAVE_NAPMS 1
#endif

#ifndef HAVE_NC_ALLOC_H
#define HAVE_NC_ALLOC_H 0
#endif

#ifndef HAVE_PANEL_H
#define HAVE_PANEL_H 0
#endif

#ifndef HAVE_PUTWIN
#define HAVE_PUTWIN 0
#endif

#ifndef HAVE_RESIZE_TERM
#define HAVE_RESIZE_TERM 0
#endif

#ifndef HAVE_RIPOFFLINE
#define HAVE_RIPOFFLINE 0
#endif

#ifndef HAVE_SETUPTERM
#define HAVE_SETUPTERM 0
#endif

#ifndef HAVE_SLK_COLOR
#define HAVE_SLK_COLOR 0
#endif

#ifndef HAVE_SLK_INIT
#define HAVE_SLK_INIT 0
#endif

#ifndef HAVE_TERMATTRS
#define HAVE_TERMATTRS 0
#endif

#ifndef HAVE_TERMNAME
#define HAVE_TERMNAME 0
#endif

#ifndef HAVE_TGETENT
#define HAVE_TGETENT 0
#endif

#ifndef HAVE_TIGETNUM
#define HAVE_TIGETNUM 0
#endif

#ifndef HAVE_TYPEAHEAD
#define HAVE_TYPEAHEAD 0
#endif

#ifndef HAVE_TIGETSTR
#define HAVE_TIGETSTR 0
#endif

#ifndef HAVE_WINSSTR
#define HAVE_WINSSTR 0
#endif

#ifndef HAVE_USE_DEFAULT_COLORS
#define HAVE_USE_DEFAULT_COLORS 0
#endif

#ifndef HAVE_WRESIZE
#define HAVE_WRESIZE 0
#endif

#ifndef NCURSES_EXT_FUNCS
#define NCURSES_EXT_FUNCS 0
#endif

#ifndef NEED_PTEM_H
#define NEED_PTEM_H 0
#endif

#ifndef NO_LEAKS
#define NO_LEAKS 0
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <signal.h>	/* include before curses.h to work around glibc bug */

#if defined(HAVE_XCURSES)
#include <xcurses.h>
#elif defined(HAVE_NCURSESW_NCURSES_H)
#include <ncursesw/curses.h>
#elif defined(HAVE_NCURSES_NCURSES_H)
#include <ncurses/curses.h>
#else
#include <curses.h>
#endif

#if defined(HAVE_XCURSES)
/* no other headers */
#undef  HAVE_SETUPTERM		/* nonfunctional */
#define HAVE_SETUPTERM 0
#undef  HAVE_TGETENT		/* nonfunctional */
#define HAVE_TGETENT 0
#undef  HAVE_TIGETSTR		/* nonfunctional */
#define HAVE_TIGETSTR 0
#elif defined(HAVE_NCURSESW_TERM_H)
#include <ncursesw/term.h>
#elif defined(HAVE_NCURSES_TERM_H)
#include <ncurses/term.h>
#elif defined(HAVE_TERM_H)
#include <term.h>
#endif

/*
 * Not all curses.h implementations include unctrl.h,
 * Solaris 10 xpg4 for example.
 */
#if defined(NCURSES_VERSION) || defined(_XOPEN_CURSES)
#if defined(HAVE_NCURSESW_NCURSES_H)
#include <ncursesw/unctrl.h>
#elif defined(HAVE_NCURSES_NCURSES_H)
#include <ncurses/unctrl.h>
#else
#include <unctrl.h>
#endif
#endif

#if HAVE_GETOPT_H
#include <getopt.h>
#else
/* 'getopt()' may be prototyped in <stdlib.h>, but declaring its variables
 * doesn't hurt.
 */
extern char *optarg;
extern int optind;
#endif /* HAVE_GETOPT_H */

#if HAVE_LOCALE_H
#include <locale.h>
#else
#define setlocale(name,string) /* nothing */
#endif

#include <assert.h>
#include <ctype.h>

#ifndef GCC_NORETURN
#define GCC_NORETURN /* nothing */
#endif
#ifndef GCC_PRINTFLIKE
#define GCC_PRINTFLIKE(a,b) /* nothing */
#endif
#ifndef GCC_UNUSED
#define GCC_UNUSED /* nothing */
#endif

#ifndef HAVE_GETNSTR
#define getnstr(s,n) getstr(s)
#endif

#ifndef USE_SOFTKEYS
#if HAVE_SLK_INIT
#define USE_SOFTKEYS 1
#else
#define USE_SOFTKEYS 0
#endif
#endif

#ifndef USE_WIDEC_SUPPORT
#if defined(_XOPEN_SOURCE_EXTENDED) && defined(WACS_ULCORNER)
#define USE_WIDEC_SUPPORT 1
#else
#define USE_WIDEC_SUPPORT 0
#endif
#endif

#if HAVE_PANEL_H && HAVE_LIBPANEL
#define USE_LIBPANEL 1
#else
#define USE_LIBPANEL 0
#endif

#if HAVE_MENU_H && HAVE_LIBMENU
#define USE_LIBMENU 1
#else
#define USE_LIBMENU 0
#endif

#if HAVE_FORM_H && HAVE_LIBFORM
#define USE_LIBFORM 1
#else
#define USE_LIBFORM 0
#endif

#ifndef HAVE_TYPE_ATTR_T
#if !USE_WIDEC_SUPPORT && !defined(attr_t)
#define attr_t chtype
#endif
#endif

#undef NCURSES_CH_T
#if !USE_WIDEC_SUPPORT
#define NCURSES_CH_T chtype
#else
#define NCURSES_CH_T cchar_t
#endif

#ifndef NCURSES_OPAQUE
#define NCURSES_OPAQUE 0
#endif

#ifndef CCHARW_MAX
#define CCHARW_MAX 5
#endif

#undef CTRL
#define CTRL(x)	((x) & 0x1f)

#define QUIT		CTRL('Q')
#define ESCAPE		CTRL('[')

#ifndef KEY_MIN
#define KEY_MIN 256	/* not defined in Solaris 8 */
#endif

/*
 * Workaround for HPUX
 */
#if defined(__hpux) && !defined(NCURSES_VERSION)
#define getbegx(w) __getbegx(w)
#define getbegy(w) __getbegy(w)
#define getcurx(w) __getcurx(w)
#define getcury(w) __getcury(w)
#define getmaxx(w) __getmaxx(w)
#define getmaxy(w) __getmaxy(w)
#define getparx(w) __getparx(w)
#define getpary(w) __getpary(w)
#endif

/*
 * These usually are implemented as macros, but may be functions.
 */
#if !defined(getcurx) && !HAVE_GETCURX
#define getcurx(win)            ((win)?(win)->_curx:ERR)
#define getcury(win)            ((win)?(win)->_cury:ERR)
#endif

#if !defined(getbegx) && !HAVE_GETBEGX
#define getbegx(win)            ((win)?(win)->_begx:ERR)
#define getbegy(win)            ((win)?(win)->_begy:ERR)
#endif

#if !defined(getmaxx) && !HAVE_GETMAXX
#define getmaxx(win)            ((win)?((win)->_maxx + 1):ERR)
#define getmaxy(win)            ((win)?((win)->_maxy + 1):ERR)
#endif

/*
 * Solaris 10 xpg4:
#define	__m_getparx(w)		((w)->_parent == (WINDOW *) 0 ? -1 \
				: (w)->_begx - (w)->_parent->_begx)
 */
#if !defined(getparx) && !HAVE_GETPARX
#ifdef __m_getparx
#define getparx(win)            __m_getparx(win)
#define getpary(win)            __m_getpary(win)
#else
#define getparx(win)            ((win)?((win)->_parx + 1):ERR)
#define getpary(win)            ((win)?((win)->_pary + 1):ERR)
#endif
#endif

#if !defined(mvwvline) && !HAVE_MVWVLINE
#define mvwvline(w,y,x,ch,n)    (move(y,x) == ERR ? ERR : wvline(w,ch,n))
#define mvwhline(w,y,x,ch,n)    (move(y,x) == ERR ? ERR : whline(w,ch,n))
#endif

#if !defined(mvvline) && !HAVE_MVVLINE
#define mvvline(y,x,ch,n)       (move(y,x) == ERR ? ERR : vline(ch,n))
#define mvhline(y,x,ch,n)       (move(y,x) == ERR ? ERR : hline(ch,n))
#endif

/*
 * Try to accommodate curses implementations that have no terminfo support.
 */
#if HAVE_TIGETNUM
#define TIGETNUM(ti,tc) tigetnum(ti)
#else
#define TIGETNUM(ti,tc) tgetnum(tc)
#endif

#if HAVE_TIGETSTR
#define TIGETSTR(ti,tc) tigetstr(ti)
#else
#define TIGETSTR(ti,tc) tgetstr(tc,&area_pointer)
#endif

/* ncurses implements tparm() with varargs, X/Open with a fixed-parameter list
 * (which is incompatible with legacy usage, doesn't solve any problems).
 */
#define tparm3(a,b,c) tparm(a,b,c,0,0,0,0,0,0,0)
#define tparm2(a,b)   tparm(a,b,0,0,0,0,0,0,0,0)

#define UChar(c)    ((unsigned char)(c))

#define SIZEOF(table)	(sizeof(table)/sizeof(table[0]))

#if defined(NCURSES_VERSION) && HAVE_NC_ALLOC_H
#include <nc_alloc.h>
#if HAVE_NC_FREEALL && defined(USE_TINFO)
#undef ExitProgram
#define ExitProgram(code) _nc_free_tinfo(code)
#endif
#else
#define typeMalloc(type,n) (type *) malloc((n) * sizeof(type))
#define typeCalloc(type,elts) (type *) calloc((elts), sizeof(type))
#define typeRealloc(type,n,p) (type *) realloc(p, (n) * sizeof(type))
#endif

#ifndef ExitProgram
#define ExitProgram(code) exit(code)
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

/* Use this to quiet gcc's -Wwrite-strings warnings, but accommodate SVr4
 * curses which doesn't have const parameters declared (so far) in the places
 * that XSI shows.
 */
#ifndef NCURSES_CONST
#define NCURSES_CONST /* nothing */
#endif

/* out-of-band values for representing absent capabilities */
#define ABSENT_BOOLEAN		((signed char)-1)	/* 255 */
#define ABSENT_NUMERIC		(-1)
#define ABSENT_STRING		(char *)0

/* out-of-band values for representing cancels */
#define CANCELLED_BOOLEAN	((signed char)-2)	/* 254 */
#define CANCELLED_NUMERIC	(-2)
#define CANCELLED_STRING	(char *)(-1)

#define VALID_BOOLEAN(s) ((unsigned char)(s) <= 1) /* reject "-1" */
#define VALID_NUMERIC(s) ((s) >= 0)
#define VALID_STRING(s)  ((s) != CANCELLED_STRING && (s) != ABSENT_STRING)

#define VT_ACSC "``aaffggiijjkkllmmnnooppqqrrssttuuvvwwxxyyzz{{||}}~~"

#define CATCHALL(handler) { \
		int nsig; \
		for (nsig = SIGHUP; nsig < SIGTERM; ++nsig) \
		    if (nsig != SIGKILL) \
			signal(nsig, handler); \
	    }

/*
 * Workaround for clean(er) compile with Solaris's legacy curses.
 * The same would be needed for HPUX 10.20
 */
#ifndef TPUTS_ARG
#if defined(sun) && !defined(_XOPEN_CURSES) && !defined(NCURSES_VERSION_PATCH)
#define TPUTS_ARG char
extern char *tgoto(char *, int, int);	/* available, but not prototyped */
#else
#define TPUTS_ARG int
#endif
#endif

/*
 * Workarounds for Solaris's X/Open curses
 */
#if defined(sun) && defined(_XOPEN_CURSES) && !defined(NCURSES_VERSION_PATCH)
#if !defined(KEY_MIN) && defined(__KEY_MIN)
#define KEY_MIN __KEY_MIN
#endif
#if !defined(KEY_MAX) && defined(__KEY_MIN)
#define KEY_MAX __KEY_MAX
#endif
#endif

/*
 * ncurses uses const in some places where X/Open does (or did) not allow.
 */
#ifdef NCURSES_VERSION
#define CONST_MENUS const
#else
#define CONST_MENUS /* nothing */
#endif

#ifndef HAVE_USE_WINDOW
#if !defined(NCURSES_VERSION_PATCH) || (NCURSES_VERSION_PATCH < 20070915) || !NCURSES_EXT_FUNCS
#define HAVE_USE_WINDOW 0
#else
#define HAVE_USE_WINDOW 1
#endif
#endif

/*
 * Simplify setting up demo of threading with these macros.
 */

#if !HAVE_USE_WINDOW
typedef int (*NCURSES_WINDOW_CB)(WINDOW *, void *);
typedef int (*NCURSES_SCREEN_CB)(SCREEN *, void *);
#endif

#if HAVE_USE_WINDOW
#define USING_WINDOW(w,func) use_window(w, (NCURSES_WINDOW_CB) func, w)
#define USING_WINDOW2(w,func,data) use_window(w, (NCURSES_WINDOW_CB) func, data)
#define WANT_USE_WINDOW() extern void _nc_want_use_window(void)
#else
#define USING_WINDOW(w,func) func(w)
#define USING_WINDOW2(w,func,data) func(w,data)
#define WANT_USE_WINDOW() extern void _nc_want_use_window(void)
#endif

#if HAVE_USE_WINDOW
#define USING_SCREEN(s,func,data) use_screen(s, (NCURSES_SCREEN_CB) func, data)
#define WANT_USE_SCREEN() extern void _nc_want_use_screen(void)
#else
#define USING_SCREEN(s,func,data) func(s,data)
#define WANT_USE_SCREEN() extern void _nc_want_use_screen(void)
#endif

#ifdef TRACE
#define Trace(p) _tracef p
#define USE_TRACE 1
#else
#define Trace(p)		/* nothing */
#define USE_TRACE 0
#endif

#define init_mb(state)	memset(&state, 0, sizeof(state))

#endif /* __TEST_PRIV_H */

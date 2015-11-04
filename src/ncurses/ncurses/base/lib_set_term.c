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
**	lib_set_term.c
**
**	The routine set_term().
**
*/

#include <curses.priv.h>

#include <term.h>		/* cur_term */
#include <tic.h>

MODULE_ID("$Id: lib_set_term.c,v 1.117 2008/08/04 18:11:12 tom Exp $")

NCURSES_EXPORT(SCREEN *)
set_term(SCREEN *screenp)
{
    SCREEN *oldSP;
    SCREEN *newSP;

    T((T_CALLED("set_term(%p)"), screenp));

    _nc_lock_global(curses);

    oldSP = SP;
    _nc_set_screen(screenp);
    newSP = SP;

    if (newSP != 0) {
	set_curterm(newSP->_term);
#if !USE_REENTRANT
	curscr = newSP->_curscr;
	newscr = newSP->_newscr;
	stdscr = newSP->_stdscr;
	COLORS = newSP->_color_count;
	COLOR_PAIRS = newSP->_pair_count;
#endif
    } else {
	set_curterm(0);
#if !USE_REENTRANT
	curscr = 0;
	newscr = 0;
	stdscr = 0;
	COLORS = 0;
	COLOR_PAIRS = 0;
#endif
    }

    _nc_unlock_global(curses);

    T((T_RETURN("%p"), oldSP));
    return (oldSP);
}

static void
_nc_free_keytry(TRIES * kt)
{
    if (kt != 0) {
	_nc_free_keytry(kt->child);
	_nc_free_keytry(kt->sibling);
	free(kt);
    }
}

static bool
delink_screen(SCREEN *sp)
{
    SCREEN *last = 0;
    SCREEN *temp;
    bool result = FALSE;

    for (each_screen(temp)) {
	if (temp == sp) {
	    if (last)
		last = sp->_next_screen;
	    else
		_nc_screen_chain = sp->_next_screen;
	    result = TRUE;
	    break;
	}
	last = temp;
    }
    return result;
}

/*
 * Free the storage associated with the given SCREEN sp.
 */
NCURSES_EXPORT(void)
delscreen(SCREEN *sp)
{
    int i;

    T((T_CALLED("delscreen(%p)"), sp));

    _nc_lock_global(curses);
    if (delink_screen(sp)) {

	(void) _nc_freewin(sp->_curscr);
	(void) _nc_freewin(sp->_newscr);
	(void) _nc_freewin(sp->_stdscr);

	if (sp->_slk != 0) {
	    if (sp->_slk->ent != 0) {
		for (i = 0; i < sp->_slk->labcnt; ++i) {
		    FreeIfNeeded(sp->_slk->ent[i].ent_text);
		    FreeIfNeeded(sp->_slk->ent[i].form_text);
		}
		free(sp->_slk->ent);
	    }
	    free(sp->_slk);
	    sp->_slk = 0;
	}

	_nc_free_keytry(sp->_keytry);
	sp->_keytry = 0;

	_nc_free_keytry(sp->_key_ok);
	sp->_key_ok = 0;

	FreeIfNeeded(sp->_current_attr);

	FreeIfNeeded(sp->_color_table);
	FreeIfNeeded(sp->_color_pairs);

	FreeIfNeeded(sp->oldhash);
	FreeIfNeeded(sp->newhash);
	FreeIfNeeded(sp->hashtab);

	FreeIfNeeded(sp->_acs_map);
	FreeIfNeeded(sp->_screen_acs_map);

	/*
	 * If the associated output stream has been closed, we can discard the
	 * set-buffer.  Limit the error check to EBADF, since fflush may fail
	 * for other reasons than trying to operate upon a closed stream.
	 */
	if (sp->_ofp != 0
	    && sp->_setbuf != 0
	    && fflush(sp->_ofp) != 0
	    && errno == EBADF) {
	    free(sp->_setbuf);
	}

	del_curterm(sp->_term);
	free(sp);

	/*
	 * If this was the current screen, reset everything that the
	 * application might try to use (except cur_term, which may have
	 * multiple references in different screens).
	 */
	if (sp == SP) {
#if !USE_REENTRANT
	    curscr = 0;
	    newscr = 0;
	    stdscr = 0;
	    COLORS = 0;
	    COLOR_PAIRS = 0;
#endif
	    _nc_set_screen(0);
	}
    }
    _nc_unlock_global(curses);

    returnVoid;
}

static bool
no_mouse_event(SCREEN *sp GCC_UNUSED)
{
    return FALSE;
}

static bool
no_mouse_inline(SCREEN *sp GCC_UNUSED)
{
    return FALSE;
}

static bool
no_mouse_parse(SCREEN *sp GCC_UNUSED, int code GCC_UNUSED)
{
    return TRUE;
}

static void
no_mouse_resume(SCREEN *sp GCC_UNUSED)
{
}

static void
no_mouse_wrap(SCREEN *sp GCC_UNUSED)
{
}

#if NCURSES_EXT_FUNCS && USE_COLORFGBG
static char *
extract_fgbg(char *src, int *result)
{
    char *dst = 0;
    long value = strtol(src, &dst, 0);

    if (dst == 0) {
	dst = src;
    } else if (value >= 0) {
	*result = value;
    }
    while (*dst != 0 && *dst != ';')
	dst++;
    if (*dst == ';')
	dst++;
    return dst;
}
#endif

/* OS-independent screen initializations */
NCURSES_EXPORT(int)
_nc_setupscreen(int slines GCC_UNUSED,
		int scolumns GCC_UNUSED,
		FILE *output,
		bool filtered,
		int slk_format)
{
    char *env;
    int bottom_stolen = 0;
    bool support_cookies = USE_XMC_SUPPORT;
    ripoff_t *rop;

    T((T_CALLED("_nc_setupscreen(%d, %d, %p, %d, %d)"),
       slines, scolumns, output, filtered, slk_format));

    assert(SP == 0);		/* has been reset in newterm() ! */
    if (!_nc_alloc_screen()
	|| ((SP->_acs_map = typeCalloc(chtype, ACS_LEN)) == 0)
	|| ((SP->_screen_acs_map = typeCalloc(bool, ACS_LEN)) == 0)) {
	returnCode(ERR);
    }

    T(("created SP %p", SP));
    SP->_next_screen = _nc_screen_chain;
    _nc_screen_chain = SP;

    if ((SP->_current_attr = typeCalloc(NCURSES_CH_T, 1)) == 0)
	returnCode(ERR);

    /*
     * We should always check the screensize, just in case.
     */
    _nc_get_screensize(SP, &slines, &scolumns);
    SET_LINES(slines);
    SET_COLS(scolumns);
    T((T_CREATE("screen %s %dx%d"), termname(), LINES, COLS));

    SP->_filtered = filtered;

    /* implement filter mode */
    if (filtered) {
	slines = 1;
	SET_LINES(slines);
	clear_screen = 0;
	cursor_down = parm_down_cursor = 0;
	cursor_address = 0;
	cursor_up = parm_up_cursor = 0;
	row_address = 0;

	cursor_home = carriage_return;
	T(("filter screensize %dx%d", LINES, COLS));
    }
#ifdef __DJGPP__
    T(("setting output mode to binary"));
    fflush(output);
    setmode(output, O_BINARY);
#endif
    _nc_set_buffer(output, TRUE);
    SP->_term = cur_term;
    SP->_lines = slines;
    SP->_lines_avail = slines;
    SP->_columns = scolumns;
    SP->_cursrow = -1;
    SP->_curscol = -1;
    SP->_nl = TRUE;
    SP->_raw = FALSE;
    SP->_cbreak = 0;
    SP->_echo = TRUE;
    SP->_fifohead = -1;
    SP->_endwin = TRUE;
    SP->_ofp = output;
    SP->_cursor = -1;		/* cannot know real cursor shape */

    SetNoPadding(SP);

#if NCURSES_EXT_FUNCS
    SP->_default_color = FALSE;
    SP->_has_sgr_39_49 = FALSE;

    /*
     * Set our assumption of the terminal's default foreground and background
     * colors.  The curs_color man-page states that we can assume that the
     * background is black.  The origin of this assumption appears to be
     * terminals that displayed colored text, but no colored backgrounds, e.g.,
     * the first colored terminals around 1980.  More recent ones with better
     * technology can display not only colored backgrounds, but all
     * combinations.  So a terminal might be something other than "white" on
     * black (green/black looks monochrome too), but black on white or even
     * on ivory.
     *
     * White-on-black is the simplest thing to use for monochrome.  Almost
     * all applications that use color paint both text and background, so
     * the distinction is moot.  But a few do not - which is why we leave this
     * configurable (a better solution is to use assume_default_colors() for
     * the rare applications that do require that sort of appearance, since
     * is appears that more users expect to be able to make a white-on-black
     * or black-on-white display under control of the application than not).
     */
#ifdef USE_ASSUMED_COLOR
    SP->_default_fg = COLOR_WHITE;
    SP->_default_bg = COLOR_BLACK;
#else
    SP->_default_fg = C_MASK;
    SP->_default_bg = C_MASK;
#endif

    /*
     * Allow those assumed/default color assumptions to be overridden at
     * runtime:
     */
    if ((env = getenv("NCURSES_ASSUMED_COLORS")) != 0) {
	int fg, bg;
	char sep1, sep2;
	int count = sscanf(env, "%d%c%d%c", &fg, &sep1, &bg, &sep2);
	if (count >= 1) {
	    SP->_default_fg = (fg >= 0 && fg < max_colors) ? fg : C_MASK;
	    if (count >= 3) {
		SP->_default_bg = (bg >= 0 && bg < max_colors) ? bg : C_MASK;
	    }
	    TR(TRACE_CHARPUT | TRACE_MOVE,
	       ("from environment assumed fg=%d, bg=%d",
		SP->_default_fg,
		SP->_default_bg));
	}
    }
#if USE_COLORFGBG
    /*
     * If rxvt's $COLORFGBG variable is set, use it to specify the assumed
     * default colors.  Note that rxvt (mis)uses bold colors, equating a bold
     * color to that value plus 8.  We'll only use the non-bold color for now -
     * decide later if it is worth having default attributes as well.
     */
    if (getenv("COLORFGBG") != 0) {
	char *p = getenv("COLORFGBG");
	TR(TRACE_CHARPUT | TRACE_MOVE, ("decoding COLORFGBG %s", p));
	p = extract_fgbg(p, &(SP->_default_fg));
	p = extract_fgbg(p, &(SP->_default_bg));
	if (*p)			/* assume rxvt was compiled with xpm support */
	    p = extract_fgbg(p, &(SP->_default_bg));
	TR(TRACE_CHARPUT | TRACE_MOVE, ("decoded fg=%d, bg=%d",
					SP->_default_fg, SP->_default_bg));
	if (SP->_default_fg >= max_colors) {
	    if (set_a_foreground != ABSENT_STRING
		&& !strcmp(set_a_foreground, "\033[3%p1%dm")) {
		set_a_foreground = "\033[3%?%p1%{8}%>%t9%e%p1%d%;m";
	    } else {
		SP->_default_fg %= max_colors;
	    }
	}
	if (SP->_default_bg >= max_colors) {
	    if (set_a_background != ABSENT_STRING
		&& !strcmp(set_a_background, "\033[4%p1%dm")) {
		set_a_background = "\033[4%?%p1%{8}%>%t9%e%p1%d%;m";
	    } else {
		SP->_default_bg %= max_colors;
	    }
	}
    }
#endif
#endif /* NCURSES_EXT_FUNCS */

    SP->_maxclick = DEFAULT_MAXCLICK;
    SP->_mouse_event = no_mouse_event;
    SP->_mouse_inline = no_mouse_inline;
    SP->_mouse_parse = no_mouse_parse;
    SP->_mouse_resume = no_mouse_resume;
    SP->_mouse_wrap = no_mouse_wrap;
    SP->_mouse_fd = -1;

    /*
     * If we've no magic cookie support, we suppress attributes that xmc would
     * affect, i.e., the attributes that affect the rendition of a space.
     */
    SP->_ok_attributes = termattrs();
    if (has_colors()) {
	SP->_ok_attributes |= A_COLOR;
    }
#if USE_XMC_SUPPORT
    /*
     * If we have no magic-cookie support compiled-in, or if it is suppressed
     * in the environment, reset the support-flag.
     */
    if (magic_cookie_glitch >= 0) {
	if (getenv("NCURSES_NO_MAGIC_COOKIE") != 0) {
	    support_cookies = FALSE;
	}
    }
#endif

    if (!support_cookies && magic_cookie_glitch >= 0) {
	T(("will disable attributes to work w/o magic cookies"));
    }

    if (magic_cookie_glitch > 0) {	/* tvi, wyse */

	SP->_xmc_triggers = SP->_ok_attributes & (
						     A_STANDOUT |
						     A_UNDERLINE |
						     A_REVERSE |
						     A_BLINK |
						     A_DIM |
						     A_BOLD |
						     A_INVIS |
						     A_PROTECT
	    );
#if 0
	/*
	 * We "should" treat colors as an attribute.  The wyse350 (and its
	 * clones) appear to be the only ones that have both colors and magic
	 * cookies.
	 */
	if (has_colors()) {
	    SP->_xmc_triggers |= A_COLOR;
	}
#endif
	SP->_xmc_suppress = SP->_xmc_triggers & (chtype) ~(A_BOLD);

	T(("magic cookie attributes %s", _traceattr(SP->_xmc_suppress)));
	/*
	 * Supporting line-drawing may be possible.  But make the regular
	 * video attributes work first.
	 */
	acs_chars = ABSENT_STRING;
	ena_acs = ABSENT_STRING;
	enter_alt_charset_mode = ABSENT_STRING;
	exit_alt_charset_mode = ABSENT_STRING;
#if USE_XMC_SUPPORT
	/*
	 * To keep the cookie support simple, suppress all of the optimization
	 * hooks except for clear_screen and the cursor addressing.
	 */
	if (support_cookies) {
	    clr_eol = ABSENT_STRING;
	    clr_eos = ABSENT_STRING;
	    set_attributes = ABSENT_STRING;
	}
#endif
    } else if (magic_cookie_glitch == 0) {	/* hpterm */
    }

    /*
     * If magic cookies are not supported, cancel the strings that set
     * video attributes.
     */
    if (!support_cookies && magic_cookie_glitch >= 0) {
	magic_cookie_glitch = ABSENT_NUMERIC;
	set_attributes = ABSENT_STRING;
	enter_blink_mode = ABSENT_STRING;
	enter_bold_mode = ABSENT_STRING;
	enter_dim_mode = ABSENT_STRING;
	enter_reverse_mode = ABSENT_STRING;
	enter_standout_mode = ABSENT_STRING;
	enter_underline_mode = ABSENT_STRING;
    }

    /* initialize normal acs before wide, since we use mapping in the latter */
#if !USE_WIDEC_SUPPORT
    if (_nc_unicode_locale() && _nc_locale_breaks_acs(cur_term)) {
	acs_chars = NULL;
	ena_acs = NULL;
	enter_alt_charset_mode = NULL;
	exit_alt_charset_mode = NULL;
	set_attributes = NULL;
    }
#endif
    _nc_init_acs();
#if USE_WIDEC_SUPPORT
    _nc_init_wacs();

    SP->_screen_acs_fix = (_nc_unicode_locale()
			   && _nc_locale_breaks_acs(cur_term));
#endif
    env = _nc_get_locale();
    SP->_legacy_coding = ((env == 0)
			  || !strcmp(env, "C")
			  || !strcmp(env, "POSIX"));
    T(("legacy-coding %d", SP->_legacy_coding));

    _nc_idcok = TRUE;
    _nc_idlok = FALSE;

    SP->oldhash = 0;
    SP->newhash = 0;

    T(("creating newscr"));
    if ((SP->_newscr = newwin(slines, scolumns, 0, 0)) == 0)
	returnCode(ERR);

    T(("creating curscr"));
    if ((SP->_curscr = newwin(slines, scolumns, 0, 0)) == 0)
	returnCode(ERR);

#if !USE_REENTRANT
    newscr = SP->_newscr;
    curscr = SP->_curscr;
#endif
#if USE_SIZECHANGE
    SP->_resize = resizeterm;
#endif

    newscr->_clear = TRUE;
    curscr->_clear = FALSE;

    def_shell_mode();
    def_prog_mode();

    for (rop = ripoff_stack;
	 rop != ripoff_sp && (rop - ripoff_stack) < N_RIPS;
	 rop++) {

	/* If we must simulate soft labels, grab off the line to be used.
	   We assume that we must simulate, if it is none of the standard
	   formats (4-4 or 3-2-3) for which there may be some hardware
	   support. */
	if (rop->hook == _nc_slk_initialize)
	    if (!(num_labels <= 0 || !SLK_STDFMT(slk_format)))
		continue;
	if (rop->hook) {
	    int count;
	    WINDOW *w;

	    count = (rop->line < 0) ? -rop->line : rop->line;
	    T(("ripping off %i lines at %s", count,
	       ((rop->line < 0)
		? "bottom"
		: "top")));

	    w = newwin(count, scolumns,
		       ((rop->line < 0)
			? SP->_lines_avail - count
			: 0),
		       0);
	    if (w) {
		rop->win = w;
		rop->hook(w, scolumns);
	    } else {
		returnCode(ERR);
	    }
	    if (rop->line < 0)
		bottom_stolen += count;
	    else
		SP->_topstolen += count;
	    SP->_lines_avail -= count;
	}
    }
    /* reset the stack */
    ripoff_sp = ripoff_stack;

    T(("creating stdscr"));
    assert((SP->_lines_avail + SP->_topstolen + bottom_stolen) == slines);
    if ((SP->_stdscr = newwin(SP->_lines_avail, scolumns, 0, 0)) == 0)
	returnCode(ERR);

    SET_LINES(SP->_lines_avail);
#if !USE_REENTRANT
    stdscr = SP->_stdscr;
#endif

    returnCode(OK);
}

/*
 * The internal implementation interprets line as the number of lines to rip
 * off from the top or bottom.
 */
NCURSES_EXPORT(int)
_nc_ripoffline(int line, int (*init) (WINDOW *, int))
{
    T((T_CALLED("_nc_ripoffline(%d, %p)"), line, init));

    if (line != 0) {

	if (ripoff_sp == 0)
	    ripoff_sp = ripoff_stack;
	if (ripoff_sp >= ripoff_stack + N_RIPS)
	    returnCode(ERR);

	ripoff_sp->line = line;
	ripoff_sp->hook = init;
	ripoff_sp++;
    }

    returnCode(OK);
}

NCURSES_EXPORT(int)
ripoffline(int line, int (*init) (WINDOW *, int))
{
    START_TRACE();
    T((T_CALLED("ripoffline(%d,%p)"), line, init));

    if (line == 0)
	returnCode(OK);

    returnCode(_nc_ripoffline((line < 0) ? -1 : 1, init));
}

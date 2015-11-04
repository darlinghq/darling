/****************************************************************************
 * Copyright (c) 1998-2006,2007 Free Software Foundation, Inc.              *
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

/* lib_color.c
 *
 * Handles color emulation of SYS V curses
 */

#include <curses.priv.h>

#include <term.h>
#include <tic.h>

MODULE_ID("$Id: lib_color.c,v 1.85 2007/04/07 17:07:28 tom Exp $")

/*
 * These should be screen structure members.  They need to be globals for
 * historical reasons.  So we assign them in start_color() and also in
 * set_term()'s screen-switching logic.
 */
#if USE_REENTRANT
NCURSES_EXPORT(int)
NCURSES_PUBLIC_VAR(COLOR_PAIRS) (void)
{
    return SP ? SP->_pair_count : -1;
}
NCURSES_EXPORT(int)
NCURSES_PUBLIC_VAR(COLORS) (void)
{
    return SP ? SP->_color_count : -1;
}
#else
NCURSES_EXPORT_VAR(int) COLOR_PAIRS = 0;
NCURSES_EXPORT_VAR(int) COLORS = 0;
#endif

#define DATA(r,g,b) {r,g,b, 0,0,0, 0}

#define TYPE_CALLOC(type,elts) typeCalloc(type, (unsigned)(elts))

#define MAX_PALETTE	8

#define OkColorHi(n)	(((n) < COLORS) && ((n) < max_colors))
#define InPalette(n)	((n) >= 0 && (n) < MAX_PALETTE)

/*
 * Given a RGB range of 0..1000, we'll normally set the individual values
 * to about 2/3 of the maximum, leaving full-range for bold/bright colors.
 */
#define RGB_ON  680
#define RGB_OFF 0
/* *INDENT-OFF* */
static const color_t cga_palette[] =
{
    /*  R               G               B */
    DATA(RGB_OFF,	RGB_OFF,	RGB_OFF),	/* COLOR_BLACK */
    DATA(RGB_ON,	RGB_OFF,	RGB_OFF),	/* COLOR_RED */
    DATA(RGB_OFF,	RGB_ON,		RGB_OFF),	/* COLOR_GREEN */
    DATA(RGB_ON,	RGB_ON,		RGB_OFF),	/* COLOR_YELLOW */
    DATA(RGB_OFF,	RGB_OFF,	RGB_ON),	/* COLOR_BLUE */
    DATA(RGB_ON,	RGB_OFF,	RGB_ON),	/* COLOR_MAGENTA */
    DATA(RGB_OFF,	RGB_ON,		RGB_ON),	/* COLOR_CYAN */
    DATA(RGB_ON,	RGB_ON,		RGB_ON),	/* COLOR_WHITE */
};

static const color_t hls_palette[] =
{
    /*  	H       L       S */
    DATA(	0,	0,	0),		/* COLOR_BLACK */
    DATA(	120,	50,	100),		/* COLOR_RED */
    DATA(	240,	50,	100),		/* COLOR_GREEN */
    DATA(	180,	50,	100),		/* COLOR_YELLOW */
    DATA(	330,	50,	100),		/* COLOR_BLUE */
    DATA(	60,	50,	100),		/* COLOR_MAGENTA */
    DATA(	300,	50,	100),		/* COLOR_CYAN */
    DATA(	0,	50,	100),		/* COLOR_WHITE */
};
/* *INDENT-ON* */

#if NCURSES_EXT_FUNCS
/*
 * These are called from _nc_do_color(), which in turn is called from
 * vidattr - so we have to assume that SP may be null.
 */
static int
default_fg(void)
{
    return (SP != 0) ? SP->_default_fg : COLOR_WHITE;
}

static int
default_bg(void)
{
    return SP != 0 ? SP->_default_bg : COLOR_BLACK;
}
#else
#define default_fg() COLOR_WHITE
#define default_bg() COLOR_BLACK
#endif

/*
 * SVr4 curses is known to interchange color codes (1,4) and (3,6), possibly
 * to maintain compatibility with a pre-ANSI scheme.  The same scheme is
 * also used in the FreeBSD syscons.
 */
static int
toggled_colors(int c)
{
    if (c < 16) {
	static const int table[] =
	{0, 4, 2, 6, 1, 5, 3, 7,
	 8, 12, 10, 14, 9, 13, 11, 15};
	c = table[c];
    }
    return c;
}

static void
set_background_color(int bg, int (*outc) (int))
{
    if (set_a_background) {
	TPUTS_TRACE("set_a_background");
	tputs(TPARM_1(set_a_background, bg), 1, outc);
    } else {
	TPUTS_TRACE("set_background");
	tputs(TPARM_1(set_background, toggled_colors(bg)), 1, outc);
    }
}

static void
set_foreground_color(int fg, int (*outc) (int))
{
    if (set_a_foreground) {
	TPUTS_TRACE("set_a_foreground");
	tputs(TPARM_1(set_a_foreground, fg), 1, outc);
    } else {
	TPUTS_TRACE("set_foreground");
	tputs(TPARM_1(set_foreground, toggled_colors(fg)), 1, outc);
    }
}

static void
init_color_table(void)
{
    const color_t *tp;
    int n;

    tp = (hue_lightness_saturation) ? hls_palette : cga_palette;
    for (n = 0; n < COLORS; n++) {
	if (InPalette(n)) {
	    SP->_color_table[n] = tp[n];
	} else {
	    SP->_color_table[n] = tp[n % MAX_PALETTE];
	    if (hue_lightness_saturation) {
		SP->_color_table[n].green = 100;
	    } else {
		if (SP->_color_table[n].red)
		    SP->_color_table[n].red = 1000;
		if (SP->_color_table[n].green)
		    SP->_color_table[n].green = 1000;
		if (SP->_color_table[n].blue)
		    SP->_color_table[n].blue = 1000;
	    }
	}
    }
}

/*
 * Reset the color pair, e.g., to whatever color pair 0 is.
 */
static bool
reset_color_pair(void)
{
    bool result = FALSE;

    if (orig_pair != 0) {
	TPUTS_TRACE("orig_pair");
	putp(orig_pair);
	result = TRUE;
    }
    return result;
}

/*
 * Reset color pairs and definitions.  Actually we do both more to accommodate
 * badly-written terminal descriptions than for the relatively rare case where
 * someone has changed the color definitions.
 */
bool
_nc_reset_colors(void)
{
    int result = FALSE;

    T((T_CALLED("_nc_reset_colors()")));
    if (SP->_color_defs > 0)
	SP->_color_defs = -(SP->_color_defs);

    if (reset_color_pair())
	result = TRUE;
    if (orig_colors != 0) {
	TPUTS_TRACE("orig_colors");
	putp(orig_colors);
	result = TRUE;
    }
    returnBool(result);
}

NCURSES_EXPORT(int)
start_color(void)
{
    int result = ERR;

    T((T_CALLED("start_color()")));

    if (SP == 0) {
	result = ERR;
    } else if (SP->_coloron) {
	result = OK;
    } else {

	if (reset_color_pair() != TRUE) {
	    set_foreground_color(default_fg(), _nc_outch);
	    set_background_color(default_bg(), _nc_outch);
	}

	if (max_pairs > 0 && max_colors > 0) {
	    SP->_pair_count = max_pairs;
	    SP->_color_count = max_colors;
#if !USE_REENTRANT
	    COLOR_PAIRS = max_pairs;
	    COLORS = max_colors;
#endif

	    if ((SP->_color_pairs = TYPE_CALLOC(colorpair_t,
						max_pairs)) != 0) {
		if ((SP->_color_table = TYPE_CALLOC(color_t,
						    max_colors)) != 0) {
		    SP->_color_pairs[0] = PAIR_OF(default_fg(), default_bg());
		    init_color_table();

		    T(("started color: COLORS = %d, COLOR_PAIRS = %d",
		       COLORS, COLOR_PAIRS));

		    SP->_coloron = 1;
		    result = OK;
		} else if (SP->_color_pairs != 0) {
		    FreeAndNull(SP->_color_pairs);
		}
	    }
	} else {
	    result = OK;
	}
    }
    returnCode(result);
}

/* This function was originally written by Daniel Weaver <danw@znyx.com> */
static void
rgb2hls(short r, short g, short b, short *h, short *l, short *s)
/* convert RGB to HLS system */
{
    short min, max, t;

    if ((min = g < r ? g : r) > b)
	min = b;
    if ((max = g > r ? g : r) < b)
	max = b;

    /* calculate lightness */
    *l = (min + max) / 20;

    if (min == max) {		/* black, white and all shades of gray */
	*h = 0;
	*s = 0;
	return;
    }

    /* calculate saturation */
    if (*l < 50)
	*s = ((max - min) * 100) / (max + min);
    else
	*s = ((max - min) * 100) / (2000 - max - min);

    /* calculate hue */
    if (r == max)
	t = 120 + ((g - b) * 60) / (max - min);
    else if (g == max)
	t = 240 + ((b - r) * 60) / (max - min);
    else
	t = 360 + ((r - g) * 60) / (max - min);

    *h = t % 360;
}

/*
 * Extension (1997/1/18) - Allow negative f/b values to set default color
 * values.
 */
NCURSES_EXPORT(int)
init_pair(short pair, short f, short b)
{
    colorpair_t result;

    T((T_CALLED("init_pair(%d,%d,%d)"), pair, f, b));

    if ((pair < 0) || (pair >= COLOR_PAIRS) || SP == 0 || !SP->_coloron)
	returnCode(ERR);
#if NCURSES_EXT_FUNCS
    if (SP->_default_color) {
	if (f < 0)
	    f = COLOR_DEFAULT;
	if (b < 0)
	    b = COLOR_DEFAULT;
	if (!OkColorHi(f) && !isDefaultColor(f))
	    returnCode(ERR);
	if (!OkColorHi(b) && !isDefaultColor(b))
	    returnCode(ERR);
    } else
#endif
    {
	if ((f < 0) || !OkColorHi(f)
	    || (b < 0) || !OkColorHi(b)
	    || (pair < 1))
	    returnCode(ERR);
    }

    /*
     * When a pair's content is changed, replace its colors (if pair was
     * initialized before a screen update is performed replacing original
     * pair colors with the new ones).
     */
    result = PAIR_OF(f, b);
    if (SP->_color_pairs[pair] != 0
	&& SP->_color_pairs[pair] != result) {
	int y, x;

	for (y = 0; y <= curscr->_maxy; y++) {
	    struct ldat *ptr = &(curscr->_line[y]);
	    bool changed = FALSE;
	    for (x = 0; x <= curscr->_maxx; x++) {
		if (GetPair(ptr->text[x]) == pair) {
		    /* Set the old cell to zero to ensure it will be
		       updated on the next doupdate() */
		    SetChar(ptr->text[x], 0, 0);
		    CHANGED_CELL(ptr, x);
		    changed = TRUE;
		}
	    }
	    if (changed)
		_nc_make_oldhash(y);
	}
    }
    SP->_color_pairs[pair] = result;
    if (GET_SCREEN_PAIR(SP) == pair)
	SET_SCREEN_PAIR(SP, (chtype) (~0));	/* force attribute update */

    if (initialize_pair && InPalette(f) && InPalette(b)) {
	const color_t *tp = hue_lightness_saturation ? hls_palette : cga_palette;

	TR(TRACE_ATTRS,
	   ("initializing pair: pair = %d, fg=(%d,%d,%d), bg=(%d,%d,%d)",
	    pair,
	    tp[f].red, tp[f].green, tp[f].blue,
	    tp[b].red, tp[b].green, tp[b].blue));

	TPUTS_TRACE("initialize_pair");
	putp(TPARM_7(initialize_pair,
		     pair,
		     tp[f].red, tp[f].green, tp[f].blue,
		     tp[b].red, tp[b].green, tp[b].blue));
    }

    returnCode(OK);
}

#define okRGB(n) ((n) >= 0 && (n) <= 1000)

NCURSES_EXPORT(int)
init_color(short color, short r, short g, short b)
{
    int result = ERR;

    T((T_CALLED("init_color(%d,%d,%d,%d)"), color, r, g, b));

    if (initialize_color != NULL
	&& SP != 0
	&& SP->_coloron
	&& (color >= 0 && OkColorHi(color))
	&& (okRGB(r) && okRGB(g) && okRGB(b))) {

	SP->_color_table[color].init = 1;
	SP->_color_table[color].r = r;
	SP->_color_table[color].g = g;
	SP->_color_table[color].b = b;

	if (hue_lightness_saturation) {
	    rgb2hls(r, g, b,
		    &SP->_color_table[color].red,
		    &SP->_color_table[color].green,
		    &SP->_color_table[color].blue);
	} else {
	    SP->_color_table[color].red = r;
	    SP->_color_table[color].green = g;
	    SP->_color_table[color].blue = b;
	}

	TPUTS_TRACE("initialize_color");
	putp(TPARM_4(initialize_color, color, r, g, b));
	SP->_color_defs = max(color + 1, SP->_color_defs);
	result = OK;
    }
    returnCode(result);
}

NCURSES_EXPORT(bool)
can_change_color(void)
{
    T((T_CALLED("can_change_color()")));
    returnCode((can_change != 0) ? TRUE : FALSE);
}

NCURSES_EXPORT(bool)
has_colors(void)
{
    T((T_CALLED("has_colors()")));
    returnCode((VALID_NUMERIC(max_colors) && VALID_NUMERIC(max_pairs)
		&& (((set_foreground != NULL)
		     && (set_background != NULL))
		    || ((set_a_foreground != NULL)
			&& (set_a_background != NULL))
		    || set_color_pair)) ? TRUE : FALSE);
}

NCURSES_EXPORT(int)
color_content(short color, short *r, short *g, short *b)
{
    int result;

    T((T_CALLED("color_content(%d,%p,%p,%p)"), color, r, g, b));
    if (color < 0 || !OkColorHi(color) || SP == 0 || !SP->_coloron) {
	result = ERR;
    } else {
	NCURSES_COLOR_T c_r = SP->_color_table[color].red;
	NCURSES_COLOR_T c_g = SP->_color_table[color].green;
	NCURSES_COLOR_T c_b = SP->_color_table[color].blue;

	if (r)
	    *r = c_r;
	if (g)
	    *g = c_g;
	if (b)
	    *b = c_b;

	TR(TRACE_ATTRS, ("...color_content(%d,%d,%d,%d)",
			 color, c_r, c_g, c_b));
	result = OK;
    }
    returnCode(result);
}

NCURSES_EXPORT(int)
pair_content(short pair, short *f, short *b)
{
    int result;

    T((T_CALLED("pair_content(%d,%p,%p)"), pair, f, b));

    if ((pair < 0) || (pair >= COLOR_PAIRS) || SP == 0 || !SP->_coloron) {
	result = ERR;
    } else {
	NCURSES_COLOR_T fg = ((SP->_color_pairs[pair] >> C_SHIFT) & C_MASK);
	NCURSES_COLOR_T bg = (SP->_color_pairs[pair] & C_MASK);

#if NCURSES_EXT_FUNCS
	if (fg == COLOR_DEFAULT)
	    fg = -1;
	if (bg == COLOR_DEFAULT)
	    bg = -1;
#endif

	if (f)
	    *f = fg;
	if (b)
	    *b = bg;

	TR(TRACE_ATTRS, ("...pair_content(%d,%d,%d)", pair, fg, bg));
	result = OK;
    }
    returnCode(result);
}

NCURSES_EXPORT(void)
_nc_do_color(short old_pair, short pair, bool reverse, int (*outc) (int))
{
    NCURSES_COLOR_T fg = COLOR_DEFAULT;
    NCURSES_COLOR_T bg = COLOR_DEFAULT;
    NCURSES_COLOR_T old_fg, old_bg;

    if (pair < 0 || pair >= COLOR_PAIRS) {
	return;
    } else if (pair != 0) {
	if (set_color_pair) {
	    TPUTS_TRACE("set_color_pair");
	    tputs(TPARM_1(set_color_pair, pair), 1, outc);
	    return;
	} else if (SP != 0) {
	    pair_content((short) pair, &fg, &bg);
	}
    }

    if (old_pair >= 0
	&& SP != 0
	&& pair_content(old_pair, &old_fg, &old_bg) != ERR) {
	if ((isDefaultColor(fg) && !isDefaultColor(old_fg))
	    || (isDefaultColor(bg) && !isDefaultColor(old_bg))) {
#if NCURSES_EXT_FUNCS
	    /*
	     * A minor optimization - but extension.  If "AX" is specified in
	     * the terminal description, treat it as screen's indicator of ECMA
	     * SGR 39 and SGR 49, and assume the two sequences are independent.
	     */
	    if (SP->_has_sgr_39_49
		&& isDefaultColor(old_bg)
		&& !isDefaultColor(old_fg)) {
		tputs("\033[39m", 1, outc);
	    } else if (SP->_has_sgr_39_49
		       && isDefaultColor(old_fg)
		       && !isDefaultColor(old_bg)) {
		tputs("\033[49m", 1, outc);
	    } else
#endif
		reset_color_pair();
	}
    } else {
	reset_color_pair();
	if (old_pair < 0)
	    return;
    }

#if NCURSES_EXT_FUNCS
    if (isDefaultColor(fg))
	fg = default_fg();
    if (isDefaultColor(bg))
	bg = default_bg();
#endif

    if (reverse) {
	NCURSES_COLOR_T xx = fg;
	fg = bg;
	bg = xx;
    }

    TR(TRACE_ATTRS, ("setting colors: pair = %d, fg = %d, bg = %d", pair,
		     fg, bg));

    if (!isDefaultColor(fg)) {
	set_foreground_color(fg, outc);
    }
    if (!isDefaultColor(bg)) {
	set_background_color(bg, outc);
    }
}

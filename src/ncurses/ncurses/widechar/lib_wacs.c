/****************************************************************************
 * Copyright (c) 2002,2006 Free Software Foundation, Inc.                   *
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
 *  Author: Thomas Dickey 2002                                              *
 ****************************************************************************/

#include <curses.priv.h>
#include <term.h>

MODULE_ID("$Id: lib_wacs.c,v 1.7 2006/12/17 15:16:17 tom Exp $")

NCURSES_EXPORT_VAR(cchar_t) * _nc_wacs = 0;

NCURSES_EXPORT(void)
_nc_init_wacs(void)
{
    /* *INDENT-OFF* */
    static const struct {
	int	map;
	int	value[2];
    } table[] = {
	/* VT100 symbols */
	{ 'l',	{ '+',	0x250c }},	/* upper left corner */
	{ 'm',	{ '+',	0x2514 }},	/* lower left corner */
	{ 'k',	{ '+',	0x2510 }},	/* upper right corner */
	{ 'j',	{ '+',	0x2518 }},	/* lower right corner */
	{ 't',	{ '+',	0x251c }},	/* tee pointing left */
	{ 'u',	{ '+',	0x2524 }},	/* tee pointing right */
	{ 'v',	{ '+',	0x2534 }},	/* tee pointing up */
	{ 'w',	{ '+',	0x252c }},	/* tee pointing down */
	{ 'q',	{ '-',	0x2500 }},	/* horizontal line */
	{ 'x',	{ '|',	0x2502 }},	/* vertical line */
	{ 'n',	{ '+',	0x253c }},	/* large plus or crossover */
	{ 'o',	{ '~',	0x23ba }},	/* scan line 1 */
	{ 's',	{ '_',	0x23bd }},	/* scan line 9 */
	{ '`',	{ '+',	0x25c6 }},	/* diamond */
	{ 'a',	{ ':',	0x2592 }},	/* checker board (stipple) */
	{ 'f',	{ '\'',	0x00b0 }},	/* degree symbol */
	{ 'g',	{ '#',	0x00b1 }},	/* plus/minus */
	{ '~',	{ 'o',	0x00b7 }},	/* bullet */
	/* Teletype 5410v1 symbols */
	{ ',',	{ '<',	0x2190 }},	/* arrow pointing left */
	{ '+',	{ '>',	0x2192 }},	/* arrow pointing right */
	{ '.',	{ 'v',	0x2193 }},	/* arrow pointing down */
	{ '-',	{ '^',	0x2191 }},	/* arrow pointing up */
	{ 'h',	{ '#',	0x2592 }},	/* board of squares */
	{ 'i',	{ '#',	0x2603 }},	/* lantern symbol */
	{ '0',	{ '#',	0x25ae }},	/* solid square block */
	/* these defaults were invented for ncurses */
	{ 'p',	{ '-',	0x23bb }},	/* scan line 3 */
	{ 'r',	{ '-',	0x23bc }},	/* scan line 7 */
	{ 'y',	{ '<',	0x2264 }},	/* less-than-or-equal-to */
	{ 'z',	{ '>',	0x2265 }},	/* greater-than-or-equal-to */
	{ '{',	{ '*',	0x03c0 }},	/* greek pi */
	{ '|',	{ '!',	0x2260 }},	/* not-equal */
	{ '}',	{ 'f',	0x00a3 }},	/* pound-sterling symbol */
    };
    /* *INDENT-ON* */

    unsigned n, m;
    int active = _nc_unicode_locale();

    /*
     * If we're running in a UTF-8 locale, will use the Unicode equivalents
     * rather than the terminfo information.  Actually the terminfo should
     * be the rule, but there are people who are offended by the notion that
     * a Unicode-capable terminal would have something resembling a mode.
     * So the smacs/rmacs may be disabled -- sometime.
     */
    T(("initializing WIDE-ACS map (Unicode is%s active)",
       active ? "" : " not"));

    _nc_wacs = typeCalloc(cchar_t, ACS_LEN);
    for (n = 0; n < SIZEOF(table); ++n) {
	int wide = wcwidth(table[n].value[active]);

	m = table[n].map;
	if (active && (wide == 1)) {
	    SetChar(_nc_wacs[m], table[n].value[active], A_NORMAL);
	} else if (acs_map[m] & A_ALTCHARSET) {
	    SetChar(_nc_wacs[m], m, A_ALTCHARSET);
	} else {
	    SetChar(_nc_wacs[m], table[n].value[0], A_NORMAL);
	}

	T(("#%d, SetChar(%c, %#04x) = %s",
	   n, m,
	   table[n].value[active],
	   _tracecchar_t(&_nc_wacs[m])));
    }
}

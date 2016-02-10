/****************************************************************************
 * Copyright (c) 1998-2005,2008 Free Software Foundation, Inc.              *
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
 *  Authors:                                                                *
 *          Gerhard Fuernkranz                      1993 (original)         *
 *          Zeyd M. Ben-Halim                       1992,1995 (sic)         *
 *          Eric S. Raymond                                                 *
 *          Juergen Pfeifer                         1996-on                 *
 *          Thomas E. Dickey                                                *
 ****************************************************************************/

/*
 *	lib_slk.c
 *	Soft key routines.
 */

#include <curses.priv.h>

#include <ctype.h>
#include <term.h>		/* num_labels, label_*, plab_norm */

MODULE_ID("$Id: lib_slk.c,v 1.35 2008/09/27 14:07:33 juergen Exp $")

/*
 * Free any memory related to soft labels, return an error.
 */
static int
slk_failed(void)
{
    if (SP->_slk) {
	FreeIfNeeded(SP->_slk->ent);
	free(SP->_slk);
	SP->_slk = (SLK *) 0;
    }
    return ERR;
}

/*
 * Initialize soft labels.
 * Called from newterm()
 */
NCURSES_EXPORT(int)
_nc_slk_initialize(WINDOW *stwin, int cols)
{
    int i, x;
    int res = OK;
    unsigned max_length;

    T((T_CALLED("_nc_slk_initialize()")));

    if (SP->_slk) {		/* we did this already, so simply return */
	returnCode(OK);
    } else if ((SP->_slk = typeCalloc(SLK, 1)) == 0)
	returnCode(ERR);

    SP->_slk->ent = NULL;

    /*
     * If we use colors, vidputs() will suppress video attributes that conflict
     * with colors.  In that case, we're still guaranteed that "reverse" would
     * work.
     */
    if ((no_color_video & 1) == 0)
	SetAttr(SP->_slk->attr, A_STANDOUT);
    else
	SetAttr(SP->_slk->attr, A_REVERSE);

    SP->_slk->maxlab = ((num_labels > 0)
			? num_labels
			: MAX_SKEY(_nc_globals.slk_format));
    SP->_slk->maxlen = ((num_labels > 0)
			? label_width * label_height
			: MAX_SKEY_LEN(_nc_globals.slk_format));
    SP->_slk->labcnt = ((SP->_slk->maxlab < MAX_SKEY(_nc_globals.slk_format))
			? MAX_SKEY(_nc_globals.slk_format)
			: SP->_slk->maxlab);

    if (SP->_slk->maxlen <= 0
	|| SP->_slk->labcnt <= 0
	|| (SP->_slk->ent = typeCalloc(slk_ent,
				       (unsigned) SP->_slk->labcnt)) == NULL)
	returnCode(slk_failed());

    max_length = SP->_slk->maxlen;
    for (i = 0; i < SP->_slk->labcnt; i++) {
	size_t used = max_length + 1;

	if ((SP->_slk->ent[i].ent_text = (char *) _nc_doalloc(0, used)) == 0)
	    returnCode(slk_failed());
	memset(SP->_slk->ent[i].ent_text, 0, used);

	if ((SP->_slk->ent[i].form_text = (char *) _nc_doalloc(0, used)) == 0)
	    returnCode(slk_failed());
	memset(SP->_slk->ent[i].form_text, 0, used);

	memset(SP->_slk->ent[i].form_text, ' ', max_length);
	SP->_slk->ent[i].visible = (char) (i < SP->_slk->maxlab);
    }
    if (_nc_globals.slk_format >= 3) {	/* PC style */
	int gap = (cols - 3 * (3 + 4 * max_length)) / 2;

	if (gap < 1)
	    gap = 1;

	for (i = x = 0; i < SP->_slk->maxlab; i++) {
	    SP->_slk->ent[i].ent_x = x;
	    x += max_length;
	    x += (i == 3 || i == 7) ? gap : 1;
	}
    } else {
	if (_nc_globals.slk_format == 2) {	/* 4-4 */
	    int gap = cols - (SP->_slk->maxlab * max_length) - 6;

	    if (gap < 1)
		gap = 1;
	    for (i = x = 0; i < SP->_slk->maxlab; i++) {
		SP->_slk->ent[i].ent_x = x;
		x += max_length;
		x += (i == 3) ? gap : 1;
	    }
	} else {
	    if (_nc_globals.slk_format == 1) {	/* 1 -> 3-2-3 */
		int gap = (cols - (SP->_slk->maxlab * max_length) - 5)
		/ 2;

		if (gap < 1)
		    gap = 1;
		for (i = x = 0; i < SP->_slk->maxlab; i++) {
		    SP->_slk->ent[i].ent_x = x;
		    x += max_length;
		    x += (i == 2 || i == 4) ? gap : 1;
		}
	    } else
		returnCode(slk_failed());
	}
    }
    SP->_slk->dirty = TRUE;
    if ((SP->_slk->win = stwin) == NULL) {
	returnCode(slk_failed());
    }

    /* We now reset the format so that the next newterm has again
     * per default no SLK keys and may call slk_init again to
     * define a new layout. (juergen 03-Mar-1999)
     */
    SP->slk_format = _nc_globals.slk_format;
    _nc_globals.slk_format = 0;
    returnCode(res);
}

/*
 * Restore the soft labels on the screen.
 */
NCURSES_EXPORT(int)
slk_restore(void)
{
    T((T_CALLED("slk_restore()")));

    if (SP->_slk == NULL)
	return (ERR);
    SP->_slk->hidden = FALSE;
    SP->_slk->dirty = TRUE;

    returnCode(slk_refresh());
}

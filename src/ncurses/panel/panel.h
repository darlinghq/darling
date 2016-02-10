/****************************************************************************
 * Copyright (c) 1998-2001,2006 Free Software Foundation, Inc.              *
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
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1995                    *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 ****************************************************************************/

/* $Id: panel.h,v 1.10 2006/05/27 19:49:40 tom Exp $ */

/* panel.h -- interface file for panels library */

#ifndef NCURSES_PANEL_H_incl
#define NCURSES_PANEL_H_incl 1

#include <curses.h>

typedef struct panel
{
  WINDOW *win;
  struct panel *below;
  struct panel *above;
  NCURSES_CONST void *user;
} PANEL;

#if	defined(__cplusplus)
extern "C" {
#endif

extern NCURSES_EXPORT(WINDOW*) panel_window (const PANEL *);
extern NCURSES_EXPORT(void)    update_panels (void);
extern NCURSES_EXPORT(int)     hide_panel (PANEL *);
extern NCURSES_EXPORT(int)     show_panel (PANEL *);
extern NCURSES_EXPORT(int)     del_panel (PANEL *);
extern NCURSES_EXPORT(int)     top_panel (PANEL *);
extern NCURSES_EXPORT(int)     bottom_panel (PANEL *);
extern NCURSES_EXPORT(PANEL*)  new_panel (WINDOW *);
extern NCURSES_EXPORT(PANEL*)  panel_above (const PANEL *);
extern NCURSES_EXPORT(PANEL*)  panel_below (const PANEL *);
extern NCURSES_EXPORT(int)     set_panel_userptr (PANEL *, NCURSES_CONST void *);
extern NCURSES_EXPORT(NCURSES_CONST void*) panel_userptr (const PANEL *);
extern NCURSES_EXPORT(int)     move_panel (PANEL *, int, int);
extern NCURSES_EXPORT(int)     replace_panel (PANEL *,WINDOW *);
extern NCURSES_EXPORT(int)     panel_hidden (const PANEL *);

#if	defined(__cplusplus)
}
#endif

#endif /* NCURSES_PANEL_H_incl */

/* end of panel.h */

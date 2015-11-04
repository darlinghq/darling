/****************************************************************************
 * Copyright (c) 2007 Free Software Foundation, Inc.                        *
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
/*
 * $Id: test_get_wstr.c,v 1.5 2007/08/11 17:01:43 tom Exp $
 *
 * Author: Thomas E Dickey
 *
 * Demonstrate the get_wstr functions from the curses library.

       int get_wstr(wint_t *wstr);
       int getn_wstr(wint_t *wstr, int n);
       int wget_wstr(WINDOW *win, wint_t *wstr);
       int wgetn_wstr(WINDOW *win, wint_t *wstr, int n);
       int mvget_wstr(int y, int x, wint_t *wstr);
       int mvgetn_wstr(int y, int x, wint_t *wstr, int n);
       int mvwget_wstr(WINDOW *win, int y, int x, wint_t *wstr);
       int mvwgetn_wstr(WINDOW *win, int y, int x, wint_t *wstr, int n);
 */

#include <test.priv.h>

#if USE_WIDEC_SUPPORT

#define BASE_Y 6
#define MAX_COLS 1024

typedef enum {
    eGetStr = 0,
    eGetNStr,
    eMvGetStr,
    eMvGetNStr,
    eMaxFlavor
} Flavors;

static bool
Quit(int ch)
{
    return (ch == ERR || ch == 'q' || ch == QUIT || ch == ESCAPE);
}

static int
Remainder(WINDOW *txtwin)
{
    int result = getmaxx(txtwin) - getcurx(txtwin);
    return (result > 0) ? result : 0;
}

/*
 * Show a highlighted line in the place where input will happen.
 */
static void
ShowPrompt(WINDOW *txtwin, int limit)
{
    wchgat(txtwin, limit, A_REVERSE, 0, NULL);
    wnoutrefresh(txtwin);
}

static void
MovePrompt(WINDOW *txtwin, int limit, int y, int x)
{
    wchgat(txtwin, Remainder(txtwin), A_NORMAL, 0, NULL);
    wmove(txtwin, y, x);
    ShowPrompt(txtwin, limit);
}

static int
ShowFlavor(WINDOW *strwin, WINDOW *txtwin, int flavor, int limit)
{
    const char *name = "?";
    bool limited = FALSE;
    bool wins = (txtwin != stdscr);
    int result;

    switch (flavor) {
    case eGetStr:
	name = wins ? "wget_wstr" : "get_wstr";
	break;
    case eGetNStr:
	limited = TRUE;
	name = wins ? "wgetn_wstr" : "getn_wstr";
	break;
    case eMvGetStr:
	name = wins ? "mvwget_wstr" : "mvget_wstr";
	break;
    case eMvGetNStr:
	limited = TRUE;
	name = wins ? "mvwgetn_wstr" : "mvgetn_wstr";
	break;
    case eMaxFlavor:
	break;
    }

    wmove(strwin, 0, 0);
    werase(strwin);

    if (limited) {
	wprintw(strwin, "%s(%d):", name, limit);
    } else {
	wprintw(strwin, "%s:", name);
    }
    result = limited ? limit : Remainder(txtwin);
    ShowPrompt(txtwin, result);

    wnoutrefresh(strwin);
    return result;
}

static int
test_get_wstr(int level, char **argv, WINDOW *strwin)
{
    WINDOW *txtbox = 0;
    WINDOW *txtwin = 0;
    FILE *fp;
    int ch;
    int rc;
    int txt_x = 0, txt_y = 0;
    int base_y;
    int flavor = 0;
    int limit = getmaxx(strwin) - 5;
    int actual;
    wint_t buffer[MAX_COLS];

    if (argv[level] == 0) {
	beep();
	return FALSE;
    }

    if (level > 1) {
	txtbox = newwin(LINES - BASE_Y, COLS - level, BASE_Y, level);
	box(txtbox, 0, 0);
	wnoutrefresh(txtbox);

	txtwin = derwin(txtbox,
			getmaxy(txtbox) - 2,
			getmaxx(txtbox) - 2,
			1, 1);
	base_y = 0;
    } else {
	txtwin = stdscr;
	base_y = BASE_Y;
    }

    keypad(txtwin, TRUE);	/* enable keyboard mapping */
    (void) cbreak();		/* take input chars one at a time, no wait for \n */
    (void) noecho();		/* don't echo input */

    txt_y = base_y;
    txt_x = 0;
    wmove(txtwin, txt_y, txt_x);

    if ((fp = fopen(argv[level], "r")) != 0) {
	while ((ch = fgetc(fp)) != EOF) {
	    if (waddch(txtwin, UChar(ch)) != OK) {
		break;
	    }
	}
	fclose(fp);
    } else {
	wprintw(txtwin, "Cannot open:\n%s", argv[1]);
    }

    wmove(txtwin, txt_y, txt_x);
    actual = ShowFlavor(strwin, txtwin, flavor, limit);
    while (!Quit(ch = mvwgetch(txtwin, txt_y, txt_x))) {
	switch (ch) {
	case KEY_DOWN:
	case 'j':
	    if (txt_y < getmaxy(txtwin) - 1) {
		MovePrompt(txtwin, actual, ++txt_y, txt_x);
	    } else {
		beep();
	    }
	    break;
	case KEY_UP:
	case 'k':
	    if (txt_y > base_y) {
		MovePrompt(txtwin, actual, --txt_y, txt_x);
	    } else {
		beep();
	    }
	    break;
	case KEY_LEFT:
	case 'h':
	    if (txt_x > 0) {
		MovePrompt(txtwin, actual, txt_y, --txt_x);
	    } else {
		beep();
	    }
	    break;
	case KEY_RIGHT:
	case 'l':
	    if (txt_x < getmaxx(txtwin) - 1) {
		MovePrompt(txtwin, actual, txt_y, ++txt_x);
	    } else {
		beep();
	    }
	    break;

	case 'w':
	    test_get_wstr(level + 1, argv, strwin);
	    if (txtbox != 0) {
		touchwin(txtbox);
		wnoutrefresh(txtbox);
	    } else {
		touchwin(txtwin);
		wnoutrefresh(txtwin);
	    }
	    break;

	case '-':
	    if (limit > 0) {
		actual = ShowFlavor(strwin, txtwin, flavor, --limit);
		MovePrompt(txtwin, actual, txt_y, txt_x);
	    } else {
		beep();
	    }
	    break;

	case '+':
	    actual = ShowFlavor(strwin, txtwin, flavor, ++limit);
	    MovePrompt(txtwin, actual, txt_y, txt_x);
	    break;

	case '<':
	    if (flavor > 0) {
		actual = ShowFlavor(strwin, txtwin, --flavor, limit);
		MovePrompt(txtwin, actual, txt_y, txt_x);
	    } else {
		beep();
	    }
	    break;

	case '>':
	    if (flavor + 1 < eMaxFlavor) {
		actual = ShowFlavor(strwin, txtwin, ++flavor, limit);
		MovePrompt(txtwin, actual, txt_y, txt_x);
	    } else {
		beep();
	    }
	    break;

	case ':':
	    actual = ShowFlavor(strwin, txtwin, flavor, limit);
	    *buffer = '\0';
	    rc = ERR;
	    echo();
	    wattrset(txtwin, A_REVERSE);
	    switch (flavor) {
	    case eGetStr:
		if (txtwin != stdscr) {
		    wmove(txtwin, txt_y, txt_x);
		    rc = wget_wstr(txtwin, buffer);
		} else {
		    move(txt_y, txt_x);
		    rc = get_wstr(buffer);
		}
		break;
	    case eGetNStr:
		if (txtwin != stdscr) {
		    wmove(txtwin, txt_y, txt_x);
		    rc = wgetn_wstr(txtwin, buffer, limit);
		} else {
		    move(txt_y, txt_x);
		    rc = getn_wstr(buffer, limit);
		}
		break;
	    case eMvGetStr:
		if (txtwin != stdscr) {
		    rc = mvwget_wstr(txtwin, txt_y, txt_x, buffer);
		} else {
		    rc = mvget_wstr(txt_y, txt_x, buffer);
		}
		break;
	    case eMvGetNStr:
		if (txtwin != stdscr) {
		    rc = mvwgetn_wstr(txtwin, txt_y, txt_x, buffer, limit);
		} else {
		    rc = mvgetn_wstr(txt_y, txt_x, buffer, limit);
		}
		break;
	    case eMaxFlavor:
		break;
	    }
	    noecho();
	    wattrset(txtwin, A_NORMAL);
	    wprintw(strwin, "%d", rc);
	    waddwstr(strwin, (wchar_t *) buffer);
	    wnoutrefresh(strwin);
	    break;
	default:
	    beep();
	    break;
	}
	doupdate();
    }
    if (level > 1) {
	delwin(txtwin);
	delwin(txtbox);
    }
    return TRUE;
}

int
main(int argc, char *argv[])
{
    WINDOW *chrbox;
    WINDOW *strwin;

    setlocale(LC_ALL, "");

    if (argc < 2) {
	fprintf(stderr, "usage: %s file\n", argv[0]);
	return EXIT_FAILURE;
    }

    initscr();

    chrbox = derwin(stdscr, BASE_Y, COLS, 0, 0);
    box(chrbox, 0, 0);
    wnoutrefresh(chrbox);

    strwin = derwin(chrbox, 4, COLS - 2, 1, 1);

    test_get_wstr(1, argv, strwin);

    endwin();
    ExitProgram(EXIT_SUCCESS);
}
#else
int
main(void)
{
    printf("This program requires the wide-ncurses library\n");
    ExitProgram(EXIT_FAILURE);
}
#endif

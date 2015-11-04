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
 * $Id: inchs.c,v 1.9 2007/07/21 19:01:43 tom Exp $
 *
 * Author: Thomas E Dickey
 */
/*
       chtype inch(void);
       chtype winch(WINDOW *win);
       chtype mvinch(int y, int x);
       chtype mvwinch(WINDOW *win, int y, int x);
       int inchstr(chtype *chstr);
       int inchnstr(chtype *chstr, int n);
       int winchstr(WINDOW *win, chtype *chstr);
       int winchnstr(WINDOW *win, chtype *chstr, int n);
       int mvinchstr(int y, int x, chtype *chstr);
       int mvinchnstr(int y, int x, chtype *chstr, int n);
       int mvwinchstr(WINDOW *win, int y, int x, chtype *chstr);
       int mvwinchnstr(WINDOW *win, int y, int x, chtype *chstr, int n);
*/

#include <test.priv.h>

#define BASE_Y 7
#define MAX_COLS 1024

static bool
Quit(int ch)
{
    return (ch == ERR || ch == 'q' || ch == QUIT || ch == ESCAPE);
}

static int
test_inchs(int level, char **argv, WINDOW *chrwin, WINDOW *strwin)
{
    WINDOW *txtbox = 0;
    WINDOW *txtwin = 0;
    FILE *fp;
    int ch, j;
    int txt_x = 0, txt_y = 0;
    int base_y;
    int limit;
    chtype text[MAX_COLS];

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
	while ((j = fgetc(fp)) != EOF) {
	    if (waddch(txtwin, UChar(j)) != OK) {
		break;
	    }
	}
	fclose(fp);
    } else {
	wprintw(txtwin, "Cannot open:\n%s", argv[1]);
    }

    while (!Quit(j = mvwgetch(txtwin, txt_y, txt_x))) {
	switch (j) {
	case KEY_DOWN:
	case 'j':
	    if (txt_y < getmaxy(txtwin) - 1)
		txt_y++;
	    else
		beep();
	    break;
	case KEY_UP:
	case 'k':
	    if (txt_y > base_y)
		txt_y--;
	    else
		beep();
	    break;
	case KEY_LEFT:
	case 'h':
	    if (txt_x > 0)
		txt_x--;
	    else
		beep();
	    break;
	case KEY_RIGHT:
	case 'l':
	    if (txt_x < getmaxx(txtwin) - 1)
		txt_x++;
	    else
		beep();
	    break;
	case 'w':
	    test_inchs(level + 1, argv, chrwin, strwin);
	    if (txtbox != 0) {
		touchwin(txtbox);
		wnoutrefresh(txtbox);
	    } else {
		touchwin(txtwin);
		wnoutrefresh(txtwin);
	    }
	    break;
	default:
	    beep();
	    break;
	}

	mvwprintw(chrwin, 0, 0, "char:");
	wclrtoeol(chrwin);

	if (txtwin != stdscr) {
	    wmove(txtwin, txt_y, txt_x);

	    if ((ch = winch(txtwin)) != ERR) {
		if (waddch(chrwin, (chtype) ch) != ERR) {
		    for (j = txt_x + 1; j < getmaxx(txtwin); ++j) {
			if ((ch = mvwinch(txtwin, txt_y, j)) != ERR) {
			    if (waddch(chrwin, (chtype) ch) == ERR) {
				break;
			    }
			} else {
			    break;
			}
		    }
		}
	    }
	} else {
	    move(txt_y, txt_x);

	    if ((ch = inch()) != ERR) {
		if (waddch(chrwin, (chtype) ch) != ERR) {
		    for (j = txt_x + 1; j < getmaxx(txtwin); ++j) {
			if ((ch = mvinch(txt_y, j)) != ERR) {
			    if (waddch(chrwin, (chtype) ch) == ERR) {
				break;
			    }
			} else {
			    break;
			}
		    }
		}
	    }
	}
	wnoutrefresh(chrwin);

	mvwprintw(strwin, 0, 0, "text:");
	wclrtobot(strwin);

	limit = getmaxx(strwin) - 5;

	if (txtwin != stdscr) {
	    wmove(txtwin, txt_y, txt_x);
	    if (winchstr(txtwin, text) != ERR) {
		mvwaddchstr(strwin, 0, 5, text);
	    }

	    wmove(txtwin, txt_y, txt_x);
	    if (winchnstr(txtwin, text, limit) != ERR) {
		mvwaddchstr(strwin, 1, 5, text);
	    }

	    if (mvwinchstr(txtwin, txt_y, txt_x, text) != ERR) {
		mvwaddchstr(strwin, 2, 5, text);
	    }

	    if (mvwinchnstr(txtwin, txt_y, txt_x, text, limit) != ERR) {
		mvwaddchstr(strwin, 3, 5, text);
	    }
	} else {
	    move(txt_y, txt_x);
	    if (inchstr(text) != ERR) {
		mvwaddchstr(strwin, 0, 5, text);
	    }

	    move(txt_y, txt_x);
	    if (inchnstr(text, limit) != ERR) {
		mvwaddchstr(strwin, 1, 5, text);
	    }

	    if (mvinchstr(txt_y, txt_x, text) != ERR) {
		mvwaddchstr(strwin, 2, 5, text);
	    }

	    if (mvinchnstr(txt_y, txt_x, text, limit) != ERR) {
		mvwaddchstr(strwin, 3, 5, text);
	    }
	}

	wnoutrefresh(strwin);
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
    WINDOW *chrwin;
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

    chrwin = derwin(chrbox, 1, COLS - 2, 1, 1);
    strwin = derwin(chrbox, 4, COLS - 2, 2, 1);

    test_inchs(1, argv, chrwin, strwin);

    endwin();
    ExitProgram(EXIT_SUCCESS);
}

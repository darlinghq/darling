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
 * $Id: test_inwstr.c,v 1.3 2007/07/21 22:47:42 tom Exp $
 *
 * Author: Thomas E Dickey
 *
 * Demonstrate the inwstr functions from the curses library.

       int inwstr(wchar_t *str);
       int innwstr(wchar_t *str, int n);
       int winwstr(WINDOW *win, wchar_t *str);
       int winnwstr(WINDOW *win, wchar_t *str, int n);
       int mvinwstr(int y, int x, wchar_t *str);
       int mvinnwstr(int y, int x, wchar_t *str, int n);
       int mvwinwstr(WINDOW *win, int y, int x, wchar_t *str);
       int mvwinnwstr(WINDOW *win, int y, int x, wchar_t *str, int n);
 */

#include <test.priv.h>

#if USE_WIDEC_SUPPORT

#define BASE_Y 6
#define MAX_COLS 1024

static bool
Quit(int ch)
{
    return (ch == ERR || ch == 'q' || ch == QUIT || ch == ESCAPE);
}

static void
show_1st(WINDOW *win, int line, wchar_t *buffer)
{
    mvwaddwstr(win, line, 5, buffer);
}

static void
showmore(WINDOW *win, int line, wchar_t *buffer)
{
    wmove(win, line, 0);
    wclrtoeol(win);
    show_1st(win, line, buffer);
}

static int
test_inchs(int level, char **argv, WINDOW *chrwin, WINDOW *strwin)
{
    WINDOW *txtbox = 0;
    WINDOW *txtwin = 0;
    FILE *fp;
    int ch;
    int txt_x = 0, txt_y = 0;
    int base_y;
    int limit = getmaxx(strwin) - 5;
    wchar_t buffer[MAX_COLS];

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

    while (!Quit(ch = mvwgetch(txtwin, txt_y, txt_x))) {
	switch (ch) {
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
	case '-':
	    if (limit > 0) {
		--limit;
	    } else {
		beep();
	    }
	    break;
	case '+':
	    ++limit;
	    break;
	default:
	    beep();
	    break;
	}

	mvwprintw(chrwin, 0, 0, "line:");
	wclrtoeol(chrwin);

	if (txtwin != stdscr) {
	    wmove(txtwin, txt_y, txt_x);

	    if (winwstr(txtwin, buffer) != ERR) {
		show_1st(chrwin, 0, buffer);
	    }
	    if (mvwinwstr(txtwin, txt_y, txt_x, buffer) != ERR) {
		showmore(chrwin, 1, buffer);
	    }
	} else {
	    move(txt_y, txt_x);

	    if (inwstr(buffer) != ERR) {
		show_1st(chrwin, 0, buffer);
	    }
	    if (mvinwstr(txt_y, txt_x, buffer) != ERR) {
		showmore(chrwin, 1, buffer);
	    }
	}
	wnoutrefresh(chrwin);

	mvwprintw(strwin, 0, 0, "%4d:", limit);
	wclrtobot(strwin);

	if (txtwin != stdscr) {
	    wmove(txtwin, txt_y, txt_x);
	    if (winnwstr(txtwin, buffer, limit) != ERR) {
		show_1st(strwin, 0, buffer);
	    }

	    if (mvwinnwstr(txtwin, txt_y, txt_x, buffer, limit) != ERR) {
		showmore(strwin, 1, buffer);
	    }
	} else {
	    move(txt_y, txt_x);
	    if (innwstr(buffer, limit) != ERR) {
		show_1st(strwin, 0, buffer);
	    }

	    if (mvinnwstr(txt_y, txt_x, buffer, limit) != ERR) {
		showmore(strwin, 1, buffer);
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

    chrwin = derwin(chrbox, 2, COLS - 2, 1, 1);
    strwin = derwin(chrbox, 2, COLS - 2, 3, 1);

    test_inchs(1, argv, chrwin, strwin);

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

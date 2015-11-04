/****************************************************************************
 * Copyright (c) 2007,2008 Free Software Foundation, Inc.                   *
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
 * $Id: demo_panels.c,v 1.33 2008/08/04 13:33:48 tom Exp $
 *
 * Demonstrate a variety of functions from the panel library.
 */

#include <test.priv.h>

#if USE_LIBPANEL

#include <panel.h>

#define LAST_POS '@'
#define TEMP_POS '>'

typedef void (*InitPanel) (void);
typedef void (*FillPanel) (PANEL *);

static bool use_colors = FALSE;
static bool unboxed = FALSE;
static FILE *log_in;
static FILE *log_out;

static void
close_input(void)
{
    if (log_in != 0) {
	fclose(log_in);
	log_in = 0;
    }
}

static void
close_output(void)
{
    if (log_out != 0) {
	fclose(log_out);
	log_out = 0;
    }
}

static WINDOW *
statusline(void)
{
    WINDOW *result = stdscr;

    wmove(result, LINES - 1, 0);
    wclrtoeol(result);
    return result;
}

static void
pflush(void)
{
    update_panels();
    doupdate();
}

static void
saywhat(NCURSES_CONST char *text)
{
    WINDOW *win = statusline();
    if (text != 0 && *text != '\0') {
	waddstr(win, text);
	waddstr(win, "; ");
    }
    waddstr(win, "press any key to continue");
}

static void
show_position(NCURSES_CONST char *text,
	      NCURSES_CONST char *also,
	      int which,
	      int ypos,
	      int xpos)
{
    WINDOW *win = statusline();

    wprintw(win, "%s for panel %d now %d,%d%s", text, which, ypos, xpos, also);
    wmove(stdscr, ypos, xpos);
}

static int
get_position(NCURSES_CONST char *text,
	     NCURSES_CONST char *also,
	     int which,
	     int *xpos,
	     int *ypos)
{
    int result = 0;
    int x1, y1;
    char cmd;
    WINDOW *win;

    getyx(stdscr, y1, x1);
    win = statusline();

    show_position(text, also, which, y1, x1);

    if (log_in != 0) {
	if (fscanf(log_in, "%c%d,%d\n", &cmd, &y1, &x1) == 3) {
	    switch (cmd) {
	    case LAST_POS:
		result = 1;
		(void) wgetch(stdscr);
		break;
	    case TEMP_POS:
		result = 0;
		wrefresh(stdscr);
		napms(100);
		break;
	    default:
		result = -1;
		break;
	    }
	} else {
	    result = -1;
	}
    } else {

	switch (wgetch(stdscr)) {
	case QUIT:
	case ESCAPE:
	case ERR:
	    result = -1;
	    break;
	case ' ':
	    result = 1;
	    break;
	case KEY_UP:
	    if (y1 > 0) {
		--y1;
	    } else {
		beep();
	    }
	    break;
	case KEY_DOWN:
	    if (y1 < getmaxy(stdscr)) {
		++y1;
	    } else {
		beep();
	    }
	    break;
	case KEY_LEFT:
	    if (x1 > 0) {
		--x1;
	    } else {
		beep();
	    }
	    break;
	case KEY_RIGHT:
	    if (x1 < getmaxx(stdscr)) {
		++x1;
	    } else {
		beep();
	    }
	    break;
	}
    }

    wmove(stdscr, y1, x1);
    *ypos = y1;
    *xpos = x1;

    if (result >= 0) {
	if (log_out)
	    fprintf(log_out, "%c%d,%d\n",
		    ((result > 0)
		     ? LAST_POS
		     : TEMP_POS),
		    y1, x1);
    }
    return result;
}

static PANEL *
mkpanel(short color, int rows, int cols, int tly, int tlx)
{
    WINDOW *win;
    PANEL *pan = 0;
    char *userdata = typeMalloc(char, 3);

    if ((win = newwin(rows, cols, tly, tlx)) != 0) {
	keypad(win, TRUE);
	if ((pan = new_panel(win)) == 0) {
	    delwin(win);
	} else if (use_colors) {
	    short fg = (short) ((color == COLOR_BLUE)
				? COLOR_WHITE
				: COLOR_BLACK);
	    short bg = color;

	    init_pair(color, fg, bg);
	    wbkgdset(win, (chtype) (COLOR_PAIR(color) | ' '));
	} else if (!unboxed) {
	    wbkgdset(win, A_BOLD | ' ');
	}
    }
    sprintf(userdata, "p%d", color % 8);
    set_panel_userptr(pan, (NCURSES_CONST void *) userdata);
    return pan;
}

static void
my_remove_panel(PANEL ** pans, int which)
{
    if (pans[which] != 0) {
	PANEL *pan = pans[which];
	WINDOW *win = panel_window(pan);
	char *user = (char *) panel_userptr(pan);

	free(user);
	del_panel(pan);
	delwin(win);

	pans[which] = 0;
    }
}

#undef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define ABS(a)   ((a) < 0 ? -(a) : (a))

static void
my_create_panel(PANEL ** pans, int which, FillPanel myFill)
{
    PANEL *pan = 0;
    int code;
    short pair = (short) which;
    short fg = (short) ((pair == COLOR_BLUE) ? COLOR_WHITE : COLOR_BLACK);
    short bg = pair;
    int x0, y0, x1, y1;

    init_pair(pair, fg, bg);

    /* remove the old panel, if any */
    my_remove_panel(pans, which);

    /* get the position of one corner */
    wmove(stdscr, getmaxy(stdscr) / 2, getmaxx(stdscr) / 2);
    getyx(stdscr, y0, x0);
    while ((code = get_position("First corner", "", which, &x0, &y0)) == 0) {
	;
    }

    if (code > 0) {
	char also[80];
	sprintf(also, " (first %d,%d)", y0, x0);
	/* get the position of the opposite corner */
	while ((code = get_position("Opposite corner",
				    also, which, &x1, &y1)) == 0) {
	    ;
	}

	if (code > 0) {
	    int tly = MIN(y0, y1);
	    int tlx = MIN(x0, x1);
	    pan = mkpanel(pair, ABS(y1 - y0) + 1, ABS(x1 - x0) + 1, tly, tlx);
	    /* finish */
	    myFill(pan);
	    pans[which] = pan;
	    pflush();
	    wmove(stdscr, y1, x1);
	}
    }
}

static void
my_move_panel(PANEL ** pans, int which, bool continuous)
{
    if (pans[which] != 0) {
	int code;
	int y0, x0;
	int y1, x1;
	WINDOW *win = panel_window(pans[which]);
	char also[80];

	getbegyx(win, y0, x0);
	sprintf(also, " (start %d,%d)", y0, x0);
	wmove(stdscr, y0, x0);
	while ((code = get_position("Move panel", also, which, &x1, &y1)) == 0) {
	    if (continuous) {
		move_panel(pans[which], y1, x1);
		pflush();
	    }
	}
	if (code > 0) {
	    move_panel(pans[which], y1, x1);
	}
    }
}

static void
my_resize_panel(PANEL ** pans, int which, FillPanel myFill)
{
    if (pans[which] != 0) {
	int code;
	int y0, x0;
	int y1, x1;
	WINDOW *win = panel_window(pans[which]);
	char also[80];

	getbegyx(win, y0, x0);
	sprintf(also, " (start %d,%d)", y0, x0);
	wmove(stdscr, y0, x0);
	while ((code = get_position("Resize panel",
				    also, which, &x1, &y1)) == 0) {
	    ;
	}
	if (code > 0) {
	    WINDOW *next = newwin(ABS(y1 - y0) + 1,
				  ABS(x1 - x0) + 1,
				  MIN(y0, y1),
				  MIN(x0, x1));
	    if (next != 0) {
		keypad(next, TRUE);
		if (use_colors) {
		    wbkgdset(next, (chtype) (COLOR_PAIR(which) | ' '));
		} else if (!unboxed) {
		    wbkgdset(next, A_BOLD | ' ');
		}
		replace_panel(pans[which], next);
		myFill(pans[which]);
		delwin(win);
	    }
	}
    }
}

static void
init_panel(void)
{
    register int y, x;

    for (y = 0; y < LINES - 1; y++) {
	for (x = 0; x < COLS; x++)
	    wprintw(stdscr, "%d", (y + x) % 10);
    }
}

static void
fill_panel(PANEL * pan)
{
    WINDOW *win = panel_window(pan);
    const char *userptr = (const char *) panel_userptr(pan);
    int num = (userptr && *userptr) ? userptr[1] : '?';
    int y, x;

    wmove(win, 1, 1);
    wprintw(win, "-pan%c-", num);
    wclrtoeol(win);
    box(win, 0, 0);
    for (y = 2; y < getmaxy(win) - 1; y++) {
	for (x = 1; x < getmaxx(win) - 1; x++) {
	    wmove(win, y, x);
	    waddch(win, UChar(num));
	}
    }
}

static void
fill_unboxed(PANEL * pan)
{
    WINDOW *win = panel_window(pan);
    const char *userptr = (const char *) panel_userptr(pan);
    int num = (userptr && *userptr) ? userptr[1] : '?';
    int y, x;

    for (y = 0; y < getmaxy(win); y++) {
	for (x = 0; x < getmaxx(win); x++) {
	    wmove(win, y, x);
	    waddch(win, UChar(num));
	}
    }
}

#if USE_WIDEC_SUPPORT
static void
make_fullwidth_digit(cchar_t *target, int digit)
{
    wchar_t source[2];

    source[0] = digit + 0xff10;
    source[1] = 0;
    setcchar(target, source, A_NORMAL, 0, 0);
}

static void
init_wide_panel(void)
{
    int digit;
    cchar_t temp[10];

    for (digit = 0; digit < 10; ++digit)
	make_fullwidth_digit(&temp[digit], digit);

    do {
	int y, x;
	getyx(stdscr, y, x);
	digit = (y + x / 2) % 10;
    } while (add_wch(&temp[digit]) != ERR);
}

static void
fill_wide_panel(PANEL * pan)
{
    WINDOW *win = panel_window(pan);
    int num = ((const char *) panel_userptr(pan))[1];
    int y, x;

    wmove(win, 1, 1);
    wprintw(win, "-pan%c-", num);
    wclrtoeol(win);
    box(win, 0, 0);
    for (y = 2; y < getmaxy(win) - 1; y++) {
	for (x = 1; x < getmaxx(win) - 1; x++) {
	    wmove(win, y, x);
	    waddch(win, UChar(num));
	}
    }
}
#endif

#define MAX_PANELS 5

static int
which_panel(PANEL * px[MAX_PANELS + 1], PANEL * pan)
{
    int result = 0;
    int j;

    for (j = 1; j <= MAX_PANELS; ++j) {
	if (px[j] == pan) {
	    result = j;
	    break;
	}
    }
    return result;
}

static void
show_panels(PANEL * px[MAX_PANELS + 1])
{
    static const char *help[] =
    {
	"",
	"Commands are letter/digit pairs.  Digits are the panel number.",
	"",
	"  b - put the panel on the bottom of the stack",
	"  c - create the panel",
	"  d - delete the panel",
	"  h - hide the panel",
	"  m - move the panel (M for continuous move)",
	"  r - resize the panel",
	"  s - show the panel",
	"  b - put the panel on the top of the stack"
    };

    struct {
	bool valid;
	bool hidden;
	PANEL *above;
	PANEL *below;
    } table[MAX_PANELS + 1];

    WINDOW *win;
    PANEL *pan;
    int j;

    for (j = 1; j <= MAX_PANELS; ++j) {
	table[j].valid = (px[j] != 0);
	if (table[j].valid) {
	    table[j].hidden = panel_hidden(px[j]);
	    table[j].above = panel_above(px[j]);
	    table[j].below = panel_below(px[j]);
	}
    }

    if ((win = newwin(LINES - 1, COLS, 0, 0)) != 0) {
	keypad(win, TRUE);
	if ((pan = new_panel(win)) != 0) {
	    werase(win);
	    mvwprintw(win, 0, 0, "Panels:\n");
	    for (j = 1; j <= MAX_PANELS; ++j) {
		if (table[j].valid) {
		    wprintw(win, " %d:", j);
		    if (table[j].hidden) {
			waddstr(win, " hidden");
		    } else {
			if (table[j].above) {
			    wprintw(win, " above %d",
				    which_panel(px, table[j].above));
			}
			if (table[j].below) {
			    wprintw(win, "%s below %d",
				    table[j].above ? "," : "",
				    which_panel(px, table[j].below));
			}
		    }
		    waddch(win, '\n');
		}
	    }
	    for (j = 0; j < (int) SIZEOF(help); ++j) {
		if (wprintw(win, "%s\n", help[j]) == ERR)
		    break;
	    }
	    wgetch(win);
	    del_panel(pan);
	    pflush();
	}
	delwin(win);
    }
}

#define wrapper(func) \
static int my_##func(PANEL *pan) \
{ \
    int code = ERR; \
    if (pan != 0) { \
	code = func(pan); \
    } \
    return code; \
}
/* *INDENT-OFF* */
wrapper(bottom_panel)
wrapper(hide_panel)
wrapper(show_panel)
wrapper(top_panel)
/* *INDENT-ON* */

static void
do_panel(PANEL * px[MAX_PANELS + 1],
	 NCURSES_CONST char *cmd,
	 FillPanel myFill)
{
    int which = cmd[1] - '0';

    if (which < 1 || which > MAX_PANELS) {
	beep();
	return;
    }

    if (log_in != 0) {
	pflush();
    }

    saywhat(cmd);
    switch (*cmd) {
    case 'b':
	my_bottom_panel(px[which]);
	break;
    case 'c':
	my_create_panel(px, which, myFill);
	break;
    case 'd':
	my_remove_panel(px, which);
	break;
    case 'h':
	my_hide_panel(px[which]);
	break;
    case 'm':
	my_move_panel(px, which, FALSE);
	break;
    case 'M':
	my_move_panel(px, which, TRUE);
	break;
    case 'r':
	my_resize_panel(px, which, myFill);
	break;
    case 's':
	my_show_panel(px[which]);
	break;
    case 't':
	my_top_panel(px[which]);
	break;
    }
}

static bool
ok_letter(int ch)
{
    return isalpha(UChar(ch)) && strchr("bcdhmMrst", ch) != 0;
}

static bool
ok_digit(int ch)
{
    return isdigit(UChar(ch)) && (ch >= '1') && (ch - '0' <= MAX_PANELS);
}

/*
 * A command consists of one or more letter/digit pairs separated by a space.
 * Digits are limited to 1..MAX_PANELS.
 *
 * End the command with a newline.  Reject other characters.
 */
static bool
get_command(PANEL * px[MAX_PANELS + 1], char *buffer, int limit)
{
    int length = 0;
    int y0, x0;
    int c0, ch;
    WINDOW *win;

    getyx(stdscr, y0, x0);
    win = statusline();
    waddstr(win, "Command:");
    buffer[length = 0] = '\0';

    if (log_in != 0) {
	if (fgets(buffer, limit - 3, log_in) != 0) {
	    length = (int) strlen(buffer);
	    while (length > 0 && isspace(UChar(buffer[length - 1])))
		buffer[--length] = '\0';
	    waddstr(win, buffer);
	} else {
	    close_input();
	}
	(void) wgetch(win);
    } else {
	c0 = 0;
	for (;;) {
	    ch = wgetch(win);
	    if (ch == ERR || ch == QUIT || ch == ESCAPE) {
		buffer[0] = '\0';
		break;
	    } else if (ch == CTRL('L')) {
		wrefresh(curscr);
	    } else if (ch == '\n' || ch == KEY_ENTER) {
		break;
	    } else if (ch == '?') {
		show_panels(px);
	    } else if (length + 3 < limit) {
		if (ch >= KEY_MIN) {
		    beep();
		} else if (ok_letter(UChar(ch))) {
		    if (isalpha(UChar(c0))) {
			beep();
		    } else if (isdigit(UChar(c0))) {
			wprintw(win, " %c", ch);
			buffer[length++] = ' ';
			buffer[length++] = (char) (c0 = ch);
		    } else {
			wprintw(win, "%c", ch);
			buffer[length++] = (char) (c0 = ch);
		    }
		} else if (ok_digit(ch)) {
		    if (isalpha(UChar(c0))) {
			wprintw(win, "%c", ch);
			buffer[length++] = (char) (c0 = ch);
		    } else {
			beep();
		    }
		} else if (ch == ' ') {
		    if (isdigit(UChar(c0))) {
			wprintw(win, "%c", ch);
			buffer[length++] = (char) (c0 = ch);
		    } else {
			beep();
		    }
		} else {
		    beep();
		}
	    } else {
		beep();
	    }
	}
    }

    wmove(stdscr, y0, x0);

    buffer[length] = '\0';
    if (log_out && length) {
	fprintf(log_out, "%s\n", buffer);
    }
    return (length != 0);
}

static void
demo_panels(InitPanel myInit, FillPanel myFill)
{
    int itmp;
    PANEL *px[MAX_PANELS + 1];
    char buffer[BUFSIZ];

    scrollok(stdscr, FALSE);	/* we don't want stdscr to scroll! */
    refresh();

    myInit();
    memset(px, 0, sizeof(px));

    while (get_command(px, buffer, sizeof(buffer))) {
	int limit = (int) strlen(buffer);
	for (itmp = 0; itmp < limit; itmp += 3) {
	    do_panel(px, buffer + itmp, myFill);
	}
	pflush();
    }
#if NO_LEAKS
    for (itmp = 1; itmp <= MAX_PANELS; ++itmp) {
	my_remove_panel(px, itmp);
    }
#endif
}

static void
usage(void)
{
    static const char *const tbl[] =
    {
	"Usage: demo_panels [options]"
	,""
	,"Options:"
	,"  -i file  read commands from file"
	,"  -o file  record commands in file"
	,"  -m       do not use colors"
#if USE_WIDEC_SUPPORT
	,"  -w       use wide-characters in panels and background"
#endif
	,"  -x       do not enclose panels in boxes"
    };
    size_t n;
    for (n = 0; n < SIZEOF(tbl); n++)
	fprintf(stderr, "%s\n", tbl[n]);
    ExitProgram(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int c;
    bool monochrome = FALSE;
    InitPanel myInit = init_panel;
    FillPanel myFill = fill_panel;

    setlocale(LC_ALL, "");

    while ((c = getopt(argc, argv, "i:o:mwx")) != -1) {
	switch (c) {
	case 'i':
	    log_in = fopen(optarg, "r");
	    break;
	case 'o':
	    log_out = fopen(optarg, "w");
	    break;
	case 'm':
	    monochrome = TRUE;
	    break;
#if USE_WIDEC_SUPPORT
	case 'w':
	    myInit = init_wide_panel;
	    myFill = fill_wide_panel;
	    break;
#endif
	case 'x':
	    unboxed = TRUE;
	    break;
	default:
	    usage();
	}
    }
    if (unboxed)
	myFill = fill_unboxed;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    use_colors = monochrome ? FALSE : has_colors();
    if (use_colors)
	start_color();

    demo_panels(myInit, myFill);
    endwin();

    close_input();
    close_output();

    ExitProgram(EXIT_SUCCESS);
}
#else
int
main(void)
{
    printf("This program requires the curses panel library\n");
    ExitProgram(EXIT_FAILURE);
}
#endif

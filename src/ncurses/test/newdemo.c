/*
 *  newdemo.c	-	A demo program using PDCurses. The program illustrate
 *  	 		the use of colours for text output.
 *
 * $Id: newdemo.c,v 1.31 2008/08/03 20:19:38 tom Exp $
 */

#include <test.priv.h>

#include <time.h>

/*
 *  The Australian map
 */
static CONST_MENUS char *AusMap[16] =
{
    "           A           A ",
    "    N.T. AAAAA       AAAA ",
    "     AAAAAAAAAAA  AAAAAAAA ",
    "   AAAAAAAAAAAAAAAAAAAAAAAAA Qld.",
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAA ",
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA ",
    " AAAAAAAAAAAAAAAAAAAAAAAAAAAA ",
    "   AAAAAAAAAAAAAAAAAAAAAAAAA N.S.W.",
    "W.A. AAAAAAAAA      AAAAAA Vic.",
    "       AAA   S.A.     AA",
    "                       A  Tas.",
    ""
};

/*
 *  Funny messages
 */
#define NMESSAGES   6

static NCURSES_CONST char *messages[] =
{
    "Hello from the Land Down Under",
    "The Land of crocs. and a big Red Rock",
    "Where the sunflower runs along the highways",
    "the dusty red roads lead one to loneliness",
    "Blue sky in the morning and",
    "freezing nights and twinkling stars",
    ""
};

/*
 *  Trap interrupt
 */
static RETSIGTYPE
trap(int sig GCC_UNUSED)
{
    endwin();
    ExitProgram(EXIT_FAILURE);
}

/*
 *  Wait for user
 */
static int
WaitForUser(WINDOW *win)
{
    time_t t;
    chtype key;

    nodelay(win, TRUE);
    t = time((time_t *) 0);
    while (1) {
	if ((int) (key = wgetch(win)) != ERR) {
	    if (key == 'q' || key == 'Q')
		return 1;
	    else
		return 0;
	}
	if (time((time_t *) 0) - t > 5)
	    return 0;
    }
}

static void
set_colors(WINDOW *win, int pair, int foreground, int background)
{
    if (has_colors()) {
	if (pair > COLOR_PAIRS)
	    pair = COLOR_PAIRS;
	init_pair(pair, foreground, background);
	wattrset(win, COLOR_PAIR(pair));
    }
}

static chtype
use_colors(WINDOW *win, int pair, chtype attrs)
{
    if (has_colors()) {
	if (pair > COLOR_PAIRS)
	    pair = COLOR_PAIRS;
	attrs |= COLOR_PAIR(pair);
    }
    wattrset(win, attrs);
    return attrs;
}

/*
 * Test sub windows
 */
static int
SubWinTest(WINDOW *win)
{
    int w, h, sw, sh, bx, by;
    WINDOW *swin1, *swin2, *swin3;

    getmaxyx(win, h, w);
    getbegyx(win, by, bx);
    sw = w / 3;
    sh = h / 3;
    if ((swin1 = subwin(win, sh, sw, by + 3, bx + 5)) == NULL)
	return 1;
    if ((swin2 = subwin(win, sh, sw, by + 4, bx + 8)) == NULL)
	return 1;
    if ((swin3 = subwin(win, sh, sw, by + 5, bx + 11)) == NULL)
	return 1;

    set_colors(swin1, 8, COLOR_RED, COLOR_BLUE);
    werase(swin1);
    mvwaddstr(swin1, 0, 3, "Sub-window 1");
    wrefresh(swin1);

    set_colors(swin2, 9, COLOR_CYAN, COLOR_MAGENTA);
    werase(swin2);
    mvwaddstr(swin2, 0, 3, "Sub-window 2");
    wrefresh(swin2);

    set_colors(swin3, 10, COLOR_YELLOW, COLOR_GREEN);
    werase(swin3);
    mvwaddstr(swin3, 0, 3, "Sub-window 3");
    wrefresh(swin3);

    delwin(swin1);
    delwin(swin2);
    delwin(swin3);
    WaitForUser(win);
    return 0;
}

static int
bounce(int n, int *dir, int len)
{
    if (*dir > 0)
	++n;
    else
	--n;
    if (n <= 1 || n >= len - 2)
	*dir = *dir ? 0 : 1;
    return n;
}

/*
 *  Bouncing balls
 */
static int
BouncingBalls(WINDOW *win)
{
    int w, h;
    int x1, y1, xd1, yd1;
    int x2, y2, xd2, yd2;
    int x3, y3, xd3, yd3;

    getmaxyx(win, h, w);

    x1 = 2 + rand() % (w - 4);
    y1 = 2 + rand() % (h - 4);
    x2 = 2 + rand() % (w - 4);
    y2 = 2 + rand() % (h - 4);
    x3 = 2 + rand() % (w - 4);
    y3 = 2 + rand() % (h - 4);

    xd1 = 1;
    yd1 = 1;
    xd2 = 1;
    yd2 = 0;
    xd3 = 0;
    yd3 = 1;

    nodelay(win, TRUE);

    while (wgetch(win) == ERR) {
	x1 = bounce(x1, &xd1, w);
	y1 = bounce(y1, &yd1, h);
	x2 = bounce(x2, &xd2, w);
	y2 = bounce(y2, &yd2, h);
	x3 = bounce(x3, &xd3, w);
	y3 = bounce(y3, &yd3, h);

	set_colors(win, 11, COLOR_RED, COLOR_BLUE);
	mvwaddch(win, y1, x1, 'O');

	set_colors(win, 12, COLOR_BLUE, COLOR_RED);
	mvwaddch(win, y2, x2, '*');

	set_colors(win, 13, COLOR_YELLOW, COLOR_WHITE);
	mvwaddch(win, y3, x3, '@');

	wmove(win, 0, 0);
	wrefresh(win);
	delay_output(100);
    }
    return 0;
}

/*
 *  Main driver
 */
int
main(int argc GCC_UNUSED, char *argv[]GCC_UNUSED)
{
    WINDOW *win;
    int w, x, y, i, j, k;
    char buffer[200];
    const char *message;
    int width, height;
    chtype save[80];
    chtype c;

    setlocale(LC_ALL, "");

    CATCHALL(trap);

    initscr();
    if (has_colors())
	start_color();
    cbreak();
    curs_set(0);
    width = 48;
    height = 14;		/* Create a drawing window */
    win = newwin(height, width, (LINES - height) / 2, (COLS - width) / 2);
    if (win == NULL) {
	endwin();
	ExitProgram(EXIT_FAILURE);
    }

    while (1) {
	set_colors(win, 1, COLOR_WHITE, COLOR_BLUE);
	werase(win);

	set_colors(win, 2, COLOR_RED, COLOR_RED);
	box(win, ACS_VLINE, ACS_HLINE);
	wrefresh(win);
	/* Do ramdom output of a character */
	use_colors(win, 1, A_NORMAL);
	c = 'a';
	for (i = 0; i < 5000; ++i) {
	    x = rand() % (width - 2) + 1;
	    y = rand() % (height - 2) + 1;
	    mvwaddch(win, y, x, c);
	    wrefresh(win);
	    nodelay(win, TRUE);
	    if (wgetch(win) != ERR)
		break;
	    if (i == 2000) {
		c = 'b';
		set_colors(win, 3, COLOR_CYAN, COLOR_YELLOW);
	    }
	}

	SubWinTest(win);
	/* Erase and draw green window */
	set_colors(win, 4, COLOR_YELLOW, COLOR_GREEN);
	wbkgd(win, use_colors(win, 4, A_BOLD));
	werase(win);
	wrefresh(win);
	/* Draw RED bounding box */
	use_colors(win, 2, A_NORMAL);
	box(win, ' ', ' ');
	wrefresh(win);
	/* Display Australia map */
	use_colors(win, 4, A_BOLD);
	i = 0;
	while (*AusMap[i]) {
	    mvwaddstr(win, i + 1, 8, AusMap[i]);
	    wrefresh(win);
	    delay_output(50);
	    ++i;
	}

	set_colors(win, 5, COLOR_BLUE, COLOR_WHITE);
	use_colors(win, 5, A_BLINK);
	mvwaddstr(win, height - 2, 6, " PDCurses 2.1 for DOS, OS/2 and Unix");
	wrefresh(win);

	/* Draw running messages */
	set_colors(win, 6, COLOR_YELLOW, COLOR_WHITE);
	message = messages[j = 0];
	i = 1;
	w = width - 2;
	strcpy(buffer, message);
	while (j < NMESSAGES) {
	    while ((int) strlen(buffer) < w) {
		strcat(buffer, " ... ");
		strcat(buffer, messages[++j % NMESSAGES]);
	    }

	    if (i < w)
		mvwaddnstr(win, height / 2, w - i, buffer, i);
	    else
		mvwaddnstr(win, height / 2, 1, buffer, w);

	    wrefresh(win);
	    nodelay(win, TRUE);
	    if (wgetch(win) != ERR) {
		flushinp();
		break;
	    }
	    if (i++ >= w) {
		for (k = 0; (buffer[k] = buffer[k + 1]) != '\0'; k++) ;
	    }
	    delay_output(100);
	}

	j = 0;
	/*  Draw running As across in RED */
	set_colors(win, 7, COLOR_RED, COLOR_GREEN);
	memset(save, ' ', sizeof(save));
	for (i = 2; i < width - 4; ++i) {
	    k = mvwinch(win, 4, i);
	    if (k == ERR)
		break;
	    save[j++] = c = k;
	    c &= A_CHARTEXT;
	    mvwaddch(win, 4, i, c);
	}
	wrefresh(win);

	/* Put a message up wait for a key */
	i = height - 2;
	use_colors(win, 5, A_NORMAL);
	mvwaddstr(win, i, 5, " Type a key to continue or 'Q' to quit ");
	wrefresh(win);

	if (WaitForUser(win) == 1)
	    break;

	j = 0;			/* Restore the old line */
	for (i = 2; i < width - 4; ++i)
	    mvwaddch(win, 4, i, save[j++]);
	wrefresh(win);

	BouncingBalls(win);
	/* Put a message up wait for a key */
	i = height - 2;
	use_colors(win, 5, A_NORMAL);
	mvwaddstr(win, i, 5, " Type a key to continue or 'Q' to quit ");
	wrefresh(win);
	if (WaitForUser(win) == 1)
	    break;
    }
    endwin();
    ExitProgram(EXIT_SUCCESS);
}

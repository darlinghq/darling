/* $Id: tclock.c,v 1.25 2005/04/16 16:39:27 tom Exp $ */

#include <test.priv.h>

#include <math.h>

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

/*
  tclock - analog/digital clock for curses.
  If it gives you joy, then
  (a) I'm glad
  (b) you need to get out more :-)

  This program is copyright Howard Jones, September 1994
  (ha.jones@ic.ac.uk). It may be freely distributed as
  long as this copyright message remains intact, and any
  modifications are clearly marked as such. [In fact, if
  you modify it, I wouldn't mind the modifications back,
  especially if they add any nice features. A good one
  would be a precalc table for the 60 hand positions, so
  that the floating point stuff can be ditched. As I said,
  it was a 20 hackup minute job.]

  COMING SOON: tfishtank. Be the envy of your mac-owning
  colleagues.
*/

/* To compile: cc -o tclock tclock.c -lcurses -lm */

#ifndef PI
#define PI 3.141592654
#endif

#define sign(_x) (_x<0?-1:1)

#define ASPECT 2.2
#define ROUND(value) ((int)((value) + 0.5))

#define A2X(angle,radius) ROUND(ASPECT * radius * sin(angle))
#define A2Y(angle,radius) ROUND(radius * cos(angle))

/* Plot a point */
static void
plot(int x, int y, char col)
{
    mvaddch(y, x, (chtype) col);
}

/* Draw a diagonal(arbitrary) line using Bresenham's alogrithm. */
static void
dline(int pair, int from_x, int from_y, int x2, int y2, char ch)
{
    int dx, dy;
    int ax, ay;
    int sx, sy;
    int x, y;
    int d;

    if (has_colors())
	attrset(COLOR_PAIR(pair));

    dx = x2 - from_x;
    dy = y2 - from_y;

    ax = abs(dx * 2);
    ay = abs(dy * 2);

    sx = sign(dx);
    sy = sign(dy);

    x = from_x;
    y = from_y;

    if (ax > ay) {
	d = ay - (ax / 2);

	while (1) {
	    plot(x, y, ch);
	    if (x == x2)
		return;

	    if (d >= 0) {
		y += sy;
		d -= ax;
	    }
	    x += sx;
	    d += ay;
	}
    } else {
	d = ax - (ay / 2);

	while (1) {
	    plot(x, y, ch);
	    if (y == y2)
		return;

	    if (d >= 0) {
		x += sx;
		d -= ay;
	    }
	    y += sy;
	    d += ax;
	}
    }
}

int
main(int argc GCC_UNUSED, char *argv[]GCC_UNUSED)
{
    int i, cx, cy;
    double cr, mradius, hradius, mangle, hangle;
    double sangle, sradius, hours;
    int hdx, hdy;
    int mdx, mdy;
    int sdx, sdy;
    int ch;
    int lastbeep = -1;
    bool odd = FALSE;
    time_t tim;
    struct tm *t;
    char szChar[10];
    char *text;
    int my_bg = COLOR_BLACK;
#if HAVE_GETTIMEOFDAY
    struct timeval current;
    double fraction = 0.0;
#endif

    setlocale(LC_ALL, "");

    initscr();
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
	start_color();
#if HAVE_USE_DEFAULT_COLORS
	if (use_default_colors() == OK)
	    my_bg = -1;
#endif
	init_pair(1, COLOR_RED, my_bg);
	init_pair(2, COLOR_MAGENTA, my_bg);
	init_pair(3, COLOR_GREEN, my_bg);
	init_pair(4, COLOR_WHITE, COLOR_BLUE);
    }
#ifdef KEY_RESIZE
    keypad(stdscr, TRUE);
  restart:
#endif
    cx = (COLS - 1) / 2;	/* 39 */
    cy = LINES / 2;		/* 12 */
    if (cx / ASPECT < cy)
	cr = cx / ASPECT;
    else
	cr = cy;
    sradius = (5 * cr) / 6;	/* 10 */
    mradius = (3 * cr) / 4;	/* 9 */
    hradius = cr / 2;		/* 6 */

    for (i = 0; i < 12; i++) {
	sangle = (i + 1) * (2.0 * PI) / 12.0;
	sdx = A2X(sangle, sradius);
	sdy = A2Y(sangle, sradius);
	sprintf(szChar, "%d", i + 1);

	mvaddstr(cy - sdy, cx + sdx, szChar);
    }

    mvaddstr(0, 0, "ASCII Clock by Howard Jones (ha.jones@ic.ac.uk),1994");

    sradius = (4 * sradius) / 5;
    for (;;) {
	napms(100);

	tim = time(0);
	t = localtime(&tim);

	hours = (t->tm_hour + (t->tm_min / 60.0));
	if (hours > 12.0)
	    hours -= 12.0;

	mangle = ((t->tm_min + (t->tm_sec / 60.0)) * (2 * PI) / 60.0);
	mdx = A2X(mangle, mradius);
	mdy = A2Y(mangle, mradius);

	hangle = ((hours) * (2.0 * PI) / 12.0);
	hdx = A2X(hangle, hradius);
	hdy = A2Y(hangle, hradius);

#if HAVE_GETTIMEOFDAY
	gettimeofday(&current, 0);
	fraction = (current.tv_usec / 1.0e6);
#endif
	sangle = ((t->tm_sec + fraction) * (2.0 * PI) / 60.0);
	sdx = A2X(sangle, sradius);
	sdy = A2Y(sangle, sradius);

	dline(3, cx, cy, cx + mdx, cy - mdy, '#');

	attrset(A_REVERSE);
	dline(2, cx, cy, cx + hdx, cy - hdy, '.');
	attroff(A_REVERSE);

	if (has_colors())
	    attrset(COLOR_PAIR(1));

	dline(1, cx, cy, cx + sdx, cy - sdy, 'O');

	if (has_colors())
	    attrset(COLOR_PAIR(0));

	text = ctime(&tim);
	mvprintw(2, 0, "%.*s", (int) (strlen(text) - 1), text);
	refresh();
	if ((t->tm_sec % 5) == 0
	    && t->tm_sec != lastbeep) {
	    lastbeep = t->tm_sec;
	    if (has_colors()) {
		odd = !odd;
		bkgd((chtype) (odd ? COLOR_PAIR(4) : COLOR_PAIR(0)));
	    }
	    beep();
	}

	if ((ch = getch()) != ERR) {
#ifdef KEY_RESIZE
	    if (ch == KEY_RESIZE) {
		flash();
		erase();
		wrefresh(curscr);
		goto restart;
	    }
#endif
	    break;
	}

	dline(0, cx, cy, cx + hdx, cy - hdy, ' ');
	dline(0, cx, cy, cx + mdx, cy - mdy, ' ');
	dline(0, cx, cy, cx + sdx, cy - sdy, ' ');

    }

    curs_set(1);
    endwin();
    ExitProgram(EXIT_SUCCESS);
}

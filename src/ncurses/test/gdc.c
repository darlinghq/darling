/****************************************************************************
 * Copyright (c) 1998-2007,2008 Free Software Foundation, Inc.              *
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
 * Grand digital clock for curses compatible terminals
 * Usage: gdc [-s] [n]   -- run for n seconds (default infinity)
 * Flags: -s: scroll
 *
 * modified 10-18-89 for curses (jrl)
 * 10-18-89 added signal handling
 *
 * $Id: gdc.c,v 1.31 2008/08/03 23:58:42 tom Exp $
 */

#include <test.priv.h>

#include <time.h>

#define YBASE	10
#define XBASE	10
#define XLENGTH	54
#define YDEPTH	5

#define PAIR_DIGITS 1
#define PAIR_OTHERS 2
#define PAIR_FRAMES 3

static short disp[11] =
{
    075557, 011111, 071747, 071717, 055711,
    074717, 074757, 071111, 075757, 075717, 002020
};
static long older[6], next[6], newer[6], mask;

static int sigtermed = 0;
static bool redirected = FALSE;
static bool hascolor = FALSE;

static RETSIGTYPE
sighndl(int signo)
{
    signal(signo, sighndl);
    sigtermed = signo;
    if (redirected) {
	endwin();
	ExitProgram(EXIT_FAILURE);
    }
}

static void
drawbox(bool scrolling)
{
    chtype bottom[XLENGTH + 1];
    int n;

    if (hascolor)
	attrset(COLOR_PAIR(PAIR_FRAMES));

    mvaddch(YBASE - 1, XBASE - 1, ACS_ULCORNER);
    hline(ACS_HLINE, XLENGTH);
    mvaddch(YBASE - 1, XBASE + XLENGTH, ACS_URCORNER);

    mvaddch(YBASE + YDEPTH, XBASE - 1, ACS_LLCORNER);
    if ((mvinchnstr(YBASE + YDEPTH, XBASE, bottom, XLENGTH)) != ERR) {
	for (n = 0; n < XLENGTH; n++) {
	    if (!scrolling)
		bottom[n] &= ~A_COLOR;
	    bottom[n] = ACS_HLINE | (bottom[n] & (A_ATTRIBUTES | A_COLOR));
	}
	mvaddchnstr(YBASE + YDEPTH, XBASE, bottom, XLENGTH);
    }
    mvaddch(YBASE + YDEPTH, XBASE + XLENGTH, ACS_LRCORNER);

    move(YBASE, XBASE - 1);
    vline(ACS_VLINE, YDEPTH);

    move(YBASE, XBASE + XLENGTH);
    vline(ACS_VLINE, YDEPTH);

    if (hascolor)
	attrset(COLOR_PAIR(PAIR_OTHERS));
}

static void
standt(int on)
{
    if (on) {
	if (hascolor) {
	    attron(COLOR_PAIR(PAIR_DIGITS));
	} else {
	    attron(A_STANDOUT);
	}
    } else {
	if (hascolor) {
	    attron(COLOR_PAIR(PAIR_OTHERS));
	} else {
	    attroff(A_STANDOUT);
	}
    }
}

static void
set(int t, int n)
{
    int i, m;

    m = 7 << n;
    for (i = 0; i < 5; i++) {
	next[i] |= ((disp[t] >> ((4 - i) * 3)) & 07) << n;
	mask |= (next[i] ^ older[i]) & m;
    }
    if (mask & m)
	mask |= m;
}

static void
usage(void)
{
    static const char *msg[] =
    {
	"Usage: gdc [options] [count]"
	,""
	,"Options:"
	,"  -n  redirect input to /dev/null"
	,"  -s  scroll each number into place, rather than flipping"
	,""
	,"If you specify a count, gdc runs for that number of seconds"
    };
    unsigned j;
    for (j = 0; j < SIZEOF(msg); j++)
	fprintf(stderr, "%s\n", msg[j]);
    ExitProgram(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    time_t now;
    struct tm *tm;
    long t, a;
    int i, j, s, k;
    int count = 0;
    FILE *ofp = stdout;
    FILE *ifp = stdin;
    bool scrol = FALSE;

    setlocale(LC_ALL, "");

    CATCHALL(sighndl);

    while ((k = getopt(argc, argv, "sn")) != -1) {
	switch (k) {
	case 's':
	    scrol = TRUE;
	    break;
	case 'n':
	    ifp = fopen("/dev/null", "r");
	    redirected = TRUE;
	    break;
	default:
	    usage();
	}
    }
    if (optind < argc) {
	count = atoi(argv[optind++]);
	assert(count >= 0);
    }
    if (optind < argc)
	usage();

    if (redirected) {
	char *name = getenv("TERM");
	if (name == 0
	    || newterm(name, ofp, ifp) == 0) {
	    fprintf(stderr, "cannot open terminal\n");
	    ExitProgram(EXIT_FAILURE);
	}

    } else {
	initscr();
    }
    cbreak();
    noecho();
    nodelay(stdscr, 1);
    curs_set(0);

    hascolor = has_colors();

    if (hascolor) {
	int bg = COLOR_BLACK;
	start_color();
#if HAVE_USE_DEFAULT_COLORS
	if (use_default_colors() == OK)
	    bg = -1;
#endif
	init_pair(PAIR_DIGITS, COLOR_BLACK, COLOR_RED);
	init_pair(PAIR_OTHERS, COLOR_RED, bg);
	init_pair(PAIR_FRAMES, COLOR_WHITE, bg);
	attrset(COLOR_PAIR(PAIR_OTHERS));
    }

  restart:
    for (j = 0; j < 5; j++)
	older[j] = newer[j] = next[j] = 0;

    clear();
    drawbox(FALSE);

    do {
	char buf[30];

	time(&now);
	tm = localtime(&now);

	mask = 0;
	set(tm->tm_sec % 10, 0);
	set(tm->tm_sec / 10, 4);
	set(tm->tm_min % 10, 10);
	set(tm->tm_min / 10, 14);
	set(tm->tm_hour % 10, 20);
	set(tm->tm_hour / 10, 24);
	set(10, 7);
	set(10, 17);

	for (k = 0; k < 6; k++) {
	    if (scrol) {
		for (i = 0; i < 5; i++)
		    newer[i] = (newer[i] & ~mask) | (newer[i + 1] & mask);
		newer[5] = (newer[5] & ~mask) | (next[k] & mask);
	    } else
		newer[k] = (newer[k] & ~mask) | (next[k] & mask);
	    next[k] = 0;
	    for (s = 1; s >= 0; s--) {
		standt(s);
		for (i = 0; i < 6; i++) {
		    if ((a = (newer[i] ^ older[i]) & (s ? newer : older)[i])
			!= 0) {
			for (j = 0, t = 1 << 26; t; t >>= 1, j++) {
			    if (a & t) {
				if (!(a & (t << 1))) {
				    move(YBASE + i, XBASE + 2 * j);
				}
				addstr("  ");
			    }
			}
		    }
		    if (!s) {
			older[i] = newer[i];
		    }
		}
		if (!s) {
		    if (scrol)
			drawbox(TRUE);
		    refresh();
		    /*
		     * If we're scrolling, space out the refreshes to fake
		     * movement.  That's 7 frames, or 6 intervals, which would
		     * be 166 msec if we spread it out over a second.  It looks
		     * better (but will work on a slow terminal, e.g., less
		     * than 9600bd) to squeeze that into a half-second, and use
		     * half of 170 msec to ensure that the program doesn't eat
		     * a lot of time when asking what time it is, at the top of
		     * this loop -T.Dickey
		     */
		    if (scrol)
			napms(85);
		}
	    }
	}

	/* this depends on the detailed format of ctime(3) */
	(void) strcpy(buf, ctime(&now));
	(void) strcpy(buf + 10, buf + 19);
	mvaddstr(16, 30, buf);

	move(6, 0);
	drawbox(FALSE);
	refresh();

	/*
	 * If we're not scrolling, wait 1000 msec (1 sec).  Use napms() rather
	 * than sleep() because the latter does odd things on some systems,
	 * e.g., suspending output as well.
	 */
	if (scrol)
	    napms(500);
	else
	    napms(1000);

	/*
	 * This is a safe way to check if we're interrupted - making the signal
	 * handler set a flag that we can check.  Since we're running
	 * nodelay(), the wgetch() call returns immediately, and in particular
	 * will return an error if interrupted.  This works only if we can
	 * read from the input, of course.
	 */
	switch (wgetch(stdscr)) {
	case 'q':
	    count = 1;
	    break;
	case 's':
	    nodelay(stdscr, FALSE);
	    break;
	case ' ':
	    nodelay(stdscr, TRUE);
	    break;
#ifdef KEY_RESIZE
	case KEY_RESIZE:
#endif
	case '?':
	    goto restart;
	case ERR:
	    if (sigtermed) {
		standend();
		endwin();
		fprintf(stderr, "gdc terminated by signal %d\n", sigtermed);
		ExitProgram(EXIT_FAILURE);
	    }
	    /* FALLTHRU */
	default:
	    continue;
	}
    } while (--count);
    standend();
    endwin();
    ExitProgram(EXIT_SUCCESS);
}

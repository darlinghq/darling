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
 * $Id: savescreen.c,v 1.10 2007/07/21 17:57:37 tom Exp $
 *
 * Demonstrate save/restore functions from the curses library.
 * Thomas Dickey - 2007/7/14
 */

#include <test.priv.h>

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

static bool use_init = FALSE;

static void
setup_next(void)
{
    curs_set(1);
    reset_shell_mode();
}

static void
cleanup(char *files[])
{
    int n;

    for (n = 0; files[n] != 0; ++n) {
	unlink(files[n]);
    }
}

static int
load_screen(char *filename)
{
    int result;

    if (use_init) {
	if ((result = scr_init(filename)) != ERR)
	    result = scr_restore(filename);
    } else {
	result = scr_set(filename);
    }
    return result;
}

/*
 * scr_restore() or scr_set() operates on curscr.  If we read a character using
 * getch() that will refresh stdscr, wiping out the result.  To avoid that,
 * copy the data back from curscr to stdscr.
 */
static void
after_load(void)
{
    overwrite(curscr, stdscr);
    doupdate();
}

static void
show_what(int which, int last)
{
    int y, x;
    time_t now = time((time_t *) 0);

    getyx(stdscr, y, x);

    move(0, 0);
    printw("Saved %d of %d - %s", which, last + 1, ctime(&now));

    move(y, x);

    refresh();
}

static int
get_command(int which, int last)
{
    int ch;

    timeout(100);

    do {
	show_what(which, last);
	ch = getch();
    } while (ch == ERR);

    return ch;
}

static void
usage(void)
{
    static const char *msg[] =
    {
	"Usage: savescreen [-r] files",
	"",
	"Options:",
	" -i  use scr_init/scr_restore rather than scr_set",
	" -r  replay the screen-dump files"
    };
    unsigned n;
    for (n = 0; n < SIZEOF(msg); ++n) {
	fprintf(stderr, "%s\n", msg[n]);
    }
    ExitProgram(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int ch;
    int which = 0;
    int last;
    bool replaying = FALSE;
    bool done = FALSE;
    char **files;

    while ((ch = getopt(argc, argv, "ir")) != -1) {
	switch (ch) {
	case 'i':
	    use_init = TRUE;
	    break;
	case 'r':
	    replaying = TRUE;
	    break;
	default:
	    usage();
	    break;
	}
    }

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    if (has_colors()) {
	start_color();
	for (ch = 0; ch < COLOR_PAIRS; ++ch) {
	    short pair = ch % COLOR_PAIRS;
	    init_pair(pair, COLOR_WHITE, ch % COLORS);
	}
    }

    files = argv + optind;
    last = argc - optind - 1;
    if (replaying) {

	/*
	 * Use the last file as the initial/current screen.
	 */
	if (last < 0) {
	    endwin();
	    printf("No screen-dumps given\n");
	    ExitProgram(EXIT_FAILURE);
	}

	which = last;
	if (load_screen(files[which]) == ERR) {
	    endwin();
	    printf("Cannot load screen-dump %s\n", files[which]);
	    ExitProgram(EXIT_FAILURE);
	}
	after_load();

	while (!done && (ch = getch()) != ERR) {
	    switch (ch) {
	    case 'n':
		/*
		 * If we got a "next" here, skip to the final screen before
		 * moving to the next process.
		 */
		setup_next();
		which = last;
		done = TRUE;
		break;
	    case 'q':
		endwin();
		cleanup(files);
		done = TRUE;
		break;
	    case KEY_BACKSPACE:
	    case '\b':
		if (--which < 0)
		    which = last;
		break;
	    case ' ':
		if (++which > last)
		    which = 0;
		break;
	    default:
		beep();
		continue;
	    }

	    if (ch == 'q') {
		;
	    } else if (scr_restore(files[which]) == ERR) {
		endwin();
		printf("Cannot load screen-dump %s\n", files[which]);
		cleanup(files);
		ExitProgram(EXIT_FAILURE);
	    } else {
		wrefresh(curscr);
	    }
	}
    } else {
	int y;
	int x;

	move(2, 0);
	printw("Use h,j,k,l or arrows to move around the screen\n");
	printw("Press 'q' to quit, ' ' to dump a screen\n");
	printw("When the last screen has been dumped, press 'n' to run the\n");
	printw("screen-loader.  That allows only 'q', backspace and ' ' for\n");
	printw("stepping through the dumped/restored screens.\n");
	getyx(stdscr, y, x);

	while (!done) {
	    switch (ch = get_command(which, last)) {
	    case 'n':
		setup_next();
		done = TRUE;
		break;
	    case 'q':
		endwin();
		cleanup(files);
		done = TRUE;
		break;
	    case ' ':
		if (files[which] != 0) {
		    show_what(which + 1, last);
		    if (scr_dump(files[which]) == ERR) {
			endwin();
			printf("Cannot write screen-dump %s\n", files[which]);
			cleanup(files);
			done = TRUE;
			break;
		    }
		    ++which;
		    if (has_colors()) {
			short pair = which % COLOR_PAIRS;
			bkgd(COLOR_PAIR(pair));
		    }
		} else {
		    beep();
		}
		break;
	    case KEY_LEFT:
	    case 'h':
		if (--x < 0)
		    x = COLS - 1;
		break;
	    case KEY_DOWN:
	    case 'j':
		if (++y >= LINES)
		    y = 1;
		break;
	    case KEY_UP:
	    case 'k':
		if (--y < 1)
		    y = LINES - 1;
		break;
	    case KEY_RIGHT:
	    case 'l':
		if (++x >= COLS)
		    x = 0;
		break;
	    }
	    if (!done) {
		time_t now = time((time_t *) 0);

		move(0, 0);
		addstr(ctime(&now));
		move(y, x);
		addch('#' | A_REVERSE);
		move(y, x);
	    }
	}
    }
    ExitProgram(EXIT_SUCCESS);
}

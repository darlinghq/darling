/******************************************************************************/
/* asciixmas                                                                  */
/* December 1989             Larry Bartz           Indianapolis, IN           */
/*                                                                            */
/*                                                                            */
/* I'm dreaming of an ascii character-based monochrome Christmas,             */
/* Just like the one's I used to know!                                        */
/* Via a full duplex communications channel,                                  */
/* At 9600 bits per second,                                                   */
/* Even though it's kinda slow.                                               */
/*                                                                            */
/* I'm dreaming of an ascii character-based monochrome Christmas,             */
/* With ev'ry C program I write!                                              */
/* May your screen be merry and bright!                                       */
/* And may all your Christmases be amber or green,                            */
/* (for reduced eyestrain and improved visibility)!                           */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* IMPLEMENTATION                                                             */
/*                                                                            */
/* Feel free to modify the defined string FROMWHO to reflect you, your        */
/* organization, your site, whatever.                                         */
/*                                                                            */
/* This really looks a lot better if you can turn off your cursor before      */
/* execution. I wanted to do that here but very few termcap entries or        */
/* terminfo definitions have the appropriate string defined. If you know      */
/* the string(s) for the terminal(s) you use or which your site supports,     */
/* you could call asciixmas from within a shell in which you issue the        */
/* string to the terminal. The cursor is distracting but it doesn't really    */
/* ruin the show.                                                             */
/*                                                                            */
/* At our site, we invoke this for our users just after login and the         */
/* determination of terminal type.                                            */
/*                                                                            */
/*                                                                            */
/* PORTABILITY                                                                */
/*                                                                            */
/* I wrote this using only the very simplest curses functions so that it      */
/* might be the most portable. I was personally able to test on five          */
/* different cpu/UNIX combinations.                                           */
/*                                                                            */
/*                                                                            */
/* COMPILE                                                                    */
/*                                                                            */
/* usually this:                                                              */
/*                                                                            */
/* cc -O asciixmas.c -lcurses -o asciixmas -s                                 */
/*                                                                            */
/*                                                                            */
/* Zilog S8000 models 11, 21, 31, etc with ZEUS variant of SYSTEM III         */
/* maybe other SYSTEM III also:                                               */
/*                                                                            */
/* cc asciixmas.c -lcurses -ltermlib -o asciixmas -s                          */
/*                                                                            */
/* as above with optional "peephole optimizer" installed:                     */
/*                                                                            */
/* cc -O asciixmas.c -lcurses -ltermlib -o asciixmas -s                       */
/*                                                                            */
/*                                                                            */
/* Zilog S8000 models 32, 130 with WE32100 chip and SYS V, REL2               */
/* maybe 3B2 also?                                                            */
/*                                                                            */
/* cc -f -O -K sd asciixmas.c -lcurses -o asciixmas -s                        */
/*                                                                            */
/*                                                                            */
/* Pyramid, Sequent, any other "dual universe" types compile and execute      */
/* under either universe. The compile line for the ucb universe (as you       */
/* might expect) is the same as for SYS III UNIX:                             */
/*                                                                            */
/* cc -O asciixmas.c -lcurses -ltermlib -o asciixmas -s                       */
/*                                                                            */
/* The above compile will also hold true for other BSD systems. (I hope)      */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* For the Scrooges out there among you who don't want this thing to loop     */
/* forever (or until the user hits a key), insert this into your compile      */
/* line just after "cc" :                                                     */
/*                                                                            */
/* -DNOLOOP                                                                   */
/*                                                                            */
/* like so:                                                                   */
/*                                                                            */
/* cc -DNOLOOP -O asciixmas.c -lcurses -o asciixmas -s                        */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

/*
 * $Id: xmas.c,v 1.24 2008/08/03 11:08:59 tom Exp $
 */
#include <test.priv.h>

#define FROMWHO "Mark Hessling - (M.Hessling@gu.edu.au)"

static int my_bg = COLOR_BLACK;
static int y_pos, x_pos;

static WINDOW *treescrn;
static WINDOW *treescrn2;
static WINDOW *treescrn3;
static WINDOW *treescrn4;
static WINDOW *treescrn5;
static WINDOW *treescrn6;
static WINDOW *treescrn7;
static WINDOW *treescrn8;
static WINDOW *dotdeer0;
static WINDOW *stardeer0;
static WINDOW *lildeer0;
static WINDOW *lildeer1;
static WINDOW *lildeer2;
static WINDOW *lildeer3;
static WINDOW *middeer0;
static WINDOW *middeer1;
static WINDOW *middeer2;
static WINDOW *middeer3;
static WINDOW *bigdeer0;
static WINDOW *bigdeer1;
static WINDOW *bigdeer2;
static WINDOW *bigdeer3;
static WINDOW *bigdeer4;
static WINDOW *lookdeer0;
static WINDOW *lookdeer1;
static WINDOW *lookdeer2;
static WINDOW *lookdeer3;
static WINDOW *lookdeer4;
static WINDOW *w_holiday;
static WINDOW *w_del_msg;
static bool *my_pairs;

static int boxit(void);
static int seas(void);
static int greet(void);
static int fromwho(void);
static int tree(void);
static int balls(void);
static int star(void);
static int strng1(void);
static int strng2(void);
static int strng3(void);
static int strng4(void);
static int strng5(void);
static int reindeer(void);
static int blinkit(void);

static RETSIGTYPE done(int sig) GCC_NORETURN;

static void
set_color(WINDOW *win, chtype color)
{
    if (has_colors()) {
	int n = (color + 1);
	if (my_pairs == 0)
	    my_pairs = typeCalloc(bool, COLORS + 1);
	if (!my_pairs[n]) {
	    init_pair(n, color, my_bg);
	    my_pairs[n] = TRUE;
	}
	wattroff(win, A_COLOR);
	wattron(win, COLOR_PAIR(n));
    }
}

static void
unset_color(WINDOW *win)
{
    if (has_colors())
	wattrset(win, COLOR_PAIR(0));
}

static void
look_out(int msecs)
{
    napms(msecs);
    if (getch() != ERR) {
	beep();
	done(0);
    }
}

int
main(int argc GCC_UNUSED, char **argv GCC_UNUSED)
{
    int loopy;

    setlocale(LC_ALL, "");

    initscr();
    noecho();
    nonl();
    refresh();

    CATCHALL(done);

    if (has_colors()) {
	start_color();
#if HAVE_USE_DEFAULT_COLORS
	if (use_default_colors() == OK)
	    my_bg = -1;
#endif
    }
    curs_set(0);

    if ((treescrn = newwin(16, 27, 3, 53)) == 0 ||
	(treescrn2 = newwin(16, 27, 3, 53)) == 0 ||
	(treescrn3 = newwin(16, 27, 3, 53)) == 0 ||
	(treescrn4 = newwin(16, 27, 3, 53)) == 0 ||
	(treescrn5 = newwin(16, 27, 3, 53)) == 0 ||
	(treescrn6 = newwin(16, 27, 3, 53)) == 0 ||
	(treescrn7 = newwin(16, 27, 3, 53)) == 0 ||
	(treescrn8 = newwin(16, 27, 3, 53)) == 0 ||

	(dotdeer0 = newwin(3, 71, 0, 8)) == 0 ||

	(stardeer0 = newwin(4, 56, 0, 8)) == 0 ||

	(lildeer0 = newwin(7, 53, 0, 8)) == 0 ||
	(lildeer1 = newwin(2, 4, 0, 0)) == 0 ||
	(lildeer2 = newwin(2, 4, 0, 0)) == 0 ||
	(lildeer3 = newwin(2, 4, 0, 0)) == 0 ||

	(middeer0 = newwin(15, 42, 0, 8)) == 0 ||
	(middeer1 = newwin(3, 7, 0, 0)) == 0 ||
	(middeer2 = newwin(3, 7, 0, 0)) == 0 ||
	(middeer3 = newwin(3, 7, 0, 0)) == 0 ||

	(bigdeer0 = newwin(10, 23, 0, 0)) == 0 ||
	(bigdeer1 = newwin(10, 23, 0, 0)) == 0 ||
	(bigdeer2 = newwin(10, 23, 0, 0)) == 0 ||
	(bigdeer3 = newwin(10, 23, 0, 0)) == 0 ||
	(bigdeer4 = newwin(10, 23, 0, 0)) == 0 ||

	(lookdeer0 = newwin(10, 25, 0, 0)) == 0 ||
	(lookdeer1 = newwin(10, 25, 0, 0)) == 0 ||
	(lookdeer2 = newwin(10, 25, 0, 0)) == 0 ||
	(lookdeer3 = newwin(10, 25, 0, 0)) == 0 ||
	(lookdeer4 = newwin(10, 25, 0, 0)) == 0 ||

	(w_holiday = newwin(1, 26, 3, 27)) == 0 ||

	(w_del_msg = newwin(1, 19, 23, 60)) == 0) {
	endwin();
	fprintf(stderr, "Cannot create windows - screen too small\n");
	ExitProgram(EXIT_FAILURE);
    }

    mvwaddstr(w_del_msg, 0, 0, "Hit any key to quit");

    mvwaddstr(w_holiday, 0, 0, "H A P P Y  H O L I D A Y S");

    /* set up the windows for our various reindeer */

    /* lildeer1 */
    mvwaddch(lildeer1, 0, 0, (chtype) 'V');
    mvwaddch(lildeer1, 1, 0, (chtype) '@');
    mvwaddch(lildeer1, 1, 1, (chtype) '<');
    mvwaddch(lildeer1, 1, 2, (chtype) '>');
    mvwaddch(lildeer1, 1, 3, (chtype) '~');

    /* lildeer2 */
    mvwaddch(lildeer2, 0, 0, (chtype) 'V');
    mvwaddch(lildeer2, 1, 0, (chtype) '@');
    mvwaddch(lildeer2, 1, 1, (chtype) '|');
    mvwaddch(lildeer2, 1, 2, (chtype) '|');
    mvwaddch(lildeer2, 1, 3, (chtype) '~');

    /* lildeer3 */
    mvwaddch(lildeer3, 0, 0, (chtype) 'V');
    mvwaddch(lildeer3, 1, 0, (chtype) '@');
    mvwaddch(lildeer3, 1, 1, (chtype) '>');
    mvwaddch(lildeer3, 1, 2, (chtype) '<');
    mvwaddch(lildeer2, 1, 3, (chtype) '~');

    /* middeer1 */
    mvwaddch(middeer1, 0, 2, (chtype) 'y');
    mvwaddch(middeer1, 0, 3, (chtype) 'y');
    mvwaddch(middeer1, 1, 2, (chtype) '0');
    mvwaddch(middeer1, 1, 3, (chtype) '(');
    mvwaddch(middeer1, 1, 4, (chtype) '=');
    mvwaddch(middeer1, 1, 5, (chtype) ')');
    mvwaddch(middeer1, 1, 6, (chtype) '~');
    mvwaddch(middeer1, 2, 3, (chtype) '\\');
    mvwaddch(middeer1, 2, 4, (chtype) '/');

    /* middeer2 */
    mvwaddch(middeer2, 0, 2, (chtype) 'y');
    mvwaddch(middeer2, 0, 3, (chtype) 'y');
    mvwaddch(middeer2, 1, 2, (chtype) '0');
    mvwaddch(middeer2, 1, 3, (chtype) '(');
    mvwaddch(middeer2, 1, 4, (chtype) '=');
    mvwaddch(middeer2, 1, 5, (chtype) ')');
    mvwaddch(middeer2, 1, 6, (chtype) '~');
    mvwaddch(middeer2, 2, 3, (chtype) '|');
    mvwaddch(middeer2, 2, 5, (chtype) '|');

    /* middeer3 */
    mvwaddch(middeer3, 0, 2, (chtype) 'y');
    mvwaddch(middeer3, 0, 3, (chtype) 'y');
    mvwaddch(middeer3, 1, 2, (chtype) '0');
    mvwaddch(middeer3, 1, 3, (chtype) '(');
    mvwaddch(middeer3, 1, 4, (chtype) '=');
    mvwaddch(middeer3, 1, 5, (chtype) ')');
    mvwaddch(middeer3, 1, 6, (chtype) '~');
    mvwaddch(middeer3, 2, 2, (chtype) '/');
    mvwaddch(middeer3, 2, 6, (chtype) '\\');

    /* bigdeer1 */
    mvwaddch(bigdeer1, 0, 17, (chtype) '\\');
    mvwaddch(bigdeer1, 0, 18, (chtype) '/');
    mvwaddch(bigdeer1, 0, 20, (chtype) '\\');
    mvwaddch(bigdeer1, 0, 21, (chtype) '/');
    mvwaddch(bigdeer1, 1, 18, (chtype) '\\');
    mvwaddch(bigdeer1, 1, 20, (chtype) '/');
    mvwaddch(bigdeer1, 2, 19, (chtype) '|');
    mvwaddch(bigdeer1, 2, 20, (chtype) '_');
    mvwaddch(bigdeer1, 3, 18, (chtype) '/');
    mvwaddch(bigdeer1, 3, 19, (chtype) '^');
    mvwaddch(bigdeer1, 3, 20, (chtype) '0');
    mvwaddch(bigdeer1, 3, 21, (chtype) '\\');
    mvwaddch(bigdeer1, 4, 17, (chtype) '/');
    mvwaddch(bigdeer1, 4, 18, (chtype) '/');
    mvwaddch(bigdeer1, 4, 19, (chtype) '\\');
    mvwaddch(bigdeer1, 4, 22, (chtype) '\\');
    mvwaddstr(bigdeer1, 5, 7, "^~~~~~~~~//  ~~U");
    mvwaddstr(bigdeer1, 6, 7, "( \\_____( /");
    mvwaddstr(bigdeer1, 7, 8, "( )    /");
    mvwaddstr(bigdeer1, 8, 9, "\\\\   /");
    mvwaddstr(bigdeer1, 9, 11, "\\>/>");

    /* bigdeer2 */
    mvwaddch(bigdeer2, 0, 17, (chtype) '\\');
    mvwaddch(bigdeer2, 0, 18, (chtype) '/');
    mvwaddch(bigdeer2, 0, 20, (chtype) '\\');
    mvwaddch(bigdeer2, 0, 21, (chtype) '/');
    mvwaddch(bigdeer2, 1, 18, (chtype) '\\');
    mvwaddch(bigdeer2, 1, 20, (chtype) '/');
    mvwaddch(bigdeer2, 2, 19, (chtype) '|');
    mvwaddch(bigdeer2, 2, 20, (chtype) '_');
    mvwaddch(bigdeer2, 3, 18, (chtype) '/');
    mvwaddch(bigdeer2, 3, 19, (chtype) '^');
    mvwaddch(bigdeer2, 3, 20, (chtype) '0');
    mvwaddch(bigdeer2, 3, 21, (chtype) '\\');
    mvwaddch(bigdeer2, 4, 17, (chtype) '/');
    mvwaddch(bigdeer2, 4, 18, (chtype) '/');
    mvwaddch(bigdeer2, 4, 19, (chtype) '\\');
    mvwaddch(bigdeer2, 4, 22, (chtype) '\\');
    mvwaddstr(bigdeer2, 5, 7, "^~~~~~~~~//  ~~U");
    mvwaddstr(bigdeer2, 6, 7, "(( )____( /");
    mvwaddstr(bigdeer2, 7, 7, "( /      |");
    mvwaddstr(bigdeer2, 8, 8, "\\/      |");
    mvwaddstr(bigdeer2, 9, 9, "|>     |>");

    /* bigdeer3 */
    mvwaddch(bigdeer3, 0, 17, (chtype) '\\');
    mvwaddch(bigdeer3, 0, 18, (chtype) '/');
    mvwaddch(bigdeer3, 0, 20, (chtype) '\\');
    mvwaddch(bigdeer3, 0, 21, (chtype) '/');
    mvwaddch(bigdeer3, 1, 18, (chtype) '\\');
    mvwaddch(bigdeer3, 1, 20, (chtype) '/');
    mvwaddch(bigdeer3, 2, 19, (chtype) '|');
    mvwaddch(bigdeer3, 2, 20, (chtype) '_');
    mvwaddch(bigdeer3, 3, 18, (chtype) '/');
    mvwaddch(bigdeer3, 3, 19, (chtype) '^');
    mvwaddch(bigdeer3, 3, 20, (chtype) '0');
    mvwaddch(bigdeer3, 3, 21, (chtype) '\\');
    mvwaddch(bigdeer3, 4, 17, (chtype) '/');
    mvwaddch(bigdeer3, 4, 18, (chtype) '/');
    mvwaddch(bigdeer3, 4, 19, (chtype) '\\');
    mvwaddch(bigdeer3, 4, 22, (chtype) '\\');
    mvwaddstr(bigdeer3, 5, 7, "^~~~~~~~~//  ~~U");
    mvwaddstr(bigdeer3, 6, 6, "( ()_____( /");
    mvwaddstr(bigdeer3, 7, 6, "/ /       /");
    mvwaddstr(bigdeer3, 8, 5, "|/          \\");
    mvwaddstr(bigdeer3, 9, 5, "/>           \\>");

    /* bigdeer4 */
    mvwaddch(bigdeer4, 0, 17, (chtype) '\\');
    mvwaddch(bigdeer4, 0, 18, (chtype) '/');
    mvwaddch(bigdeer4, 0, 20, (chtype) '\\');
    mvwaddch(bigdeer4, 0, 21, (chtype) '/');
    mvwaddch(bigdeer4, 1, 18, (chtype) '\\');
    mvwaddch(bigdeer4, 1, 20, (chtype) '/');
    mvwaddch(bigdeer4, 2, 19, (chtype) '|');
    mvwaddch(bigdeer4, 2, 20, (chtype) '_');
    mvwaddch(bigdeer4, 3, 18, (chtype) '/');
    mvwaddch(bigdeer4, 3, 19, (chtype) '^');
    mvwaddch(bigdeer4, 3, 20, (chtype) '0');
    mvwaddch(bigdeer4, 3, 21, (chtype) '\\');
    mvwaddch(bigdeer4, 4, 17, (chtype) '/');
    mvwaddch(bigdeer4, 4, 18, (chtype) '/');
    mvwaddch(bigdeer4, 4, 19, (chtype) '\\');
    mvwaddch(bigdeer4, 4, 22, (chtype) '\\');
    mvwaddstr(bigdeer4, 5, 7, "^~~~~~~~~//  ~~U");
    mvwaddstr(bigdeer4, 6, 6, "( )______( /");
    mvwaddstr(bigdeer4, 7, 5, "(/          \\");
    mvwaddstr(bigdeer4, 8, 0, "v___=             ----^");

    /* lookdeer1 */
    mvwaddstr(lookdeer1, 0, 16, "\\/     \\/");
    mvwaddstr(lookdeer1, 1, 17, "\\Y/ \\Y/");
    mvwaddstr(lookdeer1, 2, 19, "\\=/");
    mvwaddstr(lookdeer1, 3, 17, "^\\o o/^");
    mvwaddstr(lookdeer1, 4, 17, "//( )");
    mvwaddstr(lookdeer1, 5, 7, "^~~~~~~~~// \\O/");
    mvwaddstr(lookdeer1, 6, 7, "( \\_____( /");
    mvwaddstr(lookdeer1, 7, 8, "( )    /");
    mvwaddstr(lookdeer1, 8, 9, "\\\\   /");
    mvwaddstr(lookdeer1, 9, 11, "\\>/>");

    /* lookdeer2 */
    mvwaddstr(lookdeer2, 0, 16, "\\/     \\/");
    mvwaddstr(lookdeer2, 1, 17, "\\Y/ \\Y/");
    mvwaddstr(lookdeer2, 2, 19, "\\=/");
    mvwaddstr(lookdeer2, 3, 17, "^\\o o/^");
    mvwaddstr(lookdeer2, 4, 17, "//( )");
    mvwaddstr(lookdeer2, 5, 7, "^~~~~~~~~// \\O/");
    mvwaddstr(lookdeer2, 6, 7, "(( )____( /");
    mvwaddstr(lookdeer2, 7, 7, "( /      |");
    mvwaddstr(lookdeer2, 8, 8, "\\/      |");
    mvwaddstr(lookdeer2, 9, 9, "|>     |>");

    /* lookdeer3 */
    mvwaddstr(lookdeer3, 0, 16, "\\/     \\/");
    mvwaddstr(lookdeer3, 1, 17, "\\Y/ \\Y/");
    mvwaddstr(lookdeer3, 2, 19, "\\=/");
    mvwaddstr(lookdeer3, 3, 17, "^\\o o/^");
    mvwaddstr(lookdeer3, 4, 17, "//( )");
    mvwaddstr(lookdeer3, 5, 7, "^~~~~~~~~// \\O/");
    mvwaddstr(lookdeer3, 6, 6, "( ()_____( /");
    mvwaddstr(lookdeer3, 7, 6, "/ /       /");
    mvwaddstr(lookdeer3, 8, 5, "|/          \\");
    mvwaddstr(lookdeer3, 9, 5, "/>           \\>");

    /* lookdeer4 */
    mvwaddstr(lookdeer4, 0, 16, "\\/     \\/");
    mvwaddstr(lookdeer4, 1, 17, "\\Y/ \\Y/");
    mvwaddstr(lookdeer4, 2, 19, "\\=/");
    mvwaddstr(lookdeer4, 3, 17, "^\\o o/^");
    mvwaddstr(lookdeer4, 4, 17, "//( )");
    mvwaddstr(lookdeer4, 5, 7, "^~~~~~~~~// \\O/");
    mvwaddstr(lookdeer4, 6, 6, "( )______( /");
    mvwaddstr(lookdeer4, 7, 5, "(/          \\");
    mvwaddstr(lookdeer4, 8, 0, "v___=             ----^");

	/***********************************************/
    cbreak();
    nodelay(stdscr, TRUE);
    for (;;) {
	clear();
	werase(treescrn);
	touchwin(w_del_msg);
	touchwin(treescrn);
	werase(treescrn2);
	touchwin(treescrn2);
	werase(treescrn8);
	touchwin(treescrn8);
	refresh();
	look_out(150);
	boxit();
	refresh();
	look_out(150);
	seas();
	refresh();
	look_out(150);
	greet();
	refresh();
	look_out(150);
	fromwho();
	refresh();
	look_out(150);
	tree();
	look_out(150);
	balls();
	look_out(150);
	star();
	look_out(150);
	strng1();
	strng2();
	strng3();
	strng4();
	strng5();

	/* set up the windows for our blinking trees */
	/* **************************************** */
	/* treescrn3 */

	overlay(treescrn, treescrn3);

	/*balls */
	mvwaddch(treescrn3, 4, 18, ' ');
	mvwaddch(treescrn3, 7, 6, ' ');
	mvwaddch(treescrn3, 8, 19, ' ');
	mvwaddch(treescrn3, 11, 22, ' ');

	/*star */
	mvwaddch(treescrn3, 0, 12, '*');

	/*strng1 */
	mvwaddch(treescrn3, 3, 11, ' ');

	/*strng2 */
	mvwaddch(treescrn3, 5, 13, ' ');
	mvwaddch(treescrn3, 6, 10, ' ');

	/*strng3 */
	mvwaddch(treescrn3, 7, 16, ' ');
	mvwaddch(treescrn3, 7, 14, ' ');

	/*strng4 */
	mvwaddch(treescrn3, 10, 13, ' ');
	mvwaddch(treescrn3, 10, 10, ' ');
	mvwaddch(treescrn3, 11, 8, ' ');

	/*strng5 */
	mvwaddch(treescrn3, 11, 18, ' ');
	mvwaddch(treescrn3, 12, 13, ' ');

	/* treescrn4 */

	overlay(treescrn, treescrn4);

	/*balls */
	mvwaddch(treescrn4, 3, 9, ' ');
	mvwaddch(treescrn4, 4, 16, ' ');
	mvwaddch(treescrn4, 7, 6, ' ');
	mvwaddch(treescrn4, 8, 19, ' ');
	mvwaddch(treescrn4, 11, 2, ' ');
	mvwaddch(treescrn4, 12, 23, ' ');

	/*star */
	wstandout(treescrn4);
	mvwaddch(treescrn4, 0, 12, '*');
	wstandend(treescrn4);

	/*strng1 */
	mvwaddch(treescrn4, 3, 13, ' ');

	/*strng2 */

	/*strng3 */
	mvwaddch(treescrn4, 7, 15, ' ');
	mvwaddch(treescrn4, 8, 11, ' ');

	/*strng4 */
	mvwaddch(treescrn4, 9, 16, ' ');
	mvwaddch(treescrn4, 10, 12, ' ');
	mvwaddch(treescrn4, 11, 8, ' ');

	/*strng5 */
	mvwaddch(treescrn4, 11, 18, ' ');
	mvwaddch(treescrn4, 12, 14, ' ');

	/* treescrn5 */

	overlay(treescrn, treescrn5);

	/*balls */
	mvwaddch(treescrn5, 3, 15, ' ');
	mvwaddch(treescrn5, 10, 20, ' ');
	mvwaddch(treescrn5, 12, 1, ' ');

	/*star */
	mvwaddch(treescrn5, 0, 12, '*');

	/*strng1 */
	mvwaddch(treescrn5, 3, 11, ' ');

	/*strng2 */
	mvwaddch(treescrn5, 5, 12, ' ');

	/*strng3 */
	mvwaddch(treescrn5, 7, 14, ' ');
	mvwaddch(treescrn5, 8, 10, ' ');

	/*strng4 */
	mvwaddch(treescrn5, 9, 15, ' ');
	mvwaddch(treescrn5, 10, 11, ' ');
	mvwaddch(treescrn5, 11, 7, ' ');

	/*strng5 */
	mvwaddch(treescrn5, 11, 17, ' ');
	mvwaddch(treescrn5, 12, 13, ' ');

	/* treescrn6 */

	overlay(treescrn, treescrn6);

	/*balls */
	mvwaddch(treescrn6, 6, 7, ' ');
	mvwaddch(treescrn6, 7, 18, ' ');
	mvwaddch(treescrn6, 10, 4, ' ');
	mvwaddch(treescrn6, 11, 23, ' ');

	/*star */
	wstandout(treescrn6);
	mvwaddch(treescrn6, 0, 12, '*');
	wstandend(treescrn6);

	/*strng1 */

	/*strng2 */
	mvwaddch(treescrn6, 5, 11, ' ');

	/*strng3 */
	mvwaddch(treescrn6, 7, 13, ' ');
	mvwaddch(treescrn6, 8, 9, ' ');

	/*strng4 */
	mvwaddch(treescrn6, 9, 14, ' ');
	mvwaddch(treescrn6, 10, 10, ' ');
	mvwaddch(treescrn6, 11, 6, ' ');

	/*strng5 */
	mvwaddch(treescrn6, 11, 16, ' ');
	mvwaddch(treescrn6, 12, 12, ' ');

	/* treescrn7 */

	overlay(treescrn, treescrn7);

	/*balls */
	mvwaddch(treescrn7, 3, 15, ' ');
	mvwaddch(treescrn7, 6, 7, ' ');
	mvwaddch(treescrn7, 7, 18, ' ');
	mvwaddch(treescrn7, 10, 4, ' ');
	mvwaddch(treescrn7, 11, 22, ' ');

	/*star */
	mvwaddch(treescrn7, 0, 12, '*');

	/*strng1 */
	mvwaddch(treescrn7, 3, 12, ' ');

	/*strng2 */
	mvwaddch(treescrn7, 5, 13, ' ');
	mvwaddch(treescrn7, 6, 9, ' ');

	/*strng3 */
	mvwaddch(treescrn7, 7, 15, ' ');
	mvwaddch(treescrn7, 8, 11, ' ');

	/*strng4 */
	mvwaddch(treescrn7, 9, 16, ' ');
	mvwaddch(treescrn7, 10, 12, ' ');
	mvwaddch(treescrn7, 11, 8, ' ');

	/*strng5 */
	mvwaddch(treescrn7, 11, 18, ' ');
	mvwaddch(treescrn7, 12, 14, ' ');

	look_out(150);
	reindeer();

	touchwin(w_holiday);
	wrefresh(w_holiday);
	wrefresh(w_del_msg);

	look_out(500);
	for (loopy = 0; loopy < 100; loopy++) {
	    blinkit();
	}

#ifdef NOLOOP
	done(0);
#endif
    }
    /*NOTREACHED */
}

static int
boxit(void)
{
    int x = 0;

    while (x < 20) {
	mvaddch(x, 7, '|');
	++x;
    }

    x = 8;

    while (x < 80) {
	mvaddch(19, x, '_');
	++x;
    }

    x = 0;

    while (x < 80) {
	mvaddch(22, x, '_');
	++x;
    }

    return (0);
}

static int
seas(void)
{
    mvaddch(4, 1, 'S');
    mvaddch(6, 1, 'E');
    mvaddch(8, 1, 'A');
    mvaddch(10, 1, 'S');
    mvaddch(12, 1, 'O');
    mvaddch(14, 1, 'N');
    mvaddch(16, 1, '`');
    mvaddch(18, 1, 'S');

    return (0);
}

static int
greet(void)
{
    mvaddch(3, 5, 'G');
    mvaddch(5, 5, 'R');
    mvaddch(7, 5, 'E');
    mvaddch(9, 5, 'E');
    mvaddch(11, 5, 'T');
    mvaddch(13, 5, 'I');
    mvaddch(15, 5, 'N');
    mvaddch(17, 5, 'G');
    mvaddch(19, 5, 'S');

    return (0);
}

static int
fromwho(void)
{
    mvaddstr(21, 13, FROMWHO);
    return (0);
}

static int
tree(void)
{
    set_color(treescrn, COLOR_GREEN);
    mvwaddch(treescrn, 1, 11, (chtype) '/');
    mvwaddch(treescrn, 2, 11, (chtype) '/');
    mvwaddch(treescrn, 3, 10, (chtype) '/');
    mvwaddch(treescrn, 4, 9, (chtype) '/');
    mvwaddch(treescrn, 5, 9, (chtype) '/');
    mvwaddch(treescrn, 6, 8, (chtype) '/');
    mvwaddch(treescrn, 7, 7, (chtype) '/');
    mvwaddch(treescrn, 8, 6, (chtype) '/');
    mvwaddch(treescrn, 9, 6, (chtype) '/');
    mvwaddch(treescrn, 10, 5, (chtype) '/');
    mvwaddch(treescrn, 11, 3, (chtype) '/');
    mvwaddch(treescrn, 12, 2, (chtype) '/');

    mvwaddch(treescrn, 1, 13, (chtype) '\\');
    mvwaddch(treescrn, 2, 13, (chtype) '\\');
    mvwaddch(treescrn, 3, 14, (chtype) '\\');
    mvwaddch(treescrn, 4, 15, (chtype) '\\');
    mvwaddch(treescrn, 5, 15, (chtype) '\\');
    mvwaddch(treescrn, 6, 16, (chtype) '\\');
    mvwaddch(treescrn, 7, 17, (chtype) '\\');
    mvwaddch(treescrn, 8, 18, (chtype) '\\');
    mvwaddch(treescrn, 9, 18, (chtype) '\\');
    mvwaddch(treescrn, 10, 19, (chtype) '\\');
    mvwaddch(treescrn, 11, 21, (chtype) '\\');
    mvwaddch(treescrn, 12, 22, (chtype) '\\');

    mvwaddch(treescrn, 4, 10, (chtype) '_');
    mvwaddch(treescrn, 4, 14, (chtype) '_');
    mvwaddch(treescrn, 8, 7, (chtype) '_');
    mvwaddch(treescrn, 8, 17, (chtype) '_');

    mvwaddstr(treescrn, 13, 0, "//////////// \\\\\\\\\\\\\\\\\\\\\\\\");

    mvwaddstr(treescrn, 14, 11, "| |");
    mvwaddstr(treescrn, 15, 11, "|_|");

    unset_color(treescrn);
    wrefresh(treescrn);
    wrefresh(w_del_msg);

    return (0);
}

static int
balls(void)
{
    overlay(treescrn, treescrn2);

    set_color(treescrn2, COLOR_BLUE);
    mvwaddch(treescrn2, 3, 9, (chtype) '@');
    mvwaddch(treescrn2, 3, 15, (chtype) '@');
    mvwaddch(treescrn2, 4, 8, (chtype) '@');
    mvwaddch(treescrn2, 4, 16, (chtype) '@');
    mvwaddch(treescrn2, 5, 7, (chtype) '@');
    mvwaddch(treescrn2, 5, 17, (chtype) '@');
    mvwaddch(treescrn2, 7, 6, (chtype) '@');
    mvwaddch(treescrn2, 7, 18, (chtype) '@');
    mvwaddch(treescrn2, 8, 5, (chtype) '@');
    mvwaddch(treescrn2, 8, 19, (chtype) '@');
    mvwaddch(treescrn2, 10, 4, (chtype) '@');
    mvwaddch(treescrn2, 10, 20, (chtype) '@');
    mvwaddch(treescrn2, 11, 2, (chtype) '@');
    mvwaddch(treescrn2, 11, 22, (chtype) '@');
    mvwaddch(treescrn2, 12, 1, (chtype) '@');
    mvwaddch(treescrn2, 12, 23, (chtype) '@');

    unset_color(treescrn2);
    wrefresh(treescrn2);
    wrefresh(w_del_msg);
    return (0);
}

static int
star(void)
{
    wattrset(treescrn2, A_BOLD | A_BLINK);
    set_color(treescrn2, COLOR_YELLOW);

    mvwaddch(treescrn2, 0, 12, (chtype) '*');
    wstandend(treescrn2);

    unset_color(treescrn2);
    wrefresh(treescrn2);
    wrefresh(w_del_msg);
    return (0);
}

static int
strng1(void)
{
    wattrset(treescrn2, A_BOLD | A_BLINK);
    set_color(treescrn2, COLOR_WHITE);

    mvwaddch(treescrn2, 3, 13, (chtype) '\'');
    mvwaddch(treescrn2, 3, 12, (chtype) ':');
    mvwaddch(treescrn2, 3, 11, (chtype) '.');

    wattroff(treescrn2, A_BOLD | A_BLINK);
    unset_color(treescrn2);

    wrefresh(treescrn2);
    wrefresh(w_del_msg);
    return (0);
}

static int
strng2(void)
{
    wattrset(treescrn2, A_BOLD | A_BLINK);
    set_color(treescrn2, COLOR_WHITE);

    mvwaddch(treescrn2, 5, 14, (chtype) '\'');
    mvwaddch(treescrn2, 5, 13, (chtype) ':');
    mvwaddch(treescrn2, 5, 12, (chtype) '.');
    mvwaddch(treescrn2, 5, 11, (chtype) ',');
    mvwaddch(treescrn2, 6, 10, (chtype) '\'');
    mvwaddch(treescrn2, 6, 9, (chtype) ':');

    wattroff(treescrn2, A_BOLD | A_BLINK);
    unset_color(treescrn2);

    wrefresh(treescrn2);
    wrefresh(w_del_msg);
    return (0);
}

static int
strng3(void)
{
    wattrset(treescrn2, A_BOLD | A_BLINK);
    set_color(treescrn2, COLOR_WHITE);

    mvwaddch(treescrn2, 7, 16, (chtype) '\'');
    mvwaddch(treescrn2, 7, 15, (chtype) ':');
    mvwaddch(treescrn2, 7, 14, (chtype) '.');
    mvwaddch(treescrn2, 7, 13, (chtype) ',');
    mvwaddch(treescrn2, 8, 12, (chtype) '\'');
    mvwaddch(treescrn2, 8, 11, (chtype) ':');
    mvwaddch(treescrn2, 8, 10, (chtype) '.');
    mvwaddch(treescrn2, 8, 9, (chtype) ',');

    wattroff(treescrn2, A_BOLD | A_BLINK);
    unset_color(treescrn2);

    wrefresh(treescrn2);
    wrefresh(w_del_msg);
    return (0);
}

static int
strng4(void)
{
    wattrset(treescrn2, A_BOLD | A_BLINK);
    set_color(treescrn2, COLOR_WHITE);

    mvwaddch(treescrn2, 9, 17, (chtype) '\'');
    mvwaddch(treescrn2, 9, 16, (chtype) ':');
    mvwaddch(treescrn2, 9, 15, (chtype) '.');
    mvwaddch(treescrn2, 9, 14, (chtype) ',');
    mvwaddch(treescrn2, 10, 13, (chtype) '\'');
    mvwaddch(treescrn2, 10, 12, (chtype) ':');
    mvwaddch(treescrn2, 10, 11, (chtype) '.');
    mvwaddch(treescrn2, 10, 10, (chtype) ',');
    mvwaddch(treescrn2, 11, 9, (chtype) '\'');
    mvwaddch(treescrn2, 11, 8, (chtype) ':');
    mvwaddch(treescrn2, 11, 7, (chtype) '.');
    mvwaddch(treescrn2, 11, 6, (chtype) ',');
    mvwaddch(treescrn2, 12, 5, (chtype) '\'');

    wattroff(treescrn2, A_BOLD | A_BLINK);
    unset_color(treescrn2);

    wrefresh(treescrn2);
    wrefresh(w_del_msg);
    return (0);
}

static int
strng5(void)
{
    wattrset(treescrn2, A_BOLD | A_BLINK);
    set_color(treescrn2, COLOR_WHITE);

    mvwaddch(treescrn2, 11, 19, (chtype) '\'');
    mvwaddch(treescrn2, 11, 18, (chtype) ':');
    mvwaddch(treescrn2, 11, 17, (chtype) '.');
    mvwaddch(treescrn2, 11, 16, (chtype) ',');
    mvwaddch(treescrn2, 12, 15, (chtype) '\'');
    mvwaddch(treescrn2, 12, 14, (chtype) ':');
    mvwaddch(treescrn2, 12, 13, (chtype) '.');
    mvwaddch(treescrn2, 12, 12, (chtype) ',');

    wattroff(treescrn2, A_BOLD | A_BLINK);
    unset_color(treescrn2);

    /* save a fully lit tree */
    overlay(treescrn2, treescrn);

    wrefresh(treescrn2);
    wrefresh(w_del_msg);
    return (0);
}

static int
blinkit(void)
{
    static int cycle;

    if (cycle > 4) {
	cycle = 0;
    }

    touchwin(treescrn8);

    switch (cycle) {
    case 0:
	overlay(treescrn3, treescrn8);
	wrefresh(treescrn8);
	wrefresh(w_del_msg);
	break;
    case 1:
	overlay(treescrn4, treescrn8);
	wrefresh(treescrn8);
	wrefresh(w_del_msg);
	break;
    case 2:
	overlay(treescrn5, treescrn8);
	wrefresh(treescrn8);
	wrefresh(w_del_msg);
	break;
    case 3:
	overlay(treescrn6, treescrn8);
	wrefresh(treescrn8);
	wrefresh(w_del_msg);
	break;
    case 4:
	overlay(treescrn7, treescrn8);
	wrefresh(treescrn8);
	wrefresh(w_del_msg);
	break;
    }
    touchwin(treescrn8);

    /*ALL ON************************************************** */

    overlay(treescrn, treescrn8);
    wrefresh(treescrn8);
    wrefresh(w_del_msg);

    ++cycle;
    return (0);
}

static void
deer_step(WINDOW *win, int y, int x)
{
    mvwin(win, y, x);
    wrefresh(win);
    wrefresh(w_del_msg);
    look_out(5);
}

static int
reindeer(void)
{
    int looper;
    y_pos = 0;

    for (x_pos = 70; x_pos > 62; x_pos--) {
	if (x_pos < 62) {
	    y_pos = 1;
	}
	for (looper = 0; looper < 4; looper++) {
	    mvwaddch(dotdeer0, y_pos, x_pos, (chtype) '.');
	    wrefresh(dotdeer0);
	    wrefresh(w_del_msg);
	    werase(dotdeer0);
	    wrefresh(dotdeer0);
	    wrefresh(w_del_msg);
	    look_out(50);
	}
    }

    y_pos = 2;

    for (; x_pos > 50; x_pos--) {
	for (looper = 0; looper < 4; looper++) {

	    if (x_pos < 56) {
		y_pos = 3;

		mvwaddch(stardeer0, y_pos, x_pos, (chtype) '*');
		wrefresh(stardeer0);
		wrefresh(w_del_msg);
		werase(stardeer0);
		wrefresh(stardeer0);
		wrefresh(w_del_msg);
	    } else {
		mvwaddch(dotdeer0, y_pos, x_pos, (chtype) '*');
		wrefresh(dotdeer0);
		wrefresh(w_del_msg);
		werase(dotdeer0);
		wrefresh(dotdeer0);
		wrefresh(w_del_msg);
	    }
	}
    }

    x_pos = 58;

    for (y_pos = 2; y_pos < 5; y_pos++) {

	touchwin(lildeer0);
	wrefresh(lildeer0);
	wrefresh(w_del_msg);

	for (looper = 0; looper < 4; looper++) {
	    deer_step(lildeer3, y_pos, x_pos);
	    deer_step(lildeer2, y_pos, x_pos);
	    deer_step(lildeer1, y_pos, x_pos);
	    deer_step(lildeer2, y_pos, x_pos);
	    deer_step(lildeer3, y_pos, x_pos);

	    touchwin(lildeer0);
	    wrefresh(lildeer0);
	    wrefresh(w_del_msg);

	    x_pos -= 2;
	}
    }

    x_pos = 35;

    for (y_pos = 5; y_pos < 10; y_pos++) {

	touchwin(middeer0);
	wrefresh(middeer0);
	wrefresh(w_del_msg);

	for (looper = 0; looper < 2; looper++) {
	    deer_step(middeer3, y_pos, x_pos);
	    deer_step(middeer2, y_pos, x_pos);
	    deer_step(middeer1, y_pos, x_pos);
	    deer_step(middeer2, y_pos, x_pos);
	    deer_step(middeer3, y_pos, x_pos);

	    touchwin(middeer0);
	    wrefresh(middeer0);
	    wrefresh(w_del_msg);

	    x_pos -= 3;
	}
    }

    look_out(300);

    y_pos = 1;

    for (x_pos = 8; x_pos < 16; x_pos++) {
	deer_step(bigdeer4, y_pos, x_pos);
	deer_step(bigdeer3, y_pos, x_pos);
	deer_step(bigdeer2, y_pos, x_pos);
	deer_step(bigdeer1, y_pos, x_pos);
	deer_step(bigdeer2, y_pos, x_pos);
	deer_step(bigdeer3, y_pos, x_pos);
	deer_step(bigdeer4, y_pos, x_pos);
	deer_step(bigdeer0, y_pos, x_pos);
    }

    --x_pos;

    for (looper = 0; looper < 6; looper++) {
	deer_step(lookdeer4, y_pos, x_pos);
	deer_step(lookdeer3, y_pos, x_pos);
	deer_step(lookdeer2, y_pos, x_pos);
	deer_step(lookdeer1, y_pos, x_pos);
	deer_step(lookdeer2, y_pos, x_pos);
	deer_step(lookdeer3, y_pos, x_pos);
	deer_step(lookdeer4, y_pos, x_pos);
    }

    deer_step(lookdeer0, y_pos, x_pos);

    for (; y_pos < 10; y_pos++) {
	for (looper = 0; looper < 2; looper++) {
	    deer_step(bigdeer4, y_pos, x_pos);
	    deer_step(bigdeer3, y_pos, x_pos);
	    deer_step(bigdeer2, y_pos, x_pos);
	    deer_step(bigdeer1, y_pos, x_pos);
	    deer_step(bigdeer2, y_pos, x_pos);
	    deer_step(bigdeer3, y_pos, x_pos);
	    deer_step(bigdeer4, y_pos, x_pos);
	}
	deer_step(bigdeer0, y_pos, x_pos);
    }

    --y_pos;

    deer_step(lookdeer3, y_pos, x_pos);
    return (0);
}

static RETSIGTYPE
done(int sig GCC_UNUSED)
{
    CATCHALL(done);

    move(LINES - 1, 0);
    refresh();
    endwin();
    curs_set(1);

#if NO_LEAKS
    if (my_pairs != 0)
	free(my_pairs);
#endif

    ExitProgram(EXIT_SUCCESS);
}

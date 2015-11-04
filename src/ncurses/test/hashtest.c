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
 * hashtest.c -- test hash mapping
 *
 * Generate timing statistics for vertical-motion optimization.
 *
 * $Id: hashtest.c,v 1.29 2008/08/16 17:26:44 tom Exp $
 */

#include <test.priv.h>

#define LO_CHAR ' '
#define HI_CHAR '~'

static bool continuous = FALSE;
static bool reverse_loops = FALSE;
static bool single_step = FALSE;
static bool extend_corner = FALSE;
static int foot_lines = 0;
static int head_lines = 0;

static void
cleanup(void)
{
    move(LINES - 1, 0);
    clrtoeol();
    refresh();
    endwin();
}

static RETSIGTYPE
finish(int sig GCC_UNUSED)
{
    cleanup();
    ExitProgram(EXIT_FAILURE);
}

static void
genlines(int base)
{
    int i, j;

#if USE_TRACE
    if (base == 'a')
	Trace(("Resetting screen"));
    else
	Trace(("Painting `%c' screen", base));
#endif

    /* Do this so writes to lower-right corner don't cause a spurious
     * scrolling operation.  This _shouldn't_ break the scrolling
     * optimization, since that's computed in the refresh() call.
     */
    scrollok(stdscr, FALSE);

    move(0, 0);
    for (i = 0; i < head_lines; i++)
	for (j = 0; j < COLS; j++)
	    addch(UChar((j % 8 == 0) ? ('A' + j / 8) : '-'));

    move(head_lines, 0);
    for (i = head_lines; i < LINES - foot_lines; i++) {
	chtype c = (base - LO_CHAR + i) % (HI_CHAR - LO_CHAR + 1) + LO_CHAR;
	int hi = (extend_corner || (i < LINES - 1)) ? COLS : COLS - 1;
	for (j = 0; j < hi; j++)
	    addch(c);
    }

    for (i = LINES - foot_lines; i < LINES; i++) {
	move(i, 0);
	for (j = 0; j < (extend_corner ? COLS : COLS - 1); j++)
	    addch(UChar((j % 8 == 0) ? ('A' + j / 8) : '-'));
    }

    scrollok(stdscr, TRUE);
    if (single_step) {
	move(LINES - 1, 0);
	getch();
    } else
	refresh();
}

static void
one_cycle(int ch)
{
    if (continuous) {
	genlines(ch);
    } else if (ch != 'a') {
	genlines('a');
	genlines(ch);
    }
}

static void
run_test(bool optimized GCC_UNUSED)
{
    char ch;
    int lo = continuous ? LO_CHAR : 'a' - LINES;
    int hi = continuous ? HI_CHAR : 'a' + LINES;

    if (lo < LO_CHAR)
	lo = LO_CHAR;
    if (hi > HI_CHAR)
	hi = HI_CHAR;

#if defined(TRACE) || defined(NCURSES_TEST)
    if (optimized) {
	Trace(("With hash mapping"));
	_nc_optimize_enable |= OPTIMIZE_HASHMAP;
    } else {
	Trace(("Without hash mapping"));
	_nc_optimize_enable &= ~OPTIMIZE_HASHMAP;
    }
#endif

    if (reverse_loops)
	for (ch = hi; ch >= lo; ch--)
	    one_cycle(ch);
    else
	for (ch = lo; ch <= hi; ch++)
	    one_cycle(ch);
}

static void
usage(void)
{
    static const char *const tbl[] =
    {
	"Usage: hashtest [options]"
	,""
	,"Options:"
	,"  -c      continuous (don't reset between refresh's)"
	,"  -f num  leave 'num' lines constant for footer"
	,"  -h num  leave 'num' lines constant for header"
	,"  -l num  repeat test 'num' times"
	,"  -n      test the normal optimizer"
	,"  -o      test the hashed optimizer"
	,"  -r      reverse the loops"
	,"  -s      single-step"
	,"  -x      assume lower-right corner extension"
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
    int test_loops = 1;
    int test_normal = FALSE;
    int test_optimize = FALSE;

    setlocale(LC_ALL, "");

    while ((c = getopt(argc, argv, "cf:h:l:norsx")) != -1) {
	switch (c) {
	case 'c':
	    continuous = TRUE;
	    break;
	case 'f':
	    foot_lines = atoi(optarg);
	    break;
	case 'h':
	    head_lines = atoi(optarg);
	    break;
	case 'l':
	    test_loops = atoi(optarg);
	    assert(test_loops >= 0);
	    break;
	case 'n':
	    test_normal = TRUE;
	    break;
	case 'o':
	    test_optimize = TRUE;
	    break;
	case 'r':
	    reverse_loops = TRUE;
	    break;
	case 's':
	    single_step = TRUE;
	    break;
	case 'x':
	    extend_corner = TRUE;
	    break;
	default:
	    usage();
	}
    }
    if (!test_normal && !test_optimize) {
	test_normal = TRUE;
	test_optimize = TRUE;
    }
#if USE_TRACE
    trace(TRACE_TIMES);
#endif

    CATCHALL(finish);		/* arrange interrupts to terminate */

    (void) initscr();		/* initialize the curses library */
    keypad(stdscr, TRUE);	/* enable keyboard mapping */
    (void) nonl();		/* tell curses not to do NL->CR/NL on output */
    (void) cbreak();		/* take input chars one at a time, no wait for \n */
    (void) noecho();		/* don't echo input */
    scrollok(stdscr, TRUE);

    while (test_loops-- > 0) {
	if (test_normal)
	    run_test(FALSE);
	if (test_optimize)
	    run_test(TRUE);
    }

    cleanup();			/* we're done */
    ExitProgram(EXIT_SUCCESS);
}
/* hashtest.c ends here */

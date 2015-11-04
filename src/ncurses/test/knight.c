/****************************************************************************
 * Copyright (c) 1998-2006,2008 Free Software Foundation, Inc.              *
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
 * Knight's Tour - a brain game
 *
 * The original of this game was anonymous.  It had an unbelievably bogus
 * interface, you actually had to enter square coordinates!  Redesign by
 * Eric S. Raymond <esr@snark.thyrsus.com> July 22 1995.  Mouse support
 * added September 20th 1995.
 *
 * $Id: knight.c,v 1.28 2008/08/03 23:04:26 tom Exp $
 */

#include <test.priv.h>

/* board size */
#define BDEPTH	8
#define BWIDTH	8

/* where to start the instructions */
#define INSTRY	2
#define INSTRX	35

/* corner of board */
#define BOARDY	2
#define BOARDX	0

/* notification line */
#define NOTIFYY	21

/* virtual color values */
#define TRAIL_COLOR	1
#define PLUS_COLOR	2
#define MINUS_COLOR	3

#define CX(x)		(2 + 4 * (x))
#define CY(y)		(1 + 2 * (y))
#define cellmove(y, x)	wmove(boardwin, CY(y), CX(x))
#define CXINV(x)	(((x) - 1) / 4)
#define CYINV(y)	(((y) - 2) / 2)

typedef struct {
    short x, y;
} cell;

static WINDOW *boardwin;	/* the board window */
static WINDOW *helpwin;		/* the help window */
static WINDOW *msgwin;		/* the message window */
static cell history[BDEPTH * BWIDTH + 1];	/* choice history */
static chtype minus = '-';	/* possible-move character */
static chtype oldch;
static chtype plus = '+';	/* cursor hot-spot character */
static chtype trail = '#';	/* trail character */
static int movecount;		/* count of moves so far */
static int trialcount;		/* count of trials so far */
static short board[BDEPTH][BWIDTH];	/* the squares */
/* *INDENT-OFF* */
static const struct {
    int y;
    int x;
} offsets[] = {
    {  2,  1 },
    {  1,  2 },
    { -1,  2 },
    { -2,  1 },
    { -2, -1 },
    { -1, -2 },
    {  1, -2 },
    {  2, -1 },
};
/* *INDENT-ON* */

static void
init_program(void)
{
    setlocale(LC_ALL, "");

    srand((unsigned) getpid());
    initscr();
    cbreak();			/* immediate char return */
    noecho();			/* no immediate echo */
    boardwin = newwin(BDEPTH * 2 + 1, BWIDTH * 4 + 1, BOARDY, BOARDX);
    helpwin = newwin(0, 0, INSTRY, INSTRX);
    msgwin = newwin(1, INSTRX - 1, NOTIFYY, 0);
    scrollok(msgwin, TRUE);
    keypad(boardwin, TRUE);

    if (has_colors()) {
	int bg = COLOR_BLACK;

	start_color();
#if HAVE_USE_DEFAULT_COLORS
	if (use_default_colors() == OK)
	    bg = -1;
#endif

	(void) init_pair(TRAIL_COLOR, COLOR_CYAN, bg);
	(void) init_pair(PLUS_COLOR, COLOR_RED, bg);
	(void) init_pair(MINUS_COLOR, COLOR_GREEN, bg);

	trail |= COLOR_PAIR(TRAIL_COLOR);
	plus |= COLOR_PAIR(PLUS_COLOR);
	minus |= COLOR_PAIR(MINUS_COLOR);
    }
#ifdef NCURSES_MOUSE_VERSION
    (void) mousemask(BUTTON1_CLICKED, (mmask_t *) NULL);
#endif /* NCURSES_MOUSE_VERSION */

    oldch = minus;
}

static void
help1(void)
/* game explanation -- initial help screen */
{
    (void) waddstr(helpwin, "Knight's move is a solitaire puzzle.  Your\n");
    (void) waddstr(helpwin, "objective is to visit each square of the  \n");
    (void) waddstr(helpwin, "chessboard exactly once by making knight's\n");
    (void) waddstr(helpwin, "moves (one square right or left followed  \n");
    (void) waddstr(helpwin, "by two squares up or down, or two squares \n");
    (void) waddstr(helpwin, "right or left followed by one square up or\n");
    (void) waddstr(helpwin, "down).  You may start anywhere.\n\n");

    (void) waddstr(helpwin, "Use arrow keys to move the cursor around.\n");
    (void) waddstr(helpwin, "When you want to move your knight to the \n");
    (void) waddstr(helpwin, "cursor location, press <space> or Enter.\n");
    (void) waddstr(helpwin, "Illegal moves will be rejected with an  \n");
    (void) waddstr(helpwin, "audible beep.\n\n");
    (void) waddstr(helpwin, "The program will detect if you solve the\n");
    (void) waddstr(helpwin, "puzzle; also inform you when you run out\n");
    (void) waddstr(helpwin, "of legal moves.\n\n");

    (void) mvwaddstr(helpwin, NOTIFYY - INSTRY, 0,
		     "Press `?' to go to keystroke help.");
}

static void
help2(void)
/* keystroke help screen */
{
    (void) waddstr(helpwin, "Possible moves are shown with `-'.\n\n");

    (void) waddstr(helpwin, "You can move around with the arrow keys or\n");
    (void) waddstr(helpwin, "with the rogue/hack movement keys.  Other\n");
    (void) waddstr(helpwin, "commands allow you to undo moves or redraw.\n");
    (void) waddstr(helpwin, "Your mouse may work; try left-button to\n");
    (void) waddstr(helpwin, "move to the square under the pointer.\n\n");

    (void) waddstr(helpwin, "x,q -- exit             y k u    7 8 9\n");
    (void) waddstr(helpwin, "r -- redraw screen       \\|/      \\|/ \n");
    (void) waddstr(helpwin, "bksp -- undo move       h-+-l    4-+-6\n");
    (void) waddstr(helpwin, "a -- autojump            /|\\      /|\\ \n");
    (void) waddstr(helpwin, "                        b j n    1 2 3\n");

    (void) waddstr(helpwin, "\nYou can place your knight on the selected\n");
    (void) waddstr(helpwin, "square with spacebar, Enter, or the keypad\n");
    (void) waddstr(helpwin, "center key.  Use F/B to review the path.\n");

    (void) mvwaddstr(helpwin, NOTIFYY - INSTRY, 0,
		     "Press `?' to go to game explanation");
}

static void
show_help(bool * keyhelp)
{
    werase(helpwin);
    if (*keyhelp) {
	help1();
	*keyhelp = FALSE;
    } else {
	help2();
	*keyhelp = TRUE;
    }
    wrefresh(helpwin);
}

static bool
chksqr(int r1, int c1)
{
    if ((r1 < 0) || (r1 > BDEPTH - 1))
	return (FALSE);
    if ((c1 < 0) || (c1 > BWIDTH - 1))
	return (FALSE);
    return ((!board[r1][c1]) ? TRUE : FALSE);
}

static bool
chkmoves(int rw, int col)
/* check to see if valid moves are available */
{
    unsigned n;

    for (n = 0; n < SIZEOF(offsets); n++)
	if (chksqr(rw + offsets[n].y, col + offsets[n].x))
	    return (TRUE);
    return (FALSE);
}

static void
dosquares(void)
{
    int i, j;

    mvaddstr(0, 20, "KNIGHT'S MOVE -- a logical solitaire");

    move(BOARDY, BOARDX);
    waddch(boardwin, ACS_ULCORNER);
    for (j = 0; j < 7; j++) {
	waddch(boardwin, ACS_HLINE);
	waddch(boardwin, ACS_HLINE);
	waddch(boardwin, ACS_HLINE);
	waddch(boardwin, ACS_TTEE);
    }
    waddch(boardwin, ACS_HLINE);
    waddch(boardwin, ACS_HLINE);
    waddch(boardwin, ACS_HLINE);
    waddch(boardwin, ACS_URCORNER);

    for (i = 1; i < BDEPTH; i++) {
	move(BOARDY + i * 2 - 1, BOARDX);
	waddch(boardwin, ACS_VLINE);
	for (j = 0; j < BWIDTH; j++) {
	    waddch(boardwin, ' ');
	    waddch(boardwin, ' ');
	    waddch(boardwin, ' ');
	    waddch(boardwin, ACS_VLINE);
	}
	move(BOARDY + i * 2, BOARDX);
	waddch(boardwin, ACS_LTEE);
	for (j = 0; j < BWIDTH - 1; j++) {
	    waddch(boardwin, ACS_HLINE);
	    waddch(boardwin, ACS_HLINE);
	    waddch(boardwin, ACS_HLINE);
	    waddch(boardwin, ACS_PLUS);
	}
	waddch(boardwin, ACS_HLINE);
	waddch(boardwin, ACS_HLINE);
	waddch(boardwin, ACS_HLINE);
	waddch(boardwin, ACS_RTEE);
    }

    move(BOARDY + i * 2 - 1, BOARDX);
    waddch(boardwin, ACS_VLINE);
    for (j = 0; j < BWIDTH; j++) {
	waddch(boardwin, ' ');
	waddch(boardwin, ' ');
	waddch(boardwin, ' ');
	waddch(boardwin, ACS_VLINE);
    }

    move(BOARDY + i * 2, BOARDX);
    waddch(boardwin, ACS_LLCORNER);
    for (j = 0; j < BWIDTH - 1; j++) {
	waddch(boardwin, ACS_HLINE);
	waddch(boardwin, ACS_HLINE);
	waddch(boardwin, ACS_HLINE);
	waddch(boardwin, ACS_BTEE);
    }
    waddch(boardwin, ACS_HLINE);
    waddch(boardwin, ACS_HLINE);
    waddch(boardwin, ACS_HLINE);
    waddch(boardwin, ACS_LRCORNER);
}

static void
mark_possibles(int prow, int pcol, chtype mark)
{
    unsigned n;

    for (n = 0; n < SIZEOF(offsets); n++) {
	if (chksqr(prow + offsets[n].y, pcol + offsets[n].x)) {
	    cellmove(prow + offsets[n].y, pcol + offsets[n].x);
	    waddch(boardwin, mark);
	}
    }
}

static void
find_next_move(int *y, int *x)
{
    unsigned j, k;
    int found = -1;
    int first = -1;
    int next = 0;
    int oldy, oldx;
    int newy, newx;

    if (movecount > 1) {
	oldy = history[movecount - 1].y;
	oldx = history[movecount - 1].x;
	for (j = 0; j < SIZEOF(offsets) * 2; j++) {
	    k = j % SIZEOF(offsets);
	    newy = oldy + offsets[k].y;
	    newx = oldx + offsets[k].x;
	    if (chksqr(newy, newx)) {
		if (first < 0)
		    first = k;
		if (newy == *y
		    && newx == *x) {
		    found = k;
		} else if (found >= 0) {
		    next = k;
		    break;
		}
	    }
	}
	if (found < 0)
	    next = first;
	if (next >= 0) {
	    *y = oldy + offsets[next].y;
	    *x = oldx + offsets[next].x;
	}
    } else {
	beep();
    }
}

static void
unmarkcell(int row, int column)
{
    cellmove(row, column);
    waddch(boardwin, '\b');
    waddch(boardwin, ' ');
    waddch(boardwin, minus);
    waddch(boardwin, ' ');
}

static void
markcell(chtype tchar, int row, int column)
{
    cellmove(row, column);
    waddch(boardwin, '\b');
    waddch(boardwin, tchar);
    waddch(boardwin, tchar);
    waddch(boardwin, tchar);
}

static void
drawmove(chtype tchar, int oldy, int oldx, int row, int column)
/* place the stars, update board & currents */
{
    if (movecount <= 1) {
	int i, j;

	for (i = 0; i < BDEPTH; i++) {
	    for (j = 0; j < BWIDTH; j++) {
		if (movecount == 0) {
		    unmarkcell(i, j);
		} else {
		    cellmove(i, j);
		    if (winch(boardwin) == minus)
			waddch(boardwin, movecount ? ' ' : minus);
		}
	    }
	}
    } else {
	markcell(tchar, oldy, oldx);
	mark_possibles(oldy, oldx, ' ');
    }

    if (row >= 0 && column >= 0) {
	markcell(trail, row, column);
	mark_possibles(row, column, minus);
	board[row][column] = TRUE;
    }

    wprintw(msgwin, "\nMove %d", movecount);
    if (trialcount != movecount)
	wprintw(msgwin, " (%d tries)", trialcount);
    wclrtoeol(msgwin);
}

static int
iabs(int num)
{
    if (num < 0)
	return (-num);
    else
	return (num);
}

static bool
evalmove(int row, int column)
/* evaluate move */
{
    if (movecount == 1)
	return (TRUE);
    else if (board[row][column] == TRUE) {
	waddstr(msgwin, "\nYou've already been there.");
	return (FALSE);
    } else {
	int rdif = iabs(row - history[movecount - 1].y);
	int cdif = iabs(column - history[movecount - 1].x);

	if (!((rdif == 1) && (cdif == 2)) && !((rdif == 2) && (cdif == 1))) {
	    waddstr(msgwin, "\nThat's not a legal knight's move.");
	    return (FALSE);
	}
    }

    return (TRUE);
}

static int
completed(void)
{
    int i, j, count = 0;

    for (i = 0; i < BDEPTH; i++)
	for (j = 0; j < BWIDTH; j++)
	    if (board[i][j] != 0)
		count += 1;
    return (count == (BWIDTH * BDEPTH) ? -1 : count);
}

static void
no_previous_move(void)
{
    waddstr(msgwin, "\nNo previous move.");
    beep();
}

static void
play(void)
/* play the game */
{
    bool keyhelp;		/* TRUE if keystroke help is up */
    int i, j, count;
    int lastcol = 0;		/* last location visited */
    int lastrow = 0;
    int ny = 0, nx = 0;
    int review = 0;		/* review history */
    int rw = 0, col = 0;	/* current row and column */

    do {
	/* clear screen and draw board */
	werase(boardwin);
	werase(helpwin);
	werase(msgwin);
	dosquares();
	help1();
	wnoutrefresh(stdscr);
	wnoutrefresh(helpwin);
	wnoutrefresh(msgwin);
	wnoutrefresh(boardwin);
	doupdate();

	movecount = 0;
	for (i = 0; i < BDEPTH; i++) {
	    for (j = 0; j < BWIDTH; j++) {
		board[i][j] = FALSE;
		unmarkcell(i, j);
	    }
	}
	memset(history, 0, sizeof(history));
	history[0].y = history[0].x = -1;
	history[1].y = history[1].x = -1;
	lastrow = lastcol = -2;
	movecount = 1;
	trialcount = 1;
	keyhelp = FALSE;
	show_help(&keyhelp);

	for (;;) {
	    if (rw != lastrow || col != lastcol) {
		if (lastrow >= 0 && lastcol >= 0) {
		    cellmove(lastrow, lastcol);
		    if (board[lastrow][lastcol])
			waddch(boardwin, trail);
		    else
			waddch(boardwin, oldch);
		}

		cellmove(rw, col);
		oldch = winch(boardwin);

		lastrow = rw;
		lastcol = col;
	    }
	    cellmove(rw, col);
	    waddch(boardwin, plus);
	    cellmove(rw, col);

	    wrefresh(msgwin);

	    switch (wgetch(boardwin)) {
	    case 'k':
	    case '8':
	    case KEY_UP:
		ny = rw + BDEPTH - 1;
		nx = col;
		break;
	    case 'j':
	    case '2':
	    case KEY_DOWN:
		ny = rw + 1;
		nx = col;
		break;
	    case 'h':
	    case '4':
	    case KEY_LEFT:
		ny = rw;
		nx = col + BWIDTH - 1;
		break;
	    case 'l':
	    case '6':
	    case KEY_RIGHT:
		ny = rw;
		nx = col + 1;
		break;
	    case 'y':
	    case '7':
	    case KEY_A1:
		ny = rw + BDEPTH - 1;
		nx = col + BWIDTH - 1;
		break;
	    case 'b':
	    case '1':
	    case KEY_C1:
		ny = rw + 1;
		nx = col + BWIDTH - 1;
		break;
	    case 'u':
	    case '9':
	    case KEY_A3:
		ny = rw + BDEPTH - 1;
		nx = col + 1;
		break;
	    case 'n':
	    case '3':
	    case KEY_C3:
		ny = rw + 1;
		nx = col + 1;
		break;

#ifdef NCURSES_MOUSE_VERSION
	    case KEY_MOUSE:
		{
		    MEVENT myevent;

		    getmouse(&myevent);
		    if (myevent.y >= CY(0) && myevent.y <= CY(BDEPTH)
			&& myevent.x >= CX(0) && myevent.x <= CX(BWIDTH)) {
			nx = CXINV(myevent.x);
			ny = CYINV(myevent.y);
			ungetch('\n');
			break;
		    } else {
			beep();
			continue;
		    }
		}
#endif /* NCURSES_MOUSE_VERSION */

	    case KEY_B2:
	    case '\n':
	    case ' ':
		review = 0;
		if (evalmove(rw, col)) {
		    drawmove(trail,
			     history[movecount - 1].y,
			     history[movecount - 1].x,
			     rw, col);
		    history[movecount].y = rw;
		    history[movecount].x = col;
		    movecount++;
		    trialcount++;

		    if (!chkmoves(rw, col)) {
			if (completed() < 0) {
			    waddstr(msgwin, "\nYou won.");
			} else {
			    waddstr(msgwin,
				    "\nNo further moves are possible.");
			}
		    }
		} else {
		    beep();
		}
		break;

	    case KEY_UNDO:
	    case KEY_BACKSPACE:
	    case '\b':
		review = 0;
		if (movecount <= 0) {
		    no_previous_move();
		} else if (movecount <= 1) {
		    ny = history[movecount].y;
		    nx = history[movecount].x;
		    if (nx < 0 || ny < 0) {
			ny = lastrow;
			nx = lastcol;
		    }
		    movecount = 0;
		    board[ny][nx] = FALSE;
		    oldch = minus;
		    drawmove(' ', ny, nx, -1, -1);
		    movecount = 1;
		    trialcount = 1;
		    no_previous_move();
		} else {
		    int oldy = history[movecount - 1].y;
		    int oldx = history[movecount - 1].x;

		    if (!board[rw][col]) {
			cellmove(rw, col);
			waddch(boardwin, ' ');
		    }

		    board[oldy][oldx] = FALSE;
		    --movecount;
		    ny = history[movecount - 1].y;
		    nx = history[movecount - 1].x;
		    if (nx < 0 || ny < 0) {
			ny = oldy;
			nx = oldx;
		    }
		    drawmove(' ', oldy, oldx, ny, nx);

		    /* avoid problems if we just changed the current cell */
		    cellmove(lastrow, lastcol);
		    oldch = winch(boardwin);
		}
		break;

	    case 'a':
		nx = col;
		ny = rw;
		find_next_move(&ny, &nx);
		break;

	    case 'F':
		if (review > 0) {
		    review--;
		    ny = history[movecount - review - 1].y;
		    nx = history[movecount - review - 1].x;
		} else {
		    beep();
		}
		break;

	    case 'B':
		if (review < movecount - 2) {
		    review++;
		    ny = history[movecount - review - 1].y;
		    nx = history[movecount - review - 1].x;
		} else {
		    beep();
		}
		break;

	    case KEY_REDO:
	    case '\f':
	    case 'r':
		clearok(curscr, TRUE);
		wnoutrefresh(stdscr);
		wnoutrefresh(boardwin);
		wnoutrefresh(msgwin);
		wnoutrefresh(helpwin);
		doupdate();
		break;

	    case 'q':
	    case 'x':
		goto dropout;

	    case '?':
		show_help(&keyhelp);
		break;

	    default:
		beep();
		break;
	    }

	    col = nx % BWIDTH;
	    rw = ny % BDEPTH;
	}

      dropout:
	if ((count = completed()) < 0)
	    wprintw(msgwin, "\nYou won.  Care to try again? ");
	else
	    wprintw(msgwin, "\n%d squares filled.  Try again? ", count);
	wclrtoeol(msgwin);
    } while
	(tolower(wgetch(msgwin)) == 'y');
}

int
main(int argc GCC_UNUSED, char *argv[]GCC_UNUSED)
{
    init_program();

    play();

    endwin();
    ExitProgram(EXIT_SUCCESS);
}

/* knight.c ends here */

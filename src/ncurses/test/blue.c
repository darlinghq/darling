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
/*****************************************************************************
 *                                                                           *
 *                         B l u e   M o o n                                 *
 *                         =================                                 *
 *                               V2.2                                        *
 *                   A patience game by T.A.Lister                           *
 *            Integral screen support by Eric S. Raymond                     *
 *                                                                           *
 *****************************************************************************/

/*
 * Compile this with the command `cc -O blue.c -lcurses -o blue'.  For best
 * results, use the ncurses(3) library.  On non-Intel machines, SVr4 curses is
 * just as good.
 *
 * $Id: blue.c,v 1.30 2008/08/03 18:20:27 tom Exp $
 */

#include <test.priv.h>

#include <time.h>

#define NOCARD		(-1)

#define ACE		0
#define KING		12
#define SUIT_LENGTH	13

#define HEARTS		0
#define SPADES		1
#define DIAMONDS	2
#define CLUBS		3
#define NSUITS		4

#define GRID_WIDTH	14	/*    13+1  */
#define GRID_LENGTH	56	/* 4*(13+1) */
#define PACK_SIZE	52

#define BASEROW		1
#define PROMPTROW	11

#define RED_ON_WHITE    1
#define BLACK_ON_WHITE  2
#define BLUE_ON_WHITE   3

static RETSIGTYPE die(int onsig) GCC_NORETURN;

static int deck_size = PACK_SIZE;	/* initial deck */
static int deck[PACK_SIZE];

static int grid[GRID_LENGTH];	/* card layout grid */
static int freeptr[4];		/* free card space pointers */

static int deal_number = 0;

static chtype ranks[SUIT_LENGTH][2] =
{
    {' ', 'A'},
    {' ', '2'},
    {' ', '3'},
    {' ', '4'},
    {' ', '5'},
    {' ', '6'},
    {' ', '7'},
    {' ', '8'},
    {' ', '9'},
    {'1', '0'},
    {' ', 'J'},
    {' ', 'Q'},
    {' ', 'K'}
};

/* Please note, that this is a bad example.
   Color values should not be or'ed in. This
   only works, because the characters used here
   are plain and have no color attribute themselves. */
#ifdef COLOR_PAIR
#define OR_COLORS(value,pair) ((value) | COLOR_PAIR(pair))
#else
#define OR_COLORS(value,pair) (value)
#endif

#define PC_COLORS(value,pair) (OR_COLORS(value,pair) | A_ALTCHARSET)

static chtype letters[4] =
{
    OR_COLORS('h', RED_ON_WHITE),	/* hearts */
    OR_COLORS('s', BLACK_ON_WHITE),	/* spades */
    OR_COLORS('d', RED_ON_WHITE),	/* diamonds */
    OR_COLORS('c', BLACK_ON_WHITE),	/* clubs */
};

#if defined(__i386__) && defined(A_ALTCHARSET) && HAVE_TIGETSTR
static chtype glyphs[] =
{
    PC_COLORS('\003', RED_ON_WHITE),	/* hearts */
    PC_COLORS('\006', BLACK_ON_WHITE),	/* spades */
    PC_COLORS('\004', RED_ON_WHITE),	/* diamonds */
    PC_COLORS('\005', BLACK_ON_WHITE),	/* clubs */
};
#define USE_CP437 1
#else
#define USE_CP437 0
#endif /* __i386__ */

static chtype *suits = letters;	/* this may change to glyphs below */

static RETSIGTYPE
die(int onsig)
{
    (void) signal(onsig, SIG_IGN);
    endwin();
    ExitProgram(EXIT_SUCCESS);
}

static void
init_vars(void)
{
    int i;

    deck_size = PACK_SIZE;
    for (i = 0; i < PACK_SIZE; i++)
	deck[i] = i;
    for (i = 0; i < 4; i++)
	freeptr[i] = i * GRID_WIDTH;
}

static void
shuffle(int size)
{
    int i, j, numswaps, swapnum, temp;

    numswaps = size * 10;	/* an arbitrary figure */

    for (swapnum = 0; swapnum < numswaps; swapnum++) {
	i = rand() % size;
	j = rand() % size;
	temp = deck[i];
	deck[i] = deck[j];
	deck[j] = temp;
    }
}

static void
deal_cards(void)
{
    int ptr, card = 0, value, csuit, crank, suit, aces[4];

    memset(aces, 0, sizeof(aces));
    for (suit = HEARTS; suit <= CLUBS; suit++) {
	ptr = freeptr[suit];
	grid[ptr++] = NOCARD;	/* 1st card space is blank */
	while ((ptr % GRID_WIDTH) != 0) {
	    value = deck[card++];
	    crank = value % SUIT_LENGTH;
	    csuit = value / SUIT_LENGTH;
	    if (crank == ACE)
		aces[csuit] = ptr;
	    grid[ptr++] = value;
	}
    }

    if (deal_number == 1)	/* shift the aces down to the 1st column */
	for (suit = HEARTS; suit <= CLUBS; suit++) {
	    grid[suit * GRID_WIDTH] = suit * SUIT_LENGTH;
	    grid[aces[suit]] = NOCARD;
	    freeptr[suit] = aces[suit];
	}
}

static void
printcard(int value)
{
    (void) addch(' ');
    if (value == NOCARD)
	(void) addstr("   ");
    else {
	addch(ranks[value % SUIT_LENGTH][0] | COLOR_PAIR(BLUE_ON_WHITE));
	addch(ranks[value % SUIT_LENGTH][1] | COLOR_PAIR(BLUE_ON_WHITE));
	addch(suits[value / SUIT_LENGTH]);
    }
    (void) addch(' ');
}

static void
display_cards(int deal)
{
    int row, card;

    clear();
    (void) printw(
		     "Blue Moon 2.1 - by Tim Lister & Eric Raymond - Deal %d.\n",
		     deal);
    for (row = HEARTS; row <= CLUBS; row++) {
	move(BASEROW + row + row + 2, 1);
	for (card = 0; card < GRID_WIDTH; card++)
	    printcard(grid[row * GRID_WIDTH + card]);
    }

    move(PROMPTROW + 2, 0);
    refresh();
#define P(x)	(void)printw("%s\n", x)
    P("   This 52-card solitaire starts with  the entire deck shuffled and dealt");
    P("out in four rows.  The aces are then moved to the left end of the layout,");
    P("making 4 initial free spaces.  You may move to a space only the card that");
    P("matches the left neighbor in suit, and is one greater in rank.  Kings are");
    P("high, so no cards may be placed to their right (they create dead spaces).");
    P("  When no moves can be made,  cards still out of sequence are  reshuffled");
    P("and dealt face up after the ends of the partial sequences, leaving a card");
    P("space after each sequence, so that each row looks like a partial sequence");
    P("followed by a space, followed by enough cards to make a row of 14.       ");
    P("  A moment's reflection will show that this game cannot take more than 13");
    P("deals. A good score is 1-3 deals, 4-7 is average, 8 or more is poor.     ");
#undef P
    refresh();
}

static int
find(int card)
{
    int i;

    if ((card < 0) || (card >= PACK_SIZE))
	return (NOCARD);
    for (i = 0; i < GRID_LENGTH; i++)
	if (grid[i] == card)
	    return i;
    return (NOCARD);
}

static void
movecard(int src, int dst)
{
    grid[dst] = grid[src];
    grid[src] = NOCARD;

    move(BASEROW + (dst / GRID_WIDTH) * 2 + 2, (dst % GRID_WIDTH) * 5 + 1);
    printcard(grid[dst]);

    move(BASEROW + (src / GRID_WIDTH) * 2 + 2, (src % GRID_WIDTH) * 5 + 1);
    printcard(grid[src]);

    refresh();
}

static void
play_game(void)
{
    int dead = 0, i, j;
    char c;
    int selection[4], card;

    while (dead < 4) {
	dead = 0;
	for (i = 0; i < 4; i++) {
	    card = grid[freeptr[i] - 1];

	    if (((card % SUIT_LENGTH) == KING)
		||
		(card == NOCARD))
		selection[i] = NOCARD;
	    else
		selection[i] = find(card + 1);

	    if (selection[i] == NOCARD)
		dead++;
	};

	if (dead < 4) {
	    char live[NSUITS + 1], *lp = live;

	    for (i = 0; i < 4; i++) {
		if (selection[i] != NOCARD) {
		    move(BASEROW + (selection[i] / GRID_WIDTH) * 2 + 3,
			 (selection[i] % GRID_WIDTH) * 5);
		    (void) printw("   %c ", *lp++ = 'a' + i);
		}
	    };
	    *lp = '\0';

	    if (strlen(live) == 1) {
		move(PROMPTROW, 0);
		(void) printw(
				 "Making forced moves...                                 ");
		refresh();
		(void) sleep(1);
		c = live[0];
	    } else {
		char buf[BUFSIZ];

		(void) sprintf(buf,
			       "Type [%s] to move, r to redraw, q or INTR to quit: ",
			       live);

		do {
		    move(PROMPTROW, 0);
		    (void) addstr(buf);
		    move(PROMPTROW, (int) strlen(buf));
		    clrtoeol();
		    (void) addch(' ');
		} while
		    (((c = getch()) < 'a' || c > 'd') && (c != 'r') && (c != 'q'));
	    }

	    for (j = 0; j < 4; j++)
		if (selection[j] != NOCARD) {
		    move(BASEROW + (selection[j] / GRID_WIDTH) * 2 + 3,
			 (selection[j] % GRID_WIDTH) * 5);
		    (void) printw("     ");
		}

	    if (c == 'r')
		display_cards(deal_number);
	    else if (c == 'q')
		die(SIGINT);
	    else {
		i = c - 'a';
		if (selection[i] == NOCARD)
		    beep();
		else {
		    movecard(selection[i], freeptr[i]);
		    freeptr[i] = selection[i];
		}
	    }
	}
    }

    move(PROMPTROW, 0);
    standout();
    (void) printw("Finished deal %d - type any character to continue...", deal_number);
    standend();
    (void) getch();
}

static int
collect_discards(void)
{
    int row, col, cardno = 0, finish, gridno;

    for (row = HEARTS; row <= CLUBS; row++) {
	finish = 0;
	for (col = 1; col < GRID_WIDTH; col++) {
	    gridno = row * GRID_WIDTH + col;

	    if ((grid[gridno] != (grid[gridno - 1] + 1)) && (finish == 0)) {
		finish = 1;
		freeptr[row] = gridno;
	    };

	    if ((finish != 0) && (grid[gridno] != NOCARD))
		deck[cardno++] = grid[gridno];
	}
    }
    return cardno;
}

static void
game_finished(int deal)
{
    clear();
    (void) printw("You finished the game in %d deals. This is ", deal);
    standout();
    if (deal < 2)
	(void) addstr("excellent");
    else if (deal < 4)
	(void) addstr("good");
    else if (deal < 8)
	(void) addstr("average");
    else
	(void) addstr("poor");
    standend();
    (void) addstr(".         ");
    refresh();
}

int
main(int argc, char *argv[])
{
    CATCHALL(die);

    setlocale(LC_ALL, "");

    initscr();

    /*
     * We use COLOR_GREEN because COLOR_BLACK is wired to the wrong thing.
     */
    start_color();
    init_pair(RED_ON_WHITE, COLOR_RED, COLOR_WHITE);
    init_pair(BLUE_ON_WHITE, COLOR_BLUE, COLOR_WHITE);
    init_pair(BLACK_ON_WHITE, COLOR_BLACK, COLOR_WHITE);

#ifndef COLOR_PAIR
    letters[0] = OR_COLORS('h', RED_ON_WHITE);	/* hearts */
    letters[1] = OR_COLORS('s', BLACK_ON_WHITE);	/* spades */
    letters[2] = OR_COLORS('d', RED_ON_WHITE);	/* diamonds */
    letters[3] = OR_COLORS('c', BLACK_ON_WHITE);	/* clubs */
#if USE_CP437
    glyphs[0] = PC_COLORS('\003', RED_ON_WHITE);	/* hearts */
    glyphs[1] = PC_COLORS('\006', BLACK_ON_WHITE);	/* spades */
    glyphs[2] = PC_COLORS('\004', RED_ON_WHITE);	/* diamonds */
    glyphs[3] = PC_COLORS('\005', BLACK_ON_WHITE);	/* clubs */
#endif
#endif

#if USE_CP437
    if (tigetstr("smpch"))
	suits = glyphs;
#endif /* USE_CP437 */

    cbreak();

    if (argc == 2)
	srand((unsigned) atoi(argv[1]));
    else
	srand((unsigned) time((time_t *) 0));

    init_vars();

    do {
	deal_number++;
	shuffle(deck_size);
	deal_cards();
	display_cards(deal_number);
	play_game();
    }
    while
	((deck_size = collect_discards()) != 0);

    game_finished(deal_number);

    die(SIGINT);
    /*NOTREACHED */
}

/* blue.c ends here */

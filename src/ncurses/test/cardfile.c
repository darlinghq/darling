/****************************************************************************
 * Copyright (c) 1999-2007,2008 Free Software Foundation, Inc.              *
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
 * Author: Thomas E. Dickey
 *
 * $Id: cardfile.c,v 1.35 2008/08/05 00:42:24 tom Exp $
 *
 * File format: text beginning in column 1 is a title; other text is content.
 */

#include <test.priv.h>

#if USE_LIBFORM && USE_LIBPANEL

#include <form.h>
#include <panel.h>

#define VISIBLE_CARDS 10
#define OFFSET_CARD 2
#define pair_1 1
#define pair_2 2

#define isVisible(cardp) ((cardp)->panel != 0)

enum {
    MY_CTRL_x = MAX_FORM_COMMAND
    ,MY_CTRL_N
    ,MY_CTRL_P
    ,MY_CTRL_Q
    ,MY_CTRL_W
};

typedef struct _card {
    struct _card *link;
    PANEL *panel;
    FORM *form;
    char *title;
    char *content;
} CARD;

static CARD *all_cards;
static bool try_color = FALSE;
static char default_name[] = "cardfile.dat";

#if !HAVE_STRDUP
#define strdup my_strdup
static char *
strdup(const char *s)
{
    char *p = typeMalloc(char, strlen(s) + 1);
    if (p)
	strcpy(p, s);
    return (p);
}
#endif /* not HAVE_STRDUP */

static const char *
skip(const char *buffer)
{
    while (isspace(UChar(*buffer)))
	buffer++;
    return buffer;
}

static void
trim(char *buffer)
{
    unsigned n = strlen(buffer);
    while (n-- && isspace(UChar(buffer[n])))
	buffer[n] = 0;
}

/*******************************************************************************/

static CARD *
add_title(const char *title)
{
    CARD *card, *p, *q;

    for (p = all_cards, q = 0; p != 0; q = p, p = p->link) {
	int cmp = strcmp(p->title, title);
	if (cmp == 0)
	    return p;
	if (cmp > 0)
	    break;
    }

    card = typeCalloc(CARD, 1);
    card->title = strdup(title);
    card->content = strdup("");

    if (q == 0) {
	card->link = all_cards;
	all_cards = card;
    } else {
	card->link = q->link;
	q->link = card;
    }

    return card;
}

static void
add_content(CARD * card, const char *content)
{
    unsigned total, offset;

    content = skip(content);
    if ((total = strlen(content)) != 0) {
	if (card->content != 0 && (offset = strlen(card->content)) != 0) {
	    total += 1 + offset;
	    card->content = typeRealloc(char, total + 1, card->content);
	    if (card->content)
		strcpy(card->content + offset++, " ");
	} else {
	    offset = 0;
	    if (card->content != 0)
		free(card->content);
	    card->content = typeMalloc(char, total + 1);
	}
	if (card->content)
	    strcpy(card->content + offset, content);
    }
}

static CARD *
new_card(void)
{
    CARD *card = add_title("");
    add_content(card, "");
    return card;
}

static CARD *
find_card(char *title)
{
    CARD *card;

    for (card = all_cards; card != 0; card = card->link)
	if (!strcmp(card->title, title))
	    break;

    return card;
}

static void
read_data(char *fname)
{
    FILE *fp;
    CARD *card = 0;
    char buffer[BUFSIZ];

    if ((fp = fopen(fname, "r")) != 0) {
	while (fgets(buffer, sizeof(buffer), fp)) {
	    trim(buffer);
	    if (isspace(UChar(*buffer))) {
		if (card == 0)
		    card = add_title("");
		add_content(card, buffer);
	    } else if ((card = find_card(buffer)) == 0) {
		card = add_title(buffer);
	    }
	}
	fclose(fp);
    }
}

/*******************************************************************************/

static void
write_data(const char *fname)
{
    FILE *fp;
    CARD *p = 0;
    int n;

    if (!strcmp(fname, default_name))
	fname = "cardfile.out";

    if ((fp = fopen(fname, "w")) != 0) {
	for (p = all_cards; p != 0; p = p->link) {
	    FIELD **f = form_fields(p->form);
	    for (n = 0; f[n] != 0; n++) {
		char *s = field_buffer(f[n], 0);
		if (s != 0
		    && (s = strdup(s)) != 0) {
		    trim(s);
		    fprintf(fp, "%s%s\n", n ? "\t" : "", s);
		    free(s);
		}
	    }
	}
	fclose(fp);
    }
}

/*******************************************************************************/

/*
 * Count the cards
 */
static int
count_cards(void)
{
    CARD *p;
    int count = 0;

    for (p = all_cards; p != 0; p = p->link)
	count++;

    return count;
}

/*
 * Shuffle the panels to keep them in a natural hierarchy.
 */
static void
order_cards(CARD * first, int depth)
{
    if (first) {
	if (depth && first->link)
	    order_cards(first->link, depth - 1);
	if (isVisible(first))
	    top_panel(first->panel);
    }
}

/*
 * Return the next card in the list
 */
static CARD *
next_card(CARD * now)
{
    if (now->link != 0) {
	CARD *tst = now->link;
	if (isVisible(tst))
	    now = tst;
	else
	    tst = next_card(tst);
    }
    return now;
}

/*
 * Return the previous card in the list
 */
static CARD *
prev_card(CARD * now)
{
    CARD *p;
    for (p = all_cards; p != 0; p = p->link) {
	if (p->link == now) {
	    if (!isVisible(p))
		p = prev_card(p);
	    return p;
	}
    }
    return now;
}

/*
 * Returns the first card in the list that we will display.
 */
static CARD *
first_card(CARD * now)
{
    if (!isVisible(now))
	now = next_card(now);
    return now;
}

/*******************************************************************************/

static int
form_virtualize(WINDOW *w)
{
    int c = wgetch(w);

    switch (c) {
    case CTRL('W'):
	return (MY_CTRL_W);
    case CTRL('N'):
	return (MY_CTRL_N);
    case CTRL('P'):
	return (MY_CTRL_P);
    case QUIT:
    case ESCAPE:
	return (MY_CTRL_Q);

    case KEY_BACKSPACE:
	return (REQ_DEL_PREV);
    case KEY_DC:
	return (REQ_DEL_CHAR);
    case KEY_LEFT:
	return (REQ_LEFT_CHAR);
    case KEY_RIGHT:
	return (REQ_RIGHT_CHAR);

    case KEY_DOWN:
    case KEY_NEXT:
	return (REQ_NEXT_FIELD);
    case KEY_UP:
    case KEY_PREVIOUS:
	return (REQ_PREV_FIELD);

    default:
	return (c);
    }
}

static FIELD **
make_fields(CARD * p, int form_high, int form_wide)
{
    FIELD **f = typeCalloc(FIELD *, 3);

    f[0] = new_field(1, form_wide, 0, 0, 0, 0);
    set_field_back(f[0], A_REVERSE);
    set_field_buffer(f[0], 0, p->title);
    field_opts_off(f[0], O_BLANK);

    f[1] = new_field(form_high - 1, form_wide, 1, 0, 0, 0);
    set_field_buffer(f[1], 0, p->content);
    set_field_just(f[1], JUSTIFY_LEFT);
    field_opts_off(f[1], O_BLANK);

    f[2] = 0;
    return f;
}

static void
show_legend(void)
{
    erase();
    move(LINES - 3, 0);
    addstr("^Q/ESC -- exit form            ^W   -- writes data to file\n");
    addstr("^N   -- go to next card        ^P   -- go to previous card\n");
    addstr("Arrow keys move left/right within a field, up/down between fields");
}

#if (defined(KEY_RESIZE) && HAVE_WRESIZE) || NO_LEAKS
static void
free_form_fields(FIELD ** f)
{
    int n;

    for (n = 0; f[n] != 0; ++n) {
	free_field(f[n]);
    }
    free(f);
}
#endif

/*******************************************************************************/

static void
cardfile(char *fname)
{
    WINDOW *win;
    CARD *p;
    CARD *top_card;
    int visible_cards;
    int panel_wide;
    int panel_high;
    int form_wide;
    int form_high;
    int y;
    int x;
    int ch = ERR;
    int finished = FALSE;

    show_legend();

    /* decide how many cards we can display */
    visible_cards = count_cards();
    while (
	      (panel_wide = COLS - (visible_cards * OFFSET_CARD)) < 10 ||
	      (panel_high = LINES - (visible_cards * OFFSET_CARD) - 5) < 5) {
	--visible_cards;
    }
    form_wide = panel_wide - 2;
    form_high = panel_high - 2;
    y = (visible_cards - 1) * OFFSET_CARD;
    x = 0;

    /* make a panel for each CARD */
    for (p = all_cards; p != 0; p = p->link) {

	if ((win = newwin(panel_high, panel_wide, y, x)) == 0)
	    break;

	wbkgd(win, COLOR_PAIR(pair_2));
	keypad(win, TRUE);
	p->panel = new_panel(win);
	box(win, 0, 0);

	p->form = new_form(make_fields(p, form_high, form_wide));
	set_form_win(p->form, win);
	set_form_sub(p->form, derwin(win, form_high, form_wide, 1, 1));
	post_form(p->form);

	y -= OFFSET_CARD;
	x += OFFSET_CARD;
    }

    top_card = first_card(all_cards);
    order_cards(top_card, visible_cards);

    while (!finished) {
	update_panels();
	doupdate();

	ch = form_virtualize(panel_window(top_card->panel));
	switch (form_driver(top_card->form, ch)) {
	case E_OK:
	    break;
	case E_UNKNOWN_COMMAND:
	    switch (ch) {
	    case MY_CTRL_Q:
		finished = TRUE;
		break;
	    case MY_CTRL_P:
		top_card = prev_card(top_card);
		order_cards(top_card, visible_cards);
		break;
	    case MY_CTRL_N:
		top_card = next_card(top_card);
		order_cards(top_card, visible_cards);
		break;
	    case MY_CTRL_W:
		form_driver(top_card->form, REQ_VALIDATION);
		write_data(fname);
		break;
#if defined(KEY_RESIZE) && HAVE_WRESIZE
	    case KEY_RESIZE:
		/* resizeterm already did "something" reasonable, but it cannot
		 * know much about layout.  So let's make it nicer.
		 */
		panel_wide = COLS - (visible_cards * OFFSET_CARD);
		panel_high = LINES - (visible_cards * OFFSET_CARD) - 5;

		form_wide = panel_wide - 2;
		form_high = panel_high - 2;

		y = (visible_cards - 1) * OFFSET_CARD;
		x = 0;

		show_legend();
		for (p = all_cards; p != 0; p = p->link) {
		    FIELD **oldf = form_fields(p->form);
		    WINDOW *olds = form_sub(p->form);

		    if (!isVisible(p))
			continue;
		    win = form_win(p->form);

		    /* move and resize the card as needed
		     * FIXME: if the windows are shrunk too much, this won't do
		     */
		    mvwin(win, y, x);
		    wresize(win, panel_high, panel_wide);

		    /* reconstruct each form.  Forms are not resizable, and
		     * there appears to be no good way to reload the text in
		     * a resized window.
		     */
		    werase(win);

		    unpost_form(p->form);
		    free_form(p->form);

		    p->form = new_form(make_fields(p, form_high, form_wide));
		    set_form_win(p->form, win);
		    set_form_sub(p->form, derwin(win, form_high, form_wide,
						 1, 1));
		    post_form(p->form);

		    free_form_fields(oldf);
		    delwin(olds);

		    box(win, 0, 0);

		    y -= OFFSET_CARD;
		    x += OFFSET_CARD;
		}
		break;
#endif
	    default:
		beep();
		break;
	    }
	    break;
	default:
	    flash();
	    break;
	}
    }
#if NO_LEAKS
    while (all_cards != 0) {
	FIELD **f;
	int count;

	p = all_cards;
	all_cards = all_cards->link;

	if (isVisible(p)) {
	    f = form_fields(p->form);
	    count = field_count(p->form);

	    unpost_form(p->form);	/* ...so we can free it */
	    free_form(p->form);	/* this also disconnects the fields */

	    free_form_fields(f);

	    del_panel(p->panel);
	}
	free(p->title);
	free(p->content);
	free(p);
    }
#endif
}

static void
usage(void)
{
    static const char *msg[] =
    {
	"Usage: view [options] file"
	,""
	,"Options:"
	," -c       use color if terminal supports it"
    };
    size_t n;
    for (n = 0; n < SIZEOF(msg); n++)
	fprintf(stderr, "%s\n", msg[n]);
    ExitProgram(EXIT_FAILURE);
}

/*******************************************************************************/

int
main(int argc, char *argv[])
{
    int n;

    setlocale(LC_ALL, "");

    while ((n = getopt(argc, argv, "c")) != -1) {
	switch (n) {
	case 'c':
	    try_color = TRUE;
	    break;
	default:
	    usage();
	}
    }

    initscr();
    cbreak();
    noecho();

    if (try_color) {
	if (has_colors()) {
	    start_color();
	    init_pair(pair_1, COLOR_WHITE, COLOR_BLUE);
	    init_pair(pair_2, COLOR_WHITE, COLOR_CYAN);
	    bkgd(COLOR_PAIR(pair_1));
	} else {
	    try_color = FALSE;
	}
    }

    if (optind + 1 == argc) {
	for (n = 1; n < argc; n++)
	    read_data(argv[n]);
	if (count_cards() == 0)
	    new_card();
	cardfile(argv[1]);
    } else {
	read_data(default_name);
	if (count_cards() == 0)
	    new_card();
	cardfile(default_name);
    }

    endwin();

    ExitProgram(EXIT_SUCCESS);
}
#else
int
main(void)
{
    printf("This program requires the curses form and panel libraries\n");
    ExitProgram(EXIT_FAILURE);
}
#endif

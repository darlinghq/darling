/*
 *
 * This is a test program for the PDCurses screen package for IBM PC type
 * machines.
 *
 * This program was written by John Burnell (johnb@kea.am.dsir.govt.nz)
 *  wrs(5/28/93) -- modified to be consistent (perform identically) with either
 *                  PDCurses or under Unix System V, R4
 *
 * $Id: testcurs.c,v 1.39 2008/08/03 17:58:09 tom Exp $
 */

#include <test.priv.h>

#if defined(XCURSES)
char *XCursesProgramName = "testcurs";
#endif

static int initTest(WINDOW **);
static void display_menu(int, int);
static void inputTest(WINDOW *);
static void introTest(WINDOW *);
static void outputTest(WINDOW *);
static void padTest(WINDOW *);
static void scrollTest(WINDOW *);
#if defined(PDCURSES) && !defined(XCURSES)
static void resizeTest(WINDOW *);
#endif

struct commands {
    NCURSES_CONST char *text;
    void (*function) (WINDOW *);
};
typedef struct commands COMMAND;

static const COMMAND command[] =
{
    {"General Test", introTest},
    {"Pad Test", padTest},
#if defined(PDCURSES) && !defined(XCURSES)
    {"Resize Test", resizeTest},
#endif
    {"Scroll Test", scrollTest},
    {"Input Test", inputTest},
    {"Output Test", outputTest}
};
#define MAX_OPTIONS (int) SIZEOF(command)

#if !HAVE_STRDUP
#define strdup my_strdup
static char *
strdup(char *s)
{
    char *p = typeMalloc(char, strlen(s) + 1);
    if (p)
	strcpy(p, s);
    return (p);
}
#endif /* not HAVE_STRDUP */

static int width, height;

int
main(
	int argc GCC_UNUSED,
	char *argv[]GCC_UNUSED)
{
    WINDOW *win;
    int key;
    int old_option = (-1);
    int new_option = 0;
    bool quit = FALSE;
    int n;

    setlocale(LC_ALL, "");

#ifdef PDCDEBUG
    PDC_debug("testcurs started\n");
#endif
    if (!initTest(&win))
	ExitProgram(EXIT_FAILURE);

    erase();
    display_menu(old_option, new_option);
    for (;;) {
#ifdef A_COLOR
	if (has_colors()) {
	    init_pair(1, COLOR_WHITE, COLOR_BLUE);
	    wbkgd(win, COLOR_PAIR(1));
	} else
	    wbkgd(win, A_REVERSE);
#else
	wbkgd(win, A_REVERSE);
#endif
	werase(win);

	noecho();
	keypad(stdscr, TRUE);
	raw();
	key = getch();
	if (key < KEY_MIN && key > 0 && isalpha(key)) {
	    if (islower(key))
		key = toupper(key);
	    for (n = 0; n < MAX_OPTIONS; ++n) {
		if (key == command[n].text[0]) {
		    display_menu(old_option, new_option = n);
		    key = ' ';
		    break;
		}
	    }
	}
	switch (key) {
	case 10:
	case 13:
	case KEY_ENTER:
	    erase();
	    refresh();
	    (*command[new_option].function) (win);
	    erase();
	    display_menu(old_option, new_option);
	    break;
	case KEY_UP:
	    new_option = ((new_option == 0)
			  ? new_option
			  : new_option - 1);
	    display_menu(old_option, new_option);
	    break;
	case KEY_DOWN:
	    new_option = ((new_option == (MAX_OPTIONS - 1))
			  ? new_option
			  : new_option + 1);
	    display_menu(old_option, new_option);
	    break;
	case 'Q':
	case 'q':
	    quit = TRUE;
	    break;
	default:
	    beep();
	    break;
	case ' ':
	    break;
	}
	if (quit == TRUE)
	    break;
    }

    delwin(win);

    endwin();
#ifdef XCURSES
    XCursesExit();
#endif
    ExitProgram(EXIT_SUCCESS);
}

static void
Continue(WINDOW *win)
{
    int y1 = getmaxy(win);
    int x1 = getmaxx(win);
    int y0 = y1 < 10 ? y1 : 10;
    int x0 = 1;
    chtype save;

    save = mvwinch(win, y0, x1 - 1);

    mvwaddstr(win, y0, x0, " Press any key to continue");
    wclrtoeol(win);
    getyx(win, y0, x0);

    mvwaddch(win, y0, x1 - 1, save);

    wmove(win, y0, x0);
    raw();
    wgetch(win);
}

static int
initTest(WINDOW **win)
{
#ifdef PDCDEBUG
    PDC_debug("initTest called\n");
#endif
#ifdef TRACE
    trace(TRACE_MAXIMUM);
#endif
    initscr();
#ifdef PDCDEBUG
    PDC_debug("after initscr()\n");
#endif
#ifdef A_COLOR
    if (has_colors())
	start_color();
#endif
    width = 60;
    height = 13;		/* Create a drawing window */
    *win = newwin(height, width, (LINES - height) / 2, (COLS - width) / 2);
    if (*win == NULL) {
	endwin();
	return 0;
    }
    return 1;
}

static void
introTest(WINDOW *win)
{
    wmove(win, height / 2 - 5, width / 2);
    wvline(win, ACS_VLINE, 10);
    wmove(win, height / 2, width / 2 - 10);
    whline(win, ACS_HLINE, 20);
    Continue(win);

    beep();
    werase(win);

    box(win, ACS_VLINE, ACS_HLINE);
    wrefresh(win);
    cbreak();
    mvwaddstr(win, 1, 1,
	      "You should have rectangle in the middle of the screen");
    mvwaddstr(win, 2, 1, "You should have heard a beep");
    Continue(win);
    return;
}

static void
scrollTest(WINDOW *win)
{
    int i;
    int half;
    int OldY;
    NCURSES_CONST char *Message = "The window will now scroll slowly";

    wclear(win);
    OldY = getmaxy(win);
    half = OldY / 2;
    mvwprintw(win, OldY - 2, 1, Message);
    wrefresh(win);
    scrollok(win, TRUE);
    for (i = 1; i <= OldY; i++) {
	napms(600);
	scroll(win);
	wrefresh(win);
    }

    werase(win);
    for (i = 1; i < OldY; i++) {
	mvwprintw(win, i, 1, "Line %d", i);
    }
    mvwprintw(win, OldY - 2, 1, "The top of the window will scroll");
    wmove(win, 1, 1);
    wsetscrreg(win, 0, half - 1);
    box(win, ACS_VLINE, ACS_HLINE);
    wrefresh(win);
    for (i = 1; i <= half; i++) {
	napms(600);
	scroll(win);
	box(win, ACS_VLINE, ACS_HLINE);
	wrefresh(win);
    }

    werase(win);
    for (i = 1; i < OldY; i++) {
	mvwprintw(win, i, 1, "Line %d", i);
    }
    mvwprintw(win, 1, 1, "The bottom of the window will scroll");
    wmove(win, OldY - 2, 1);
    wsetscrreg(win, half, --OldY);
    box(win, ACS_VLINE, ACS_HLINE);
    wrefresh(win);
    for (i = half; i <= OldY; i++) {
	napms(600);
	wscrl(win, -1);
	box(win, ACS_VLINE, ACS_HLINE);
	wrefresh(win);
    }
    wsetscrreg(win, 0, OldY);
}

static void
inputTest(WINDOW *win)
{
    int answered;
    int repeat;
    int w, h, bx, by, sw, sh, i, c, num;
    char buffer[80];
    WINDOW *subWin;
    wclear(win);

    getmaxyx(win, h, w);
    getbegyx(win, by, bx);
    sw = w / 3;
    sh = h / 3;
    if ((subWin = subwin(win, sh, sw, by + h - sh - 2, bx + w - sw - 2)) == NULL)
	return;

#ifdef A_COLOR
    if (has_colors()) {
	init_pair(2, COLOR_WHITE, COLOR_RED);
	wbkgd(subWin, COLOR_PAIR(2) | A_BOLD);
    } else
	wbkgd(subWin, A_BOLD);
#else
    wbkgd(subWin, A_BOLD);
#endif
    box(subWin, ACS_VLINE, ACS_HLINE);
    wrefresh(win);

    nocbreak();
    mvwaddstr(win, 2, 1, "Press some keys for 5 seconds");
    mvwaddstr(win, 1, 1, "Pressing ^C should do nothing");
    wrefresh(win);

    werase(subWin);
    box(subWin, ACS_VLINE, ACS_HLINE);
    for (i = 0; i < 5; i++) {
	mvwprintw(subWin, 1, 1, "Time = %d", i);
	wrefresh(subWin);
	napms(1000);
	flushinp();
    }

    delwin(subWin);
    werase(win);
    flash();
    wrefresh(win);
    napms(500);

    mvwaddstr(win, 2, 1, "Press a key, followed by ENTER");
    wmove(win, 9, 10);
    wrefresh(win);
    echo();
    noraw();
    wgetch(win);
    flushinp();

    wmove(win, 9, 10);
    wdelch(win);
    mvwaddstr(win, 4, 1, "The character should now have been deleted");
    Continue(win);

    wclear(win);
    mvwaddstr(win, 1, 1, "Press keys (or mouse buttons) to show their names");
    mvwaddstr(win, 2, 1, "Press spacebar to finish");
    wrefresh(win);

    keypad(win, TRUE);
    raw();
    noecho();

#if HAVE_TYPEAHEAD
    typeahead(-1);
#endif

#if defined(PDCURSES)
    mouse_set(ALL_MOUSE_EVENTS);
#endif

    for (;;) {
	wmove(win, 3, 5);
	c = wgetch(win);
	wclrtobot(win);
	if (c >= KEY_MIN)
	    wprintw(win, "Key Pressed: %s", keyname(c));
	else if (isprint(c))
	    wprintw(win, "Key Pressed: %c", c);
	else
	    wprintw(win, "Key Pressed: %s", unctrl(UChar(c)));
#if defined(PDCURSES)
	if (c == KEY_MOUSE) {
	    int button = 0;
	    request_mouse_pos();
	    if (BUTTON_CHANGED(1))
		button = 1;
	    else if (BUTTON_CHANGED(2))
		button = 2;
	    else if (BUTTON_CHANGED(3))
		button = 3;
	    else
		button = 0;
	    wmove(win, 4, 18);
	    wprintw(win, "Button %d: ", button);
	    if (MOUSE_MOVED)
		wprintw(win, "moved: ");
	    else if ((BUTTON_STATUS(button) & BUTTON_ACTION_MASK) == BUTTON_PRESSED)
		wprintw(win, "pressed: ");
	    else if ((BUTTON_STATUS(button) & BUTTON_ACTION_MASK) == BUTTON_DOUBLE_CLICKED)
		wprintw(win, "double: ");
	    else
		wprintw(win, "released: ");
	    wprintw(win, " Position: Y: %d X: %d", MOUSE_Y_POS, MOUSE_X_POS);
	}
#endif
	wrefresh(win);
	if (c == ' ')
	    break;
    }
#if 0
    nodelay(win, TRUE);
    wgetch(win);
    nodelay(win, FALSE);
#endif
#if defined(PDCURSES)
    mouse_set(0L);
#endif
    refresh();

    repeat = 0;
    do {
	static const char *fmt[] =
	{
	    "%d %10s",
	    "%d %[a-zA-Z]s",
	    "%d %[][a-zA-Z]s",
	    "%d %[^0-9]"
	};
	const char *format = fmt[repeat % SIZEOF(fmt)];

	wclear(win);
	mvwaddstr(win, 3, 2, "The window should have moved");
	mvwaddstr(win, 4, 2,
		  "This text should have appeared without you pressing a key");
	mvwprintw(win, 6, 2,
		  "Scanning with format \"%s\"", format);
	mvwin(win, 2 + 2 * (repeat % 4), 1 + 2 * (repeat % 4));
	erase();
	refresh();
	wrefresh(win);
	echo();
	noraw();
	num = 0;
	*buffer = 0;
	answered = mvwscanw(win, 7, 6, strdup(format), &num, buffer);
	mvwprintw(win, 8, 6,
		  "String: %s Number: %d (%d values read)",
		  buffer, num, answered);
	Continue(win);
	++repeat;
    } while (answered > 0);
}

static void
outputTest(WINDOW *win)
{
    WINDOW *win1;
    char Buffer[80];
    chtype ch;
    int by, bx;

#if !HAVE_TIGETSTR
#if HAVE_TGETENT
    char tc_buffer[4096];
    char tc_parsed[4096];
    char *area_pointer = tc_parsed;
    tgetent(tc_buffer, getenv("TERM"));
#else
#define tgetstr(a,b) 0
#endif
#endif /* !HAVE_TIGETSTR */

    nl();
    wclear(win);
    mvwaddstr(win, 1, 1,
	      "You should now have a screen in the upper left corner, and this text should have wrapped");
    mvwin(win, 2, 1);
    waddstr(win, "\nThis text should be down\n");
    waddstr(win, "and broken into two here ^");
    Continue(win);

    wclear(win);
    wattron(win, A_BOLD);
    mvwaddstr(win, 1, 1, "A new window will appear with this text in it");
    mvwaddstr(win, 8, 1, "Press any key to continue");
    wrefresh(win);
    wgetch(win);

    getbegyx(win, by, bx);

    if (LINES < 24 || COLS < 75) {
	mvwaddstr(win, 5, 1,
		  "Some tests have been skipped as they require a");
	mvwaddstr(win, 6, 1, "display of at least 24 LINES by 75 COLUMNS");
	Continue(win);
    } else {
	win1 = newwin(10, 50, 14, 25);
	if (win1 == NULL) {
	    endwin();
	    return;
	}
#ifdef A_COLOR
	if (has_colors()) {
	    init_pair(3, COLOR_BLUE, COLOR_WHITE);
	    wbkgd(win1, COLOR_PAIR(3));
	} else
	    wbkgd(win1, A_NORMAL);
#else
	wbkgd(win1, A_NORMAL);
#endif
	wclear(win1);
	mvwaddstr(win1, 5, 1,
		  "This text should appear; using overlay option");
	copywin(win, win1, 0, 0, 0, 0, 9, 49, TRUE);

#if defined(PDCURSES) && !defined(XCURSES)
	box(win1, 0xb3, 0xc4);
#else
	box(win1, ACS_VLINE, ACS_HLINE);
#endif
	wmove(win1, 8, 26);
	wrefresh(win1);
	wgetch(win1);

	wclear(win1);
	wattron(win1, A_BLINK);
	mvwaddstr(win1, 4, 1,
		  "This blinking text should appear in only the second window");
	wattroff(win1, A_BLINK);
	mvwin(win1, by, bx);
	overlay(win, win1);
	mvwin(win1, 14, 25);
	wmove(win1, 8, 26);
	wrefresh(win1);
	wgetch(win1);
	delwin(win1);
    }

    clear();
    wclear(win);
    wrefresh(win);
    mvwaddstr(win, 6, 2, "This line shouldn't appear");
    mvwaddstr(win, 4, 2, "Only half of the next line is visible");
    mvwaddstr(win, 5, 2, "Only half of the next line is visible");
    wmove(win, 6, 1);
    wclrtobot(win);
    wmove(win, 5, 20);
    wclrtoeol(win);
    mvwaddstr(win, 8, 2, "This line also shouldn't appear");
    wmove(win, 8, 1);
    wdeleteln(win);
    Continue(win);

    wmove(win, 5, 9);
    ch = winch(win);

    wclear(win);
    wmove(win, 6, 2);
    waddstr(win, "The next char should be l:  ");
    winsch(win, ch);
    Continue(win);

#if HAVE_WINSSTR
    mvwinsstr(win, 6, 2, "A1B2C3D4E5");
    Continue(win);
#endif

    wmove(win, 5, 1);
    winsertln(win);
    mvwaddstr(win, 5, 2, "The lines below should have moved down");
    Continue(win);

    wclear(win);
    wmove(win, 2, 2);
    wprintw(win, "This is a formatted string in a window: %d %s\n", 42,
	    "is it");
    mvwaddstr(win, 10, 1, "Enter a string: ");
    wrefresh(win);
    noraw();
    echo();
    *Buffer = 0;
    wscanw(win, "%s", Buffer);

    printw("This is a formatted string in stdscr: %d %s\n", 42, "is it");
    mvaddstr(10, 1, "Enter a string: ");
    *Buffer = 0;
    scanw("%s", Buffer);

    if (TIGETSTR("cvvis", "vs") != 0) {
	wclear(win);
	curs_set(2);
	mvwaddstr(win, 1, 1, "The cursor should appear as a block (visible)");
	Continue(win);
    }

    if (TIGETSTR("civis", "vi") != 0) {
	wclear(win);
	curs_set(0);
	mvwaddstr(win, 1, 1,
		  "The cursor should have disappeared (invisible)");
	Continue(win);
    }

    if (TIGETSTR("cnorm", "ve") != 0) {
	wclear(win);
	curs_set(1);
	mvwaddstr(win, 1, 1, "The cursor should be an underline (normal)");
	Continue(win);
    }
#ifdef A_COLOR
    if (has_colors()) {
	wclear(win);
	mvwaddstr(win, 1, 1, "Colors should change after you press a key");
	Continue(win);
	init_pair(1, COLOR_RED, COLOR_WHITE);
	wrefresh(win);
    }
#endif

    werase(win);

#if HAVE_TERMNAME
    mvwaddstr(win, 1, 1, "Information About Your Terminal");
    mvwaddstr(win, 3, 1, termname());
    mvwaddstr(win, 4, 1, longname());
    if (termattrs() & A_BLINK)
	mvwaddstr(win, 5, 1, "This terminal supports blinking.");
    else
	mvwaddstr(win, 5, 1, "This terminal does NOT support blinking.");
#endif

    mvwaddnstr(win, 7, 5, "Have a nice day!ok", 16);
    wrefresh(win);

    mvwinnstr(win, 7, 5, Buffer, 18);
    mvaddstr(LINES - 2, 10, Buffer);
    refresh();
    Continue(win);
}

#if defined(PDCURSES) && !defined(XCURSES)
static void
resizeTest(WINDOW *dummy GCC_UNUSED)
{
    WINDOW *win1;

    savetty();

    clear();
    refresh();
#  if defined(OS2)
    resize_term(50, 120);
#  else
    resize_term(50, 80);
#  endif

    win1 = newwin(10, 50, 14, 25);
    if (win1 == NULL) {
	endwin();
	return;
    }
#ifdef A_COLOR
    if (has_colors()) {
	init_pair(3, COLOR_BLUE, COLOR_WHITE);
	wattrset(win1, COLOR_PAIR(3));
    }
#endif
    wclear(win1);

    mvwaddstr(win1, 1, 1, "The screen may now have 50 lines");
    Continue(win1);

    wclear(win1);
    resetty();

    mvwaddstr(win1, 1, 1, "The screen should now be reset");
    Continue(win1);

    delwin(win1);

    clear();
    refresh();

}
#endif

static void
padTest(WINDOW *dummy GCC_UNUSED)
{
    WINDOW *pad, *spad;

    if ((pad = newpad(50, 100)) != 0) {
	wattron(pad, A_REVERSE);
	mvwaddstr(pad, 5, 2, "This is a new pad");
	wattrset(pad, A_NORMAL);
	mvwaddstr(pad, 8, 0,
		  "The end of this line should be truncated here:except  now");
	mvwaddstr(pad, 11, 1, "This line should not appear.It will now");
	wmove(pad, 10, 1);
	wclrtoeol(pad);
	mvwaddstr(pad, 10, 1, " Press any key to continue");
	prefresh(pad, 0, 0, 0, 0, 10, 45);
	keypad(pad, TRUE);
	raw();
	wgetch(pad);

	spad = subpad(pad, 12, 25, 6, 52);
	mvwaddstr(spad, 2, 2, "This is a new subpad");
	box(spad, 0, 0);
	prefresh(pad, 0, 0, 0, 0, 15, 75);
	keypad(pad, TRUE);
	raw();
	wgetch(pad);

	mvwaddstr(pad, 35, 2, "This is displayed at line 35 in the pad");
	mvwaddstr(pad, 40, 1, " Press any key to continue");
	prefresh(pad, 30, 0, 0, 0, 10, 45);
	keypad(pad, TRUE);
	raw();
	wgetch(pad);

	delwin(pad);
    }
}

static void
display_menu(int old_option, int new_option)
{
    int i;

    assert((new_option >= 0) && (new_option < MAX_OPTIONS));

    attrset(A_NORMAL);
    mvaddstr(3, 20, "PDCurses Test Program");

    for (i = 0; i < (int) MAX_OPTIONS; i++)
	mvaddstr(5 + i, 25, command[i].text);

    if ((old_option >= 0) && (old_option < MAX_OPTIONS))
	mvaddstr(5 + old_option, 25, command[old_option].text);

    attrset(A_REVERSE);
    mvaddstr(5 + new_option, 25, command[new_option].text);
    attrset(A_NORMAL);
    mvaddstr(13, 3,
	     "Use Up and Down Arrows to select - Enter to run - Q to quit");
    refresh();
}

/****************************************************************************
 * Copyright (c) 2005-2007,2008 Free Software Foundation, Inc.              *
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
 * $Id: demo_menus.c,v 1.28 2008/08/23 20:31:54 tom Exp $
 *
 * Demonstrate a variety of functions from the menu library.
 * Thomas Dickey - 2005/4/9
 */
/*
item_description		-
item_init			-
item_opts			-
item_opts_off			-
item_opts_on			-
item_term			-
item_userptr			-
item_visible			-
menu_back			-
menu_fore			-
menu_format			-
menu_grey			-
menu_init			-
menu_opts			-
menu_pad			-
menu_request_by_name		-
menu_request_name		-
menu_sub			-
menu_term			-
menu_userptr			-
set_current_item		-
set_item_init			-
set_item_opts			-
set_item_term			-
set_item_userptr		-
set_menu_grey			-
set_menu_init			-
set_menu_items			-
set_menu_opts			-
set_menu_pad			-
set_menu_pattern		-
set_menu_spacing		-
set_menu_term			-
set_menu_userptr		-
set_top_row			-
top_row				-
*/

#include <test.priv.h>

#if USE_LIBMENU

#include <menu.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef NCURSES_VERSION
#ifdef TRACE
static unsigned save_trace = TRACE_ORDINARY | TRACE_CALLS;
extern unsigned _nc_tracing;
static MENU *mpTrace;
#endif
#else
#undef TRACE
#endif

typedef enum {
    eBanner = -1
    ,eFile
    ,eSelect
#ifdef TRACE
    ,eTrace
#endif
    ,eMAX
} MenuNo;

#define okMenuNo(n) (((n) > eBanner) && ((n) < eMAX))

#define MENU_Y	1

static MENU *mpBanner;
static MENU *mpFile;
static MENU *mpSelect;

static bool loaded_file = FALSE;

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

/* Common function to allow ^T to toggle trace-mode in the middle of a test
 * so that trace-files can be made smaller.
 */
static int
wGetchar(WINDOW *win)
{
    int c;
#ifdef TRACE
    while ((c = wgetch(win)) == CTRL('T')) {
	if (_nc_tracing) {
	    save_trace = _nc_tracing;
	    Trace(("TOGGLE-TRACING OFF"));
	    _nc_tracing = 0;
	} else {
	    _nc_tracing = save_trace;
	}
	trace(_nc_tracing);
	if (_nc_tracing)
	    Trace(("TOGGLE-TRACING ON"));
    }
#else
    c = wgetch(win);
#endif
    return c;
}
#define Getchar() wGetchar(stdscr)

static int
menu_virtualize(int c)
{
    int result;

    if (c == '\n' || c == KEY_EXIT)
	result = (MAX_COMMAND + 1);
    else if (c == 'u')
	result = (REQ_SCR_ULINE);
    else if (c == 'd')
	result = (REQ_SCR_DLINE);
    else if (c == 'b' || c == KEY_NPAGE)
	result = (REQ_SCR_UPAGE);
    else if (c == 'f' || c == KEY_PPAGE)
	result = (REQ_SCR_DPAGE);
    else if (c == 'l' || c == KEY_LEFT || c == KEY_BTAB)
	result = (REQ_LEFT_ITEM);
    else if (c == 'n' || c == KEY_DOWN)
	result = (REQ_NEXT_ITEM);
    else if (c == 'p' || c == KEY_UP)
	result = (REQ_PREV_ITEM);
    else if (c == 'r' || c == KEY_RIGHT || c == '\t')
	result = (REQ_RIGHT_ITEM);
    else if (c == ' ')
	result = (REQ_TOGGLE_ITEM);
    else {
	if (c != KEY_MOUSE)
	    beep();
	result = (c);
    }
    return result;
}

static int
menu_getc(MENU * m)
{
    return wGetchar(menu_win(m));
}

static int
menu_offset(MenuNo number)
{
    int result = 0;

    if (okMenuNo(number)) {
	int spc_desc, spc_rows, spc_cols;

#ifdef NCURSES_VERSION
	menu_spacing(mpBanner, &spc_desc, &spc_rows, &spc_cols);
#else
	spc_rows = 0;
#endif

	/* FIXME: MENU.itemlen seems the only way to get actual width of items */
	result = (number - (eBanner + 1)) * (mpBanner->itemlen + spc_rows);
    }
    return result;
}

static MENU *
menu_create(ITEM ** items, int count, int ncols, MenuNo number)
{
    MENU *result;
    WINDOW *menuwin;
    int mrows, mcols;
    int y = okMenuNo(number) ? MENU_Y : 0;
    int x = menu_offset(number);
    int margin = (y == MENU_Y) ? 1 : 0;
    int maxcol = (ncols + x) < COLS ? ncols : (COLS - x - 1);
    int maxrow = (count + 1) / ncols;

    if ((maxrow + y) >= (LINES - 4))
	maxrow = LINES - 4 - y;

    result = new_menu(items);

    if (has_colors()) {
	set_menu_fore(result, COLOR_PAIR(1));
	set_menu_back(result, COLOR_PAIR(2));
    }

    set_menu_format(result, maxrow, maxcol);
    scale_menu(result, &mrows, &mcols);

    if (mcols + (2 * margin + x) >= COLS)
	mcols = COLS - (2 * margin + x);

#ifdef TRACE
    if (number == eTrace)
	menu_opts_off(result, O_ONEVALUE);
    else
	menu_opts_on(result, O_ONEVALUE);
#endif

    menuwin = newwin(mrows + (2 * margin), mcols + (2 * margin), y, x);
    set_menu_win(result, menuwin);
    keypad(menuwin, TRUE);
    if (margin)
	box(menuwin, 0, 0);

    set_menu_sub(result, derwin(menuwin, mrows, mcols, margin, margin));

    post_menu(result);

    return result;
}

static void
menu_destroy(MENU * m)
{
    int count;

    Trace(("menu_destroy %p", m));
    if (m != 0) {
	ITEM **items = menu_items(m);
	const char *blob = 0;

	count = item_count(m);
	Trace(("menu_destroy %p count %d", m, count));
	if ((count > 0) && (m == mpSelect)) {
	    blob = item_name(*items);
	}

	unpost_menu(m);
	free_menu(m);

	/* free the extra data allocated in build_select_menu() */
	if ((count > 0) && (m == mpSelect)) {
	    if (blob && loaded_file) {
		Trace(("freeing blob %p", blob));
		free((char *) blob);
	    }
	    free(items);
	}
#ifdef TRACE
	if ((count > 0) && (m == mpTrace)) {
	    ITEM **ip = items;
	    while (*ip)
		free(*ip++);
	}
#endif
    }
}

/* force the given menu to appear */
static void
menu_display(MENU * m)
{
    touchwin(menu_win(m));
    wrefresh(menu_win(m));
}

/*****************************************************************************/

static void
build_file_menu(MenuNo number)
{
    static CONST_MENUS char *labels[] =
    {
	"Exit",
	(char *) 0
    };
    static ITEM *items[SIZEOF(labels)];

    ITEM **ip = items;
    CONST_MENUS char **ap;

    for (ap = labels; *ap; ap++)
	*ip++ = new_item(*ap, "");
    *ip = (ITEM *) 0;

    mpFile = menu_create(items, SIZEOF(labels) - 1, 1, number);
}

static int
perform_file_menu(int cmd)
{
    return menu_driver(mpFile, cmd);
}

/*****************************************************************************/

static void
build_select_menu(MenuNo number, char *filename)
{
    static CONST_MENUS char *labels[] =
    {
	"Lions",
	"Tigers",
	"Bears",
	"(Oh my!)",
	"Newts",
	"Platypi",
	"Lemurs",
	"(Oh really?!)",
	"Leopards",
	"Panthers",
	"Pumas",
	"Lions, Tigers, Bears, (Oh my!), Newts, Platypi, Lemurs",
	"Lions, Tigers, Bears, (Oh my!), Newts, Platypi, Lemurs, Lions, Tigers, Bears, (Oh my!), Newts, Platypi, Lemurs",
	(char *) 0
    };
    static ITEM **items;

    ITEM **ip;
    CONST_MENUS char **ap = 0;
    CONST_MENUS char **myList = 0;
    unsigned count = 0;

    if (filename != 0) {
	struct stat sb;
	if (stat(filename, &sb) == 0
	    && (sb.st_mode & S_IFMT) == S_IFREG
	    && sb.st_size != 0) {
	    size_t size = (size_t) sb.st_size;
	    unsigned j, k;
	    char *blob = typeMalloc(char, size + 1);
	    CONST_MENUS char **list = typeCalloc(CONST_MENUS char *, size + 1);

	    items = typeCalloc(ITEM *, size + 1);
	    Trace(("build_select_menu blob=%p, items=%p", blob, items));
	    if (blob != 0 && list != 0) {
		FILE *fp = fopen(filename, "r");
		if (fp != 0) {
		    if (fread(blob, sizeof(char), size, fp) == size) {
			bool mark = TRUE;
			for (j = k = 0; j < size; ++j) {
			    if (mark) {
				list[k++] = blob + j;
				mark = FALSE;
			    }
			    if (blob[j] == '\n') {
				blob[j] = '\0';
				if (k > 0 && *list[k - 1] == '\0')
				    --k;
				mark = TRUE;
			    } else if (blob[j] == '\t') {
				blob[j] = ' ';	/* menu items are printable */
			    }
			}
			list[k] = 0;
			count = k;
			ap = myList = list;
		    }
		    fclose(fp);
		}
		loaded_file = TRUE;
	    }
	}
    }
    if (ap == 0) {
	count = SIZEOF(labels) - 1;
	items = typeCalloc(ITEM *, count + 1);
	ap = labels;
    }

    ip = items;
    while (*ap != 0)
	*ip++ = new_item(*ap++, "");
    *ip = 0;

    mpSelect = menu_create(items, (int) count, 1, number);
    if (myList != 0)
	free(myList);
}

static int
perform_select_menu(int cmd)
{
    return menu_driver(mpSelect, cmd);
}

/*****************************************************************************/

#ifdef TRACE
#define T_TBL(name) { #name, name }
static struct {
    const char *name;
    unsigned mask;
} t_tbl[] = {

    T_TBL(TRACE_DISABLE),
	T_TBL(TRACE_TIMES),
	T_TBL(TRACE_TPUTS),
	T_TBL(TRACE_UPDATE),
	T_TBL(TRACE_MOVE),
	T_TBL(TRACE_CHARPUT),
	T_TBL(TRACE_ORDINARY),
	T_TBL(TRACE_CALLS),
	T_TBL(TRACE_VIRTPUT),
	T_TBL(TRACE_IEVENT),
	T_TBL(TRACE_BITS),
	T_TBL(TRACE_ICALLS),
	T_TBL(TRACE_CCALLS),
	T_TBL(TRACE_DATABASE),
	T_TBL(TRACE_ATTRS),
	T_TBL(TRACE_MAXIMUM),
    {
	(char *) 0, 0
    }
};

static void
build_trace_menu(MenuNo number)
{
    static ITEM *items[SIZEOF(t_tbl)];

    ITEM **ip = items;
    int n;

    for (n = 0; t_tbl[n].name != 0; n++)
	*ip++ = new_item(t_tbl[n].name, "");
    *ip = (ITEM *) 0;

    mpTrace = menu_create(items, SIZEOF(t_tbl) - 1, 2, number);
}

static char *
tracetrace(unsigned tlevel)
{
    static char *buf;
    int n;

    if (buf == 0) {
	size_t need = 12;
	for (n = 0; t_tbl[n].name != 0; n++)
	    need += strlen(t_tbl[n].name) + 2;
	buf = typeMalloc(char, need);
    }
    sprintf(buf, "0x%02x = {", tlevel);
    if (tlevel == 0) {
	sprintf(buf + strlen(buf), "%s, ", t_tbl[0].name);
    } else {
	for (n = 1; t_tbl[n].name != 0; n++)
	    if ((tlevel & t_tbl[n].mask) == t_tbl[n].mask) {
		strcat(buf, t_tbl[n].name);
		strcat(buf, ", ");
	    }
    }
    if (buf[strlen(buf) - 2] == ',')
	buf[strlen(buf) - 2] = '\0';
    return (strcat(buf, "}"));
}

/* fake a dynamically reconfigurable menu using the 0th entry to deselect
 * the others
 */
static bool
update_trace_menu(MENU * m)
{
    ITEM **items;
    ITEM *i, **p;
    bool changed = FALSE;

    items = menu_items(m);
    i = current_item(m);
    if (i == items[0]) {
	if (item_value(i)) {
	    for (p = items + 1; *p != 0; p++)
		if (item_value(*p)) {
		    set_item_value(*p, FALSE);
		    changed = TRUE;
		}
	}
    }
    return changed;
}

static int
perform_trace_menu(int cmd)
/* interactively set the trace level */
{
    ITEM **ip;
    unsigned newtrace;
    int result;

    for (ip = menu_items(mpTrace); *ip; ip++) {
	unsigned mask = t_tbl[item_index(*ip)].mask;
	if (mask == 0)
	    set_item_value(*ip, _nc_tracing == 0);
	else if ((mask & _nc_tracing) == mask)
	    set_item_value(*ip, TRUE);
    }

    result = menu_driver(mpTrace, cmd);

    if (result == E_OK) {
	if (update_trace_menu(mpTrace) || cmd == REQ_TOGGLE_ITEM) {
	    newtrace = 0;
	    for (ip = menu_items(mpTrace); *ip; ip++) {
		if (item_value(*ip))
		    newtrace |= t_tbl[item_index(*ip)].mask;
	    }
	    trace(newtrace);
	    Trace(("trace level interactively set to %s", tracetrace(_nc_tracing)));

	    (void) mvprintw(LINES - 2, 0,
			    "Trace level is %s\n", tracetrace(_nc_tracing));
	    refresh();
	}
    }
    return result;
}
#endif /* TRACE */

/*****************************************************************************/

static int
menu_number(void)
{
    return item_index(current_item(mpBanner)) - (eBanner + 1);
}

static MENU *
current_menu(void)
{
    MENU *result;

    switch (menu_number()) {
    case eFile:
	result = mpFile;
	break;
    case eSelect:
	result = mpSelect;
	break;
#ifdef TRACE
    case eTrace:
	result = mpTrace;
	break;
#endif
    default:
	result = 0;
	break;
    }
    return result;
}

static void
build_menus(char *filename)
{
    static CONST_MENUS char *labels[] =
    {
	"File",
	"Select",
#ifdef TRACE
	"Trace",
#endif
	(char *) 0
    };
    static ITEM *items[SIZEOF(labels)];

    ITEM **ip = items;
    CONST_MENUS char **ap;

    for (ap = labels; *ap; ap++)
	*ip++ = new_item(*ap, "");
    *ip = (ITEM *) 0;

    mpBanner = menu_create(items, SIZEOF(labels) - 1, SIZEOF(labels) - 1, eBanner);
    set_menu_mark(mpBanner, ">");

    build_file_menu(eFile);
    build_select_menu(eSelect, filename);
#ifdef TRACE
    build_trace_menu(eTrace);
#endif
}

static int
move_menu(MENU * menu, MENU * current, int by_y, int by_x)
{
    WINDOW *top_win = menu_win(menu);
    WINDOW *sub_win = menu_sub(menu);
    int y0, x0;
    int y1, x1;
    int result;

    getbegyx(top_win, y0, x0);
    y0 += by_y;
    x0 += by_x;

    getbegyx(sub_win, y1, x1);
    y1 += by_y;
    x1 += by_x;

    if ((result = mvwin(top_win, y0, x0)) != ERR) {
#if defined(NCURSES_VERSION_PATCH) && (NCURSES_VERSION_PATCH < 20060218)
	sub_win->_begy = y1;
	sub_win->_begx = x1;
#else
	mvwin(sub_win, y1, x1);
#endif
	if (menu == current) {
	    touchwin(top_win);
	    wnoutrefresh(top_win);
	}
    }
    return result;
}

/*
 * Move the menus around on the screen, to test mvwin().
 */
static void
move_menus(MENU * current, int by_y, int by_x)
{
    if (move_menu(mpBanner, current, by_y, by_x) != ERR) {
	erase();
	wnoutrefresh(stdscr);
	move_menu(mpFile, current, by_y, by_x);
	move_menu(mpSelect, current, by_y, by_x);
#ifdef TRACE
	move_menu(mpTrace, current, by_y, by_x);
#endif
	doupdate();
    }
}

static void
show_status(int ch, MENU * menu)
{
    move(LINES - 1, 0);
    printw("key %s, menu %d, mark %s, match %s",
	   keyname(ch),
	   menu_number(),
	   menu_mark(menu),
	   menu_pattern(menu));
    clrtoeol();
    refresh();
}

static void
perform_menus(void)
{
    MENU *this_menu;
    MENU *last_menu = mpFile;
    int code = E_UNKNOWN_COMMAND;
    int cmd;
    int ch = ERR;

#ifdef NCURSES_MOUSE_VERSION
    mousemask(ALL_MOUSE_EVENTS, (mmask_t *) 0);
#endif

    menu_display(last_menu);

    for (;;) {

	if (ch != ERR)
	    show_status(ch, last_menu);

	ch = menu_getc(mpBanner);

	/*
	 * Provide for moving the menu around in the screen using shifted
	 * cursor keys.
	 */
	switch (ch) {
	case KEY_SF:
	    move_menus(last_menu, 1, 0);
	    continue;
	case KEY_SR:
	    move_menus(last_menu, -1, 0);
	    continue;
	case KEY_SLEFT:
	    move_menus(last_menu, 0, -1);
	    continue;
	case KEY_SRIGHT:
	    move_menus(last_menu, 0, 1);
	    continue;
	}
	cmd = menu_virtualize(ch);

	switch (cmd) {
	    /*
	     * The banner menu acts solely to select one of the other menus.
	     * Move between its items, wrapping at the left/right limits.
	     */
	case REQ_LEFT_ITEM:
	case REQ_RIGHT_ITEM:
	    code = menu_driver(mpBanner, cmd);
	    if (code == E_REQUEST_DENIED) {
		if (menu_number() > 0)
		    code = menu_driver(mpBanner, REQ_FIRST_ITEM);
		else
		    code = menu_driver(mpBanner, REQ_LAST_ITEM);
	    }
	    break;
	default:
	    switch (menu_number()) {
	    case eFile:
		code = perform_file_menu(cmd);
		break;
	    case eSelect:
		code = perform_select_menu(cmd);
		break;
#ifdef TRACE
	    case eTrace:
		code = perform_trace_menu(cmd);
		break;
#endif
	    }

	    if ((code == E_REQUEST_DENIED) && (cmd == KEY_MOUSE)) {
		code = menu_driver(mpBanner, cmd);
	    }

	    break;
	}

	if (code == E_OK) {
	    this_menu = current_menu();
	    if (this_menu != last_menu) {
		move(1, 0);
		clrtobot();
		box(menu_win(this_menu), 0, 0);
		refresh();

		/* force the current menu to appear */
		menu_display(this_menu);

		last_menu = this_menu;
	    }
	}
	wrefresh(menu_win(last_menu));
	if (code == E_UNKNOWN_COMMAND
	    || code == E_NOT_POSTED) {
	    if (menu_number() == eFile)
		break;
	    beep();
	}
	if (code == E_REQUEST_DENIED)
	    beep();
	continue;
    }

#ifdef NCURSES_MOUSE_VERSION
    mousemask(0, (mmask_t *) 0);
#endif
}

static void
destroy_menus(void)
{
    menu_destroy(mpFile);
    menu_destroy(mpSelect);
#ifdef TRACE
    menu_destroy(mpTrace);
#endif
    menu_destroy(mpBanner);
}

#if HAVE_RIPOFFLINE
static int
rip_footer(WINDOW *win, int cols)
{
    wbkgd(win, A_REVERSE);
    werase(win);
    wmove(win, 0, 0);
    wprintw(win, "footer: %d columns", cols);
    wnoutrefresh(win);
    return OK;
}

static int
rip_header(WINDOW *win, int cols)
{
    wbkgd(win, A_REVERSE);
    werase(win);
    wmove(win, 0, 0);
    wprintw(win, "header: %d columns", cols);
    wnoutrefresh(win);
    return OK;
}
#endif /* HAVE_RIPOFFLINE */

static void
usage(void)
{
    static const char *const tbl[] =
    {
	"Usage: demo_menus [options]"
	,""
	,"Options:"
#if HAVE_RIPOFFLINE
	,"  -f       rip-off footer line (can repeat)"
	,"  -h       rip-off header line (can repeat)"
#endif
#ifdef TRACE
	,"  -t mask  specify default trace-level (may toggle with ^T)"
#endif
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

    setlocale(LC_ALL, "");

    while ((c = getopt(argc, argv, "a:de:fhmp:s:t:")) != -1) {
	switch (c) {
#if HAVE_RIPOFFLINE
	case 'f':
	    ripoffline(-1, rip_footer);
	    break;
	case 'h':
	    ripoffline(1, rip_header);
	    break;
#endif /* HAVE_RIPOFFLINE */
#ifdef TRACE
	case 't':
	    trace(strtoul(optarg, 0, 0));
	    break;
#endif
	default:
	    usage();
	}
    }

    initscr();
    noraw();
    cbreak();
    noecho();

    if (has_colors()) {
	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_WHITE);
    }
    build_menus(argc > 1 ? argv[1] : 0);
    perform_menus();
    destroy_menus();

    endwin();
    ExitProgram(EXIT_SUCCESS);
}
#else
int
main(void)
{
    printf("This program requires the curses menu library\n");
    ExitProgram(EXIT_FAILURE);
}
#endif

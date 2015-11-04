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
 * Author: Thomas E. Dickey (1998-on)
 *
 * $Id: ditto.c,v 1.32 2008/08/04 13:21:41 tom Exp $
 *
 * The program illustrates how to set up multiple screens from a single
 * program.
 *
 * If openpty() is supported, the command line parameters are titles for
 * the windows showing each screen's data.
 *
 * If openpty() is not supported, you must invoke the program by specifying
 * another terminal on the same machine by specifying its device, e.g.,
 *	ditto /dev/ttyp1
 */
#include <test.priv.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef USE_PTHREADS
#include <pthread.h>
#endif

#ifdef USE_XTERM_PTY
#include USE_OPENPTY_HEADER
#endif

#define MAX_FIFO 256

#define THIS_FIFO(n) ((n) % MAX_FIFO)
#define NEXT_FIFO(n) THIS_FIFO((n) + 1)

typedef struct {
    unsigned long sequence;
    int head;
    int tail;
    int data[MAX_FIFO];
} FIFO;

typedef struct {
    unsigned long sequence;
} PEEK;

/*
 * Data "owned" for a single screen.  Each screen is divided into windows that
 * show the text read from each terminal.  Input from a given screen will also
 * be read into one window per screen.
 */
typedef struct {
    FILE *input;
    FILE *output;
    SCREEN *screen;		/* this screen - curses internal data */
    int which1;			/* this screen's index in DITTO[] array */
    int length;			/* length of windows[] and peeks[] */
    char **titles;		/* per-window titles */
    WINDOW **windows;		/* display data from each screen */
    PEEK *peeks;		/* indices for each screen's fifo */
    FIFO fifo;			/* fifo for this screen */
#ifdef USE_PTHREADS
    pthread_t thread;
#endif
} DITTO;

/*
 * Structure used to pass multiple parameters via the use_screen()
 * single-parameter interface.
 */
typedef struct {
    int source;			/* which screen did character come from */
    int target;			/* which screen is character going to */
    DITTO *ditto;		/* data for all screens */
} DDATA;

static void
failed(const char *s)
{
    perror(s);
    ExitProgram(EXIT_FAILURE);
}

static void
usage(void)
{
    fprintf(stderr, "usage: ditto [terminal1 ...]\n");
    ExitProgram(EXIT_FAILURE);
}

/* Add to the head of the fifo, checking for overflow. */
static void
put_fifo(FIFO * fifo, int value)
{
    int next = NEXT_FIFO(fifo->head);
    if (next == fifo->tail)
	fifo->tail = NEXT_FIFO(fifo->tail);
    fifo->data[next] = value;
    fifo->head = next;
    fifo->sequence += 1;
}

/* Get data from the tail (oldest part) of the fifo, returning -1 if no data.
 * Since each screen can peek into the fifo, we do not update the tail index,
 * but modify the peek-index.
 *
 * FIXME - test/workaround for case where fifo gets more than a buffer
 * ahead of peek.
 */
static int
peek_fifo(FIFO * fifo, PEEK * peek)
{
    int result = -1;
    if (peek->sequence < fifo->sequence) {
	peek->sequence += 1;
	result = fifo->data[THIS_FIFO(peek->sequence)];
    }
    return result;
}

static FILE *
open_tty(char *path)
{
    FILE *fp;
#ifdef USE_XTERM_PTY
    int amaster;
    int aslave;
    char slave_name[1024];
    char s_option[sizeof(slave_name) + 80];
    char *leaf;

    if (openpty(&amaster, &aslave, slave_name, 0, 0) != 0
	|| strlen(slave_name) > sizeof(slave_name) - 1)
	failed("openpty");
    if ((leaf = strrchr(slave_name, '/')) == 0) {
	errno = EISDIR;
	failed(slave_name);
    }
    sprintf(s_option, "-S%s/%d", slave_name, aslave);
    if (fork()) {
	execlp("xterm", "xterm", s_option, "-title", path, (char *) 0);
	_exit(0);
    }
    fp = fdopen(amaster, "r+");
    if (fp == 0)
	failed(path);
#else
    struct stat sb;

    if (stat(path, &sb) < 0)
	failed(path);
    if ((sb.st_mode & S_IFMT) != S_IFCHR) {
	errno = ENOTTY;
	failed(path);
    }
    fp = fopen(path, "r+");
    if (fp == 0)
	failed(path);
    printf("opened %s\n", path);
#endif
    assert(fp != 0);
    return fp;
}

static void
init_screen(SCREEN *sp GCC_UNUSED, void *arg)
{
    DITTO *target = (DITTO *) arg;
    int high, wide;
    int k;

    cbreak();
    noecho();
    scrollok(stdscr, TRUE);
    box(stdscr, 0, 0);

    target->windows = typeCalloc(WINDOW *, (size_t) target->length);
    target->peeks = typeCalloc(PEEK, (size_t) target->length);

    high = (LINES - 2) / target->length;
    wide = (COLS - 2);
    for (k = 0; k < target->length; ++k) {
	WINDOW *outer = newwin(high, wide, 1 + (high * k), 1);
	WINDOW *inner = derwin(outer, high - 2, wide - 2, 1, 1);

	box(outer, 0, 0);
	mvwaddstr(outer, 0, 2, target->titles[k]);
	wnoutrefresh(outer);

	scrollok(inner, TRUE);
	keypad(inner, TRUE);
#ifndef USE_PTHREADS
	nodelay(inner, TRUE);
#endif

	target->windows[k] = inner;
    }
    doupdate();
}

static void
open_screen(DITTO * target, char **source, int length, int which1)
{
    if (which1 != 0) {
	target->input =
	    target->output = open_tty(source[which1]);
    } else {
	target->input = stdin;
	target->output = stdout;
    }

    target->which1 = which1;
    target->titles = source;
    target->length = length;
    target->screen = newterm((char *) 0,	/* assume $TERM is the same */
			     target->output,
			     target->input);

    if (target->screen == 0)
	failed("newterm");

    (void) USING_SCREEN(target->screen, init_screen, target);
}

static int
close_screen(SCREEN *sp GCC_UNUSED, void *arg GCC_UNUSED)
{
    (void) sp;
    (void) arg;
    return endwin();
}

/*
 * Read data from the 'source' screen.
 */
static int
read_screen(SCREEN *sp GCC_UNUSED, void *arg)
{
    DDATA *data = (DDATA *) arg;
    DITTO *ditto = &(data->ditto[data->source]);
    WINDOW *win = ditto->windows[data->source];
    int ch = wgetch(win);

    if (ch > 0 && ch < 256)
	put_fifo(&(ditto->fifo), ch);
    else
	ch = ERR;

    return ch;
}

/*
 * Write all of the data that's in fifos for the 'target' screen.
 */
static int
write_screen(SCREEN *sp GCC_UNUSED, void *arg GCC_UNUSED)
{
    DDATA *data = (DDATA *) arg;
    DITTO *ditto = &(data->ditto[data->target]);
    bool changed = FALSE;
    int which;

    for (which = 0; which < ditto->length; ++which) {
	WINDOW *win = ditto->windows[which];
	FIFO *fifo = &(data->ditto[which].fifo);
	PEEK *peek = &(ditto->peeks[which]);
	int ch;

	while ((ch = peek_fifo(fifo, peek)) > 0) {
	    changed = TRUE;

	    waddch(win, (chtype) ch);
	    wnoutrefresh(win);
	}
    }

    if (changed)
	doupdate();
    return OK;
}

static void
show_ditto(DITTO * data, int count, DDATA * ddata)
{
    int n;

    for (n = 0; n < count; n++) {
	ddata->target = n;
	USING_SCREEN(data[n].screen, write_screen, (void *) ddata);
    }
}

#ifdef USE_PTHREADS
static void *
handle_screen(void *arg)
{
    DDATA ddata;
    int ch;

    memset(&ddata, 0, sizeof(ddata));
    ddata.ditto = (DITTO *) arg;
    ddata.source = ddata.ditto->which1;
    ddata.ditto -= ddata.source;	/* -> base of array */

    for (;;) {
	ch = read_screen(ddata.ditto->screen, &ddata);
	if (ch == CTRL('D')) {
	    int later = (ddata.source ? ddata.source : -1);
	    int j;

	    for (j = ddata.ditto->length - 1; j > 0; --j) {
		if (j != later) {
		    pthread_cancel(ddata.ditto[j].thread);
		}
	    }
	    if (later > 0) {
		pthread_cancel(ddata.ditto[later].thread);
	    }
	    break;
	}
	show_ditto(ddata.ditto, ddata.ditto->length, &ddata);
    }
    return NULL;
}
#endif

int
main(int argc, char *argv[])
{
    int j;
    DITTO *data;
#ifndef USE_PTHREADS
    int count;
#endif

    if (argc <= 1)
	usage();

    if ((data = typeCalloc(DITTO, (size_t) argc)) == 0)
	failed("calloc data");

    for (j = 0; j < argc; j++) {
	open_screen(&data[j], argv, argc, j);
    }

#ifdef USE_PTHREADS
    /*
     * For multi-threaded operation, set up a reader for each of the screens.
     * That uses blocking I/O rather than polling for input, so no calls to
     * napms() are needed.
     */
    for (j = 0; j < argc; j++) {
	(void) pthread_create(&(data[j].thread), NULL, handle_screen, &data[j]);
    }
    pthread_join(data[1].thread, NULL);
#else
    /*
     * Loop, reading characters from any of the inputs and writing to all
     * of the screens.
     */
    for (count = 0;; ++count) {
	DDATA ddata;
	int ch;
	int which = (count % argc);

	napms(20);

	ddata.source = which;
	ddata.ditto = data;

	ch = USING_SCREEN(data[which].screen, read_screen, &ddata);
	if (ch == CTRL('D')) {
	    break;
	} else if (ch != ERR) {
	    show_ditto(data, argc, &ddata);
	}
    }
#endif

    /*
     * Cleanup and exit
     */
    for (j = argc - 1; j >= 0; j--) {
	USING_SCREEN(data[j].screen, close_screen, 0);
	fprintf(data[j].output, "**Closed\r\n");

	/*
	 * Closing before a delscreen() helps ncurses determine that there
	 * is no valid output buffer, and can remove the setbuf() data.
	 */
	fflush(data[j].output);
	fclose(data[j].output);
	delscreen(data[j].screen);
    }
    ExitProgram(EXIT_SUCCESS);
}

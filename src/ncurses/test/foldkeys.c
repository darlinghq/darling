/****************************************************************************
 * Copyright (c) 2006 Free Software Foundation, Inc.                        *
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
 * Author: Thomas E. Dickey, 2006
 *
 * $Id: foldkeys.c,v 1.3 2006/12/09 16:54:07 tom Exp $
 *
 * Demonstrate a method for altering key definitions at runtime.
 *
 * This program reads the key definitions, merging those which have xterm-style
 * modifiers into their equivalents which have no modifiers.  It does this
 * merging only for the keys which are defined in the terminal description.
 */

#include <test.priv.h>

#if defined(NCURSES_VERSION) && NCURSES_EXT_FUNCS

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

#define MY_LOGFILE "demo_foldkeys.log"
#define MY_KEYS (KEY_MAX + 1)

/*
 * Log the most recently-written line to our logfile
 */
static void
log_last_line(WINDOW *win)
{
    FILE *fp;
    int y, x, n;
    char temp[256];

    if ((fp = fopen(MY_LOGFILE, "a")) != 0) {
	int need = sizeof(temp) - 1;
	if (need > COLS)
	    need = COLS;
	getyx(win, y, x);
	wmove(win, y - 1, 0);
	n = winnstr(win, temp, need);
	while (n-- > 0) {
	    if (isspace(UChar(temp[n])))
		temp[n] = '\0';
	    else
		break;
	}
	wmove(win, y, x);
	fprintf(fp, "%s\n", temp);
	fclose(fp);
    }
}

/*
 * ncurses has no API for telling what the actual last key-code is.  That is
 * a secret because the codes past KEY_MAX are computed at run-time and may
 * differ depending on the previous calls to newterm(), etc.  It is unlikely
 * that one could have more than a thousand key definitions...
 */
#define MAX_KEYS 2000

typedef struct {
    const char *name;
    const char *value;
    int code;
    int state;
} KeyInfo;

static void
demo_foldkeys(void)
{
    KeyInfo info[MAX_KEYS];
    int info_len = 0;
    int merged = 0;
    int code;
    int j, k;

    /*
     * Tell ncurses that we want to use function keys.  That will make it add
     * any user-defined keys that appear in the terminfo.
     */
    keypad(stdscr, TRUE);

    /*
     * List the predefined keys using the strnames[] array.
     */
    for (code = 0; code < STRCOUNT; ++code) {
	NCURSES_CONST char *name = strnames[code];
	NCURSES_CONST char *value = tigetstr(name);
	if (value != 0 && value != (NCURSES_CONST char *) -1) {
	    info[info_len].name = strnames[code];
	    info[info_len].code = key_defined(value);
	    info[info_len].value = value;
	    info[info_len].state = 0;
	    if (info[info_len].code > 0)
		++info_len;
	}
    }

    /*
     * We can get the names for user-defined keys from keyname().  It returns
     * a name like KEY_foo for the predefined keys, which tigetstr() does not
     * understand.
     */
    for (code = KEY_MAX; code < MAX_KEYS; ++code) {
	NCURSES_CONST char *name = keyname(code);
	if (name != 0) {
	    info[info_len].name = name;
	    info[info_len].code = code;
	    info[info_len].value = tigetstr(name);
	    info[info_len].state = 0;
	    ++info_len;
	}
    }
    printw("Initially %d key definitions\n", info_len);

    /*
     * Look for keys that have xterm-style modifiers.
     */
    for (j = 0; j < info_len; ++j) {
	int first, second;
	char final[2];
	char *value;
	if (info[j].state == 0
	    && sscanf(info[j].value,
		      "\033[%d;%d%c",
		      &first,
		      &second,
		      final) == 3
	    && *final != ';'
	    && (value = strdup(info[j].value)) != 0) {
	    sprintf(value, "\033[%d%c", first, *final);
	    for (k = 0; k < info_len; ++k) {
		if (info[k].state == 0
		    && !strcmp(info[k].value, value)) {
		    info[j].state = 1;
		    break;
		}
	    }
	    if (info[j].state == 0) {
		sprintf(value, "\033O%c", *final);
		for (k = 0; k < info_len; ++k) {
		    if (info[k].state == 0
			&& !strcmp(info[k].value, value)) {
			info[j].state = 1;
			break;
		    }
		}
	    }
	    if (info[j].state == 1) {
		if ((define_key(info[j].value, info[k].code)) != ERR) {
		    printw("map %s to %s\n", info[j].value, info[k].value);
		    keyok(info[j].code, FALSE);
		    ++merged;
		} else {
		    printw("? cannot define_key %d:%s\n", j, info[j].value);
		}
	    } else {
		printw("? cannot merge %d:%s\n", j, info[j].value);
	    }
	    free(value);
	}
    }
    printw("Merged to %d key definitions\n", info_len - merged);
}

int
main(int argc GCC_UNUSED, char *argv[]GCC_UNUSED)
{
    int ch;
#if HAVE_GETTIMEOFDAY
    int secs, msecs;
    struct timeval current, previous;
#endif

    unlink(MY_LOGFILE);

    newterm(0, stdout, stdin);
    (void) cbreak();		/* take input chars one at a time, no wait for \n */
    (void) noecho();		/* don't echo input */

    scrollok(stdscr, TRUE);
    keypad(stdscr, TRUE);
    move(0, 0);

    demo_foldkeys();

#if HAVE_GETTIMEOFDAY
    gettimeofday(&previous, 0);
#endif

    while ((ch = getch()) != ERR) {
	bool escaped = (ch >= MY_KEYS);
	const char *name = keyname(escaped ? (ch - MY_KEYS) : ch);

#if HAVE_GETTIMEOFDAY
	gettimeofday(&current, 0);
	secs = current.tv_sec - previous.tv_sec;
	msecs = (current.tv_usec - previous.tv_usec) / 1000;
	if (msecs < 0) {
	    msecs += 1000;
	    --secs;
	}
	if (msecs >= 1000) {
	    secs += msecs / 1000;
	    msecs %= 1000;
	}
	printw("%6d.%03d ", secs, msecs);
	previous = current;
#endif
	printw("Keycode %d, name %s%s\n",
	       ch,
	       escaped ? "ESC-" : "",
	       name != 0 ? name : "<null>");
	log_last_line(stdscr);
	clrtoeol();
	if (ch == 'q')
	    break;
    }
    endwin();
    ExitProgram(EXIT_SUCCESS);
}
#else
int
main(void)
{
    printf("This program requires the ncurses library\n");
    ExitProgram(EXIT_FAILURE);
}
#endif

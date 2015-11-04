/****************************************************************************
 * Copyright (c) 2000-2007,2008 Free Software Foundation, Inc.              *
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
 * Author: Thomas E. Dickey - 2000
 *
 * $Id: railroad.c,v 1.16 2008/02/09 18:08:43 tom Exp $
 *
 * A simple demo of the termcap interface.
 */
#define USE_TINFO
#include <test.priv.h>

#if HAVE_TGETENT

static char *wipeit;
static char *moveit;
static int length;
static int height;

static char *finisC;
static char *finisS;
static char *finisU;

static char *startC;
static char *startS;
static char *startU;

static char *backup;

static bool interrupted = FALSE;

static int
outc(TPUTS_ARG c)
{
    if (interrupted) {
	char tmp = c;
	write(STDOUT_FILENO, &tmp, 1);
    } else {
	putc(c, stdout);
    }
    return 0;
}

static void
PutChar(int ch)
{
    putchar(ch);
    fflush(stdout);
    napms(moveit ? 10 : 50);	/* not really termcap... */
}

static void
Backup(void)
{
    tputs(backup != 0 ? backup : "\b", 1, outc);
}

static void
ShowCursor(int flag)
{
    if (startC != 0 && finisC != 0) {
	tputs(flag ? startC : finisC, 1, outc);
    }
}

static void
StandOut(int flag)
{
    if (startS != 0 && finisS != 0) {
	tputs(flag ? startS : finisS, 1, outc);
    }
}

static void
Underline(int flag)
{
    if (startU != 0 && finisU != 0) {
	tputs(flag ? startU : finisU, 1, outc);
    }
}

static void
ShowSign(char *string)
{
    char *base = string;
    int ch, first, last;

    if (moveit != 0) {
	tputs(tgoto(moveit, 0, height - 1), 1, outc);
	tputs(wipeit, 1, outc);
    }

    while (*string != 0) {
	ch = *string;
	if (ch != ' ') {
	    if (moveit != 0) {
		for (first = length - 2; first >= (string - base); first--) {
		    if (first < length - 1) {
			tputs(tgoto(moveit, first + 1, height - 1), 1, outc);
			PutChar(' ');
		    }
		    tputs(tgoto(moveit, first, height - 1), 1, outc);
		    PutChar(ch);
		}
	    } else {
		last = ch;
		if (isalpha(ch)) {
		    first = isupper(ch) ? 'A' : 'a';
		} else if (isdigit(ch)) {
		    first = '0';
		} else {
		    first = ch;
		}
		if (first < last) {
		    Underline(1);
		    while (first < last) {
			PutChar(first);
			Backup();
			first++;
		    }
		    Underline(0);
		}
	    }
	    if (moveit != 0)
		Backup();
	}
	StandOut(1);
	PutChar(ch);
	StandOut(0);
	fflush(stdout);
	string++;
    }
    if (moveit != 0)
	tputs(wipeit, 1, outc);
    putchar('\n');
}

static void
cleanup(void)
{
    Underline(0);
    StandOut(0);
    ShowCursor(1);
}

static void
onsig(int n GCC_UNUSED)
{
    interrupted = TRUE;
    cleanup();
    ExitProgram(EXIT_FAILURE);
}

static void
railroad(char **args)
{
    NCURSES_CONST char *name = getenv("TERM");
    char buffer[1024];
    char area[1024], *ap = area;

    if (name == 0)
	name = "dumb";
    if (tgetent(buffer, name) >= 0) {

	wipeit = tgetstr("ce", &ap);
	height = tgetnum("li");
	length = tgetnum("co");
	moveit = tgetstr("cm", &ap);

	if (wipeit == 0
	    || moveit == 0
	    || height <= 0
	    || length <= 0) {
	    wipeit = 0;
	    moveit = 0;
	    height = 0;
	    length = 0;
	}

	startS = tgetstr("so", &ap);
	finisS = tgetstr("se", &ap);

	startU = tgetstr("us", &ap);
	finisU = tgetstr("ue", &ap);

	backup = tgetstr("le", &ap);

	startC = tgetstr("ve", &ap);
	finisC = tgetstr("vi", &ap);

	ShowCursor(0);

	CATCHALL(onsig);

	while (*args) {
	    ShowSign(*args++);
	}
	ShowCursor(1);
    }
}

int
main(int argc, char *argv[])
{
    if (argc > 1) {
	railroad(argv + 1);
    } else {
	static char world[] = "Hello World";
	static char *hello[] =
	{world, 0};
	railroad(hello);
    }
    ExitProgram(EXIT_SUCCESS);
}

#else
int
main(int argc GCC_UNUSED,
     char *argv[]GCC_UNUSED)
{
    printf("This program requires termcap\n");
    exit(EXIT_FAILURE);
}
#endif

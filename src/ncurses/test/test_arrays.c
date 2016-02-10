/****************************************************************************
 * Copyright (c) 2007,2008 Free Software Foundation, Inc.                   *
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
 * $Id: test_arrays.c,v 1.3 2008/02/09 18:09:43 tom Exp $
 *
 * Author: Thomas E Dickey
 *
 * Demonstrate the public arrays from the terminfo library.

extern NCURSES_EXPORT_VAR(NCURSES_CONST char * const ) boolnames[];
extern NCURSES_EXPORT_VAR(NCURSES_CONST char * const ) boolcodes[];
extern NCURSES_EXPORT_VAR(NCURSES_CONST char * const ) boolfnames[];
extern NCURSES_EXPORT_VAR(NCURSES_CONST char * const ) numnames[];
extern NCURSES_EXPORT_VAR(NCURSES_CONST char * const ) numcodes[];
extern NCURSES_EXPORT_VAR(NCURSES_CONST char * const ) numfnames[];
extern NCURSES_EXPORT_VAR(NCURSES_CONST char * const ) strnames[];
extern NCURSES_EXPORT_VAR(NCURSES_CONST char * const ) strcodes[];
extern NCURSES_EXPORT_VAR(NCURSES_CONST char * const ) strfnames[];

 */

#define USE_TINFO
#include <test.priv.h>

#if defined(NCURSES_VERSION) || !defined(_XOPEN_CURSES)

#define DUMP(name) dump_array(#name, name)

static void
dump_array(const char *name, NCURSES_CONST char *const *list)
{
    int n;

    printf("%s:\n", name);
    for (n = 0; list[n] != 0; ++n) {
	printf("%5d:%s\n", n, list[n]);
    }
}

int
main(int argc GCC_UNUSED, char *argv[]GCC_UNUSED)
{
    DUMP(boolnames);
    DUMP(boolcodes);
    DUMP(boolfnames);

    DUMP(numnames);
    DUMP(numcodes);
    DUMP(numfnames);

    DUMP(strnames);
    DUMP(strcodes);
    DUMP(strfnames);

    ExitProgram(EXIT_SUCCESS);
}

#else
int
main(int argc GCC_UNUSED, char *argv[]GCC_UNUSED)
{
    printf("This program requires the terminfo arrays\n");
    ExitProgram(EXIT_FAILURE);
}
#endif

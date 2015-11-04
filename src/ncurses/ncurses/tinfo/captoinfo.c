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

/****************************************************************************
 *  Author: Zeyd M. Ben-Halim <zmbenhal@netcom.com> 1992,1995               *
 *     and: Eric S. Raymond <esr@snark.thyrsus.com>                         *
 *     and: Thomas E. Dickey                        1996-on                 *
 ****************************************************************************/

/*
 *	captoinfo.c --- conversion between termcap and terminfo formats
 *
 *	The captoinfo() code was swiped from Ross Ridge's mytinfo package,
 *	adapted to fit ncurses by Eric S. Raymond <esr@snark.thyrsus.com>.
 *
 *	There is just one entry point:
 *
 *	char *_nc_captoinfo(n, s, parameterized)
 *
 *	Convert value s for termcap string capability named n into terminfo
 *	format.
 *
 *	This code recognizes all the standard 4.4BSD %-escapes:
 *
 *	%%       output `%'
 *	%d       output value as in printf %d
 *	%2       output value as in printf %2d
 *	%3       output value as in printf %3d
 *	%.       output value as in printf %c
 *	%+x      add x to value, then do %.
 *	%>xy     if value > x then add y, no output
 *	%r       reverse order of two parameters, no output
 *	%i       increment by one, no output
 *	%n       exclusive-or all parameters with 0140 (Datamedia 2500)
 *	%B       BCD (16*(value/10)) + (value%10), no output
 *	%D       Reverse coding (value - 2*(value%16)), no output (Delta Data).
 *
 *	Also, %02 and %03 are accepted as synonyms for %2 and %3.
 *
 *	Besides all the standard termcap escapes, this translator understands
 *	the following extended escapes:
 *
 *	used by GNU Emacs termcap libraries
 *		%a[+*-/=][cp]x	GNU arithmetic.
 *		%m		xor the first two parameters by 0177
 *		%b		backup to previous parameter
 *		%f		skip this parameter
 *
 *	used by the University of Waterloo (MFCF) termcap libraries
 *		%-x	 subtract parameter FROM char x and output it as a char
 *		%ax	 add the character x to parameter
 *
 *	If #define WATERLOO is on, also enable these translations:
 *
 *		%sx	 subtract parameter FROM the character x
 *
 *	By default, this Waterloo translations are not compiled in, because
 *	the Waterloo %s conflicts with the way terminfo uses %s in strings for
 *	function programming.
 *
 *	Note the two definitions of %a: the GNU definition is translated if the
 *	characters after the 'a' are valid for it, otherwise the UW definition
 *	is translated.
 */

#include <curses.priv.h>

#include <ctype.h>
#include <tic.h>

MODULE_ID("$Id: captoinfo.c,v 1.52 2008/08/16 19:24:51 tom Exp $")

#define MAX_PUSHED	16	/* max # args we can push onto the stack */

static int stack[MAX_PUSHED];	/* the stack */
static int stackptr;		/* the next empty place on the stack */
static int onstack;		/* the top of stack */
static int seenm;		/* seen a %m */
static int seenn;		/* seen a %n */
static int seenr;		/* seen a %r */
static int param;		/* current parameter */
static char *dp;		/* pointer to end of the converted string */

static char *my_string;
static size_t my_length;

static char *
init_string(void)
/* initialize 'my_string', 'my_length' */
{
    if (my_string == 0)
	my_string = typeMalloc(char, my_length = 256);
    if (my_string == 0)
	_nc_err_abort(MSG_NO_MEMORY);

    *my_string = '\0';
    return my_string;
}

static char *
save_string(char *d, const char *const s)
{
    size_t have = (d - my_string);
    size_t need = have + strlen(s) + 2;
    if (need > my_length) {
	my_string = (char *) realloc(my_string, my_length = (need + need));
	if (my_string == 0)
	    _nc_err_abort(MSG_NO_MEMORY);
	d = my_string + have;
    }
    (void) strcpy(d, s);
    return d + strlen(d);
}

static NCURSES_INLINE char *
save_char(char *s, int c)
{
    static char temp[2];
    temp[0] = (char) c;
    return save_string(s, temp);
}

static void
push(void)
/* push onstack on to the stack */
{
    if (stackptr >= MAX_PUSHED)
	_nc_warning("string too complex to convert");
    else
	stack[stackptr++] = onstack;
}

static void
pop(void)
/* pop the top of the stack into onstack */
{
    if (stackptr == 0) {
	if (onstack == 0)
	    _nc_warning("I'm confused");
	else
	    onstack = 0;
    } else
	onstack = stack[--stackptr];
    param++;
}

static int
cvtchar(register const char *sp)
/* convert a character to a terminfo push */
{
    unsigned char c = 0;
    int len;

    switch (*sp) {
    case '\\':
	switch (*++sp) {
	case '\'':
	case '$':
	case '\\':
	case '%':
	    c = (unsigned char) (*sp);
	    len = 2;
	    break;
	case '\0':
	    c = '\\';
	    len = 1;
	    break;
	case '0':
	case '1':
	case '2':
	case '3':
	    len = 1;
	    while (isdigit(UChar(*sp))) {
		c = 8 * c + (*sp++ - '0');
		len++;
	    }
	    break;
	default:
	    c = (unsigned char) (*sp);
	    len = 2;
	    break;
	}
	break;
    case '^':
	c = (*++sp & 0x1f);
	len = 2;
	break;
    default:
	c = (unsigned char) (*sp);
	len = 1;
    }
    if (isgraph(c) && c != ',' && c != '\'' && c != '\\' && c != ':') {
	dp = save_string(dp, "%\'");
	dp = save_char(dp, c);
	dp = save_char(dp, '\'');
    } else {
	dp = save_string(dp, "%{");
	if (c > 99)
	    dp = save_char(dp, c / 100 + '0');
	if (c > 9)
	    dp = save_char(dp, ((int) (c / 10)) % 10 + '0');
	dp = save_char(dp, c % 10 + '0');
	dp = save_char(dp, '}');
    }
    return len;
}

static void
getparm(int parm, int n)
/* push n copies of param on the terminfo stack if not already there */
{
    if (seenr) {
	if (parm == 1)
	    parm = 2;
	else if (parm == 2)
	    parm = 1;
    }
    if (onstack == parm) {
	if (n > 1) {
	    _nc_warning("string may not be optimal");
	    dp = save_string(dp, "%Pa");
	    while (n--) {
		dp = save_string(dp, "%ga");
	    }
	}
	return;
    }
    if (onstack != 0)
	push();

    onstack = parm;

    while (n--) {
	dp = save_string(dp, "%p");
	dp = save_char(dp, '0' + parm);
    }

    if (seenn && parm < 3) {
	dp = save_string(dp, "%{96}%^");
    }

    if (seenm && parm < 3) {
	dp = save_string(dp, "%{127}%^");
    }
}

/*
 * Convert a termcap string to terminfo format.
 * 'cap' is the relevant terminfo capability index.
 * 's' is the string value of the capability.
 * 'parameterized' tells what type of translations to do:
 *	% translations if 1
 *	pad translations if >=0
 */
NCURSES_EXPORT(char *)
_nc_captoinfo(const char *cap, const char *s, int const parameterized)
{
    const char *capstart;

    stackptr = 0;
    onstack = 0;
    seenm = 0;
    seenn = 0;
    seenr = 0;
    param = 1;

    dp = init_string();

    /* skip the initial padding (if we haven't been told not to) */
    capstart = 0;
    if (s == 0)
	s = "";
    if (parameterized >= 0 && isdigit(UChar(*s)))
	for (capstart = s;; s++)
	    if (!(isdigit(UChar(*s)) || *s == '*' || *s == '.'))
		break;

    while (*s != '\0') {
	switch (*s) {
	case '%':
	    s++;
	    if (parameterized < 1) {
		dp = save_char(dp, '%');
		break;
	    }
	    switch (*s++) {
	    case '%':
		dp = save_char(dp, '%');
		break;
	    case 'r':
		if (seenr++ == 1) {
		    _nc_warning("saw %%r twice in %s", cap);
		}
		break;
	    case 'm':
		if (seenm++ == 1) {
		    _nc_warning("saw %%m twice in %s", cap);
		}
		break;
	    case 'n':
		if (seenn++ == 1) {
		    _nc_warning("saw %%n twice in %s", cap);
		}
		break;
	    case 'i':
		dp = save_string(dp, "%i");
		break;
	    case '6':
	    case 'B':
		getparm(param, 1);
		dp = save_string(dp, "%{10}%/%{16}%*");
		getparm(param, 1);
		dp = save_string(dp, "%{10}%m%+");
		break;
	    case '8':
	    case 'D':
		getparm(param, 2);
		dp = save_string(dp, "%{2}%*%-");
		break;
	    case '>':
		getparm(param, 2);
		/* %?%{x}%>%t%{y}%+%; */
		dp = save_string(dp, "%?");
		s += cvtchar(s);
		dp = save_string(dp, "%>%t");
		s += cvtchar(s);
		dp = save_string(dp, "%+%;");
		break;
	    case 'a':
		if ((*s == '=' || *s == '+' || *s == '-'
		     || *s == '*' || *s == '/')
		    && (s[1] == 'p' || s[1] == 'c')
		    && s[2] != '\0') {
		    int l;
		    l = 2;
		    if (*s != '=')
			getparm(param, 1);
		    if (s[1] == 'p') {
			getparm(param + s[2] - '@', 1);
			if (param != onstack) {
			    pop();
			    param--;
			}
			l++;
		    } else
			l += cvtchar(s + 2);
		    switch (*s) {
		    case '+':
			dp = save_string(dp, "%+");
			break;
		    case '-':
			dp = save_string(dp, "%-");
			break;
		    case '*':
			dp = save_string(dp, "%*");
			break;
		    case '/':
			dp = save_string(dp, "%/");
			break;
		    case '=':
			if (seenr) {
			    if (param == 1)
				onstack = 2;
			    else if (param == 2)
				onstack = 1;
			    else
				onstack = param;
			} else
			    onstack = param;
			break;
		    }
		    s += l;
		    break;
		}
		getparm(param, 1);
		s += cvtchar(s);
		dp = save_string(dp, "%+");
		break;
	    case '+':
		getparm(param, 1);
		s += cvtchar(s);
		dp = save_string(dp, "%+%c");
		pop();
		break;
	    case 's':
#ifdef WATERLOO
		s += cvtchar(s);
		getparm(param, 1);
		dp = save_string(dp, "%-");
#else
		getparm(param, 1);
		dp = save_string(dp, "%s");
		pop();
#endif /* WATERLOO */
		break;
	    case '-':
		s += cvtchar(s);
		getparm(param, 1);
		dp = save_string(dp, "%-%c");
		pop();
		break;
	    case '.':
		getparm(param, 1);
		dp = save_string(dp, "%c");
		pop();
		break;
	    case '0':		/* not clear any of the historical termcaps did this */
		if (*s == '3')
		    goto see03;
		else if (*s != '2')
		    goto invalid;
		/* FALLTHRU */
	    case '2':
		getparm(param, 1);
		dp = save_string(dp, "%2d");
		pop();
		break;
	    case '3':
	      see03:
		getparm(param, 1);
		dp = save_string(dp, "%3d");
		pop();
		break;
	    case 'd':
		getparm(param, 1);
		dp = save_string(dp, "%d");
		pop();
		break;
	    case 'f':
		param++;
		break;
	    case 'b':
		param--;
		break;
	    case '\\':
		dp = save_string(dp, "%\\");
		break;
	    default:
	      invalid:
		dp = save_char(dp, '%');
		s--;
		_nc_warning("unknown %% code %s (%#x) in %s",
			    unctrl((chtype) *s), UChar(*s), cap);
		break;
	    }
	    break;
#ifdef REVISIBILIZE
	case '\\':
	    dp = save_char(dp, *s++);
	    dp = save_char(dp, *s++);
	    break;
	case '\n':
	    dp = save_string(dp, "\\n");
	    s++;
	    break;
	case '\t':
	    dp = save_string(dp, "\\t");
	    s++;
	    break;
	case '\r':
	    dp = save_string(dp, "\\r");
	    s++;
	    break;
	case '\200':
	    dp = save_string(dp, "\\0");
	    s++;
	    break;
	case '\f':
	    dp = save_string(dp, "\\f");
	    s++;
	    break;
	case '\b':
	    dp = save_string(dp, "\\b");
	    s++;
	    break;
	case ' ':
	    dp = save_string(dp, "\\s");
	    s++;
	    break;
	case '^':
	    dp = save_string(dp, "\\^");
	    s++;
	    break;
	case ':':
	    dp = save_string(dp, "\\:");
	    s++;
	    break;
	case ',':
	    dp = save_string(dp, "\\,");
	    s++;
	    break;
	default:
	    if (*s == '\033') {
		dp = save_string(dp, "\\E");
		s++;
	    } else if (*s > 0 && *s < 32) {
		dp = save_char(dp, '^');
		dp = save_char(dp, *s + '@');
		s++;
	    } else if (*s <= 0 || *s >= 127) {
		dp = save_char(dp, '\\');
		dp = save_char(dp, ((*s & 0300) >> 6) + '0');
		dp = save_char(dp, ((*s & 0070) >> 3) + '0');
		dp = save_char(dp, (*s & 0007) + '0');
		s++;
	    } else
		dp = save_char(dp, *s++);
	    break;
#else
	default:
	    dp = save_char(dp, *s++);
	    break;
#endif
	}
    }

    /*
     * Now, if we stripped off some leading padding, add it at the end
     * of the string as mandatory padding.
     */
    if (capstart) {
	dp = save_string(dp, "$<");
	for (s = capstart;; s++)
	    if (isdigit(UChar(*s)) || *s == '*' || *s == '.')
		dp = save_char(dp, *s);
	    else
		break;
	dp = save_string(dp, "/>");
    }

    (void) save_char(dp, '\0');
    return (my_string);
}

/*
 * Check for an expression that corresponds to "%B" (BCD):
 *	(parameter / 10) * 16 + (parameter % 10)
 */
static int
bcd_expression(const char *str)
{
    /* leave this non-const for HPUX */
    static char fmt[] = "%%p%c%%{10}%%/%%{16}%%*%%p%c%%{10}%%m%%+";
    int len = 0;
    char ch1, ch2;

    if (sscanf(str, fmt, &ch1, &ch2) == 2
	&& isdigit(UChar(ch1))
	&& isdigit(UChar(ch2))
	&& (ch1 == ch2)) {
	len = 28;
#ifndef NDEBUG
	{
	    char buffer[80];
	    int tst;
	    sprintf(buffer, fmt, ch1, ch2);
	    tst = strlen(buffer) - 1;
	    assert(len == tst);
	}
#endif
    }
    return len;
}

static char *
save_tc_char(char *bufptr, int c1)
{
    char temp[80];

    if (is7bits(c1) && isprint(c1)) {
	if (c1 == ':' || c1 == '\\')
	    bufptr = save_char(bufptr, '\\');
	bufptr = save_char(bufptr, c1);
    } else {
	if (c1 == (c1 & 0x1f))	/* iscntrl() returns T on 255 */
	    (void) strcpy(temp, unctrl((chtype) c1));
	else
	    (void) sprintf(temp, "\\%03o", c1);
	bufptr = save_string(bufptr, temp);
    }
    return bufptr;
}

static char *
save_tc_inequality(char *bufptr, int c1, int c2)
{
    bufptr = save_string(bufptr, "%>");
    bufptr = save_tc_char(bufptr, c1);
    bufptr = save_tc_char(bufptr, c2);
    return bufptr;
}

/*
 * Here are the capabilities infotocap assumes it can translate to:
 *
 *     %%       output `%'
 *     %d       output value as in printf %d
 *     %2       output value as in printf %2d
 *     %3       output value as in printf %3d
 *     %.       output value as in printf %c
 *     %+c      add character c to value, then do %.
 *     %>xy     if value > x then add y, no output
 *     %r       reverse order of two parameters, no output
 *     %i       increment by one, no output
 *     %n       exclusive-or all parameters with 0140 (Datamedia 2500)
 *     %B       BCD (16*(value/10)) + (value%10), no output
 *     %D       Reverse coding (value - 2*(value%16)), no output (Delta Data).
 *     %m       exclusive-or all parameters with 0177 (not in 4.4BSD)
 */

/*
 * Convert a terminfo string to termcap format.  Parameters are as in
 * _nc_captoinfo().
 */
NCURSES_EXPORT(char *)
_nc_infotocap(const char *cap GCC_UNUSED, const char *str, int const parameterized)
{
    int seenone = 0, seentwo = 0, saw_m = 0, saw_n = 0;
    const char *padding;
    const char *trimmed = 0;
    char ch1 = 0, ch2 = 0;
    char *bufptr = init_string();
    int len;
    bool syntax_error = FALSE;

    /* we may have to move some trailing mandatory padding up front */
    padding = str + strlen(str) - 1;
    if (padding > str && *padding == '>' && *--padding == '/') {
	--padding;
	while (isdigit(UChar(*padding)) || *padding == '.' || *padding == '*')
	    padding--;
	if (padding > str && *padding == '<' && *--padding == '$')
	    trimmed = padding;
	padding += 2;

	while (isdigit(UChar(*padding)) || *padding == '.' || *padding == '*')
	    bufptr = save_char(bufptr, *padding++);
    }

    for (; *str && str != trimmed; str++) {
	int c1, c2;
	char *cp = 0;

	if (str[0] == '\\' && (str[1] == '^' || str[1] == ',')) {
	    bufptr = save_char(bufptr, *++str);
	} else if (str[0] == '$' && str[1] == '<') {	/* discard padding */
	    str += 2;
	    while (isdigit(UChar(*str))
		   || *str == '.'
		   || *str == '*'
		   || *str == '/'
		   || *str == '>')
		str++;
	    --str;
	} else if (str[0] == '%' && str[1] == '%') {	/* escaped '%' */
	    bufptr = save_string(bufptr, "%%");
	    ++str;
	} else if (*str != '%' || (parameterized < 1)) {
	    bufptr = save_char(bufptr, *str);
	} else if (sscanf(str, "%%?%%{%d}%%>%%t%%{%d}%%+%%;", &c1, &c2) == 2) {
	    str = strchr(str, ';');
	    bufptr = save_tc_inequality(bufptr, c1, c2);
	} else if (sscanf(str, "%%?%%{%d}%%>%%t%%'%c'%%+%%;", &c1, &ch2) == 2) {
	    str = strchr(str, ';');
	    bufptr = save_tc_inequality(bufptr, c1, c2);
	} else if (sscanf(str, "%%?%%'%c'%%>%%t%%{%d}%%+%%;", &ch1, &c2) == 2) {
	    str = strchr(str, ';');
	    bufptr = save_tc_inequality(bufptr, c1, c2);
	} else if (sscanf(str, "%%?%%'%c'%%>%%t%%'%c'%%+%%;", &ch1, &ch2) == 2) {
	    str = strchr(str, ';');
	    bufptr = save_tc_inequality(bufptr, c1, c2);
	} else if ((len = bcd_expression(str)) != 0) {
	    str += len;
	    bufptr = save_string(bufptr, "%B");
	} else if ((sscanf(str, "%%{%d}%%+%%c", &c1) == 1
		    || sscanf(str, "%%'%c'%%+%%c", &ch1) == 1)
		   && (cp = strchr(str, '+'))) {
	    str = cp + 2;
	    bufptr = save_string(bufptr, "%+");

	    if (ch1)
		c1 = ch1;
	    bufptr = save_tc_char(bufptr, c1);
	}
	/* FIXME: this "works" for 'delta' */
	else if (strncmp(str, "%{2}%*%-", 8) == 0) {
	    str += 7;
	    bufptr = save_string(bufptr, "%D");
	} else if (strncmp(str, "%{96}%^", 7) == 0) {
	    str += 6;
	    if (saw_m++ == 0) {
		bufptr = save_string(bufptr, "%n");
	    }
	} else if (strncmp(str, "%{127}%^", 8) == 0) {
	    str += 7;
	    if (saw_n++ == 0) {
		bufptr = save_string(bufptr, "%m");
	    }
	} else {		/* cm-style format element */
	    str++;
	    switch (*str) {
	    case '%':
		bufptr = save_char(bufptr, '%');
		break;

	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
		bufptr = save_char(bufptr, '%');
		while (isdigit(UChar(*str)))
		    bufptr = save_char(bufptr, *str++);
		if (strchr("doxX.", *str)) {
		    if (*str != 'd')	/* termcap doesn't have octal, hex */
			return 0;
		}
		break;

	    case 'd':
		bufptr = save_string(bufptr, "%d");
		break;

	    case 'c':
		bufptr = save_string(bufptr, "%.");
		break;

		/*
		 * %s isn't in termcap, but it's convenient to pass it through
		 * so we can represent things like terminfo pfkey strings in
		 * termcap notation.
		 */
	    case 's':
		bufptr = save_string(bufptr, "%s");
		break;

	    case 'p':
		str++;
		if (*str == '1')
		    seenone = 1;
		else if (*str == '2') {
		    if (!seenone && !seentwo) {
			bufptr = save_string(bufptr, "%r");
			seentwo++;
		    }
		} else if (*str >= '3')
		    return (0);
		break;

	    case 'i':
		bufptr = save_string(bufptr, "%i");
		break;

	    default:
		bufptr = save_char(bufptr, *str);
		syntax_error = TRUE;
		break;
	    }			/* endswitch (*str) */
	}			/* endelse (*str == '%') */

	/*
	 * 'str' always points to the end of what was scanned in this step,
	 * but that may not be the end of the string.
	 */
	assert(str != 0);
	if (*str == '\0')
	    break;

    }				/* endwhile (*str) */

    return (syntax_error ? NULL : my_string);
}

#ifdef MAIN

int curr_line;

int
main(int argc, char *argv[])
{
    int c, tc = FALSE;

    while ((c = getopt(argc, argv, "c")) != EOF)
	switch (c) {
	case 'c':
	    tc = TRUE;
	    break;
	}

    curr_line = 0;
    for (;;) {
	char buf[BUFSIZ];

	++curr_line;
	if (fgets(buf, sizeof(buf), stdin) == 0)
	    break;
	buf[strlen(buf) - 1] = '\0';
	_nc_set_source(buf);

	if (tc) {
	    char *cp = _nc_infotocap("to termcap", buf, 1);

	    if (cp)
		(void) fputs(cp, stdout);
	} else
	    (void) fputs(_nc_captoinfo("to terminfo", buf, 1), stdout);
	(void) putchar('\n');
    }
    return (0);
}
#endif /* MAIN */

#if NO_LEAKS
NCURSES_EXPORT(void)
_nc_captoinfo_leaks(void)
{
    if (my_string != 0) {
	FreeAndNull(my_string);
    }
    my_length = 0;
}
#endif

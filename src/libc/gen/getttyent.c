/*
 * Copyright (c) 1999, 2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "xlocale_private.h"

#include <ttyent.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <limits.h>

static char zapchar;
static FILE *tf;

struct ttyent *
getttynam(tty)
	const char *tty;
{
	register struct ttyent *t;

	setttyent();
	while ((t = getttyent()) != NULL)
		if (!strcmp(tty, t->ty_name))
			break;
	endttyent();
	return (t);
}

static char *skip(), *value();

/*
 * 4372480: Support for sequences in the tty name.  Expressions like [000-999]
 * for decimal sequences and [0x0-0xf] for hexidecimal sequences causes a
 * sequence of all combinations of names to be returned by getttyent().
 *
 * There is also a slot=nn option, which will cause getttyent() to return
 * non-existent ttyent structs until the slot number nn is reached.  Note, slot
 * numbers begin at 1.
 */
struct seq {
	int first;
	int count;
	int index;
	char fmt[NAME_MAX + 17];
};

static char brapat[] = "\\[(.*)]";
static regex_t brapreg;
static char decpat[] = "^([0-9]+)-([0-9]+)$";
static regex_t decpreg;
static char hexpat[] = "^0x([0-9a-f]+)-0x([0-9a-f]+)$";
static regex_t hexpreg;
static struct seq *seq = NULL;
static int slot;

struct ttyent *
getttyent()
{
	static struct ttyent tty;
	static struct ttyent nonexistent = {
		"\01",	/* this shouldn't match anything */
		NULL,
		NULL,
		0,
		NULL,
		NULL,
		NULL
	};
	register int c;
	register char *p;
#define	MAXLINELENGTH	1024
	static char *line = NULL;
	locale_t loc = __current_locale();
	int newslot, hex;
	long b, e;
	regmatch_t match[3], save, bracket;

	if ( line == NULL ) {
		line = malloc(MAXLINELENGTH);
		if( line == NULL )
			return NULL;
	}

	if (!tf && !setttyent())
		return (NULL);

restart:
	if (slot < seq->first) {
		/*
		 * the slot= option was set, and we are returning non-existent
		 * entries until we catch up.
		 */
		slot++;
		return &nonexistent;
	}

	if (seq->count > 0) {
		/*
		 * generate the next tty name; the rest of the tty entries
		 * is the same.
		 */
		sprintf(tty.ty_name, seq->fmt, seq->index++);
		slot++;
		seq->count--;
		return &tty;
	}

	if (slot == seq->first) {
		/*
		 * this was a regular entry with slot=
		 */
		slot++;
		return &tty;
	}

	for (;;) {
		if (!fgets(p = line, MAXLINELENGTH, tf))
			return (NULL);
		/* skip lines that are too big */
		if (!index(p, '\n')) {
			while ((c = getc(tf)) != '\n' && c != EOF)
				;
			continue;
		}
		while (isspace_l(*p, loc))
			++p;
		if (*p && *p != '#')
			break;
	}

	zapchar = 0;
	tty.ty_name = p;
	p = skip(p);
	if (!*(tty.ty_getty = p))
		tty.ty_getty = tty.ty_type = NULL;
	else {
		p = skip(p);
		if (!*(tty.ty_type = p))
			tty.ty_type = NULL;
		else
			p = skip(p);
	}
	tty.ty_status = 0;
	tty.ty_window = NULL;
	tty.ty_onerror = NULL;
	tty.ty_onoption = NULL;

#define	scmp(e)	!strncmp(p, e, sizeof(e) - 1) && isspace_l(p[sizeof(e) - 1], loc)
#define	vcmp(e)	!strncmp(p, e, sizeof(e) - 1) && p[sizeof(e) - 1] == '='
	newslot = -1;
	for (; *p; p = skip(p)) {
		if (scmp(_TTYS_OFF))
			tty.ty_status &= ~TTY_ON;
		else if (scmp(_TTYS_ON))
			tty.ty_status |= TTY_ON;
		else if (scmp(_TTYS_SECURE))
			tty.ty_status |= TTY_SECURE;
		else if (vcmp(_TTYS_WINDOW))
			tty.ty_window = value(p);
		else if (vcmp(_TTYS_ONERROR))
			tty.ty_onerror = value(p);
		else if (vcmp(_TTYS_ONOPTION))
			tty.ty_onoption = value(p);
		else if (vcmp(_TTYS_SLOT)) {
			char *slotstr = value(p);
			if (slotstr)
				newslot = atoi(slotstr);
		} else
			break;
	}

	if (zapchar == '#' || *p == '#')
		while ((c = *++p) == ' ' || c == '\t')
			;
	tty.ty_comment = p;
	if (*p == 0)
		tty.ty_comment = 0;
	if ((p = index(p, '\n')) != NULL)
		*p = '\0';

	/* check if tty.tyname has a sequence */
	if (regexec(&brapreg, tty.ty_name, 3, match, 0) != 0)
		goto out;

	/*
	 * save the range of the bracketed range, so we can find the strings
	 * before and after
	 */
	bracket = match[0];
	/* use REG_STARTEND to limit matching with the bracketed range */
	match[0] = save = match[1];
	if (regexec(&decpreg, tty.ty_name, 3, match, REG_STARTEND) == 0) {
		/* a decimal range */
		b = strtol(tty.ty_name + match[1].rm_so, NULL, 10);
		e = strtol(tty.ty_name + match[2].rm_so, NULL, 10);
		hex = 0;
	} else {
		match[0] = save;
		if (regexec(&hexpreg, tty.ty_name, 3, match, REG_STARTEND) == 0) {
			/* a hexidecimal range */
			b = strtol(tty.ty_name + match[1].rm_so, NULL, 16);
			e = strtol(tty.ty_name + match[2].rm_so, NULL, 16);
			hex = 1;
		} else
			goto out;
	}
	if (b > e)	/* skip */
		goto restart;

	/* seq->first is already less than slot, so just leave it */
	seq->count = e - b + 1;
	seq->index = b;
	/*
	 * The fmt string contains the characters before the bracket, the
	 * a format specifier (either decimal or hex) and any characters
	 * after the bracket.  Note that the length of the lower range is
	 * use as a minimum digit length, with zero fill, so the format
	 * specifier will look something like %03d.
	 */
	sprintf(seq->fmt, "%.*s%%0%d%c%s",
		(int)bracket.rm_so, tty.ty_name,
		(int)(match[1].rm_eo - match[1].rm_so),
		hex ? 'x' : 'd',
		tty.ty_name + bracket.rm_eo);

out:
	if (newslot > slot) {
		/* set up to skip until newslot */
		seq->first = newslot;
		goto restart;
	}
	if (seq->count > 0)	/* restart if we are doing a sequence */
		goto restart;
	/* regular single value return */
	slot++;
	return (&tty);
}

#define	QUOTED	1

/*
 * Skip over the current field, removing quotes, and return a pointer to
 * the next field.
 */
static char *
skip(p)
	register char *p;
{
	register char *t;
	register int c, q;

	for (q = 0, t = p; (c = *p) != '\0'; p++) {
		if (c == '"') {
			q ^= QUOTED;	/* obscure, but nice */
			continue;
		}
		if (q == QUOTED && *p == '\\' && *(p+1) == '"')
			p++;
		*t++ = *p;
		if (q == QUOTED)
			continue;
		if (c == '#') {
			zapchar = c;
			*p = 0;
			break;
		}
		if (c == '\t' || c == ' ' || c == '\n') {
			zapchar = c;
			*p++ = 0;
			while ((c = *p) == '\t' || c == ' ' || c == '\n')
				p++;
			break;
		}
	}
	*--t = '\0';
	return (p);
}

static char *
value(p)
	register char *p;
{

	return ((p = index(p, '=')) ? ++p : NULL);
}

int
setttyent()
{

	/* initialize seq and the three regexp patterns */
	if (!seq) {
		if (regcomp(&brapreg, brapat, REG_EXTENDED) != 0)
			return 0;
		if (regcomp(&decpreg, decpat, REG_EXTENDED) != 0) {
			regfree(&brapreg);
			return 0;
		}
		if (regcomp(&hexpreg, hexpat, REG_EXTENDED | REG_ICASE) != 0) {
			regfree(&decpreg);
			regfree(&brapreg);
			return 0;
		}
		if ((seq = malloc(sizeof(struct seq))) == NULL) {
			regfree(&hexpreg);
			regfree(&decpreg);
			regfree(&brapreg);
			return 0;
		}
	}
	seq->first = seq->count = 0;
	slot = 1;

	if (tf) {
		(void)rewind(tf);
		return (1);
	} else if ((tf = fopen(_PATH_TTYS, "r")) != NULL)
		return (1);
	return (0);
}

int
endttyent()
{
	int rval;

	if (tf) {
		rval = !(fclose(tf) == EOF);
		tf = NULL;
		return (rval);
	}
	return (1);
}

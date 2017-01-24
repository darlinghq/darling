/*-
 * Copyright (c) 2014 Gary Mills
 * Copyright 2011, Nexenta Systems, Inc.  All rights reserved.
 * Copyright (c) 1994 Powerdog Industries.  All rights reserved.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 * All rights reserved.
 * Portions of this software were developed by David Chisnall
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY POWERDOG INDUSTRIES ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE POWERDOG INDUSTRIES BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of Powerdog Industries.
 */

#include <sys/cdefs.h>
#ifndef lint
#ifndef NOID
static char copyright[] __unused =
"@(#) Copyright (c) 1994 Powerdog Industries.  All rights reserved.";
static char sccsid[] __unused = "@(#)strptime.c	0.1 (Powerdog) 94/03/27";
#endif /* !defined NOID */
#endif /* not lint */
__FBSDID("$FreeBSD$");

#include "xlocale_private.h"

#include "namespace.h"
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "timelocal.h"
#include "tzfile.h"

time_t _mktime(struct tm *, const char *);

#define	asizeof(a)	(sizeof(a) / sizeof((a)[0]))

enum {CONVERT_NONE, CONVERT_GMT, CONVERT_ZONE};
enum week_kind { WEEK_U = 'U', WEEK_V = 'V', WEEK_W = 'W'};

#define _strptime(b,f,t,c,l)	_strptime0(b,f,t,c,l,FLAG_NONE,0,WEEK_U)

#define	FLAG_NONE	0x01
#define	FLAG_YEAR	0x02
#define	FLAG_MONTH	0x04
#define	FLAG_YDAY	0x08
#define	FLAG_MDAY	0x10
#define	FLAG_WDAY	0x20
#define FLAG_WEEK	0x40
#define FLAG_CENTURY 0x100
#define FLAG_YEAR_IN_CENTURY 0x200

/*
 * Calculate the week day of the first day of a year. Valid for
 * the Gregorian calendar, which began Sept 14, 1752 in the UK
 * and its colonies. Ref:
 * http://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week
 */

static int
first_wday_of(int year)
{
	return (((2 * (3 - (year / 100) % 4)) + (year % 100) +
		((year % 100) / 4) + (isleap(year) ? 6 : 0) + 1) % 7);
}

static char *
_strptime0(const char *buf, const char *fmt, struct tm *tm, int *convp, locale_t locale, int flags, int week_number, enum week_kind week_kind)
{
	char	c;
	const char *ptr;
	int wday_offset;
	int	i, len;
	int Ealternative, Oalternative;
	const struct lc_time_T *tptr = __get_current_time_locale(locale);
	static int start_of_month[2][13] = {
		{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
		{0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
	};

	ptr = fmt;
	while (*ptr != 0) {
		c = *ptr++;

		if (c != '%') {
			if (isspace_l((unsigned char)c, locale))
				while (*buf != 0 && 
				       isspace_l((unsigned char)*buf, locale))
					buf++;
			else if (c != *buf++)
				return (NULL);
			continue;
		}

		Ealternative = 0;
		Oalternative = 0;
label:
		c = *ptr++;
		switch (c) {
		case '%':
			if (*buf++ != '%')
				return (NULL);
			break;

		case '+':
			buf = _strptime(buf, tptr->date_fmt, tm, convp, locale);
			if (buf == NULL)
				return (NULL);
			flags |= FLAG_WDAY | FLAG_MONTH | FLAG_MDAY | FLAG_YEAR;
			break;

		case 'C':
			if (!isdigit_l((unsigned char)*buf, locale))
				return (NULL);

			/* XXX This will break for 3-digit centuries. */
			len = 2;
			for (i = 0; len && *buf != 0 &&
			     isdigit_l((unsigned char)*buf, locale); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (i < 19)
				return (NULL);

			if (flags & FLAG_YEAR_IN_CENTURY) {
				tm->tm_year = i * 100 + (tm->tm_year % 100) - TM_YEAR_BASE;
				flags &= ~FLAG_YEAR_IN_CENTURY;
			} else {
				tm->tm_year = i * 100 - TM_YEAR_BASE;
				flags |= FLAG_YEAR;
				flags |= FLAG_CENTURY;
			}

			break;

		case 'c':
			buf = _strptime(buf, tptr->c_fmt, tm, convp, locale);
			if (buf == NULL)
				return (NULL);
			flags |= FLAG_WDAY | FLAG_MONTH | FLAG_MDAY | FLAG_YEAR;
			flags &= ~(FLAG_CENTURY | FLAG_YEAR_IN_CENTURY);
			break;

		case 'D':
			buf = _strptime(buf, "%m/%d/%y", tm, convp, locale);
			if (buf == NULL)
				return (NULL);
			flags |= FLAG_MONTH | FLAG_MDAY | FLAG_YEAR;
			flags &= ~(FLAG_CENTURY | FLAG_YEAR_IN_CENTURY);
			break;

		case 'E':
			if (Ealternative || Oalternative)
				break;
			Ealternative++;
			if (*ptr == '%') return (NULL);
			goto label;

		case 'O':
			if (Ealternative || Oalternative)
				break;
			Oalternative++;
			if (*ptr == '%') return (NULL);
			goto label;

		case 'F':
			buf = _strptime(buf, "%Y-%m-%d", tm, convp, locale);
			if (buf == NULL)
				return (NULL);
			flags |= FLAG_MONTH | FLAG_MDAY | FLAG_YEAR;
			flags &= ~(FLAG_CENTURY | FLAG_YEAR_IN_CENTURY);
			break;

		case 'R':
			buf = _strptime(buf, "%H:%M", tm, convp, locale);
			if (buf == NULL)
				return (NULL);
			break;

		case 'r':
			buf = _strptime(buf, tptr->ampm_fmt, tm, convp, locale);
			if (buf == NULL)
				return (NULL);
			break;

		case 'T':
			buf = _strptime(buf, "%H:%M:%S", tm, convp, locale);
			if (buf == NULL)
				return (NULL);
			break;

		case 'X':
			buf = _strptime(buf, tptr->X_fmt, tm, convp, locale);
			if (buf == NULL)
				return (NULL);
			break;

		case 'x':
			buf = _strptime(buf, tptr->x_fmt, tm, convp, locale);
			if (buf == NULL)
				return (NULL);
			flags |= FLAG_MONTH | FLAG_MDAY | FLAG_YEAR;
			flags &= ~(FLAG_CENTURY | FLAG_YEAR_IN_CENTURY);
			break;

		case 'j':
			if (!isdigit_l((unsigned char)*buf, locale))
				return (NULL);

			len = 3;
			for (i = 0; len && *buf != 0 &&
			     isdigit_l((unsigned char)*buf, locale); buf++){
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (i < 1 || i > 366)
				return (NULL);

			tm->tm_yday = i - 1;
			flags |= FLAG_YDAY;

			break;

		case 'M':
		case 'S':
			if (*buf == 0 ||
				isspace_l((unsigned char)*buf, locale))
				break;

			if (!isdigit_l((unsigned char)*buf, locale))
				return (NULL);

			len = 2;
			for (i = 0; len && *buf != 0 &&
				isdigit_l((unsigned char)*buf, locale); buf++){
				i *= 10;
				i += *buf - '0';
				len--;
			}

			if (c == 'M') {
				if (i > 59)
					return (NULL);
				tm->tm_min = i;
			} else {
				if (i > 60)
					return (NULL);
				tm->tm_sec = i;
			}

			break;

		case 'H':
		case 'I':
		case 'k':
		case 'l':
			/*
			 * Of these, %l is the only specifier explicitly
			 * documented as not being zero-padded.  However,
			 * there is no harm in allowing zero-padding.
			 *
			 * XXX The %l specifier may gobble one too many
			 * digits if used incorrectly.
			 */
			if (!isdigit_l((unsigned char)*buf, locale))
				return (NULL);

			len = 2;
			for (i = 0; len && *buf != 0 &&
			     isdigit_l((unsigned char)*buf, locale); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (c == 'H' || c == 'k') {
				if (i > 23)
					return (NULL);
			} else if (i > 12)
				return (NULL);

			tm->tm_hour = i;

			break;

		case 'p':
			/*
			 * XXX This is bogus if parsed before hour-related
			 * specifiers.
			 */
			len = strlen(tptr->am);
			if (strncasecmp_l(buf, tptr->am, len, locale) == 0) {
				if (tm->tm_hour > 12)
					return (NULL);
				if (tm->tm_hour == 12)
					tm->tm_hour = 0;
				buf += len;
				break;
			}

			len = strlen(tptr->pm);
			if (strncasecmp_l(buf, tptr->pm, len, locale) == 0) {
				if (tm->tm_hour > 12)
					return (NULL);
				if (tm->tm_hour != 12)
					tm->tm_hour += 12;
				buf += len;
				break;
			}

			return (NULL);

		case 'A':
		case 'a':
			for (i = 0; i < asizeof(tptr->weekday); i++) {
				len = strlen(tptr->weekday[i]);
				if (strncasecmp_l(buf, tptr->weekday[i],
						len, locale) == 0)
					break;
				len = strlen(tptr->wday[i]);
				if (strncasecmp_l(buf, tptr->wday[i],
						len, locale) == 0)
					break;
			}
			if (i == asizeof(tptr->weekday))
				return (NULL);

			buf += len;
			tm->tm_wday = i;
			flags |= FLAG_WDAY;
			break;

		case 'U':       /* Sunday week */
		case 'V':       /* ISO 8601 week */
		case 'W':       /* Monday week */
			if (!isdigit_l((unsigned char)*buf, locale))
				return (NULL);

			len = 2;
			for (i = 0; len && *buf != 0 &&
			     isdigit_l((unsigned char)*buf, locale); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (i > 53)
				return (NULL);
			if (c == WEEK_V && i < 1)
				return (NULL);

			week_number = i;
			week_kind = c;
			flags |= FLAG_WEEK;

			break;

		case 'u':       /* [1,7] */
		case 'w':       /* [0,6] */
			if (!isdigit_l((unsigned char)*buf, locale))
				return (NULL);

			i = *buf - '0';
			if (i > 6 + (c == 'u'))
				return (NULL);
			if (i == 7)
				i = 0;

			tm->tm_wday = i;
			flags |= FLAG_WDAY;
			buf++;

			break;

		case 'e':
			/*
			 * With %e format, our strftime(3) adds a blank space
			 * before single digits.
			 */
			if (*buf != 0 &&
			    isspace_l((unsigned char)*buf, locale))
			       buf++;
			/* FALLTHROUGH */
		case 'd':
			/*
			 * The %e specifier was once explicitly documented as
			 * not being zero-padded but was later changed to
			 * equivalent to %d.  There is no harm in allowing
			 * such padding.
			 *
			 * XXX The %e specifier may gobble one too many
			 * digits if used incorrectly.
			 */
			/* Leading space is ok if date is single digit */
			len = 2;
			if (isspace_l((unsigned char)buf[0], locale) &&
				isdigit_l((unsigned char)buf[1], locale) &&
				!isdigit_l((unsigned char)buf[2], locale)) {
				len = 1;
				buf++;
			}
			if (!isdigit_l((unsigned char)*buf, locale))
				return (NULL);

			for (i = 0; len && *buf != 0 &&
			     isdigit_l((unsigned char)*buf, locale); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (i > 31)
				return (NULL);

			tm->tm_mday = i;
			flags |= FLAG_MDAY;

			break;

		case 'B':
		case 'b':
		case 'h':
			for (i = 0; i < asizeof(tptr->month); i++) {
				if (Oalternative) {
					if (c == 'B') {
						len = strlen(tptr->alt_month[i]);
						if (strncasecmp_l(buf,
								tptr->alt_month[i],
								len, locale) == 0)
							break;
					}
				} else {
					len = strlen(tptr->month[i]);
					if (strncasecmp_l(buf, tptr->month[i],
							len, locale) == 0)
						break;
				}
			}
			/*
			 * Try the abbreviated month name if the full name
			 * wasn't found and Oalternative was not requested.
			 */
			if (i == asizeof(tptr->month) && !Oalternative) {
				for (i = 0; i < asizeof(tptr->month); i++) {
					len = strlen(tptr->mon[i]);
					if (strncasecmp_l(buf, tptr->mon[i],
							len, locale) == 0)
						break;
				}
			}
			if (i == asizeof(tptr->month))
				return (NULL);

			tm->tm_mon = i;
			buf += len;
			flags |= FLAG_MONTH;

			break;

		case 'm':
			if (!isdigit_l((unsigned char)*buf, locale))
				return (NULL);

			len = 2;
			for (i = 0; len && *buf != 0 &&
			     isdigit_l((unsigned char)*buf, locale); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (i < 1 || i > 12)
				return (NULL);

			tm->tm_mon = i - 1;
			flags |= FLAG_MONTH;

			break;

		case 's':
			{
			char *cp;
			int sverrno;
			long n;
			time_t t;

			sverrno = errno;
			errno = 0;
			n = strtol_l(buf, &cp, 10, locale);
			if (errno == ERANGE || (long)(t = n) != n) {
				errno = sverrno;
				return (NULL);
			}
			errno = sverrno;
			buf = cp;
			if (gmtime_r(&t, tm) == NULL)
				return (NULL);
			*convp = CONVERT_GMT;
			flags |= FLAG_YDAY | FLAG_WDAY | FLAG_MONTH |
			    FLAG_MDAY | FLAG_YEAR;
			flags &= ~(FLAG_CENTURY | FLAG_YEAR_IN_CENTURY);
			}
			break;

		case 'Y':
		case 'y':
			if (*buf == 0 ||
			    isspace_l((unsigned char)*buf, locale))
				break;

			if (!isdigit_l((unsigned char)*buf, locale))
				return (NULL);

#if __DARWIN_UNIX03
			if (c == 'Y') {
				int savei = 0;
				const char *savebuf = buf;
				int64_t i64 = 0;
				int overflow = 0;

				for (len = 0; *buf != 0 && isdigit_l((unsigned char)*buf, locale); buf++) {
					i64 *= 10;
					i64 += *buf - '0';
					if (++len <= 4) {
						savei = i64;
						savebuf = buf + 1;
					}
					if (i64 > INT_MAX) {
						overflow++;
						break;
					}
				}
				/*
				 * Conformance requires %Y to be more then 4
				 * digits.  However, there are several cases
				 * where %Y is immediately followed by other
				 * digits values.  So we do the conformance
				 * case first (as many digits as possible),
				 * and if we fail, we backup and try just 4
				 * digits for %Y.
				 */
				if (len > 4 && !overflow) {
					struct tm savetm = *tm;
					int saveconv = *convp;
					const char *saveptr = ptr;
					char *ret;

					if (i64 < 1900)
						return 0;

					tm->tm_year = i64 - 1900;

					if (*buf != 0 && isspace_l((unsigned char)*buf, locale))
						while (*ptr != 0 && !isspace_l((unsigned char)*ptr, locale) && *ptr != '%')
							ptr++;
					ret = _strptime0(buf, ptr, tm, convp, locale, flags, week_number, week_kind);
					if (ret) return ret;
					/* Failed, so try 4-digit year */
					*tm = savetm;
					*convp = saveconv;
					ptr = saveptr;
				}
				buf = savebuf;
				i = savei;
			} else {
				len = 2;
#else /* !__DARWIN_UNIX03 */
				len = (c == 'Y') ? 4 : 2;
#endif /* __DARWIN_UNIX03 */
			
				for (i = 0; len && *buf != 0 &&
					 isdigit_l((unsigned char)*buf, locale); buf++) {
					i *= 10;
					i += *buf - '0';
					len--;
				}
#if __DARWIN_UNIX03
			}
#endif /* __DARWIN_UNIX03 */

			if (i < 0)
				return (NULL);

			if (c == 'Y'){
				i -= TM_YEAR_BASE;
			} else if (c == 'y' && flags & FLAG_CENTURY) {
				i = tm->tm_year + (i % 100);
				flags &= ~FLAG_CENTURY;
			} else if (c == 'y'){
				if (i < 69) i += 100;
				flags |= FLAG_YEAR_IN_CENTURY;
			}

			tm->tm_year = i;
			flags |= FLAG_YEAR;
			if (c == 'Y'){
				flags &= ~(FLAG_CENTURY | FLAG_YEAR_IN_CENTURY);
			}

			break;

		case 'Z':
			{
				const char *cp;
				size_t tzlen, len;

				for (cp = buf; *cp &&
					 isupper_l((unsigned char)*cp, locale); ++cp) {
					/*empty*/
				}
				len = cp - buf;
				if (len == 3 && strncmp(buf, "GMT", 3) == 0) {
					*convp = CONVERT_GMT;
					buf += len;
					break;
				}

				tzset();
				tzlen = strlen(tzname[0]);
				if (len == tzlen && strncmp(buf, tzname[0], tzlen) == 0) {
					tm->tm_isdst = 0;
					buf += len;
					break;
				}
				tzlen = strlen(tzname[1]);
				if (len == tzlen && strncmp(buf, tzname[1], tzlen) == 0) {
					tm->tm_isdst = 1;
					buf += len;
					break;
				}
				return (NULL);
			}

		case 'z':
			{
				char sign;
				int hr, min;
				if ((buf[0] != '+' && buf[0] != '-')
					|| !isdigit_l((unsigned char)buf[1], locale)
					|| !isdigit_l((unsigned char)buf[2], locale)
					|| !isdigit_l((unsigned char)buf[3], locale)
					|| !isdigit_l((unsigned char)buf[4], locale))
					return 0;
				sscanf(buf, "%c%2d%2d", &sign, &hr, &min);
				*convp = CONVERT_ZONE;
				tm->tm_gmtoff = 60 * (60 * hr + min);
				if (sign == '-')
					tm->tm_gmtoff = -tm->tm_gmtoff;
				buf += 5;
			}
			break;

		case 'n':
		case 't':
			if (!isspace((unsigned char)*buf))
				return 0;
			while (isspace_l((unsigned char)*buf, locale))
				buf++;
			break;

		default:
			return (NULL);
		}
	}

	if (!(flags & FLAG_YDAY) && (flags & FLAG_YEAR)) {
		if ((flags & (FLAG_MONTH | FLAG_MDAY)) ==
		    (FLAG_MONTH | FLAG_MDAY)) {
			tm->tm_yday = start_of_month[isleap(tm->tm_year +
			    TM_YEAR_BASE)][tm->tm_mon] + (tm->tm_mday - 1);
			flags |= FLAG_YDAY;
		} else if (flags & FLAG_WEEK){
			if (!(flags & FLAG_WDAY)) {
				tm->tm_wday = week_kind == WEEK_U ? TM_SUNDAY : TM_MONDAY;
				flags |= FLAG_WDAY;
			}

			struct tm t = {0};
			t.tm_mday = week_kind == WEEK_V ? 4 : 1;
			t.tm_hour = 12; /* avoid any DST effects */
			t.tm_year = tm->tm_year;
			if (timegm(&t) == (time_t)-1) return 0;

			int off = t.tm_wday;
			int wday = tm->tm_wday;

			if (week_kind != WEEK_U) {
				off = (off + 6) % 7;
				wday = (wday + 6) % 7;
			}

			if (week_kind == WEEK_V) {
				t.tm_mday = 7 * week_number + wday - off - 3;
			} else {
				if(off == 0) off = 7;
				t.tm_mday = 7 * week_number + wday - off + 1;
			}
			if (timegm(&t) == (time_t)-1) return 0;

			tm->tm_yday = t.tm_yday;

			flags |= FLAG_YDAY;
		}
	}

	if ((flags & (FLAG_YEAR | FLAG_YDAY)) == (FLAG_YEAR | FLAG_YDAY)) {
		if (!(flags & FLAG_MONTH)) {
			i = 0;
			while (tm->tm_yday >=
			    start_of_month[isleap(tm->tm_year +
			    TM_YEAR_BASE)][i])
				i++;
			if (i > 12) {
				i = 1;
				tm->tm_yday -=
				    start_of_month[isleap(tm->tm_year +
				    TM_YEAR_BASE)][12];
				tm->tm_year++;
			}
			tm->tm_mon = i - 1;
			flags |= FLAG_MONTH;
		}
		if (!(flags & FLAG_MDAY)) {
			tm->tm_mday = tm->tm_yday -
			    start_of_month[isleap(tm->tm_year + TM_YEAR_BASE)]
			    [tm->tm_mon] + 1;
			flags |= FLAG_MDAY;
		}
		if (!(flags & FLAG_WDAY)) {
			i = 0;
			wday_offset = first_wday_of(tm->tm_year);
			while (i++ <= tm->tm_yday) {
				if (wday_offset++ >= 6)
					wday_offset = 0;
			}
			tm->tm_wday = wday_offset;
			flags |= FLAG_WDAY;
		}
	}

	return ((char *)buf);
}


char *
strptime(const char * __restrict buf, const char * __restrict fmt,
		 struct tm * __restrict tm)
{
	return strptime_l(buf, fmt, tm, __current_locale());
}

extern time_t timeoff(struct tm *, long);

char *
strptime_l(const char * __restrict buf, const char * __restrict fmt,
    struct tm * __restrict tm, locale_t loc)
{
	char *ret;
	int conv;

	NORMALIZE_LOCALE(loc);
	conv = CONVERT_NONE;
	tm->tm_zone = NULL;
	ret = _strptime(buf, fmt, tm, &conv, loc);
	if (ret) {
		time_t t;

		switch(conv) {
		case CONVERT_GMT:
			t = timegm(tm);
			localtime_r(&t, tm);
			break;
		case CONVERT_ZONE:
		    {
			long offset = tm->tm_gmtoff;
			tm->tm_gmtoff = 0;
			t = timeoff(tm, offset);
			localtime_r(&t, tm);
			break;
		    }
		}
	}

	return (ret);
}

/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

enum {
    DATEMSK_NOT_DEFINED = 1,
    CANT_OPEN,
    CANT_FSTAT,
    NOT_FILE,
    IO_ERROR,
    OUT_OF_MEMORY,
    NO_MATCH,
    INVALID_DATE
};

#define	DATEMSK		"DATEMSK"
#define	TM_SEC_SET	0x01
#define	TM_MIN_SET	0x02
#define	TM_HOUR_SET	0x04
#define	TM_MDAY_SET	0x01
#define	TM_MON_SET	0x02
#define	TM_YEAR_SET	0x04
#define	UNDEFINED	-1

static const struct tm tmundef = {
    UNDEFINED,
    UNDEFINED,
    UNDEFINED,
    UNDEFINED,
    UNDEFINED,
    UNDEFINED,
    UNDEFINED
};

int getdate_err;

/*
 * getdate
 *
 * Call strptime() on each line of the DATEMSK file, and manipulate the
 * resulting tm structure.  
 */
struct tm *
getdate(const char *str)
{
    static struct tm tm;
    struct tm *now, *result = NULL;
    time_t t;
    FILE *fp;
    int bufsiz, offset, len, dateset, timeset, saveerrno, wday_set, save_mon;
    char *buf;
    struct stat st;
    char *file = getenv(DATEMSK);

    if(!file || !*file) {
	getdate_err = DATEMSK_NOT_DEFINED;
	return NULL;
    }
    saveerrno = errno;
    if((fp = fopen(file, "r")) == NULL) {
	getdate_err = CANT_OPEN;
	errno = saveerrno;
	return NULL;
    }
    do {
	if(fstat(fileno(fp), &st) < 0) {
	    getdate_err = CANT_FSTAT;
	    break;
	}
	if((st.st_mode & S_IFMT) != S_IFREG) {
	    getdate_err = NOT_FILE;
	    break;
	}
	if((buf = malloc(bufsiz = BUFSIZ)) == NULL) {
	    getdate_err = OUT_OF_MEMORY;
	    break;
	}
	do {
	    offset = 0;
	    for(;;) {
		if(fgets(buf + offset, bufsiz - offset, fp) == NULL) {
		    if(ferror(fp)) {
			getdate_err = IO_ERROR;
			break;
		    }
		    if(offset == 0) {
			getdate_err = NO_MATCH;
			break;
		    }
		    len = strlen(buf);
		} else if((len = strlen(buf)) == bufsiz - 1
		  && buf[len - 1] != '\n') {
		    char *newptr = realloc(buf, (bufsiz += BUFSIZ));
		    if(newptr == NULL) {
			getdate_err = OUT_OF_MEMORY;
			break;
		    }
		    buf = newptr;
		    offset = len;
		    continue;
		}
		if(buf[len - 1] == '\n')
		    buf[len - 1] = 0;
		tm = tmundef;
		if(strptime(str, buf, &tm) == NULL) {
		    offset = 0;
		    continue;
		}
		time(&t);
		now = localtime(&t);
		dateset = timeset = 0;
		if(tm.tm_sec != UNDEFINED)
		    timeset |= TM_SEC_SET;
		if(tm.tm_min != UNDEFINED)
		    timeset |= TM_MIN_SET;
		if(tm.tm_hour != UNDEFINED)
		    timeset |= TM_HOUR_SET;
		if(tm.tm_mday != UNDEFINED)
		    dateset |= TM_MDAY_SET;
		if(tm.tm_mon != UNDEFINED)
		    dateset |= TM_MON_SET;
		if(tm.tm_year != UNDEFINED)
		    dateset |= TM_YEAR_SET;
		wday_set = tm.tm_wday;

		switch(timeset) {
		case 0:
		    tm.tm_sec = now->tm_sec;
		    tm.tm_min = now->tm_min;
		    tm.tm_hour = now->tm_hour;
		    break;

		case TM_SEC_SET:
		    tm.tm_hour = now->tm_hour;
		    tm.tm_min = now->tm_min;
		    if(tm.tm_sec < now->tm_sec)
			tm.tm_min++;
		    break;

		case TM_MIN_SET:
		    tm.tm_hour = now->tm_hour;
		    if(tm.tm_min < now->tm_min)
			tm.tm_hour++;
		    tm.tm_sec = 0;
		    break;

		case TM_MIN_SET | TM_SEC_SET:
		    tm.tm_hour = now->tm_hour;
		    if((60 * tm.tm_min + tm.tm_sec)
		      < (60 * now->tm_min + now->tm_sec))
			tm.tm_hour++;
		    break;

		case TM_HOUR_SET:
		    tm.tm_min = 0;
		    tm.tm_sec = 0;
		    break;

		case TM_HOUR_SET | TM_SEC_SET:
		    tm.tm_min = 0;
		    break;

		case TM_HOUR_SET | TM_MIN_SET:
		    tm.tm_sec = 0;
		    break;
		}

		switch(dateset) {
		case 0:
		    tm.tm_mday = now->tm_mday;
		    if(tm.tm_hour < now->tm_hour)
			tm.tm_mday++;
		    tm.tm_mon = now->tm_mon;
		    tm.tm_year = now->tm_year;
		    break;

		case TM_MDAY_SET:
		    tm.tm_year = now->tm_year;
		    tm.tm_mon = now->tm_mon;
		    if(tm.tm_mday < now->tm_mday)
			tm.tm_mon++;
		    break;

		case TM_MON_SET:
		case TM_MON_SET | TM_MDAY_SET:
		    tm.tm_year = now->tm_year;
		    if(tm.tm_mon < now->tm_mon)
			tm.tm_year++;
		    if(!(dateset & TM_MDAY_SET))
			tm.tm_mday = 1;
		    break;

		case TM_YEAR_SET:
		case TM_YEAR_SET | TM_MON_SET:
		    if(!(dateset & TM_MON_SET))
			tm.tm_mon = 0;
		    tm.tm_mday = 1;
		    break;

		case TM_YEAR_SET | TM_MDAY_SET:
		    tm.tm_mon = now->tm_mon;
		    if(tm.tm_mday < now->tm_mday)
			tm.tm_mon++;
		    break;
		}

		tm.tm_wday = now->tm_wday;
		tm.tm_gmtoff = now->tm_gmtoff;	/* XXX: can't grok timezones */
		tm.tm_isdst = -1;
		save_mon = tm.tm_mon;
		if(mktime(&tm) == (time_t)-1) {
		    getdate_err = INVALID_DATE;
		    break;
		} else if ((dateset & TM_MON_SET) && (tm.tm_mon != save_mon)) { /* Did mktime fixup an overflow date? */
			getdate_err = INVALID_DATE;
			break;
		}
		if(wday_set != UNDEFINED) {
		    int delta = wday_set - tm.tm_wday;
		    if(delta && (dateset & TM_MDAY_SET)) {
			getdate_err = INVALID_DATE;
			break;
		    }
		    if(delta < 0)
			delta += 7;
		    tm.tm_mday += delta;
		    if(mktime(&tm) == (time_t)-1) {
			getdate_err = INVALID_DATE;
			break;
		    }
		}
		result = &tm;
		break;
	    }
	} while(0);

	free(buf);
    } while(0);

    fclose(fp);
    errno = saveerrno;
    return result;
}

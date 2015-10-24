/*
** This file is in the public domain, so clarified as of
** 1996-06-05 by Arthur David Olson (arthur_david_olson@nih.gov).
*/

#include <sys/cdefs.h>
#ifndef lint
#ifndef NOID
static char	elsieid[] __unused = "@(#)localtime.c	7.78";
#endif /* !defined NOID */
#endif /* !defined lint */
__FBSDID("$FreeBSD: src/lib/libc/stdtime/localtime.c,v 1.43 2008/04/01 06:56:11 davidxu Exp $");

/*
** Leap second handling from Bradley White (bww@k.gp.cs.cmu.edu).
** POSIX-style TZ environment variable handling from Guy Harris
** (guy@auspex.com).
*/

/*LINTLIBRARY*/

#include "namespace.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#ifdef NOTIFY_TZ
//#define NOTIFY_TZ_DEBUG
//#define NOTIFY_TZ_DEBUG_FILE	"/var/log/localtime.debug"
//#define NOTIFY_TZ_LOG	"/var/log/localtime.log"
/* force ALL_STATE if NOTIFY_TZ is set */
#ifndef ALL_STATE
#define ALL_STATE
#endif /* ALL_STATE */
#include <mach/mach_init.h>
#include <notify.h>
#include <alloca.h>
#endif /* NOTIFY_TZ */
#include "private.h"
#include "un-namespace.h"

#include "tzfile.h"

#include "libc_private.h"

#define	_MUTEX_LOCK(x)		if (__isthreaded) _pthread_mutex_lock(x)
#define	_MUTEX_UNLOCK(x)	if (__isthreaded) _pthread_mutex_unlock(x)

#define _RWLOCK_RDLOCK(x)						\
		do {							\
			if (__isthreaded) _pthread_rwlock_rdlock(x);	\
		} while (0)

#define _RWLOCK_WRLOCK(x)						\
		do {							\
			if (__isthreaded) _pthread_rwlock_wrlock(x);	\
		} while (0)

#define _RWLOCK_UNLOCK(x)						\
		do {							\
			if (__isthreaded) _pthread_rwlock_unlock(x);	\
		} while (0)

/*
** SunOS 4.1.1 headers lack O_BINARY.
*/

#ifdef O_BINARY
#define OPEN_MODE	(O_RDONLY | O_BINARY)
#endif /* defined O_BINARY */
#ifndef O_BINARY
#define OPEN_MODE	O_RDONLY
#endif /* !defined O_BINARY */

#ifndef WILDABBR
/*
** Someone might make incorrect use of a time zone abbreviation:
**	1.	They might reference tzname[0] before calling tzset (explicitly
**		or implicitly).
**	2.	They might reference tzname[1] before calling tzset (explicitly
**		or implicitly).
**	3.	They might reference tzname[1] after setting to a time zone
**		in which Daylight Saving Time is never observed.
**	4.	They might reference tzname[0] after setting to a time zone
**		in which Standard Time is never observed.
**	5.	They might reference tm.TM_ZONE after calling offtime.
** What's best to do in the above cases is open to debate;
** for now, we just set things up so that in any of the five cases
** WILDABBR is used.  Another possibility:  initialize tzname[0] to the
** string "tzname[0] used before set", and similarly for the other cases.
** And another:  initialize tzname[0] to "ERA", with an explanation in the
** manual page of what this "time zone abbreviation" means (doing this so
** that tzname[0] has the "normal" length of three characters).
*/
#define WILDABBR	"   "
#endif /* !defined WILDABBR */

static char		wildabbr[] = "WILDABBR";

/*
 * In June 2004 it was decided UTC was a more appropriate default time
 * zone than GMT.
 */

static const char	gmt[] = "UTC";

/*
** The DST rules to use if TZ has no rules and we can't load TZDEFRULES.
** We default to US rules as of 1999-08-17.
** POSIX 1003.1 section 8.1.1 says that the default DST rules are
** implementation dependent; for historical reasons, US rules are a
** common default.
*/
#ifndef TZDEFRULESTRING
#define TZDEFRULESTRING ",M4.1.0,M10.5.0"
#endif /* !defined TZDEFDST */

struct ttinfo {				/* time type information */
	long		tt_gmtoff;	/* UTC offset in seconds */
	int		tt_isdst;	/* used to set tm_isdst */
	int		tt_abbrind;	/* abbreviation list index */
	int		tt_ttisstd;	/* TRUE if transition is std time */
	int		tt_ttisgmt;	/* TRUE if transition is UTC */
};

struct lsinfo {				/* leap second information */
	time_t		ls_trans;	/* transition time */
	long		ls_corr;	/* correction to apply */
};

#define BIGGEST(a, b)	(((a) > (b)) ? (a) : (b))

#ifdef TZNAME_MAX
#define MY_TZNAME_MAX	TZNAME_MAX
#endif /* defined TZNAME_MAX */
#ifndef TZNAME_MAX
#define MY_TZNAME_MAX	255
#endif /* !defined TZNAME_MAX */

struct state {
	int		leapcnt;
	int		timecnt;
	int		typecnt;
	int		charcnt;
	time_t		ats[TZ_MAX_TIMES];
	unsigned char	types[TZ_MAX_TIMES];
	struct ttinfo	ttis[TZ_MAX_TYPES];
	char		chars[BIGGEST(BIGGEST(TZ_MAX_CHARS + 1, sizeof gmt),
				(2 * (MY_TZNAME_MAX + 1)))];
	struct lsinfo	lsis[TZ_MAX_LEAPS];
};

struct rule {
	int		r_type;		/* type of rule--see below */
	int		r_day;		/* day number of rule */
	int		r_week;		/* week number of rule */
	int		r_mon;		/* month number of rule */
	long		r_time;		/* transition time of rule */
};

#define JULIAN_DAY		0	/* Jn - Julian day */
#define DAY_OF_YEAR		1	/* n - day of year */
#define MONTH_NTH_DAY_OF_WEEK	2	/* Mm.n.d - month, week, day of week */

#ifdef NOTIFY_TZ
typedef struct {
	int token;
	int notify_was_off;
	int is_set;
} notify_tz_t;

#define NOTIFY_TZ_NAME		"com.apple.system.timezone"
#endif /* NOTIFY_TZ */

/*
** Prototypes for static functions.
*/
#define	localsub	_st_localsub
#define	time1		_st_time1
#define	tzset_basic	_st_tzset_basic
__private_extern__
#ifdef __LP64__
struct tm *		localsub(const time_t * timep, long offset,
				struct tm * tmp);
#else /* !__LP64__ */
void			localsub(const time_t * timep, long offset,
				struct tm * tmp);
#endif /* __LP64__ */
__private_extern__
time_t			time1(struct tm * tmp,
#ifdef __LP64__
				struct tm *(*funcp) (const time_t *,
				long, struct tm *),
#else /* !__LP64__ */
				void(*funcp) (const time_t *,
				long, struct tm *),
#endif /* __LP64__ */
				long offset,
			        int unix03);
__private_extern__
void			tzset_basic(int);

#if !BUILDING_VARIANT
static long		detzcode(const char * codep);
static const char *	getzname(const char * strp, char **name, size_t *len);
static const char *	getnum(const char * strp, int * nump, int min,
				int max);
static const char *	getsecs(const char * strp, long * secsp);
static const char *	getoffset(const char * strp, long * offsetp);
static const char *	getrule(const char * strp, struct rule * rulep);
static void		gmtload(struct state * sp);
#ifdef __LP64__
static struct tm *	gmtsub(const time_t * timep, long offset,
				struct tm * tmp);
#else /* !__LP64__ */
static void		gmtsub(const time_t * timep, long offset,
				struct tm * tmp);
#endif /* __LP64__ */
static int		increment_overflow(int * number, int delta);
static int		normalize_overflow(int * tensptr, int * unitsptr,
				int base);
#ifdef NOTIFY_TZ
static void		notify_check_tz(notify_tz_t *p);
static void		notify_register_tz(char *file, notify_tz_t *p);
#endif /* NOTIFY_TZ */
static void		settzname(void);
static time_t		time2(struct tm *tmp,
#ifdef __LP64__
				struct tm *(*funcp) (const time_t *,
				long, struct tm*),
#else /* !__LP64__ */
				void(*funcp) (const time_t *,
				long, struct tm*),
#endif /* __LP64__ */
				long offset, int * okayp, int unix03);
static time_t		time2sub(struct tm *tmp,
#ifdef __LP64__
				struct tm *(*funcp) (const time_t *,
				long, struct tm*),
#else /* !__LP64__ */
				void(*funcp) (const time_t *,
				long, struct tm*),
#endif /* __LP64__ */
				long offset, int * okayp, int do_norm_secs,
				int unix03);
#ifdef __LP64__
static struct tm *	timesub(const time_t * timep, long offset,
				const struct state * sp, struct tm * tmp);
#else /* !__LP64__ */
static void		timesub(const time_t * timep, long offset,
				const struct state * sp, struct tm * tmp);
#endif /* __LP64__ */
static int		tmcomp(const struct tm * atmp,
				const struct tm * btmp);
static time_t		transtime(time_t janfirst, int year,
				const struct rule * rulep, long offset);
static int		tzload(const char * name, struct state * sp);
static int		tzparse(const char * name, struct state * sp,
				int lastditch);

#ifdef ALL_STATE
static struct state *	lclptr;
static struct state *	gmtptr;
#endif /* defined ALL_STATE */

#ifndef ALL_STATE
static struct state	lclmem;
static struct state	gmtmem;
#define lclptr		(&lclmem)
#define gmtptr		(&gmtmem)
#endif /* State Farm */

#ifndef TZ_STRLEN_MAX
#define TZ_STRLEN_MAX 255
#endif /* !defined TZ_STRLEN_MAX */

static char		lcl_TZname[TZ_STRLEN_MAX + 1];
#ifdef NOTIFY_TZ
#define lcl_is_set    (lcl_notify.is_set)
#define gmt_is_set    (gmt_notify.is_set)
#else /* ! NOTIFY_TZ */
static int		lcl_is_set;
static int		gmt_is_set;
#endif /* NOTIFY_TZ */
__private_extern__ pthread_rwlock_t	lcl_rwlock = PTHREAD_RWLOCK_INITIALIZER;
static pthread_mutex_t	gmt_mutex = PTHREAD_MUTEX_INITIALIZER;

char *			tzname[2] = {
	wildabbr,
	wildabbr
};

/*
** Section 4.12.3 of X3.159-1989 requires that
**	Except for the strftime function, these functions [asctime,
**	ctime, gmtime, localtime] return values in one of two static
**	objects: a broken-down time structure and an array of char.
** Thanks to Paul Eggert (eggert@twinsun.com) for noting this.
*/

static struct tm	tm;

#define USG_COMPAT
#define ALTZONE
#ifdef USG_COMPAT
int			daylight = 0;
__private_extern__ void _st_set_timezone(long);
#endif /* defined USG_COMPAT */

#ifdef ALTZONE
__private_extern__ long		__darwin_altzone = 0;
#define altzone __darwin_altzone
#endif /* defined ALTZONE */

#ifdef NOTIFY_TZ
#ifdef NOTIFY_TZ_DEBUG
#ifdef NOTIFY_TZ_DEBUG_FILE
#define NOTIFY_TZ_PRINTF(fmt, args...)	\
{ \
	FILE *_notify_tz_fp_; \
	if((_notify_tz_fp_ = fopen(NOTIFY_TZ_DEBUG_FILE, "a")) != NULL) { \
		fprintf(_notify_tz_fp_, "%d: " fmt, getpid(), ## args); \
		fclose(_notify_tz_fp_); \
	} \
}
#else /* ! NOTIFY_TZ_DEBUG_FILE */
#define NOTIFY_TZ_PRINTF(args...)	fprintf(stdout, ## args)
#endif /* NOTIFY_TZ_DEBUG_FILE */
#endif /* NOTIFY_TZ_DEBUG */
#ifdef NOTIFY_TZ_LOG
#define NOTIFY_LOG(fmt, args...)	\
{ \
	FILE *_notify_log_fp_; \
	if((_notify_log_fp_ = fopen(NOTIFY_TZ_LOG, "a")) != NULL) { \
		fprintf(_notify_log_fp_, "%d: " fmt, getpid(), ## args); \
		fclose(_notify_log_fp_); \
	} \
}
#endif /* NOTIFY_TZ_LOG */
/*--------------------------------------------------------------------
 * __notify_78945668_info__ is a global variable (defined in Libnotify)
 * that can be used to disable the notify mechanism.  Set to a negative
 * value to disable.  It can then be set back to zero to re-enable.
 *-------------------------------------------------------------------- */
extern int		__notify_78945668_info__;

/*--------------------------------------------------------------------
 * fullname is used to pass the actual path of the timezone file to the
 * notify routines.  If it is a nil string, that means no timezone file
 * is being used.
 *-------------------------------------------------------------------- */
static char *		fullname = NULL;

static notify_tz_t	gmt_notify = {-1, 0, 0};
static notify_tz_t	lcl_notify = {-1, 0, 0};
static char		notify_tz_name[] = NOTIFY_TZ_NAME;
#endif /* NOTIFY_TZ */

static long
detzcode(codep)
const char * const	codep;
{
	long	result;
	int	i;

	result = (codep[0] & 0x80) ? ~0L : 0L;
	for (i = 0; i < 4; ++i)
		result = (result << 8) | (codep[i] & 0xff);
	return result;
}

static void
settzname(void)
{
	struct state * 	sp = lclptr;
	int			i, need;
	unsigned char * types;
#define NEED_STD	1
#define NEED_DST	2
#define NEED_DAYLIGHT	4
#define NEED_ALL	(NEED_STD | NEED_DST | NEED_DAYLIGHT)

	tzname[0] = wildabbr;
	tzname[1] = wildabbr;
#ifdef USG_COMPAT
	daylight = 0;
	_st_set_timezone(0);
#endif /* defined USG_COMPAT */
#ifdef ALTZONE
	altzone = 0;
#endif /* defined ALTZONE */
#ifdef ALL_STATE
	if (sp == NULL) {
		tzname[0] = tzname[1] = (char *)gmt;
		return;
	}
#endif /* defined ALL_STATE */
	/*
	 * PR-3765457: The original settzname went sequentially through the ttis
	 * array, rather than correctly indexing via the types array, to get
	 * the real order of the timezone changes.  In addition, as a speed up,
	 * we start at the end of the changes, and work back, so that most of
	 * the time, we don't have to look through the entire array.
	 */
	if (sp->timecnt == 0 && sp->typecnt == 1) {
		/*
		 * Unfortunately, there is an edge case when typecnt == 1 and
		 * timecnt == 0, which would cause the loop to never run.  So
		 * in that case, we fudge things up so that it is as if
		 * timecnt == 1.
		 */
		i = 0;
		types = (unsigned char *)""; /* we use the null as index */
	} else {
		/* the usual case */
		i = sp->timecnt - 1;
		types = sp->types;
	}
	need = NEED_ALL;
	for (; i >= 0 && need; --i) {
		const struct ttinfo * const	ttisp = &sp->ttis[types[i]];

#ifdef USG_COMPAT
		if ((need & NEED_DAYLIGHT) && ttisp->tt_isdst) {
			need &= ~NEED_DAYLIGHT;
			daylight = 1;
		}
#endif /* defined USG_COMPAT */
		if (ttisp->tt_isdst) {
			if (need & NEED_DST) {
				need &= ~NEED_DST;
				tzname[1] = &sp->chars[ttisp->tt_abbrind];
#ifdef ALTZONE
				altzone = -(ttisp->tt_gmtoff);
#endif /* defined ALTZONE */
			}
		} else if (need & NEED_STD) {
			need &= ~NEED_STD;
			tzname[0] = &sp->chars[ttisp->tt_abbrind];
#ifdef USG_COMPAT
			_st_set_timezone(-(ttisp->tt_gmtoff));
#endif /* defined USG_COMPAT */
		}
#if defined(ALTZONE) || defined(USG_COMPAT)
		if (i == 0) {
#endif /* defined(ALTZONE) || defined(USG_COMPAT) */
#ifdef ALTZONE
			if (need & NEED_DST)
				altzone = -(ttisp->tt_gmtoff);
#endif /* defined ALTZONE */
#ifdef USG_COMPAT
			if (need & NEED_STD)
				_st_set_timezone(-(ttisp->tt_gmtoff));
#endif /* defined USG_COMPAT */
#if defined(ALTZONE) || defined(USG_COMPAT)
		}
#endif /* defined(ALTZONE) || defined(USG_COMPAT) */
	}
}

#ifdef NOTIFY_TZ
static void
notify_check_tz(notify_tz_t *p)
{
	unsigned int nstat;
	int ncheck;

	if (__notify_78945668_info__ < 0) {
#ifdef NOTIFY_TZ_DEBUG
		if(!p->notify_was_off) NOTIFY_TZ_PRINTF("notify_check_tz: setting %s_notify->notify_was_off\n", (p == &lcl_notify ? "lcl" : "gmt"));
#endif /* NOTIFY_TZ_DEBUG */
		p->notify_was_off = 1;
		return;
	}
	/* force rereading the timezone file if notify was off */
	if (p->notify_was_off) {
#ifdef NOTIFY_TZ_DEBUG
		NOTIFY_TZ_PRINTF("notify_check_tz: saw %s_notify->notify_was_off\n", (p == &lcl_notify ? "lcl" : "gmt"));
#endif /* NOTIFY_TZ_DEBUG */
		p->is_set = 0;
		p->notify_was_off = 0;
		return;
	}
	if (p->token < 0)
		return;
	nstat = notify_check(p->token, &ncheck);
	if (nstat || ncheck) {
		p->is_set = 0;
#ifdef NOTIFY_TZ_DEBUG
		NOTIFY_TZ_PRINTF("notify_check_tz: %s changed\n", (p == &lcl_notify) ? "lcl" : "gmt");
#endif /* NOTIFY_TZ_DEBUG */
	}
#ifdef NOTIFY_TZ_DEBUG
		NOTIFY_TZ_PRINTF("notify_check_tz: %s unchanged\n", (p == &lcl_notify) ? "lcl" : "gmt");
#endif /* NOTIFY_TZ_DEBUG */
}

extern uint32_t notify_monitor_file(int token, char *path, int flags);

static void
notify_register_tz(char *file, notify_tz_t *p)
{
	char *name;
	unsigned int nstat;
	int ncheck;

	if (__notify_78945668_info__ < 0)
		return;
	/*----------------------------------------------------------------
	 * Since we don't record the last time zone filename, just cancel
	 * (which should remove the file monitor) and setup from scratch
	 *----------------------------------------------------------------*/
	if (p->token >= 0)
		notify_cancel(p->token);
	if (!file || *file == 0) {
		/* no time zone file to monitor */
		p->token = -1;
		return;
	}
	/*----------------------------------------------------------------
	 * Just use com.apple.system.timezone if the path is /etc/localtime.
	 * Otherwise use com.apple.system.timezone.<fullpath>
	 *----------------------------------------------------------------*/
	if (TZDEFAULT && strcmp(file, TZDEFAULT) == 0)
		name = notify_tz_name;
	else {
		name = alloca(sizeof(notify_tz_name) + strlen(file) + 1);
		if (name == NULL) {
			p->token = -1;
			return;
		}
		strcpy(name, notify_tz_name);
		strcat(name, ".");
		strcat(name, file);
	}
#ifdef NOTIFY_TZ_DEBUG
	NOTIFY_TZ_PRINTF("notify_register_tz: file=%s name=%s\n", file, name);
#endif /* NOTIFY_TZ_DEBUG */
	nstat = notify_register_check(name, &p->token);
	if (nstat != 0) {
		p->token = -1;
		p->is_set = 0;
#ifdef NOTIFY_TZ_DEBUG
		NOTIFY_TZ_PRINTF("***notify_register_tz: notify_register_check failed: %u\n", nstat);
#endif /* NOTIFY_TZ_DEBUG */
#ifdef NOTIFY_TZ_LOG
		NOTIFY_LOG("notify_register_check(%s) failed: %u\n", name, nstat);
#endif /* NOTIFY_TZ_LOG */
		return;
	}
	/* don't need to request monitoring /etc/localtime */
	if (name != notify_tz_name) {
#ifdef NOTIFY_TZ_DEBUG
		NOTIFY_TZ_PRINTF("notify_register_tz: monitor %s\n", file);
#endif /* NOTIFY_TZ_DEBUG */
		nstat = notify_monitor_file(p->token, file, 0);
		if (nstat != 0) {
			notify_cancel(p->token);
			p->token = -1;
			p->is_set = 0;
#ifdef NOTIFY_TZ_DEBUG
			NOTIFY_TZ_PRINTF("***notify_register_tz: notify_monitor_file failed: %u\n", nstat);
#endif /* NOTIFY_TZ_DEBUG */
#ifdef NOTIFY_TZ_LOG
			NOTIFY_LOG("notify_monitor_file(%s) failed: %u\n", file, nstat);
#endif /* NOTIFY_TZ_LOG */
			return;
		}
	}
	notify_check(p->token, &ncheck);	/* this always returns true */
}
#endif /* NOTIFY_TZ */

static int
tzload(name, sp)
const char *		name;
struct state * const	sp;
{
	const char *	p;
	int		i;
	int		fid;

#ifdef NOTIFY_TZ_DEBUG
	NOTIFY_TZ_PRINTF("tzload: name=%s\n", name);
#endif /* NOTIFY_TZ_DEBUG */
	/* XXX The following is from OpenBSD, and I'm not sure it is correct */
	if (name != NULL && issetugid() != 0)
		if ((name[0] == ':' && name[1] == '/') || 
		    name[0] == '/' || strchr(name, '.'))
			name = NULL;
	if (name == NULL && (name = TZDEFAULT) == NULL)
		return -1;
	{
		int	doaccess;
		struct stat	stab;
		/*
		** Section 4.9.1 of the C standard says that
		** "FILENAME_MAX expands to an integral constant expression
		** that is the size needed for an array of char large enough
		** to hold the longest file name string that the implementation
		** guarantees can be opened."
		*/
#ifdef NOTIFY_TZ
		if (!fullname) {
			fullname = malloc(FILENAME_MAX + 1);
			if (!fullname)
				return -1;
		}
#else /* ! NOTIFY_TZ */
		char		fullname[FILENAME_MAX + 1];
#endif /* NOTIFY_TZ */

		if (name[0] == ':')
			++name;
		doaccess = name[0] == '/';
		if (!doaccess) {
			if ((p = TZDIR) == NULL)
				return -1;
#ifdef NOTIFY_TZ
			if ((strlen(p) + 1 + strlen(name) + 1) >= (FILENAME_MAX + 1))
#else /* ! NOTIFY_TZ */
			if ((strlen(p) + 1 + strlen(name) + 1) >= sizeof fullname)
#endif /* NOTIFY_TZ */
				return -1;
			(void) strcpy(fullname, p);
			(void) strcat(fullname, "/");
			(void) strcat(fullname, name);
			/*
			** Set doaccess if '.' (as in "../") shows up in name.
			*/
			if (strchr(name, '.') != NULL)
				doaccess = TRUE;
			name = fullname;
		}
#ifdef NOTIFY_TZ
		else
			strcpy(fullname, name);
#endif /* NOTIFY_TZ */
		if (doaccess && access(name, R_OK) != 0)
		     	return -1;
		if ((fid = _open(name, OPEN_MODE)) == -1)
			return -1;
		if ((_fstat(fid, &stab) < 0) || !S_ISREG(stab.st_mode)) {
			_close(fid);
			return -1;
		}
	}
	{
		struct tzhead *	tzhp;
		union {
			struct tzhead	tzhead;
			char		buf[sizeof *sp + sizeof *tzhp];
		} u;
		int		ttisstdcnt;
		int		ttisgmtcnt;

#ifdef NOTIFY_TZ_DEBUG
		NOTIFY_TZ_PRINTF("tzload: reading %s\n", name);
#endif /* NOTIFY_TZ_DEBUG */
		i = _read(fid, u.buf, sizeof u.buf);
		if (_close(fid) != 0)
			return -1;
		ttisstdcnt = (int) detzcode(u.tzhead.tzh_ttisstdcnt);
		ttisgmtcnt = (int) detzcode(u.tzhead.tzh_ttisgmtcnt);
		sp->leapcnt = (int) detzcode(u.tzhead.tzh_leapcnt);
		sp->timecnt = (int) detzcode(u.tzhead.tzh_timecnt);
		sp->typecnt = (int) detzcode(u.tzhead.tzh_typecnt);
		sp->charcnt = (int) detzcode(u.tzhead.tzh_charcnt);
		p = u.tzhead.tzh_charcnt + sizeof u.tzhead.tzh_charcnt;
		if (sp->leapcnt < 0 || sp->leapcnt > TZ_MAX_LEAPS ||
			sp->typecnt <= 0 || sp->typecnt > TZ_MAX_TYPES ||
			sp->timecnt < 0 || sp->timecnt > TZ_MAX_TIMES ||
			sp->charcnt < 0 || sp->charcnt > TZ_MAX_CHARS ||
			(ttisstdcnt != sp->typecnt && ttisstdcnt != 0) ||
			(ttisgmtcnt != sp->typecnt && ttisgmtcnt != 0))
				return -1;
		if (i - (p - u.buf) < sp->timecnt * 4 +	/* ats */
			sp->timecnt +			/* types */
			sp->typecnt * (4 + 2) +		/* ttinfos */
			sp->charcnt +			/* chars */
			sp->leapcnt * (4 + 4) +		/* lsinfos */
			ttisstdcnt +			/* ttisstds */
			ttisgmtcnt)			/* ttisgmts */
				return -1;
		for (i = 0; i < sp->timecnt; ++i) {
			sp->ats[i] = detzcode(p);
			p += 4;
		}
		for (i = 0; i < sp->timecnt; ++i) {
			sp->types[i] = (unsigned char) *p++;
			if (sp->types[i] >= sp->typecnt)
				return -1;
		}
		for (i = 0; i < sp->typecnt; ++i) {
			struct ttinfo *	ttisp;

			ttisp = &sp->ttis[i];
			ttisp->tt_gmtoff = detzcode(p);
			p += 4;
			ttisp->tt_isdst = (unsigned char) *p++;
			if (ttisp->tt_isdst != 0 && ttisp->tt_isdst != 1)
				return -1;
			ttisp->tt_abbrind = (unsigned char) *p++;
			if (ttisp->tt_abbrind < 0 ||
				ttisp->tt_abbrind > sp->charcnt)
					return -1;
		}
		for (i = 0; i < sp->charcnt; ++i)
			sp->chars[i] = *p++;
		sp->chars[i] = '\0';	/* ensure '\0' at end */
		for (i = 0; i < sp->leapcnt; ++i) {
			struct lsinfo *	lsisp;

			lsisp = &sp->lsis[i];
			lsisp->ls_trans = detzcode(p);
			p += 4;
			lsisp->ls_corr = detzcode(p);
			p += 4;
		}
		for (i = 0; i < sp->typecnt; ++i) {
			struct ttinfo *	ttisp;

			ttisp = &sp->ttis[i];
			if (ttisstdcnt == 0)
				ttisp->tt_ttisstd = FALSE;
			else {
				ttisp->tt_ttisstd = *p++;
				if (ttisp->tt_ttisstd != TRUE &&
					ttisp->tt_ttisstd != FALSE)
						return -1;
			}
		}
		for (i = 0; i < sp->typecnt; ++i) {
			struct ttinfo *	ttisp;

			ttisp = &sp->ttis[i];
			if (ttisgmtcnt == 0)
				ttisp->tt_ttisgmt = FALSE;
			else {
				ttisp->tt_ttisgmt = *p++;
				if (ttisp->tt_ttisgmt != TRUE &&
					ttisp->tt_ttisgmt != FALSE)
						return -1;
			}
		}
	}
	return 0;
}

static const int	mon_lengths[2][MONSPERYEAR] = {
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static const int	year_lengths[2] = {
	DAYSPERNYEAR, DAYSPERLYEAR
};

/*
** Given a pointer into a time zone string, scan until a character that is not
** a valid character in a zone name is found.  Return a pointer to that
** character.
*/

static const char *
getzname(strp, name, len)
const char *	strp;
char **		name;
size_t *	len;
{
	char	c;
	char *	ket;

	if (*strp == '<' && (ket = strchr(strp, '>')) != NULL) {
		*name = (char *)(strp + 1);
		*len = ket - strp - 1;
		return ket + 1;
	}
	*name = (char *)strp;
	while ((c = *strp) != '\0' && !is_digit(c) && c != ',' && c != '-' &&
		c != '+')
			++strp;
	*len = strp - *name;
	return strp;
}

/*
** Given a pointer into a time zone string, extract a number from that string.
** Check that the number is within a specified range; if it is not, return
** NULL.
** Otherwise, return a pointer to the first character not part of the number.
*/

static const char *
getnum(strp, nump, min, max)
const char *	strp;
int * const		nump;
const int		min;
const int		max;
{
	char	c;
	int	num;

	if (strp == NULL || !is_digit(c = *strp))
		return NULL;
	num = 0;
	do {
		num = num * 10 + (c - '0');
		if (num > max)
			return NULL;	/* illegal value */
		c = *++strp;
	} while (is_digit(c));
	if (num < min)
		return NULL;		/* illegal value */
	*nump = num;
	return strp;
}

/*
** Given a pointer into a time zone string, extract a number of seconds,
** in hh[:mm[:ss]] form, from the string.
** If any error occurs, return NULL.
** Otherwise, return a pointer to the first character not part of the number
** of seconds.
*/

static const char *
getsecs(strp, secsp)
const char *	strp;
long * const		secsp;
{
	int	num;

	/*
	** `HOURSPERDAY * DAYSPERWEEK - 1' allows quasi-Posix rules like
	** "M10.4.6/26", which does not conform to Posix,
	** but which specifies the equivalent of
	** ``02:00 on the first Sunday on or after 23 Oct''.
	*/
	strp = getnum(strp, &num, 0, HOURSPERDAY * DAYSPERWEEK - 1);
	if (strp == NULL)
		return NULL;
	*secsp = num * (long) SECSPERHOUR;
	if (*strp == ':') {
		++strp;
		strp = getnum(strp, &num, 0, MINSPERHOUR - 1);
		if (strp == NULL)
			return NULL;
		*secsp += num * SECSPERMIN;
		if (*strp == ':') {
			++strp;
			/* `SECSPERMIN' allows for leap seconds.  */
			strp = getnum(strp, &num, 0, SECSPERMIN);
			if (strp == NULL)
				return NULL;
			*secsp += num;
		}
	}
	return strp;
}

/*
** Given a pointer into a time zone string, extract an offset, in
** [+-]hh[:mm[:ss]] form, from the string.
** If any error occurs, return NULL.
** Otherwise, return a pointer to the first character not part of the time.
*/

static const char *
getoffset(strp, offsetp)
const char *	strp;
long * const		offsetp;
{
	int	neg = 0;

	if (*strp == '-') {
		neg = 1;
		++strp;
	} else if (*strp == '+')
		++strp;
	strp = getsecs(strp, offsetp);
	if (strp == NULL)
		return NULL;		/* illegal time */
	if (neg)
		*offsetp = -*offsetp;
	return strp;
}

/*
** Given a pointer into a time zone string, extract a rule in the form
** date[/time].  See POSIX section 8 for the format of "date" and "time".
** If a valid rule is not found, return NULL.
** Otherwise, return a pointer to the first character not part of the rule.
*/

static const char *
getrule(strp, rulep)
const char *			strp;
struct rule * const	rulep;
{
	if (*strp == 'J') {
		/*
		** Julian day.
		*/
		rulep->r_type = JULIAN_DAY;
		++strp;
		strp = getnum(strp, &rulep->r_day, 1, DAYSPERNYEAR);
	} else if (*strp == 'M') {
		/*
		** Month, week, day.
		*/
		rulep->r_type = MONTH_NTH_DAY_OF_WEEK;
		++strp;
		strp = getnum(strp, &rulep->r_mon, 1, MONSPERYEAR);
		if (strp == NULL)
			return NULL;
		if (*strp++ != '.')
			return NULL;
		strp = getnum(strp, &rulep->r_week, 1, 5);
		if (strp == NULL)
			return NULL;
		if (*strp++ != '.')
			return NULL;
		strp = getnum(strp, &rulep->r_day, 0, DAYSPERWEEK - 1);
	} else if (is_digit(*strp)) {
		/*
		** Day of year.
		*/
		rulep->r_type = DAY_OF_YEAR;
		strp = getnum(strp, &rulep->r_day, 0, DAYSPERLYEAR - 1);
	} else	return NULL;		/* invalid format */
	if (strp == NULL)
		return NULL;
	if (*strp == '/') {
		/*
		** Time specified.
		*/
		++strp;
		strp = getsecs(strp, &rulep->r_time);
	} else	rulep->r_time = 2 * SECSPERHOUR;	/* default = 2:00:00 */
	return strp;
}

/*
** Given the Epoch-relative time of January 1, 00:00:00 UTC, in a year, the
** year, a rule, and the offset from UTC at the time that rule takes effect,
** calculate the Epoch-relative time that rule takes effect.
*/

static time_t
transtime(janfirst, year, rulep, offset)
const time_t				janfirst;
const int				year;
const struct rule * const	rulep;
const long				offset;
{
	int	leapyear;
	time_t	value;
	int	i;
	int		d, m1, yy0, yy1, yy2, dow;

	INITIALIZE(value);
	leapyear = isleap(year);
	switch (rulep->r_type) {

	case JULIAN_DAY:
		/*
		** Jn - Julian day, 1 == January 1, 60 == March 1 even in leap
		** years.
		** In non-leap years, or if the day number is 59 or less, just
		** add SECSPERDAY times the day number-1 to the time of
		** January 1, midnight, to get the day.
		*/
		value = janfirst + (rulep->r_day - 1) * SECSPERDAY;
		if (leapyear && rulep->r_day >= 60)
			value += SECSPERDAY;
		break;

	case DAY_OF_YEAR:
		/*
		** n - day of year.
		** Just add SECSPERDAY times the day number to the time of
		** January 1, midnight, to get the day.
		*/
		value = janfirst + rulep->r_day * SECSPERDAY;
		break;

	case MONTH_NTH_DAY_OF_WEEK:
		/*
		** Mm.n.d - nth "dth day" of month m.
		*/
		value = janfirst;
		for (i = 0; i < rulep->r_mon - 1; ++i)
			value += mon_lengths[leapyear][i] * SECSPERDAY;

		/*
		** Use Zeller's Congruence to get day-of-week of first day of
		** month.
		*/
		m1 = (rulep->r_mon + 9) % 12 + 1;
		yy0 = (rulep->r_mon <= 2) ? (year - 1) : year;
		yy1 = yy0 / 100;
		yy2 = yy0 % 100;
		dow = ((26 * m1 - 2) / 10 +
			1 + yy2 + yy2 / 4 + yy1 / 4 - 2 * yy1) % 7;
		if (dow < 0)
			dow += DAYSPERWEEK;

		/*
		** "dow" is the day-of-week of the first day of the month.  Get
		** the day-of-month (zero-origin) of the first "dow" day of the
		** month.
		*/
		d = rulep->r_day - dow;
		if (d < 0)
			d += DAYSPERWEEK;
		for (i = 1; i < rulep->r_week; ++i) {
			if (d + DAYSPERWEEK >=
				mon_lengths[leapyear][rulep->r_mon - 1])
					break;
			d += DAYSPERWEEK;
		}

		/*
		** "d" is the day-of-month (zero-origin) of the day we want.
		*/
		value += d * SECSPERDAY;
		break;
	}

	/*
	** "value" is the Epoch-relative time of 00:00:00 UTC on the day in
	** question.  To get the Epoch-relative time of the specified local
	** time on that day, add the transition time and the current offset
	** from UTC.
	*/
	return value + rulep->r_time + offset;
}

/*
** Given a POSIX section 8-style TZ string, fill in the rule tables as
** appropriate.
*/

static int
tzparse(name, sp, lastditch)
const char *			name;
struct state * const	sp;
const int			lastditch;
{
	const char *			stdname;
	const char *			dstname;
	size_t				stdlen;
	size_t				dstlen;
	long				stdoffset;
	long				dstoffset;
	time_t *		atp;
	unsigned char *	typep;
	char *			cp;
	int			load_result;

	INITIALIZE(dstname);
	if (lastditch) {
		stdname = name;
		stdlen = strlen(name);	/* length of standard zone name */
		name += stdlen;
		if (stdlen >= sizeof sp->chars)
			stdlen = (sizeof sp->chars) - 1;
		stdoffset = 0;
	} else {
		name = getzname(name, (char **)&stdname, &stdlen);
		if (stdlen < 3)
			return -1;
		if (*name == '\0')
			return -1;	/* was "stdoffset = 0;" */
		else {
			name = getoffset(name, &stdoffset);
			if (name == NULL)
				return -1;
		}
	}
	load_result = tzload(TZDEFRULES, sp);
#ifdef NOTIFY_TZ
	*fullname = 0;				/* mark fullname as invalid */
#endif /* NOTIFY_TZ */
	if (load_result != 0)
		sp->leapcnt = 0;		/* so, we're off a little */
	if (*name != '\0') {
		dstname = name;
		name = getzname(name, (char **)&dstname, &dstlen);
		if (dstlen < 3)
			return -1;
		if (*name != '\0' && *name != ',' && *name != ';') {
			name = getoffset(name, &dstoffset);
			if (name == NULL)
				return -1;
		} else	dstoffset = stdoffset - SECSPERHOUR;
		if (*name == '\0' && load_result != 0)
			name = TZDEFRULESTRING;
		if (*name == ',' || *name == ';') {
			struct rule	start;
			struct rule	end;
			int	year;
			time_t	janfirst;
			time_t		starttime;
			time_t		endtime;

			++name;
			if ((name = getrule(name, &start)) == NULL)
				return -1;
			if (*name++ != ',')
				return -1;
			if ((name = getrule(name, &end)) == NULL)
				return -1;
			if (*name != '\0')
				return -1;
			sp->typecnt = 2;	/* standard time and DST */
			/*
			** Two transitions per year, from EPOCH_YEAR to 2037.
			*/
			sp->timecnt = 2 * (2037 - EPOCH_YEAR + 1);
			if (sp->timecnt > TZ_MAX_TIMES)
				return -1;
			sp->ttis[0].tt_gmtoff = -dstoffset;
			sp->ttis[0].tt_isdst = 1;
			sp->ttis[0].tt_abbrind = stdlen + 1;
			sp->ttis[1].tt_gmtoff = -stdoffset;
			sp->ttis[1].tt_isdst = 0;
			sp->ttis[1].tt_abbrind = 0;
			atp = sp->ats;
			typep = sp->types;
			janfirst = 0;
			for (year = EPOCH_YEAR; year <= 2037; ++year) {
				starttime = transtime(janfirst, year, &start,
					stdoffset);
				endtime = transtime(janfirst, year, &end,
					dstoffset);
				if (starttime > endtime) {
					*atp++ = endtime;
					*typep++ = 1;	/* DST ends */
					*atp++ = starttime;
					*typep++ = 0;	/* DST begins */
				} else {
					*atp++ = starttime;
					*typep++ = 0;	/* DST begins */
					*atp++ = endtime;
					*typep++ = 1;	/* DST ends */
				}
				janfirst += year_lengths[isleap(year)] *
					SECSPERDAY;
			}
		} else {
			long	theirstdoffset;
			long	theirdstoffset;
			long	theiroffset;
			int	isdst;
			int	i;
			int	j;

			if (*name != '\0')
				return -1;
			/*
			** Initial values of theirstdoffset and theirdstoffset.
			*/
			theirstdoffset = 0;
			for (i = 0; i < sp->timecnt; ++i) {
				j = sp->types[i];
				if (!sp->ttis[j].tt_isdst) {
					theirstdoffset =
						-sp->ttis[j].tt_gmtoff;
					break;
				}
			}
			theirdstoffset = 0;
			for (i = 0; i < sp->timecnt; ++i) {
				j = sp->types[i];
				if (sp->ttis[j].tt_isdst) {
					theirdstoffset =
						-sp->ttis[j].tt_gmtoff;
					break;
				}
			}
			/*
			** Initially we're assumed to be in standard time.
			*/
			isdst = FALSE;
			theiroffset = theirstdoffset;
			/*
			** Now juggle transition times and types
			** tracking offsets as you do.
			*/
			for (i = 0; i < sp->timecnt; ++i) {
				j = sp->types[i];
				sp->types[i] = sp->ttis[j].tt_isdst;
				if (sp->ttis[j].tt_ttisgmt) {
					/* No adjustment to transition time */
				} else {
					/*
					** If summer time is in effect, and the
					** transition time was not specified as
					** standard time, add the summer time
					** offset to the transition time;
					** otherwise, add the standard time
					** offset to the transition time.
					*/
					/*
					** Transitions from DST to DDST
					** will effectively disappear since
					** POSIX provides for only one DST
					** offset.
					*/
					if (isdst && !sp->ttis[j].tt_ttisstd) {
						sp->ats[i] += dstoffset -
							theirdstoffset;
					} else {
						sp->ats[i] += stdoffset -
							theirstdoffset;
					}
				}
				theiroffset = -sp->ttis[j].tt_gmtoff;
				if (sp->ttis[j].tt_isdst)
					theirdstoffset = theiroffset;
				else	theirstdoffset = theiroffset;
			}
			/*
			** Finally, fill in ttis.
			** ttisstd and ttisgmt need not be handled.
			*/
			sp->ttis[0].tt_gmtoff = -stdoffset;
			sp->ttis[0].tt_isdst = FALSE;
			sp->ttis[0].tt_abbrind = 0;
			sp->ttis[1].tt_gmtoff = -dstoffset;
			sp->ttis[1].tt_isdst = TRUE;
			sp->ttis[1].tt_abbrind = stdlen + 1;
			sp->typecnt = 2;
		}
	} else {
		dstlen = 0;
		sp->typecnt = 1;		/* only standard time */
		sp->timecnt = 0;
		sp->ttis[0].tt_gmtoff = -stdoffset;
		sp->ttis[0].tt_isdst = 0;
		sp->ttis[0].tt_abbrind = 0;
	}
	sp->charcnt = stdlen + 1;
	if (dstlen != 0)
		sp->charcnt += dstlen + 1;
	if ((size_t) sp->charcnt > sizeof sp->chars)
		return -1;
	cp = sp->chars;
	(void) strncpy(cp, stdname, stdlen);
	cp += stdlen;
	*cp++ = '\0';
	if (dstlen != 0) {
		(void) strncpy(cp, dstname, dstlen);
		*(cp + dstlen) = '\0';
	}
	return 0;
}

static void
gmtload(sp)
struct state * const	sp;
{
	if (tzload(gmt, sp) != 0)
		(void) tzparse(gmt, sp, TRUE);
}

static void
tzsetwall_basic(int rdlocked)
{
#ifdef NOTIFY_TZ
	notify_check_tz(&lcl_notify);
#else
    if (TZDEFAULT) {
        static struct timespec last_mtimespec = {0, 0};
        struct stat statbuf;

        if (lstat(TZDEFAULT, &statbuf) == 0) {
            if (statbuf.st_mtimespec.tv_sec > last_mtimespec.tv_sec ||
                (statbuf.st_mtimespec.tv_sec == last_mtimespec.tv_sec &&
                 statbuf.st_mtimespec.tv_nsec > last_mtimespec.tv_nsec)) {
               /* Trigger resetting the local TZ */
                    lcl_is_set = 0;
            }
            last_mtimespec = statbuf.st_mtimespec;
        }
    }
#endif /* NOTIFY_TZ */
	if (!rdlocked)
		_RWLOCK_RDLOCK(&lcl_rwlock);
	if (lcl_is_set < 0) {
#ifdef NOTIFY_TZ_DEBUG
		NOTIFY_TZ_PRINTF("tzsetwall_basic lcl_is_set < 0\n");
#endif
		if (!rdlocked)
			_RWLOCK_UNLOCK(&lcl_rwlock);
		return;
	}
#ifdef NOTIFY_TZ_DEBUG
	NOTIFY_TZ_PRINTF("tzsetwall_basic not set\n");
#endif
	_RWLOCK_UNLOCK(&lcl_rwlock);

	_RWLOCK_WRLOCK(&lcl_rwlock);
	lcl_is_set = -1;

#ifdef ALL_STATE
	if (lclptr == NULL) {
		lclptr = (struct state *) malloc(sizeof *lclptr);
		if (lclptr == NULL) {
			settzname();	/* all we can do */
			_RWLOCK_UNLOCK(&lcl_rwlock);
			if (rdlocked)
				_RWLOCK_RDLOCK(&lcl_rwlock);
			return;
		}
	}
#endif /* defined ALL_STATE */
	if (tzload((char *) NULL, lclptr) != 0)
		gmtload(lclptr);
#ifdef NOTIFY_TZ
	notify_register_tz(fullname, &lcl_notify);
#endif /* NOTIFY_TZ */
	settzname();
	_RWLOCK_UNLOCK(&lcl_rwlock);

	if (rdlocked)
		_RWLOCK_RDLOCK(&lcl_rwlock);
}

void
tzsetwall(void)
{
#ifdef NOTIFY_TZ_DEBUG
	NOTIFY_TZ_PRINTF("tzsetwall called\n");
#endif /* NOTIFY_TZ_DEBUG */
	tzsetwall_basic(0);
}

__private_extern__ void
tzset_basic(int rdlocked)
{
	const char *	name;

	name = getenv("TZ");
	if (name == NULL) {
		tzsetwall_basic(rdlocked);
		return;
	}

#ifdef NOTIFY_TZ
	notify_check_tz(&lcl_notify);
#endif /* NOTIFY_TZ */
	if (!rdlocked)
		_RWLOCK_RDLOCK(&lcl_rwlock);
	if (lcl_is_set > 0 && strcmp(lcl_TZname, name) == 0) {
		if (!rdlocked)
			_RWLOCK_UNLOCK(&lcl_rwlock);
#ifdef NOTIFY_TZ_DEBUG
		NOTIFY_TZ_PRINTF("tzset_basic matched %s\n", lcl_TZname);
#endif
		return;
	}
	_RWLOCK_UNLOCK(&lcl_rwlock);

	_RWLOCK_WRLOCK(&lcl_rwlock);
	lcl_is_set = strlen(name) < sizeof lcl_TZname;
	if (lcl_is_set)
		(void) strcpy(lcl_TZname, name);

#ifdef ALL_STATE
	if (lclptr == NULL) {
		lclptr = (struct state *) malloc(sizeof *lclptr);
		if (lclptr == NULL) {
			settzname();	/* all we can do */
			_RWLOCK_UNLOCK(&lcl_rwlock);
			if (rdlocked)
				_RWLOCK_RDLOCK(&lcl_rwlock);
			return;
		}
	}
#endif /* defined ALL_STATE */
	if (*name == '\0') {
		/*
		** User wants it fast rather than right.
		*/
		lclptr->leapcnt = 0;		/* so, we're off a little */
		lclptr->timecnt = 0;
		lclptr->typecnt = 0;
		lclptr->ttis[0].tt_isdst = 0;
		lclptr->ttis[0].tt_gmtoff = 0;
		lclptr->ttis[0].tt_abbrind = 0;
		(void) strcpy(lclptr->chars, gmt);
#ifdef NOTIFY_TZ
		if (fullname)
			*fullname = 0;
#endif /* NOTIFY_TZ */
	} else if (tzload(name, lclptr) != 0)
		if (name[0] == ':' || tzparse(name, lclptr, FALSE) != 0)
			(void) gmtload(lclptr);
#ifdef NOTIFY_TZ
	notify_register_tz(fullname, &lcl_notify);
#endif /* NOTIFY_TZ */
	settzname();
	_RWLOCK_UNLOCK(&lcl_rwlock);

	if (rdlocked)
		_RWLOCK_RDLOCK(&lcl_rwlock);
}

void
tzset(void)
{
#ifdef NOTIFY_TZ_DEBUG
	NOTIFY_TZ_PRINTF("tzset called TZ=%s\n", getenv("TZ"));
#endif /* NOTIFY_TZ_DEBUG */
	tzset_basic(0);
}

/*
** The easy way to behave "as if no library function calls" localtime
** is to not call it--so we drop its guts into "localsub", which can be
** freely called.  (And no, the PANS doesn't require the above behavior--
** but it *is* desirable.)
**
** The unused offset argument is for the benefit of mktime variants.
*/

/*ARGSUSED*/
#ifdef __LP64__
__private_extern__ struct tm *
#else /* !__LP64__ */
__private_extern__ void
#endif /* __LP64__ */
localsub(timep, offset, tmp)
const time_t * const	timep;
const long		offset;
struct tm * const	tmp;
{
	struct state *		sp;
	const struct ttinfo *	ttisp;
	int			i;
	const time_t			t = *timep;

#ifdef NOTIFY_TZ_DEBUG
	NOTIFY_TZ_PRINTF("localsub called\n");
#endif /* NOTIFY_TZ_DEBUG */
	sp = lclptr;
#ifdef ALL_STATE
	if (sp == NULL) {
#ifdef __LP64__
		return gmtsub(timep, offset, tmp);
#else /* !__LP64__ */
		gmtsub(timep, offset, tmp);
		return;
#endif /* __LP64__ */
	}
#endif /* defined ALL_STATE */
	if (sp->timecnt == 0 || t < sp->ats[0]) {
		i = 0;
		while (sp->ttis[i].tt_isdst)
			if (++i >= sp->typecnt) {
				i = 0;
				break;
			}
	} else {
		for (i = 1; i < sp->timecnt; ++i)
			if (t < sp->ats[i])
				break;
		i = sp->types[i - 1];
	}
	ttisp = &sp->ttis[i];
	/*
	** To get (wrong) behavior that's compatible with System V Release 2.0
	** you'd replace the statement below with
	**	t += ttisp->tt_gmtoff;
	**	timesub(&t, 0L, sp, tmp);
	*/
#ifdef __LP64__
	if (timesub(&t, ttisp->tt_gmtoff, sp, tmp) == NULL)
		return NULL;
#else /* !__LP64__ */
	timesub(&t, ttisp->tt_gmtoff, sp, tmp);
#endif /* __LP64__ */
	tmp->tm_isdst = ttisp->tt_isdst;
	tzname[tmp->tm_isdst] = &sp->chars[ttisp->tt_abbrind];
#ifdef TM_ZONE
	tmp->TM_ZONE = &sp->chars[ttisp->tt_abbrind];
#endif /* defined TM_ZONE */
#ifdef __LP64__
	return tmp;
#endif /* __LP64__ */
}

struct tm *
localtime(timep)
const time_t * const	timep;
{
	static pthread_mutex_t localtime_mutex = PTHREAD_MUTEX_INITIALIZER;
	static pthread_key_t localtime_key = -1;
	struct tm *p_tm;

	if (__isthreaded != 0) {
		if (localtime_key == (pthread_key_t)-1) {
			_pthread_mutex_lock(&localtime_mutex);
			if (localtime_key == (pthread_key_t)-1) {
				localtime_key = __LIBC_PTHREAD_KEY_LOCALTIME;
				if (pthread_key_init_np(localtime_key, free) < 0) {
					_pthread_mutex_unlock(&localtime_mutex);
					return(NULL);
				}
			}
			_pthread_mutex_unlock(&localtime_mutex);
		}
		p_tm = _pthread_getspecific(localtime_key);
		if (p_tm == NULL) {
			if ((p_tm = (struct tm *)malloc(sizeof(struct tm)))
			    == NULL)
				return(NULL);
			_pthread_setspecific(localtime_key, p_tm);
		}
		_RWLOCK_RDLOCK(&lcl_rwlock);
		tzset_basic(1);
#ifdef __LP64__
		p_tm = localsub(timep, 0L, p_tm);
#else /* !__LP64__ */
		localsub(timep, 0L, p_tm);
#endif /* __LP64__ */
		_RWLOCK_UNLOCK(&lcl_rwlock);
		return(p_tm);
	} else {
		tzset_basic(0);
#ifdef __LP64__
		return localsub(timep, 0L, &tm);
#else /* !__LP64__ */
		localsub(timep, 0L, &tm);
		return(&tm);
#endif /* __LP64__ */
	}
}

/*
** Re-entrant version of localtime.
*/

struct tm *
localtime_r(const time_t * const __restrict timep, struct tm * __restrict tm)
{
	_RWLOCK_RDLOCK(&lcl_rwlock);
	tzset_basic(1);
#ifdef __LP64__
	tm = localsub(timep, 0L, tm);
#else /* !__LP64__ */
	localsub(timep, 0L, tm);
#endif /* __LP64__ */
	_RWLOCK_UNLOCK(&lcl_rwlock);
	return tm;
}

/*
** gmtsub is to gmtime as localsub is to localtime.
*/

#ifdef __LP64__
static struct tm *
#else /* !__LP64__ */
static void
#endif /* __LP64__ */
gmtsub(timep, offset, tmp)
const time_t * const	timep;
const long		offset;
struct tm * const	tmp;
{
#ifdef NOTIFY_TZ_DEBUG
	NOTIFY_TZ_PRINTF("gmtsub called\n");
#endif /* NOTIFY_TZ_DEBUG */
#ifdef NOTIFY_TZ
	notify_check_tz(&gmt_notify);
#endif /* NOTIFY_TZ */
	if (!gmt_is_set) {
		_MUTEX_LOCK(&gmt_mutex);
		if (!gmt_is_set) {
#ifdef ALL_STATE
#ifdef NOTIFY_TZ
			if (gmtptr == NULL)
#endif /* NOTIFY_TZ */
				gmtptr = (struct state *) malloc(sizeof *gmtptr);
			if (gmtptr != NULL)
#ifdef NOTIFY_TZ
			{
#endif /* NOTIFY_TZ */
#endif /* defined ALL_STATE */
				gmtload(gmtptr);
#ifdef NOTIFY_TZ
				notify_register_tz(fullname, &gmt_notify);
#ifdef ALL_STATE
			}
#endif
#endif /* NOTIFY_TZ */
			gmt_is_set = TRUE;
		}
		_MUTEX_UNLOCK(&gmt_mutex);
	}
#ifdef __LP64__
	if(timesub(timep, offset, gmtptr, tmp) == NULL)
		return NULL;
#else /* !__LP64__ */
	timesub(timep, offset, gmtptr, tmp);
#endif /* __LP64__ */
#ifdef TM_ZONE
	/*
	** Could get fancy here and deliver something such as
	** "UTC+xxxx" or "UTC-xxxx" if offset is non-zero,
	** but this is no time for a treasure hunt.
	*/
	if (offset != 0)
		tmp->TM_ZONE = wildabbr;
	else {
#ifdef ALL_STATE
		if (gmtptr == NULL)
			tmp->TM_ZONE = (char *)gmt;
		else	tmp->TM_ZONE = gmtptr->chars;
#endif /* defined ALL_STATE */
#ifndef ALL_STATE
		tmp->TM_ZONE = gmtptr->chars;
#endif /* State Farm */
	}
#endif /* defined TM_ZONE */
#ifdef __LP64__
	return tmp;
#endif /* __LP64__ */
}

struct tm *
gmtime(timep)
const time_t * const	timep;
{
	static pthread_mutex_t gmtime_mutex = PTHREAD_MUTEX_INITIALIZER;
	static pthread_key_t gmtime_key = -1;
	struct tm *p_tm;

	if (__isthreaded != 0) {
		if (gmtime_key == (pthread_key_t)-1) {
			_pthread_mutex_lock(&gmtime_mutex);
			if (gmtime_key == (pthread_key_t)-1) {
				gmtime_key = __LIBC_PTHREAD_KEY_GMTIME;
				if (pthread_key_init_np(gmtime_key, free) < 0) {
					_pthread_mutex_unlock(&gmtime_mutex);
					return(NULL);
				}
			}
			_pthread_mutex_unlock(&gmtime_mutex);
		}
		/*
		 * Changed to follow POSIX.1 threads standard, which
		 * is what BSD currently has.
		 */
		if ((p_tm = _pthread_getspecific(gmtime_key)) == NULL) {
			if ((p_tm = (struct tm *)malloc(sizeof(struct tm)))
			    == NULL) {
				return(NULL);
			}
			_pthread_setspecific(gmtime_key, p_tm);
		}
#ifdef __LP64__
		return gmtsub(timep, 0L, p_tm);
#else /* !__LP64__ */
		gmtsub(timep, 0L, p_tm);
		return(p_tm);
#endif /* __LP64__ */
	}
	else {
#ifdef __LP64__
		return gmtsub(timep, 0L, &tm);
#else /* !__LP64__ */
		gmtsub(timep, 0L, &tm);
		return(&tm);
#endif /* __LP64__ */
	}
}

/*
* Re-entrant version of gmtime.
*/

struct tm *
gmtime_r(timep, tm)
const time_t * const	timep;
struct tm *		tm;
{

#ifdef __LP64__
	return gmtsub(timep, 0L, tm);
#else /* !__LP64__ */
	gmtsub(timep, 0L, tm);
	return tm;
#endif /* __LP64__ */
}

#ifdef STD_INSPIRED

struct tm *
offtime(timep, offset)
const time_t * const	timep;
const long		offset;
{
#ifdef __LP64__
	return gmtsub(timep, offset, &tm);
#else /* !__LP64__ */
	gmtsub(timep, offset, &tm);
	return &tm;
#endif /* __LP64__ */
}

#endif /* defined STD_INSPIRED */

#ifdef __LP64__
static struct tm *
#else /* !__LP64__ */
static void
#endif /* __LP64__ */
timesub(timep, offset, sp, tmp)
const time_t * const			timep;
const long				offset;
const struct state * const	sp;
struct tm * const		tmp;
{
	const struct lsinfo *	lp;
	long			days;
	long			rem;
	long			y;
	int			yleap;
	const int *		ip;
	long			corr;
	int			hit;
	int			i;

	corr = 0;
	hit = 0;
#ifdef ALL_STATE
	i = (sp == NULL) ? 0 : sp->leapcnt;
#endif /* defined ALL_STATE */
#ifndef ALL_STATE
	i = sp->leapcnt;
#endif /* State Farm */
	while (--i >= 0) {
		lp = &sp->lsis[i];
		if (*timep >= lp->ls_trans) {
			if (*timep == lp->ls_trans) {
				hit = ((i == 0 && lp->ls_corr > 0) ||
					lp->ls_corr > sp->lsis[i - 1].ls_corr);
				if (hit)
					while (i > 0 &&
						sp->lsis[i].ls_trans ==
						sp->lsis[i - 1].ls_trans + 1 &&
						sp->lsis[i].ls_corr ==
						sp->lsis[i - 1].ls_corr + 1) {
							++hit;
							--i;
					}
			}
			corr = lp->ls_corr;
			break;
		}
	}
	days = *timep / SECSPERDAY;
	rem = *timep % SECSPERDAY;
#ifdef mc68k
	if (*timep == 0x80000000) {
		/*
		** A 3B1 muffs the division on the most negative number.
		*/
		days = -24855;
		rem = -11648;
	}
#endif /* defined mc68k */
	rem += (offset - corr);
	while (rem < 0) {
		rem += SECSPERDAY;
		--days;
	}
	while (rem >= SECSPERDAY) {
		rem -= SECSPERDAY;
		++days;
	}
	tmp->tm_hour = (int) (rem / SECSPERHOUR);
	rem = rem % SECSPERHOUR;
	tmp->tm_min = (int) (rem / SECSPERMIN);
	/*
	** A positive leap second requires a special
	** representation.  This uses "... ??:59:60" et seq.
	*/
	tmp->tm_sec = (int) (rem % SECSPERMIN) + hit;
	tmp->tm_wday = (int) ((EPOCH_WDAY + days) % DAYSPERWEEK);
	if (tmp->tm_wday < 0)
		tmp->tm_wday += DAYSPERWEEK;
	y = EPOCH_YEAR;
#define _LEAPS_THRU_END_OF(y)	((y) / 4 - (y) / 100 + (y) / 400)
#ifdef __LP64__
#define LEAPS_THRU_END_OF(y)	((y) >= 0 ? _LEAPS_THRU_END_OF(y) : _LEAPS_THRU_END_OF((y) + 1) - 1)
#else /* !__LP64__ */
#define LEAPS_THRU_END_OF(y)	_LEAPS_THRU_END_OF(y)
#endif /* __LP64__ */
	while (days < 0 || days >= (long) year_lengths[yleap = isleap(y)]) {
		long	newy;

		newy = y + days / DAYSPERNYEAR;
		if (days < 0)
			--newy;
		days -= (newy - y) * DAYSPERNYEAR +
			LEAPS_THRU_END_OF(newy - 1) -
			LEAPS_THRU_END_OF(y - 1);
		y = newy;
	}
#ifdef __LP64__
	y -= TM_YEAR_BASE;
	if (y < INT_MIN || y > INT_MAX) {
		errno = EOVERFLOW;
		return NULL;
	}
	tmp->tm_year = y;
#else /* !__LP64__ */
	tmp->tm_year = y - TM_YEAR_BASE;
#endif /* __LP64__ */
	tmp->tm_yday = (int) days;
	ip = mon_lengths[yleap];
	for (tmp->tm_mon = 0; days >= (long) ip[tmp->tm_mon]; ++(tmp->tm_mon))
		days = days - (long) ip[tmp->tm_mon];
	tmp->tm_mday = (int) (days + 1);
	tmp->tm_isdst = 0;
#ifdef TM_GMTOFF
	tmp->TM_GMTOFF = offset;
#endif /* defined TM_GMTOFF */
#ifdef __LP64__
	return tmp;
#endif /* __LP64__ */
}

char *
ctime(timep)
const time_t * const	timep;
{
/*
** Section 4.12.3.2 of X3.159-1989 requires that
**	The ctime function converts the calendar time pointed to by timer
**	to local time in the form of a string.  It is equivalent to
**		asctime(localtime(timer))
*/
#ifdef __LP64__
	/*
	 * In 64-bit, the timep value may produce a time value with a year
	 * that exceeds 32-bits in size (won't fit in struct tm), so localtime
	 * will return NULL.
	 */
	struct tm *tm = localtime(timep);

	if (tm == NULL)
		return NULL;
	return asctime(tm);
#else /* !__LP64__ */
	return asctime(localtime(timep));
#endif /* __LP64__ */
}

char *
ctime_r(timep, buf)
const time_t * const	timep;
char *			buf;
{
	struct tm	tm;

#ifdef __LP64__
	/*
	 * In 64-bit, the timep value may produce a time value with a year
	 * that exceeds 32-bits in size (won't fit in struct tm), so localtime_r
	 * will return NULL.
	 */
	if (localtime_r(timep, &tm) == NULL)
		return NULL;
	return asctime_r(&tm, buf);
#else /* !__LP64__ */
	return asctime_r(localtime_r(timep, &tm), buf);
#endif /* __LP64__ */
}

/*
** Adapted from code provided by Robert Elz, who writes:
**	The "best" way to do mktime I think is based on an idea of Bob
**	Kridle's (so its said...) from a long time ago.
**	[kridle@xinet.com as of 1996-01-16.]
**	It does a binary search of the time_t space.  Since time_t's are
**	just 32 bits, its a max of 32 iterations (even at 64 bits it
**	would still be very reasonable).
*/

#ifndef WRONG
#define WRONG	(-1)
#endif /* !defined WRONG */

/*
** Simplified normalize logic courtesy Paul Eggert (eggert@twinsun.com).
*/

static int
increment_overflow(number, delta)
int *	number;
int	delta;
{
	int	number0;

	number0 = *number;
	*number += delta;
	return (*number < number0) != (delta < 0);
}

static int
normalize_overflow(tensptr, unitsptr, base)
int * const	tensptr;
int * const	unitsptr;
const int	base;
{
	int	tensdelta;

	tensdelta = (*unitsptr >= 0) ?
		(*unitsptr / base) :
		(-1 - (-1 - *unitsptr) / base);
	*unitsptr -= tensdelta * base;
	return increment_overflow(tensptr, tensdelta);
}

static int
tmcomp(atmp, btmp)
const struct tm * const atmp;
const struct tm * const btmp;
{
	int	result;

	/*
	 * Assume that atmp and btmp point to normalized tm strutures.
	 * So only arithmetic with tm_year could overflow in 64-bit.
	 */
	if (atmp->tm_year != btmp->tm_year) {
		return (atmp->tm_year > btmp->tm_year ? 1 : -1);
	}
	if ((result = (atmp->tm_mon - btmp->tm_mon)) == 0 &&
		(result = (atmp->tm_mday - btmp->tm_mday)) == 0 &&
		(result = (atmp->tm_hour - btmp->tm_hour)) == 0 &&
		(result = (atmp->tm_min - btmp->tm_min)) == 0)
			result = atmp->tm_sec - btmp->tm_sec;
	return result;
}

static time_t
time2sub(tmp, funcp, offset, okayp, do_norm_secs, unix03)
struct tm * const	tmp;
#ifdef __LP64__
struct tm *(* const	funcp)(const time_t*, long, struct tm*);
#else /* !__LP64__ */
void (* const		funcp)(const time_t*, long, struct tm*);
#endif /* __LP64__ */
const long		offset;
int * const		okayp;
const int		do_norm_secs;
int			unix03;
{
	const struct state *	sp;
	int			dir;
	int			bits;
	int			i, j ;
	int			saved_seconds;
	time_t				newt;
	time_t				t;
	struct tm			yourtm, mytm;
#ifdef __LP64__
	long				year, il;
#endif /* __LP64__ */

	*okayp = FALSE;
	yourtm = *tmp;
	if (do_norm_secs) {
		if (normalize_overflow(&yourtm.tm_min, &yourtm.tm_sec,
			SECSPERMIN))
				return WRONG;
	}
	if (normalize_overflow(&yourtm.tm_hour, &yourtm.tm_min, MINSPERHOUR))
		return WRONG;
	if (normalize_overflow(&yourtm.tm_mday, &yourtm.tm_hour, HOURSPERDAY))
		return WRONG;
	if (normalize_overflow(&yourtm.tm_year, &yourtm.tm_mon, MONSPERYEAR))
		return WRONG;
	/*
	** Turn yourtm.tm_year into an actual year number for now.
	** It is converted back to an offset from TM_YEAR_BASE later.
	*/
#ifdef __LP64__
	year = (long)yourtm.tm_year + TM_YEAR_BASE;
#else /* !__LP64__ */
	if (increment_overflow(&yourtm.tm_year, TM_YEAR_BASE))
		return WRONG;
#endif /* __LP64__ */
	while (yourtm.tm_mday <= 0) {
#ifdef __LP64__
		year--;
		il = year + (1 < yourtm.tm_mon);
		yourtm.tm_mday += year_lengths[isleap(il)];
#else /* !__LP64__ */
		if (increment_overflow(&yourtm.tm_year, -1))
			return WRONG;
		i = yourtm.tm_year + (1 < yourtm.tm_mon);
		yourtm.tm_mday += year_lengths[isleap(i)];
#endif /* __LP64__ */
	}
	while (yourtm.tm_mday > DAYSPERLYEAR) {
#ifdef __LP64__
		il = year + (1 < yourtm.tm_mon);
		yourtm.tm_mday -= year_lengths[isleap(il)];
		year++;
#else /* !__LP64__ */
		i = yourtm.tm_year + (1 < yourtm.tm_mon);
		yourtm.tm_mday -= year_lengths[isleap(i)];
		if (increment_overflow(&yourtm.tm_year, 1))
			return WRONG;
#endif /* __LP64__ */
	}
	for ( ; ; ) {
#ifdef __LP64__
		i = mon_lengths[isleap(year)][yourtm.tm_mon];
#else /* !__LP64__ */
		i = mon_lengths[isleap(yourtm.tm_year)][yourtm.tm_mon];
#endif /* __LP64__ */
		if (yourtm.tm_mday <= i)
			break;
		yourtm.tm_mday -= i;
		if (++yourtm.tm_mon >= MONSPERYEAR) {
			yourtm.tm_mon = 0;
#ifdef __LP64__
			year++;
#else /* !__LP64__ */
			if (increment_overflow(&yourtm.tm_year, 1))
				return WRONG;
#endif /* __LP64__ */
		}
	}
#ifdef __LP64__
	year -= TM_YEAR_BASE;
	if (year > INT_MAX || year < INT_MIN)
		return WRONG;
	yourtm.tm_year = year;
#else /* !__LP64__ */
	if (increment_overflow(&yourtm.tm_year, -TM_YEAR_BASE))
		return WRONG;
#endif /* __LP64__ */
	/* Don't go below 1900 for POLA */
	if (yourtm.tm_year < 0)
		return WRONG;
	if (yourtm.tm_sec >= 0 && yourtm.tm_sec < SECSPERMIN)
		saved_seconds = 0;
	else if (yourtm.tm_year + TM_YEAR_BASE < EPOCH_YEAR) {
		/*
		** We can't set tm_sec to 0, because that might push the
		** time below the minimum representable time.
		** Set tm_sec to 59 instead.
		** This assumes that the minimum representable time is
		** not in the same minute that a leap second was deleted from,
		** which is a safer assumption than using 58 would be.
		*/
		if (increment_overflow(&yourtm.tm_sec, 1 - SECSPERMIN))
			return WRONG;
		saved_seconds = yourtm.tm_sec;
		yourtm.tm_sec = SECSPERMIN - 1;
	} else {
		saved_seconds = yourtm.tm_sec;
		yourtm.tm_sec = 0;
	}
	/*
	** Divide the search space in half
	** (this works whether time_t is signed or unsigned).
	*/
#ifdef __LP64__
	/* optimization: see if the value is 31-bit (signed) */
	t = (((time_t) 1) << (TYPE_BIT(int) - 1)) - 1;
	bits = ((*funcp)(&t, offset, &mytm) == NULL || tmcomp(&mytm, &yourtm) < 0) ? TYPE_BIT(time_t) - 1 : TYPE_BIT(int) - 1;
#else /* !__LP64__ */
	bits = TYPE_BIT(time_t) - 1;
#endif /* __LP64__ */
	/*
	** In 64-bit, we now return an error if we cannot represent the
	** struct tm value in a time_t.  And tmcomp() is fixed to avoid
	** overflow in tm_year.  So we only put a cap on bits because time_t
	** can't be larger that 56 bit (when tm_year == INT_MAX).
	*/
	if (bits > 56)
		bits = 56;
	/*
	** If time_t is signed, then 0 is just above the median,
	** assuming two's complement arithmetic.
	** If time_t is unsigned, then (1 << bits) is just above the median.
	*/
	t = TYPE_SIGNED(time_t) ? 0 : (((time_t) 1) << bits);
	for ( ; ; ) {
#ifdef __LP64__
		if ((*funcp)(&t, offset, &mytm) == NULL) {
		    /* we overflowed, so t is too big */
		    dir = 1;
		    goto skip_tmcomp;
		}
#else /* !__LP64__ */
		(*funcp)(&t, offset, &mytm);
#endif /* __LP64__ */
		dir = tmcomp(&mytm, &yourtm);
#ifdef __LP64__
skip_tmcomp:
#endif /* __LP64__ */
		if (dir != 0) {
			if (bits-- < 0)
				return WRONG;
			if (bits < 0)
				--t; /* may be needed if new t is minimal */
			else if (dir > 0)
				t -= ((time_t) 1) << bits;
			else	t += ((time_t) 1) << bits;
			continue;
		}
		sp = (funcp == localsub) ? lclptr : gmtptr;
		if (unix03 && sp->typecnt == 1 && yourtm.tm_isdst > 0) 
			yourtm.tm_isdst = 0; /* alternative time does not apply */
		if (yourtm.tm_isdst < 0 || mytm.tm_isdst == yourtm.tm_isdst)
			break;
		/*
		** Right time, wrong type.
		** Hunt for right time, right type.
		** It's okay to guess wrong since the guess
		** gets checked.
		*/
#ifdef ALL_STATE
		if (sp == NULL)
			return WRONG;
#endif /* defined ALL_STATE */
		for (i = sp->typecnt - 1; i >= 0; --i) {
			if (sp->ttis[i].tt_isdst != yourtm.tm_isdst)
				continue;
			for (j = sp->typecnt - 1; j >= 0; --j) {
				if (sp->ttis[j].tt_isdst == yourtm.tm_isdst)
					continue;
				newt = t + sp->ttis[j].tt_gmtoff -
					sp->ttis[i].tt_gmtoff;
#ifdef __LP64__
				if ((*funcp)(&newt, offset, &mytm) == NULL)
					return WRONG;
#else /* !__LP64__ */
				(*funcp)(&newt, offset, &mytm);
#endif /* __LP64__ */
				if (tmcomp(&mytm, &yourtm) != 0)
					continue;
				if (mytm.tm_isdst != yourtm.tm_isdst)
					continue;
				/*
				** We have a match.
				*/
				t = newt;
				goto label;
			}
		}
		return WRONG;
	}
label:
	newt = t + saved_seconds;
	if ((newt < t) != (saved_seconds < 0))
		return WRONG;
	t = newt;
#ifdef __LP64__
	if ((*funcp)(&t, offset, tmp) == NULL)
		return WRONG;
#else /* !__LP64__ */
	(*funcp)(&t, offset, tmp);
#endif /* __LP64__ */
	*okayp = TRUE;
	return t;
}

static time_t
time2(tmp, funcp, offset, okayp, unix03)
struct tm * const	tmp;
#ifdef __LP64__
struct tm *(* const	funcp)(const time_t*, long, struct tm*);
#else /* !__LP64__ */
void (* const		funcp)(const time_t*, long, struct tm*);
#endif /* __LP64__ */
const long		offset;
int * const		okayp;
int			unix03;
{
	time_t	t;

	/*
	** First try without normalization of seconds
	** (in case tm_sec contains a value associated with a leap second).
	** If that fails, try with normalization of seconds.
	*/
	t = time2sub(tmp, funcp, offset, okayp, FALSE, unix03);
	return *okayp ? t : time2sub(tmp, funcp, offset, okayp, TRUE, unix03);
}

__private_extern__ time_t
time1(tmp, funcp, offset, unix03)
struct tm * const	tmp;
#ifdef __LP64__
struct tm *(* const	funcp)(const time_t *, long, struct tm *);
#else /* !__LP64__ */
void (* const		funcp)(const time_t *, long, struct tm *);
#endif /* __LP64__ */
const long		offset;
int			unix03;
{
	time_t			t;
	const struct state *	sp;
	int			samei, otheri;
	int			sameind, otherind;
	int			i;
	int			nseen;
	int				seen[TZ_MAX_TYPES];
	int				types[TZ_MAX_TYPES];
	int				okay;

	if (tmp->tm_isdst > 1)
		tmp->tm_isdst = 1;
	t = time2(tmp, funcp, offset, &okay, unix03);
#ifdef PCTS
	/*
	** PCTS code courtesy Grant Sullivan (grant@osf.org).
	*/
	if (okay)
		return t;
	if (tmp->tm_isdst < 0)
		tmp->tm_isdst = 0;	/* reset to std and try again */
#endif /* defined PCTS */
#ifndef PCTS
	if (okay || tmp->tm_isdst < 0)
		return t;
#endif /* !defined PCTS */
	/*
	** We're supposed to assume that somebody took a time of one type
	** and did some math on it that yielded a "struct tm" that's bad.
	** We try to divine the type they started from and adjust to the
	** type they need.
	*/
	sp = (funcp == localsub) ? lclptr : gmtptr;
#ifdef ALL_STATE
	if (sp == NULL)
		return WRONG;
#endif /* defined ALL_STATE */
	for (i = 0; i < sp->typecnt; ++i)
		seen[i] = FALSE;
	nseen = 0;
	for (i = sp->timecnt - 1; i >= 0; --i)
		if (!seen[sp->types[i]]) {
			seen[sp->types[i]] = TRUE;
			types[nseen++] = sp->types[i];
		}
	for (sameind = 0; sameind < nseen; ++sameind) {
		samei = types[sameind];
		if (sp->ttis[samei].tt_isdst != tmp->tm_isdst)
			continue;
		for (otherind = 0; otherind < nseen; ++otherind) {
			otheri = types[otherind];
			if (sp->ttis[otheri].tt_isdst == tmp->tm_isdst)
				continue;
			tmp->tm_sec += sp->ttis[otheri].tt_gmtoff -
					sp->ttis[samei].tt_gmtoff;
			tmp->tm_isdst = !tmp->tm_isdst;
			t = time2(tmp, funcp, offset, &okay, unix03);
			if (okay)
				return t;
			tmp->tm_sec -= sp->ttis[otheri].tt_gmtoff -
					sp->ttis[samei].tt_gmtoff;
			tmp->tm_isdst = !tmp->tm_isdst;
		}
	}
	return WRONG;
}
#else  /* BUILDING_VARIANT */
__private_extern__ pthread_rwlock_t	lcl_rwlock;
#endif /* BUILDING_VARIANT */

time_t
mktime(tmp)
struct tm * const	tmp;
{
	time_t mktime_return_value;
	int serrno = errno;
	_RWLOCK_RDLOCK(&lcl_rwlock);
	tzset_basic(1);
	mktime_return_value = time1(tmp, localsub, 0L, __DARWIN_UNIX03);
	_RWLOCK_UNLOCK(&lcl_rwlock);
	errno = serrno;
	return(mktime_return_value);
}

#if !BUILDING_VARIANT
#ifdef STD_INSPIRED

time_t
timelocal(tmp)
struct tm * const	tmp;
{
	tmp->tm_isdst = -1;	/* in case it wasn't initialized */
	return mktime(tmp);
}

time_t
timegm(tmp)
struct tm * const	tmp;
{
	tmp->tm_isdst = 0;
	return time1(tmp, gmtsub, 0L, __DARWIN_UNIX03);
}

time_t
timeoff(tmp, offset)
struct tm * const	tmp;
const long		offset;
{
	tmp->tm_isdst = 0;
	return time1(tmp, gmtsub, offset, __DARWIN_UNIX03);
}

#endif /* defined STD_INSPIRED */

#ifdef CMUCS

/*
** The following is supplied for compatibility with
** previous versions of the CMUCS runtime library.
*/

long
gtime(tmp)
struct tm * const	tmp;
{
	const time_t	t = mktime(tmp);

	if (t == WRONG)
		return -1;
	return t;
}

#endif /* defined CMUCS */

/*
** XXX--is the below the right way to conditionalize??
*/

#ifdef STD_INSPIRED

/*
** IEEE Std 1003.1-1988 (POSIX) legislates that 536457599
** shall correspond to "Wed Dec 31 23:59:59 UTC 1986", which
** is not the case if we are accounting for leap seconds.
** So, we provide the following conversion routines for use
** when exchanging timestamps with POSIX conforming systems.
*/

static long
leapcorr(timep)
time_t *	timep;
{
	struct state *		sp;
	struct lsinfo *	lp;
	int			i;

	sp = lclptr;
	i = sp->leapcnt;
	while (--i >= 0) {
		lp = &sp->lsis[i];
		if (*timep >= lp->ls_trans)
			return lp->ls_corr;
	}
	return 0;
}

time_t
time2posix(t)
time_t	t;
{
	tzset();
	return t - leapcorr(&t);
}

time_t
posix2time(t)
time_t	t;
{
	time_t	x;
	time_t	y;

	tzset();
	/*
	** For a positive leap second hit, the result
	** is not unique.  For a negative leap second
	** hit, the corresponding time doesn't exist,
	** so we return an adjacent second.
	*/
	x = t + leapcorr(&t);
	y = x - leapcorr(&x);
	if (y < t) {
		do {
			x++;
			y = x - leapcorr(&x);
		} while (y < t);
		if (t != y)
			return x - 1;
	} else if (y > t) {
		do {
			--x;
			y = x - leapcorr(&x);
		} while (y > t);
		if (t != y)
			return x + 1;
	}
	return x;
}

#endif /* defined STD_INSPIRED */
#endif /* !BUILDING_VARIANT */

/*
  tre.h - TRE public API definitions

  This software is released under a BSD-style license.
  See the file LICENSE for details and copyright.

*/

#ifndef TRE_H
#define TRE_H 1

#include "tre-config.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_LIBUTF8_H
#include <libutf8.h>
#endif /* HAVE_LIBUTF8_H */

#ifdef TRE_USE_SYSTEM_REGEX_H
/* Include the system regex.h to make TRE ABI compatible with the
   system regex. */
#include TRE_SYSTEM_REGEX_H_PATH
#define tre_regcomp    regcomp
#define tre_regexec    regexec
#define tre_regerror   regerror
#define tre_regfree    regfree

#ifdef TRE_APPROX
#define tre_regaexec   regaexec
#define tre_reganexec  reganexec
#define tre_regawexec  regawexec
#define tre_regawnexec regawnexec
#endif /* TRE_APPROX */
#define tre_regncomp   regncomp
#define tre_regnexec   regnexec
#define tre_regwcomp   regwcomp
#define tre_regwexec   regwexec
#define tre_regwncomp  regwncomp
#define tre_regwnexec  regwnexec

#define tre_regcomp_l    regcomp_l
#define tre_regncomp_l   regncomp_l
#define tre_regwcomp_l   regwcomp_l
#define tre_regwncomp_l  regwncomp_l
#endif /* TRE_USE_SYSTEM_REGEX_H */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TRE_USE_SYSTEM_REGEX_H

#ifndef REG_OK
#define REG_OK 0
#endif /* !REG_OK */

#ifndef HAVE_REG_ERRCODE_T
typedef int reg_errcode_t;
#endif /* !HAVE_REG_ERRCODE_T */

#if !defined(REG_NOSPEC) && !defined(REG_LITERAL)
#define REG_LITERAL 0x1000
#endif

/* Extra tre_regcomp() flags. */
#ifndef REG_BASIC
#define REG_BASIC	0
#endif /* !REG_BASIC */
#if 0
#define REG_LEFT_ASSOC  (REG_LITERAL << 1)
#define REG_UNGREEDY    (REG_LEFT_ASSOC << 1)

/* Extra tre_regexec() flags. */
#define REG_APPROX_MATCHER	 0x1000
#define REG_BACKTRACKING_MATCHER (REG_APPROX_MATCHER << 1)
#endif

#define RE_MAGIC  ((('r'^0200)<<8) | 'e')

#else /* !TRE_USE_SYSTEM_REGEX_H */

/* If the we're not using system regex.h, we need to define the
   structs and enums ourselves. */

typedef int regoff_t;
typedef struct {
  size_t re_nsub;      /* Number of parenthesized subexpressions. */
  const void *re_endp; /* regex string end pointer (REG_PEND) */
  void *value;	       /* For internal use only. */
} regex_t;

typedef struct {
  regoff_t rm_so;
  regoff_t rm_eo;
} regmatch_t;


typedef enum {
  REG_OK = 0,		/* No error. */
  /* POSIX tre_regcomp() return error codes.  (In the order listed in the
     standard.)	 */
  REG_NOMATCH,		/* No match. */
  REG_BADPAT,		/* Invalid regexp. */
  REG_ECOLLATE,		/* Unknown collating element. */
  REG_ECTYPE,		/* Unknown character class name. */
  REG_EESCAPE,		/* Trailing backslash. */
  REG_ESUBREG,		/* Invalid back reference. */
  REG_EBRACK,		/* "[]" imbalance */
  REG_EPAREN,		/* "\(\)" or "()" imbalance */
  REG_EBRACE,		/* "\{\}" or "{}" imbalance */
  REG_BADBR,		/* Invalid content of {} */
  REG_ERANGE,		/* Invalid use of range operator */
  REG_ESPACE,		/* Out of memory.  */
  REG_BADRPT,           /* Invalid use of repetition operators. */
  REG_INVARG,           /* Invalid argument, e.g. negative-length string */
  REG_ILLSEQ,           /* illegal byte sequence (bad multibyte character) */
} reg_errcode_t;

/* POSIX tre_regcomp() flags. */
#define REG_EXTENDED	1
#define REG_ICASE	(REG_EXTENDED << 1)
#define REG_NEWLINE	(REG_ICASE << 1)
#define REG_NOSUB	(REG_NEWLINE << 1)

/* Extra tre_regcomp() flags. */
#define REG_BASIC	0
#define REG_LITERAL	(REG_NOSUB << 1)
#define REG_LEFT_ASSOC  (REG_LITERAL << 1)
#define REG_UNGREEDY    (REG_LEFT_ASSOC << 1)
#define REG_PEND        (REG_UNGREEDY << 1)

/* POSIX tre_regexec() flags. */
#define REG_NOTBOL 1
#define REG_NOTEOL (REG_NOTBOL << 1)

/* Extra tre_regexec() flags. */
#define REG_APPROX_MATCHER	 (REG_NOTEOL << 1)
#define REG_BACKTRACKING_MATCHER (REG_APPROX_MATCHER << 1)
#define REG_STARTEND             (REG_BACKTRACKING_MATCHER << 1)

#endif /* !TRE_USE_SYSTEM_REGEX_H */

/* REG_NOSPEC and REG_LITERAL mean the same thing. */
#if defined(REG_LITERAL) && !defined(REG_NOSPEC)
#define REG_NOSPEC	REG_LITERAL
#elif defined(REG_NOSPEC) && !defined(REG_LITERAL)
#define REG_LITERAL	REG_NOSPEC
#endif /* defined(REG_NOSPEC) */

/* The maximum number of iterations in a bound expression. */
#undef RE_DUP_MAX
#define RE_DUP_MAX 255

/* The POSIX.2 regexp functions */
extern int
tre_regcomp(regex_t * __restrict preg, const char * __restrict regex, int cflags);

extern int
tre_regexec(const regex_t * __restrict preg, const char * __restrict string, size_t nmatch,
	regmatch_t pmatch[ __restrict ], int eflags);

extern size_t
tre_regerror(int errcode, const regex_t * __restrict preg, char * __restrict errbuf,
	 size_t errbuf_size);

extern void
tre_regfree(regex_t *preg);

#ifdef TRE_WCHAR
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif /* HAVE_WCHAR_H */

/* Wide character versions (not in POSIX.2). */
extern int
tre_regwcomp(regex_t * __restrict preg, const wchar_t * __restrict regex, int cflags);

extern int
tre_regwexec(const regex_t * __restrict preg, const wchar_t * __restrict string,
	 size_t nmatch, regmatch_t pmatch[ __restrict ], int eflags);
#endif /* TRE_WCHAR */

/* Versions with a maximum length argument and therefore the capability to
   handle null characters in the middle of the strings (not in POSIX.2). */
extern int
tre_regncomp(regex_t * __restrict preg, const char * __restrict regex, size_t len, int cflags);

extern int
tre_regnexec(const regex_t * __restrict preg, const char * __restrict string, size_t len,
	 size_t nmatch, regmatch_t pmatch[ __restrict ], int eflags);

#ifdef TRE_WCHAR
extern int
tre_regwncomp(regex_t * __restrict preg, const wchar_t * __restrict regex, size_t len, int cflags);

extern int
tre_regwnexec(const regex_t * __restrict preg, const wchar_t * __restrict string, size_t len,
	  size_t nmatch, regmatch_t pmatch[ __restrict ], int eflags);
#endif /* TRE_WCHAR */

/* Extended locale versions */
#include <xlocale.h>

extern int
tre_regcomp_l(regex_t * __restrict preg, const char * __restrict regex, int cflags, locale_t __restrict loc);

#ifdef TRE_WCHAR
extern int
tre_regwcomp_l(regex_t * __restrict preg, const wchar_t * __restrict regex, int cflags, locale_t __restrict loc);
#endif /* TRE_WCHAR */

extern int
tre_regncomp_l(regex_t * __restrict preg, const char * __restrict regex, size_t len, int cflags, locale_t __restrict loc);

#ifdef TRE_WCHAR
extern int
tre_regwncomp_l(regex_t * __restrict preg, const wchar_t * __restrict regex, size_t len, int cflags, locale_t __restrict loc);
#endif /* TRE_WCHAR */

#ifdef TRE_APPROX

/* Approximate matching parameter struct. */
typedef struct {
  int cost_ins;	       /* Default cost of an inserted character. */
  int cost_del;	       /* Default cost of a deleted character. */
  int cost_subst;      /* Default cost of a substituted character. */
  int max_cost;	       /* Maximum allowed cost of a match. */

  int max_ins;	       /* Maximum allowed number of inserts. */
  int max_del;	       /* Maximum allowed number of deletes. */
  int max_subst;       /* Maximum allowed number of substitutes. */
  int max_err;	       /* Maximum allowed number of errors total. */
} regaparams_t;

/* Approximate matching result struct. */
typedef struct {
  size_t nmatch;       /* Length of pmatch[] array. */
  regmatch_t *pmatch;  /* Submatch data. */
  int cost;	       /* Cost of the match. */
  int num_ins;	       /* Number of inserts in the match. */
  int num_del;	       /* Number of deletes in the match. */
  int num_subst;       /* Number of substitutes in the match. */
} regamatch_t;


/* Approximate matching functions. */
extern int
tre_regaexec(const regex_t * __restrict preg, const char * __restrict string,
	 regamatch_t * __restrict match, regaparams_t params, int eflags);

extern int
tre_reganexec(const regex_t * __restrict preg, const char * __restrict string, size_t len,
	  regamatch_t * __restrict match, regaparams_t params, int eflags);
#ifdef TRE_WCHAR
/* Wide character approximate matching. */
extern int
tre_regawexec(const regex_t * __restrict preg, const wchar_t * __restrict string,
	  regamatch_t * __restrict match, regaparams_t params, int eflags);

extern int
tre_regawnexec(const regex_t * __restrict preg, const wchar_t * __restrict string, size_t len,
	   regamatch_t * __restrict match, regaparams_t params, int eflags);
#endif /* TRE_WCHAR */

/* Sets the parameters to default values. */
extern void
tre_regaparams_default(regaparams_t *params);
#endif /* TRE_APPROX */

#ifdef TRE_WCHAR
typedef wchar_t tre_char_t;
#else /* !TRE_WCHAR */
typedef unsigned char tre_char_t;
#endif /* !TRE_WCHAR */

#ifdef TRE_STR_USER
typedef struct {
  int (*get_next_char)(tre_char_t *c, unsigned int *pos_add, void *context);
  void (*rewind)(size_t pos, void *context);
  int (*compare)(size_t pos1, size_t pos2, size_t len, void *context);
  void *context;
} tre_str_source;

extern int
tre_reguexec(const regex_t * __restrict preg, const tre_str_source * __restrict string,
	 size_t nmatch, regmatch_t pmatch[ __restrict ], int eflags);
#endif /* TRE_STR_USER */

#ifndef __LIBC__
/* Returns the version string.	The returned string is static. */
extern char *
tre_version(void);

/* Returns the value for a config parameter.  The type to which `result'
   must point to depends of the value of `query', see documentation for
   more details. */
extern int
tre_config(int query, void *result);

enum {
  TRE_CONFIG_APPROX,
  TRE_CONFIG_WCHAR,
  TRE_CONFIG_MULTIBYTE,
  TRE_CONFIG_SYSTEM_ABI,
  TRE_CONFIG_VERSION
};

/* Returns 1 if the compiled pattern has back references, 0 if not. */
extern int
tre_have_backrefs(const regex_t *preg);

#ifdef TRE_APPROX
/* Returns 1 if the compiled pattern uses approximate matching features,
   0 if not. */
extern int
tre_have_approx(const regex_t *preg);
#endif /* TRE_APPROX */
#endif /* !__LIBC__ */

#ifdef __cplusplus
}
#endif
#endif				/* TRE_H */

/* EOF */

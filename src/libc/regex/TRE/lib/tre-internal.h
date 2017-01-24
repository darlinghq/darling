/*
  tre-internal.h - TRE internal definitions

  This software is released under a BSD-style license.
  See the file LICENSE for details and copyright.

*/

#ifndef TRE_INTERNAL_H
#define TRE_INTERNAL_H 1

#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif /* HAVE_WCHAR_H */

#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif /* !HAVE_WCTYPE_H */

#include <ctype.h>

#ifdef __LIBC__
#include <xlocale_private.h>
#else /* !__LIBC__ */
#include <xlocale.h>
#endif /* !__LIBC__ */

#include "tre.h"
#include "tre-last-matched.h"

#ifdef TRE_DEBUG
#include <stdio.h>
#define DPRINT(msg) do {printf msg; fflush(stdout);} while(/*CONSTCOND*/0)
#else /* !TRE_DEBUG */
#define DPRINT(msg) do { } while(/*CONSTCOND*/0)
#endif /* !TRE_DEBUG */

#define elementsof(x)	( sizeof(x) / sizeof(x[0]) )

#ifdef HAVE_MBRTOWC
#define tre_mbrtowc(pwc, s, n, ps) (mbrtowc((pwc), (s), (n), (ps)))
/* xlocale */
#define tre_mbrtowc_l(pwc, s, n, ps, l) (mbrtowc_l((pwc), (s), (n), (ps), (l)))
#else /* !HAVE_MBRTOWC */
#ifdef HAVE_MBTOWC
#define tre_mbrtowc(pwc, s, n, ps) (mbtowc((pwc), (s), (n)))
#endif /* HAVE_MBTOWC */
#endif /* !HAVE_MBRTOWC */

#ifdef TRE_MULTIBYTE
#ifdef HAVE_MBSTATE_T
#define TRE_MBSTATE
#endif /* TRE_MULTIBYTE */
#endif /* HAVE_MBSTATE_T */

/* Define the character types and functions. */
#ifdef TRE_WCHAR

/* Wide characters. */
typedef wint_t tre_cint_t;
#define TRE_CHAR_MAX WCHAR_MAX

#ifdef TRE_MULTIBYTE
#define TRE_MB_CUR_MAX MB_CUR_MAX
/* xlocale */
#define TRE_MB_CUR_MAX_L MB_CUR_MAX_L
#else /* !TRE_MULTIBYTE */
#define TRE_MB_CUR_MAX 1
#endif /* !TRE_MULTIBYTE */

#define tre_isalnum iswalnum
#define tre_isalpha iswalpha
#ifdef HAVE_ISWBLANK
#define tre_isblank iswblank
#endif /* HAVE_ISWBLANK */
#define tre_iscntrl iswcntrl
#define tre_isdigit iswdigit
#define tre_isgraph iswgraph
#define tre_islower iswlower
#define tre_isprint iswprint
#define tre_ispunct iswpunct
#define tre_isspace iswspace
#define tre_isupper iswupper
#define tre_isxdigit iswxdigit

#define tre_tolower towlower
#define tre_toupper towupper
#define tre_strlen  wcslen

/* xlocale */
#define tre_isalnum_l iswalnum_l
#define tre_isdigit_l iswdigit_l
#define tre_islower_l iswlower_l
#define tre_isupper_l iswupper_l
#define tre_isxdigit_l iswxdigit_l
#define tre_tolower_l towlower_l
#define tre_toupper_l towupper_l

#else /* !TRE_WCHAR */

/* 8 bit characters. */
typedef short tre_cint_t;
#define TRE_CHAR_MAX 255
#define TRE_MB_CUR_MAX 1

#define tre_isalnum isalnum
#define tre_isalpha isalpha
#ifdef HAVE_ISASCII
#define tre_isascii isascii
#endif /* HAVE_ISASCII */
#ifdef HAVE_ISBLANK
#define tre_isblank isblank
#endif /* HAVE_ISBLANK */
#define tre_iscntrl iscntrl
#define tre_isdigit isdigit
#define tre_isgraph isgraph
#define tre_islower islower
#define tre_isprint isprint
#define tre_ispunct ispunct
#define tre_isspace isspace
#define tre_isupper isupper
#define tre_isxdigit isxdigit

#define tre_tolower(c) (tre_cint_t)(tolower(c))
#define tre_toupper(c) (tre_cint_t)(toupper(c))
#define tre_strlen(s)  (strlen((const char*)s))

#endif /* !TRE_WCHAR */

#if defined(TRE_WCHAR) && defined(HAVE_ISWCTYPE) && defined(HAVE_WCTYPE)
#define TRE_USE_SYSTEM_WCTYPE 1
#endif

#ifdef TRE_USE_SYSTEM_WCTYPE
/* Use system provided iswctype() and wctype(). */
typedef wctype_t tre_ctype_t;
#define tre_isctype iswctype
#define tre_ctype   wctype

/* xlocale */
#define tre_isctype_l iswctype_l
#define tre_ctype_l   wctype_l

#else /* !TRE_USE_SYSTEM_WCTYPE */
/* Define our own versions of iswctype() and wctype(). */
typedef int (*tre_ctype_t)(tre_cint_t);
#define tre_isctype(c, type) ( (type)(c) )
tre_ctype_t tre_ctype(const char *name);
#endif /* !TRE_USE_SYSTEM_WCTYPE */

typedef enum { STR_WIDE, STR_BYTE, STR_MBS,
#ifdef TRE_STR_USER
	       STR_USER
#endif /* TRE_STR_USER */
	     } tre_str_type_t;

/* Returns number of bytes to add to (char *)ptr to make it
   properly aligned for the type. */
#define ALIGN(ptr, type) \
  ((((long)ptr) % sizeof(type)) \
   ? (sizeof(type) - (((long)ptr) % sizeof(type))) \
   : 0)

#undef MAX
#undef MIN
#define MAX(a, b) (((a) >= (b)) ? (a) : (b))
#define MIN(a, b) (((a) <= (b)) ? (a) : (b))

/* Define STRF to the correct printf formatter for strings. */
#ifdef TRE_WCHAR
#define STRF "ls"
#else /* !TRE_WCHAR */
#define STRF "s"
#endif /* !TRE_WCHAR */

/* Types to handle bracket expressions. */
typedef enum {
  TRE_BRACKET_MATCH_TYPE_UNUSED = 0,
  TRE_BRACKET_MATCH_TYPE_CHAR,		/* Single character value */
  TRE_BRACKET_MATCH_TYPE_RANGE_BEGIN,	/* Collation range begin */
  TRE_BRACKET_MATCH_TYPE_RANGE_END,	/* Collation range end */
  TRE_BRACKET_MATCH_TYPE_CLASS,		/* Character class */
  TRE_BRACKET_MATCH_TYPE_EQUIVALENCE,	/* Collation equivalence value */
} tre_bracket_match_type_t;

typedef struct {
  tre_bracket_match_type_t type;
  tre_cint_t value;
} tre_bracket_match_t;

#define TRE_BRACKET_MATCH_FLAG_NEGATE	1

typedef struct {
  int num_bracket_matches;
  int flags;
  tre_bracket_match_t bracket_matches[0];
} tre_bracket_match_list_t;

#define SIZEOF_BRACKET_MATCH_LIST_N(n)	(sizeof(tre_bracket_match_list_t) + \
					 sizeof(tre_bracket_match_t) * (n))
#define SIZEOF_BRACKET_MATCH_LIST(l)	SIZEOF_BRACKET_MATCH_LIST_N( \
					 (l)->num_bracket_matches)

/* The "count" field is the number of time the tag was set, initially zero.
   The "first" field contains the first set value (when "count" equals 1).
   The "value" field contains the current value of the tag, if "count" is
   greater than zero (the tag's current value is -1 if "count" is zero).
   The "touch" field is the touch value, a montonically increasing value
   (maintained by the caller) set each time the tag itself is set. */
typedef struct {
  int count;
  int first;
  int value;
  int touch;
} tre_tag_t;

/* TNFA transition type. A TNFA state is an array of transitions,
   the terminator is a transition with NULL `state'. */
typedef struct tnfa_transition tre_tnfa_transition_t;

struct tnfa_transition {
  /* Range of accepted characters. */
  tre_cint_t code_min;
  tre_cint_t code_max;
  /* Pointer to the destination state. */
  tre_tnfa_transition_t *state;
  /* ID number of the destination state. */
  int state_id;
  /* -1 terminated array of tags (or NULL). */
  int *tags;
  /* Matching parameters settings (or NULL). */
  int *params;
  /* Assertion bitmap. */
  int assertions;
  /* Assertion parameters. */
  union {
    /* Bracket matches. */
    tre_bracket_match_list_t *bracket_match_list;
    /* Back reference assertion. */
    int backref;
  } u;
};


/* Assertions. */
#define ASSERT_AT_BOL		  1   /* Beginning of line. */
#define ASSERT_AT_EOL		  2   /* End of line. */
#define ASSERT_BRACKET_MATCH	  4   /* Matches in `bracket_match_list'. */
#define ASSERT_AT_BOW		  8   /* Beginning of word. */
#define ASSERT_AT_EOW		 16   /* End of word. */
#define ASSERT_AT_WB		 32   /* Word boundary. */
#define ASSERT_AT_WB_NEG	 64   /* Not a word boundary. */
#define ASSERT_BACKREF		128   /* A back reference in `backref'. */
#define ASSERT_LAST		128

/* Tag directions. */
typedef enum {
  TRE_TAG_MINIMIZE = 0,
  TRE_TAG_MAXIMIZE,
  TRE_TAG_LEFT_MAXIMIZE,
} tre_tag_direction_t;

/* Parameters that can be changed dynamically while matching. */
typedef enum {
  TRE_PARAM_COST_INS	    = 0,
  TRE_PARAM_COST_DEL	    = 1,
  TRE_PARAM_COST_SUBST	    = 2,
  TRE_PARAM_COST_MAX	    = 3,
  TRE_PARAM_MAX_INS	    = 4,
  TRE_PARAM_MAX_DEL	    = 5,
  TRE_PARAM_MAX_SUBST	    = 6,
  TRE_PARAM_MAX_ERR	    = 7,
  TRE_PARAM_DEPTH	    = 8,
  TRE_PARAM_LAST	    = 9
} tre_param_t;

/* Unset matching parameter */
#define TRE_PARAM_UNSET -1

/* Signifies the default matching parameter value. */
#define TRE_PARAM_DEFAULT -2

/* Instructions to compute submatch register values from tag values
   after a successful match.  */
struct tre_submatch_data {
  /* Tag that gives the value for rm_so (submatch start offset). */
  int so_tag;
  /* Tag that gives the value for rm_eo (submatch end offset). */
  int eo_tag;
};

typedef struct tre_submatch_data tre_submatch_data_t;


/* TNFA definition. */
typedef struct tnfa tre_tnfa_t;

struct tnfa {
  tre_tnfa_transition_t *transitions;
  tre_tnfa_transition_t *initial;
  tre_tnfa_transition_t *final;
  tre_submatch_data_t *submatch_data;
#ifdef USE_FIRSTPOS_CHARS /* not defined */
  char *firstpos_chars;
#endif /* USE_FIRSTPOS_CHARS */
  tre_tag_direction_t *tag_directions;
  int *minimal_tags;
  tre_last_matched_branch_t *last_matched_branch;
  locale_t loc;
  unsigned int num_transitions;
  int first_char;
  unsigned int num_submatches;
  unsigned int num_submatches_invisible;
  int num_tags;
  int num_minimals;
  int end_tag;
  int num_states;
  int cflags;
  int have_backrefs;
  int num_reorder_tags;
  int have_approx;
  int params_depth;
};

__private_extern__ int
tre_compile(regex_t * __restrict preg, const tre_char_t * __restrict regex, size_t n, int cflags,
	    locale_t __restrict loc);

__private_extern__ void
tre_free(regex_t *preg);

__private_extern__ reg_errcode_t
tre_fill_pmatch(size_t nmatch, regmatch_t pmatch[ __restrict ], int cflags,
		const tre_tnfa_t * __restrict tnfa, const tre_tag_t * __restrict tags, int match_eo);

__private_extern__ reg_errcode_t
tre_tnfa_run_parallel(const tre_tnfa_t * __restrict tnfa, const void * __restrict string, int len,
		      tre_str_type_t type, tre_tag_t * __restrict match_tags, int eflags,
		      int * __restrict match_end_ofs);

__private_extern__ reg_errcode_t
tre_tnfa_run_backtrack(const tre_tnfa_t * __restrict tnfa, const void * __restrict string,
		       int len, tre_str_type_t type, tre_tag_t * __restrict match_tags,
		       int eflags, int * __restrict match_end_ofs);

#ifdef TRE_APPROX
__private_extern__ reg_errcode_t
tre_tnfa_run_approx(const tre_tnfa_t * __restrict tnfa, const void * __restrict string, int len,
		    tre_str_type_t type, tre_tag_t * __restrict match_tags,
		    regamatch_t * __restrict match, regaparams_t params,
		    int eflags, int * __restrict match_end_ofs);
#endif /* TRE_APPROX */

#endif /* TRE_INTERNAL_H */

/* EOF */

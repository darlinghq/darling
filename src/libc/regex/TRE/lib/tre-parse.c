/*
  tre-parse.c - Regexp parser

  This software is released under a BSD-style license.
  See the file LICENSE for details and copyright.

*/

/*
  This parser is just a simple recursive descent parser for POSIX.2
  regexps.  The parser supports both the obsolete default syntax and
  the "extended" syntax, and some nonstandard extensions.
*/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <stddef.h>

#include "xmalloc.h"
#include "tre-mem.h"
#include "tre-ast.h"
#include "tre-stack.h"
#include "tre-parse.h"

/* BSD compatibility:
     Before looking up a collating symbol, check if the name matches in
     the character names (cnames) array; if so, use the corresponding
     character.

     Also set ERE_LITERAL_LBRACE_ON_NON_NUMERIC_BOUND, which will preserve
     the implementation choice that for ERE, a non-numeric character following
     a left brace that would normally be a bound, causes the left brace to be
     literal. */
#define BSD_COMPATIBILITY
#ifdef BSD_COMPATIBILITY
#include "cname.h"
#define ERE_LITERAL_LBRACE_ON_NON_NUMERIC_BOUND
#endif /* BSD_COMPATIBILITY */

/* Characters with special meanings in regexp syntax. */
#define CHAR_PIPE	   L'|'
#define CHAR_LPAREN	   L'('
#define CHAR_RPAREN	   L')'
#define CHAR_LBRACE	   L'{'
#define CHAR_RBRACE	   L'}'
#define CHAR_LBRACKET	   L'['
#define CHAR_RBRACKET	   L']'
#define CHAR_MINUS	   L'-'
#define CHAR_STAR	   L'*'
#define CHAR_QUESTIONMARK  L'?'
#define CHAR_PLUS	   L'+'
#define CHAR_PERIOD	   L'.'
#define CHAR_COLON	   L':'
#define CHAR_EQUAL	   L'='
#define CHAR_COMMA	   L','
#define CHAR_CARET	   L'^'
#define CHAR_DOLLAR	   L'$'
#define CHAR_BACKSLASH	   L'\\'
#define CHAR_HASH	   L'#'
#define CHAR_TILDE	   L'~'


/* Some macros for expanding \w, \s, etc. */
static const struct tre_macro_struct {
  const char c;
  const char *expansion;
} tre_macros[] =
  { {'t', "\t"},	   {'n', "\n"},		   {'r', "\r"},
    {'f', "\f"},	   {'a', "\a"},		   {'e', "\033"},
    {'w', "[[:alnum:]_]"}, {'W', "[^[:alnum:]_]"}, {'s', "[[:space:]]"},
    {'S', "[^[:space:]]"}, {'d', "[[:digit:]]"},   {'D', "[^[:digit:]]"},
    { 0, NULL }
  };


/* Expands a macro delimited by `regex' and `regex_end' to `buf', which
   must have at least `len' items.  Sets buf[0] to zero if the there
   is no match in `tre_macros'. */
static void
tre_expand_macro(const tre_char_t *regex, const tre_char_t *regex_end,
		 tre_char_t *buf, size_t buf_len)
{
  int i;

  buf[0] = 0;
  if (regex >= regex_end)
    return;

  for (i = 0; tre_macros[i].expansion; i++)
    {
      if (tre_macros[i].c == *regex)
	{
	  unsigned int j;
	  DPRINT(("Expanding macro '%c' => '%s'\n",
		  tre_macros[i].c, tre_macros[i].expansion));
	  for (j = 0; tre_macros[i].expansion[j] && j < buf_len; j++)
	    buf[j] = tre_macros[i].expansion[j];
	  buf[j] = 0;
	  break;
	}
    }
}

static reg_errcode_t
tre_new_item(tre_mem_t __unused mem, int type, int val, int *max_i,
	 tre_bracket_match_list_t **items)
{
  reg_errcode_t status = REG_OK;
  tre_bracket_match_list_t *array = *items;
  int i = array->num_bracket_matches;
  /* Allocate more space if necessary. */
  if (i >= *max_i)
    {
      tre_bracket_match_list_t *new_items;
      DPRINT(("out of tre_bracket_match_list_t array space (%d)\n", i));
      /* If the array is already 1024 items large, give up -- there's
	 probably an error in the regexp (e.g. not a '\0' terminated
	 string and missing ']') */
      if (*max_i >= 1024)
	return REG_ESPACE;
      *max_i *= 2;
      new_items = xrealloc(array, SIZEOF_BRACKET_MATCH_LIST_N(*max_i));
      if (new_items == NULL)
	return REG_ESPACE;
      *items = array = new_items;
    }
  array->bracket_matches[i].type = type;
  array->bracket_matches[i].value = val;
  array->num_bracket_matches++;
  return status;
}

#ifndef TRE_USE_SYSTEM_WCTYPE

/* isalnum() and the rest may be macros, so wrap them to functions. */
int tre_isalnum_func(tre_cint_t c) { return tre_isalnum(c); }
int tre_isalpha_func(tre_cint_t c) { return tre_isalpha(c); }

#ifdef tre_isascii
int tre_isascii_func(tre_cint_t c) { return tre_isascii(c); }
#else /* !tre_isascii */
int tre_isascii_func(tre_cint_t c) { return !(c >> 7); }
#endif /* !tre_isascii */

#ifdef tre_isblank
int tre_isblank_func(tre_cint_t c) { return tre_isblank(c); }
#else /* !tre_isblank */
int tre_isblank_func(tre_cint_t c) { return ((c == ' ') || (c == '\t')); }
#endif /* !tre_isblank */

int tre_iscntrl_func(tre_cint_t c) { return tre_iscntrl(c); }
int tre_isdigit_func(tre_cint_t c) { return tre_isdigit(c); }
int tre_isgraph_func(tre_cint_t c) { return tre_isgraph(c); }
int tre_islower_func(tre_cint_t c) { return tre_islower(c); }
int tre_isprint_func(tre_cint_t c) { return tre_isprint(c); }
int tre_ispunct_func(tre_cint_t c) { return tre_ispunct(c); }
int tre_isspace_func(tre_cint_t c) { return tre_isspace(c); }
int tre_isupper_func(tre_cint_t c) { return tre_isupper(c); }
int tre_isxdigit_func(tre_cint_t c) { return tre_isxdigit(c); }

struct {
  char *name;
  int (*func)(tre_cint_t);
} tre_ctype_map[] = {
  { "alnum", &tre_isalnum_func },
  { "alpha", &tre_isalpha_func },
#ifdef tre_isascii
  { "ascii", &tre_isascii_func },
#endif /* tre_isascii */
#ifdef tre_isblank
  { "blank", &tre_isblank_func },
#endif /* tre_isblank */
  { "cntrl", &tre_iscntrl_func },
  { "digit", &tre_isdigit_func },
  { "graph", &tre_isgraph_func },
  { "lower", &tre_islower_func },
  { "print", &tre_isprint_func },
  { "punct", &tre_ispunct_func },
  { "space", &tre_isspace_func },
  { "upper", &tre_isupper_func },
  { "xdigit", &tre_isxdigit_func },
  { NULL, NULL}
};

tre_ctype_t tre_ctype(const char *name)
{
  int i;
  for (i = 0; tre_ctype_map[i].name != NULL; i++)
    {
      if (strcmp(name, tre_ctype_map[i].name) == 0)
	return tre_ctype_map[i].func;
    }
  return (tre_ctype_t)0;
}
#endif /* !TRE_USE_SYSTEM_WCTYPE */

#define REST(re) (int)(ctx->re_end - (re)), (re)

#define START_COLLATING_SYMBOLS		16
#define MAX_COLLATING_SYMBOL_LEN	4

typedef struct {
  const tre_char_t *start;
  int len;
} tre_collating_symbol;

#include <xlocale.h>

int __collate_equiv_value(locale_t loc, const wchar_t *str, size_t len);

#ifdef BSD_COMPATIBILITY
static wchar_t
tre_search_cnames(const wchar_t *name, size_t len)
{
  size_t low = 0;
  size_t high = NCNAMES - 1;
  size_t cur;
  int cmp;

  while(low <= high)
    {
      cur = (low + high) / 2;
      cmp = wcsncmp(name, cnames[cur].name, len);
      if (cmp == 0 && cnames[cur].name[len] == 0) return cnames[cur].code;
      if (cmp > 0) low = cur + 1;
      else high = cur - 1;
    }
  return (wchar_t)-1;
}
#endif /* BSD_COMPATIBILITY */

/* Scan the contents of a bracket expression, and create a
 * tre_bracket_match_list_t encoding the bracket expression.  If during
 * the scan, multi-character collating symbols are detected, switch
 * into a mode to collect those MCCSs into a tre_collating_symbol
 * list and pass them back.  tre_parse_bracket will use that to
 * create a new string composed of a union of the bracket expression
 * without the MCCSs and the MCCSs (e.g., [x[.ch.]] => [x]|ch), and
 * call tre_parse (recursive) to parse that new string (which will
 * call tre_parse_bracket and tre_parse_bracket_items again. */
static reg_errcode_t
tre_parse_bracket_items(tre_parse_ctx_t *ctx, tre_bracket_match_list_t **items,
			int *items_size, tre_collating_symbol **result)
{
  const tre_char_t *re = ctx->re;
  const tre_char_t *re_end = ctx->re_end;
  tre_collating_symbol *col_syms = NULL;
  tre_collating_symbol *cp = NULL;
  int n_col_syms = 0;
  reg_errcode_t status;
  int max_i = *items_size;
  int other = 0;  /* contains content other than multi-character collating
		   * symbols */
  int range = -1; /* -1 unset, 0 begin range set, +1 end range expected */
  tre_cint_t min, c;
  int invert = ((*items)->flags & TRE_BRACKET_MATCH_FLAG_NEGATE);
  int collect_MCCS = 0;
  const tre_char_t *start;

  for ( ;re < re_end; re++)
    {
      switch (*re)
	{
	case CHAR_MINUS:
	  /* A first hyphen */
	  if (re == ctx->re)
	    {
	      DPRINT(("tre_parse_bracket:   char: '%.*" STRF "'\n", REST(re)));
	      min = CHAR_MINUS;
	      other++;
	      range = 0;
	      break;
	    }
	  /* The hyphen is the end range */
	  if (range > 0)
	    {
	      DPRINT(("tre_parse_bracket:   char: '%.*" STRF "'\n", REST(re)));
	      c = CHAR_MINUS;
	      goto process_end_range;
	    }
	  if (re + 1 >= re_end)
	    {
	      status = REG_EBRACK;
	      goto error;
	    }
	  /* The hyphen is at the end */
	  if (re[1] == CHAR_RBRACKET)
	    {
	      DPRINT(("tre_parse_bracket:   char: '%.*" STRF "'\n", REST(re)));
	      c = CHAR_MINUS;
	      goto process_begin_range;
	    }
	  /* Two ranges are not allowed to share an endpoint, or begin
	   * range is illegal. */
	  if (range < 0)
	    {
	      status = REG_ERANGE;
	      goto error;
	    }
	  range = 1; /* Expect end range */
	  DPRINT(("tre_parse_bracket:   range: '%.*" STRF "'\n", REST(re)));
	  break;

	case CHAR_LBRACKET:
	  if (re + 1 >= re_end)
	    {
	      status = REG_EBRACK;
	      goto error;
	    }
	  switch (re[1])
	    {
	    case CHAR_PERIOD:
	      {
		re += 2;
		start = re;
		for (;; re++)
		  {
		    if (re >= re_end)
		      {
			status = REG_ECOLLATE;
			goto error;
		      }
		    if (*re == CHAR_PERIOD)
		      {
			if (re + 1 >= re_end)
			  {
			    status = REG_ECOLLATE;
			    goto error;
			  }
			/* Found end */
			if (re[1] == CHAR_RBRACKET)
			  {
			    DPRINT(("tre_parse_bracket:   collating "
				    "symbol: '%.*" STRF "'\n",
				    REST(start - 2)));
			    /* Empty name */
			    if (re == start)
			      {
				status = REG_ECOLLATE;
				goto error;
			      }
#ifdef BSD_COMPATIBILITY
			    /* Check if the name is in cnames; if so, use
			       the corresponding code */
			    c = tre_search_cnames(start, re - start);
			    if (c != (wchar_t)-1)
			      {
				re++;
				goto process_single_character;
			      }
#endif /* BSD_COMPATIBILITY */
			    /* Verify this is a known sequence */
			    if (__collate_equiv_value(ctx->loc, start,
							  re - start) <= 0)
			      {
				status = REG_ECOLLATE;
				goto error;
			      }
			    /* Process single character collating symbols */
			    if (re - start == 1)
			      {
				c = *start;
				re++;
				goto process_single_character;
			      }
			    /* Inverted MCCSs are undefined */
			    if (invert)
			      {
				status = REG_ECOLLATE;
				goto error;
			      }
			    /* Can't have MCCSs as an endpoint to a range */
			    if (range > 0)
			      {
				status = REG_ERANGE;
				goto error;
			      }
			    range = -1;
			    /* Switch into MCCS collection mode (if not
			     * already there */
#if TRE_DEBUG
			    if (!collect_MCCS)
			      {
				collect_MCCS = 1;
				DPRINT(("tre_parse_bracket: Detected MCCS\n"));
			      }
#else /* !TRE_DEBUG */
			    collect_MCCS = 1;
#endif /* !TRE_DEBUG */
			    /* Allocate a memory block the first time */
			    if (!cp)
			      {
				if ((col_syms = xmalloc(sizeof(*col_syms) *
					    (START_COLLATING_SYMBOLS + 2)))
					    == NULL)
				  return REG_ESPACE;
				cp = col_syms + 1;
				n_col_syms = START_COLLATING_SYMBOLS;
			      }
			    /* Enlarge the memory block is more is needed */
			    if ((cp - col_syms) - 1 >= n_col_syms)
			      {
				int i = n_col_syms;
				tre_collating_symbol *tmp =
				    xrealloc(col_syms, sizeof(*col_syms) *
					     ((n_col_syms *= 2) + 2));
				if (tmp == NULL)
				  {
				    xfree(col_syms);
				    return REG_ESPACE;
				  }
				DPRINT(("tre_list_collating_symbols: "
					"Enlarging col_syms to %d\n",
					n_col_syms));
				col_syms = tmp;
				cp = col_syms + i + 1;
			      }
			    cp->start = start;
			    cp->len = re - start;
			    cp++;
			    re++;
			    break;
			  }
		      }
		  }
		break;
	      }

	    case CHAR_EQUAL:
	    case CHAR_COLON:
	      {
		/* Process equivalence and character classes */
		tre_char_t kind = re[1];

		/* Can't have a class as an endpoint to a range */
		if (range > 0)
		  {
		    status = REG_ERANGE;
		    goto error;
		  }
		if (!collect_MCCS && range == 0)
		  {
		    status = tre_new_item(ctx->mem, TRE_BRACKET_MATCH_TYPE_CHAR,
					  min, &max_i, items);
		    if (status != REG_OK)
		      goto error;
		  }
		range = -1;
		re += 2;
		start = re;
		for (;; re++)
		  {
		    if (re >= re_end)
		      {
			status = kind == CHAR_EQUAL ? REG_ECOLLATE : REG_ECTYPE;
			goto error;
		      }
		    if (*re == kind)
		      {
			if (re + 1 >= re_end)
			  {
			    status = kind == CHAR_EQUAL ? REG_ECOLLATE :
							  REG_ECTYPE;
			    goto error;
			  }
			/* Found end */
			if (re[1] == CHAR_RBRACKET)
			  {
			    if (re == start)
			      {
				/* Empty class name */
				status = kind == CHAR_EQUAL ? REG_ECOLLATE :
							      REG_ECTYPE;
				goto error;
			      }
			    /* Process equivalence class */
			    if (kind == CHAR_EQUAL)
			      {
				int equiv;

				DPRINT(("tre_parse_bracket:   equivalence: '%.*"
					STRF "'\n", REST(start - 2)));

				/* While we find the collation value even for
				   multi-character collating elements , we
				   don't (yet) match any collation values
				   against multi-character sequences.  We'd have
				   to enumerate those multi-character sequences
				   and like multi-character collating symbols,
				   create a union of those sequences with the
				   rest of the bracket expression.  While
				   doable, a bracket expression matching
				   multiple characters, that doesn't explicitly
				   contain multi-character sequences, might
				   be unexpected, so we punt for now. */
				if ((equiv = __collate_equiv_value(ctx->loc,
					     start, re - start)) <= 0)
				  {
				    /* The standard says that if no collating
				       element if found, we use the collating
				       symbol itself.  But __collate_equiv_value
				       doesn't make a distinction between
				       an element that is in a equvalence
				       class with others, or is the only member,
				       so we already know there is no collating
				       symbol.  (Note that in the case of a
				       collating element whose collation value
				       is unique, matching against the
				       collating element itself, or against
				       its collation value, is equivalent.) */
#ifdef BSD_COMPATIBILITY
				    /* Check if the name is in cnames; if so,
				       use the corresponding code */
				    c = tre_search_cnames(start, re - start);
				    if (c != (wchar_t)-1)
				      {
					re++;
					goto process_single_character;
				      }
#endif /* BSD_COMPATIBILITY */
				    status = REG_ECOLLATE;
				    goto error;
				  }
				if (!collect_MCCS)
				  {
				    status = tre_new_item(ctx->mem,
					     TRE_BRACKET_MATCH_TYPE_EQUIVALENCE,
					     equiv, &max_i, items);
				    if (status != REG_OK)
				      goto error;
				  }
			      }
			    else
			      {
				/* Process character class */
				DPRINT(("tre_parse_bracket:  class: '%.*" STRF
					"'\n", REST(start - 2)));
				if (!collect_MCCS)
				  {
				    char tmp_str[64];
				    tre_ctype_t class;
				    int len = MIN(re - start, 63);
#ifdef TRE_WCHAR
				    {
				      tre_char_t tmp_wcs[64];
				      wcsncpy(tmp_wcs, start, (size_t)len);
				      tmp_wcs[len] = L'\0';
#if defined HAVE_WCSRTOMBS
				      {
					mbstate_t state;
					const tre_char_t *src = tmp_wcs;
					memset(&state, '\0', sizeof(state));
					len = wcsrtombs_l(tmp_str, &src,
						      sizeof(tmp_str), &state,
						      ctx->loc);
				      }
#elif defined HAVE_WCSTOMBS
				      len = wcstombs(tmp_str, tmp_wcs, 63);
#endif /* defined HAVE_WCSTOMBS */
				    }
#else /* !TRE_WCHAR */
				    strncpy(tmp_str, (const char*)start, len);
#endif /* !TRE_WCHAR */
				    tmp_str[len] = '\0';
				    DPRINT(("  class name: %s\n", tmp_str));
				    class = tre_ctype_l(tmp_str, ctx->loc);
				    if (!class)
				      {
					status = REG_ECTYPE;
					goto error;
				      }
				    status = tre_new_item(ctx->mem,
					     TRE_BRACKET_MATCH_TYPE_CLASS,
					     class, &max_i, items);
				    if (status != REG_OK)
				      goto error;
				  }
			      }
			    re++;
			    break;
			  }
		      }
		  }
		other++;
		break;
	      }

	    default:
	      DPRINT(("tre_parse_bracket:   char: '%.*" STRF "'\n", REST(re)));
	      c = CHAR_LBRACKET;
	      goto process_single_character;
	      break;
	    }
	  break;

	case CHAR_RBRACKET:
	  /* A first right bracket */
	  if (re == ctx->re)
	    {
	      DPRINT(("tre_parse_bracket:   char: '%.*" STRF "'\n", REST(re)));
	      min = CHAR_RBRACKET;
	      range = 0;
	      other++;
	      break;
	    }
	  /* Done */
	  if (collect_MCCS)
	    {
	      DPRINT(("tre_parse_bracket:	done: '%.*" STRF "'\n",
		      REST(re)));
	      if (col_syms)
		{
		  /* Mark the character following the right bracket.  Set len
		   * to whether there are other things besides the
		   * multi-character collating symbols */
		  col_syms->start = re + 1;
		  col_syms->len = other;
		  /* Mark the end of the list */
		  cp->start = NULL;
		}
	      *result = col_syms;
	      return REG_OK;
	    }
	  /* range > 0 is not possible, since we did a lookahead after the
	   * hyphen */
	  if (range == 0)
	    {
	      status = tre_new_item(ctx->mem, TRE_BRACKET_MATCH_TYPE_CHAR,
				    min, &max_i, items);
	      if (status != REG_OK)
		goto error;
	    }
	  DPRINT(("tre_parse_bracket:	done: '%.*" STRF "'\n", REST(re)));
	  *items_size = max_i;
	  ctx->re = re + 1;
	  return REG_OK;

	default:
	  DPRINT(("tre_parse_bracket:   char: '%.*" STRF "'\n", REST(re)));
	  c = *re;
process_single_character:
	  /* Process single character */
	  if (range > 0)
	    {
	      int mine, maxe;

process_end_range:
	      /* Get collation equivalence values */
	      mine = __collate_equiv_value(ctx->loc, &min, 1);
	      maxe = __collate_equiv_value(ctx->loc, &c, 1);
	      if (maxe < mine)
		{
		  status = REG_ERANGE;
		  goto error;
		}
	      if (!collect_MCCS)
		{
		  status = tre_new_item(ctx->mem,
					TRE_BRACKET_MATCH_TYPE_RANGE_BEGIN,
					mine, &max_i, items);
		  if (status != REG_OK)
		    goto error;
		  status = tre_new_item(ctx->mem,
					TRE_BRACKET_MATCH_TYPE_RANGE_END,
					maxe, &max_i, items);
		  if (status != REG_OK)
		    goto error;
		}
	      range = -1;
	    }
	  else
	    {
process_begin_range:
	      if (!collect_MCCS)
		{
		  if (range == 0)
		    {
		      status = tre_new_item(ctx->mem,
					    TRE_BRACKET_MATCH_TYPE_CHAR,
					    min, &max_i, items);
		      if (status != REG_OK)
			goto error;
		    }
		  min = c;
		}
	      range = 0;
	    }
	  other++;
	  break;
	}
    }
  status = REG_EBRACK;
error:
  DPRINT(("tre_parse_bracket:	error: '%.*" STRF "', status=%d\n",
	  REST(re), status));
  if (col_syms)
    xfree(col_syms);
  return status;
}

#ifdef TRE_DEBUG
static const char *bracket_match_type_str[] = {
  "unused",
  "char",
  "range begin",
  "range end",
  "class",
  "equivalence value",
};
#endif /* TRE_DEBUG */

static reg_errcode_t
tre_parse_bracket(tre_parse_ctx_t *ctx, tre_ast_node_t **result)
{
  tre_ast_node_t *node = NULL;
  reg_errcode_t status = REG_OK;
  tre_bracket_match_list_t *items;
  int max_i = 32;
  tre_collating_symbol *col_syms = NULL;

  /* Handle special cases [[:<:]] and [[:>:]] */
  if (ctx->re_end - ctx->re >= 6 && ctx->re[0] == CHAR_LBRACKET
      && ctx->re[1] == CHAR_COLON && (ctx->re[2] == L'<' || ctx->re[2] == L'>')
      && ctx->re[3] == CHAR_COLON && ctx->re[4] == CHAR_RBRACKET
      && ctx->re[5] == CHAR_RBRACKET)
    {
      *result = tre_ast_new_literal(ctx->mem, ASSERTION,
		      (ctx->re[2] == L'<') ? ASSERT_AT_BOW : ASSERT_AT_EOW,
		      -1);
      DPRINT(("tre_parse_bracket: special case %s\n", (ctx->re[2] == L'<') ?
	      "[[:<:]]" : "[[:>:]]"));
      ctx->re += 6;
      return *result ? REG_OK : REG_ESPACE;
    }

  /* Start off with an array of `max_i' elements. */
  items = xcalloc(1, SIZEOF_BRACKET_MATCH_LIST_N(max_i));
  if (items == NULL)
    return REG_ESPACE;

  if (*ctx->re == CHAR_CARET)
    {
      DPRINT(("tre_parse_bracket: negate: '%.*" STRF "'\n", REST(ctx->re)));
      items->flags |= TRE_BRACKET_MATCH_FLAG_NEGATE;
      ctx->re++;
    }

  status = tre_parse_bracket_items(ctx, &items, &max_i, &col_syms);

  if (status != REG_OK)
    goto parse_bracket_done;

  /* If there are collating symbols, split off the multi-character ones
   * into a union of the bracket expression (without the collating symbols)
   * and the multiple-character sequences.  We create an equivalent input
   * string and run tre_parse() recursively */
  if (col_syms)
    {
      tre_char_t *str, *sp;
      tre_collating_symbol *cp;
      tre_parse_ctx_t subctx;

      /* Allocate a new string.  We start with the size of the original
       * bracket expression (minus 1) and add 2 (for a leading "[" and
       * a trailing nil; don't need a "^", since it is illegal to have
       * inverted MCCSs).  Since a multi-character collating symbols
       * will be converted from "[.xx.]" to "|xx" (n+4 to n+1), we don't
       * need to worry about the new string getting too long. */
      xfree(items);
      str = xmalloc(sizeof(*str) * ((col_syms->start - ctx->re) + 2));
      if (str == NULL)
	{
	  xfree(col_syms);
	  return REG_ESPACE;
	}
      sp = str;
      if (col_syms->len > 0)
	{
	  /* There are other items in the bracket expression besides the
	   * multi-character collating symbols, so create a new bracket
	   * expression with only those other itmes. */
	  const tre_char_t *re;
	  ptrdiff_t i;

	  *sp++ = '[';
	  re = ctx->re;
	  for (cp = col_syms + 1; cp->start; cp++)
	    {
	      /* The "- 2" is to account for the "[." */
	      if ((i = ((cp->start - re) - 2)) > 0)
		{
		  memcpy(sp, re, sizeof(*sp) * i);
		  sp += i;
		}
	      /* The "+ 2" is to account for the ".]" */
	      re = cp->start + cp->len + 2;
	    }
	    i = col_syms->start - re; /* Includes the trailing right bracket */
	    memcpy(sp, re, sizeof(*sp) * i);
	    sp += i;
	    *sp++ = '|';
	}
      for (cp = col_syms + 1; cp->start; cp++)
	{
	  memcpy(sp, cp->start, sizeof(*sp) * cp->len);
	  sp += cp->len;
	  if (cp[1].start)
	    *sp++ = '|';
	}
      *sp = 0;
      DPRINT(("tre_parse_bracket: Reparsing bracket expression with '%ls'\n",
	      str));

      memcpy(&subctx, ctx, sizeof(subctx));
      subctx.re = str;
      subctx.len = sp - str;
      subctx.nofirstsub = 1;
      subctx.cflags |= REG_EXTENDED; /* Force extended mode for parsing */
      status = tre_parse(&subctx);
      xfree(str);
      if (status != REG_OK)
	{
	  xfree(col_syms);
	  return status;
	}
      ctx->re = col_syms->start;
      ctx->position = subctx.position;
      xfree(col_syms);
      *result = subctx.result;
      DPRINT(("tre_parse_bracket: Returning to original string\n"));
      return REG_OK;
    }

  DPRINT(("tre_parse_bracket: creating bracket expression literal\n"));
  node = tre_ast_new_literal(ctx->mem, 0, TRE_CHAR_MAX, ctx->position);
  if (node == NULL)
    {
      status = REG_ESPACE;
      goto parse_bracket_done;
    }
  else
    {
      tre_literal_t *l = node->obj;
      l->u.bracket_match_list = tre_mem_alloc(ctx->mem,
					 SIZEOF_BRACKET_MATCH_LIST(items));
      if (l->u.bracket_match_list == NULL)
	{
	  status = REG_ESPACE;
	  goto parse_bracket_done;
	}
      memcpy(l->u.bracket_match_list, items, SIZEOF_BRACKET_MATCH_LIST(items));
    }

#ifdef TRE_DEBUG
  {
    int i;
    tre_bracket_match_t *b;
    DPRINT(("tre_parse_bracket: %d bracket match items, flags 0x%x\n",
	    items->num_bracket_matches, items->flags));
    for (i = 0, b = items->bracket_matches;
	 i < items->num_bracket_matches; i++, b++)
      {
	DPRINT(("   %d: %s %d\n", i, bracket_match_type_str[b->type],
		b->value));
      }
  }
#endif /* TRE_DEBUG */

 parse_bracket_done:
  xfree(items);
  ctx->position++;
  *result = node;
  return status;
}


/* Parses a positive decimal integer.  Returns -1 if the string does not
   contain a valid number. */
static int
tre_parse_int(const tre_char_t **regex, const tre_char_t *regex_end)
{
  int num = -1;
  const tre_char_t *r = *regex;
  while (r < regex_end && *r >= L'0' && *r <= L'9')
    {
      if (num < 0)
	num = 0;
      num = num * 10 + *r - L'0';
      r++;
    }
  *regex = r;
  return num;
}


static reg_errcode_t
tre_parse_bound(tre_parse_ctx_t *ctx, tre_ast_node_t **result)
{
  int min, max;
#ifdef TRE_APPROX
  int i;
  int cost_ins, cost_del, cost_subst, cost_max;
  int limit_ins, limit_del, limit_subst, limit_err;
  const tre_char_t *start;
#endif /* TRE_APPROX */
  const tre_char_t *r = ctx->re;
  int minimal = (ctx->cflags & REG_UNGREEDY) ? 1 : 0;
#ifdef TRE_APPROX
  int approx = 0;
  int costs_set = 0;
  int counts_set = 0;

  cost_ins = cost_del = cost_subst = cost_max = TRE_PARAM_UNSET;
  limit_ins = limit_del = limit_subst = limit_err = TRE_PARAM_UNSET;
#endif /* TRE_APPROX */

  /* Parse number (minimum repetition count). */
  min = -1;
  if (r >= ctx->re_end)
#ifdef ERE_LITERAL_LBRACE_ON_NON_NUMERIC_BOUND
    return (ctx->cflags & REG_EXTENDED) ? REG_NOMATCH : REG_EBRACE;
#else /* !ERE_LITERAL_LBRACE_ON_NON_NUMERIC_BOUND */
    return REG_EBRACE;
#endif /* !ERE_LITERAL_LBRACE_ON_NON_NUMERIC_BOUND */
  if (*r >= L'0' && *r <= L'9') {
    DPRINT(("tre_parse:	  min count: '%.*" STRF "'\n", REST(r)));
    min = tre_parse_int(&r, ctx->re_end);
  }
#ifndef TRE_APPROX
  else
#ifdef ERE_LITERAL_LBRACE_ON_NON_NUMERIC_BOUND
      /* For ERE, return REG_NOMATCH to signal that the lbrace should
         be treated as a literal */
      return (ctx->cflags & REG_EXTENDED) ? REG_NOMATCH : REG_BADBR;
#else /* !ERE_LITERAL_LBRACE_ON_NON_NUMERIC_BOUND */
      return REG_BADBR;
#endif /* !ERE_LITERAL_LBRACE_ON_NON_NUMERIC_BOUND */
#endif /* !TRE_APPROX */

  /* Parse comma and second number (maximum repetition count). */
  max = min;
  if (r < ctx->re_end && *r == CHAR_COMMA)
    {
      r++;
      DPRINT(("tre_parse:   max count: '%.*" STRF "'\n", REST(r)));
      max = tre_parse_int(&r, ctx->re_end);
    }

  /* Check that the repeat counts are sane. */
  if ((max >= 0 && min > max) || min > RE_DUP_MAX || max > RE_DUP_MAX)
    return REG_BADBR;


#ifdef TRE_APPROX
  /*
   '{'
     optionally followed immediately by a number == minimum repcount
     optionally followed by , then a number == maximum repcount
      + then a number == maximum insertion count
      - then a number == maximum deletion count
      # then a number == maximum substitution count
      ~ then a number == maximum number of errors
      Any of +, -, # or ~ without followed by a number means that
      the maximum count/number of errors is infinite.

      An equation of the form
	Xi + Yd + Zs < C
      can be specified to set costs and the cost limit to a value
      different from the default value:
	- X is the cost of an insertion
	- Y is the cost of a deletion
	- Z is the cost of a substitution
	- C is the maximum cost

      If no count limit or cost is set for an operation, the operation
      is not allowed at all.
  */


  do {
    int done;
    start = r;

    /* Parse count limit settings */
    done = 0;
    if (!counts_set)
      while (r + 1 < ctx->re_end && !done)
	{
	  switch (*r)
	    {
	    case CHAR_PLUS:  /* Insert limit */
	      DPRINT(("tre_parse:   ins limit: '%.*" STRF "'\n", REST(r)));
	      r++;
	      limit_ins = tre_parse_int(&r, ctx->re_end);
	      if (limit_ins < 0)
		limit_ins = INT_MAX;
	      counts_set = 1;
	      break;
	    case CHAR_MINUS: /* Delete limit */
	      DPRINT(("tre_parse:   del limit: '%.*" STRF "'\n", REST(r)));
	      r++;
	      limit_del = tre_parse_int(&r, ctx->re_end);
	      if (limit_del < 0)
		limit_del = INT_MAX;
	      counts_set = 1;
	      break;
	    case CHAR_HASH:  /* Substitute limit */
	      DPRINT(("tre_parse: subst limit: '%.*" STRF "'\n", REST(r)));
	      r++;
	      limit_subst = tre_parse_int(&r, ctx->re_end);
	      if (limit_subst < 0)
		limit_subst = INT_MAX;
	      counts_set = 1;
	      break;
	    case CHAR_TILDE: /* Maximum number of changes */
	      DPRINT(("tre_parse: count limit: '%.*" STRF "'\n", REST(r)));
	      r++;
	      limit_err = tre_parse_int(&r, ctx->re_end);
	      if (limit_err < 0)
		limit_err = INT_MAX;
	      approx = 1;
	      break;
	    case CHAR_COMMA:
	      r++;
	      break;
	    case L' ':
	      r++;
	      break;
	    case L'}':
	      done = 1;
	      break;
	    default:
	      done = 1;
	      break;
	    }
	}

    /* Parse cost restriction equation. */
    done = 0;
    if (!costs_set)
      while (r + 1 < ctx->re_end && !done)
	{
	  switch (*r)
	    {
	    case CHAR_PLUS:
	    case L' ':
	      r++;
	      break;
	    case L'<':
	      DPRINT(("tre_parse:    max cost: '%.*" STRF "'\n", REST(r)));
	      r++;
	      while (*r == L' ')
		r++;
	      cost_max = tre_parse_int(&r, ctx->re_end);
	      if (cost_max < 0)
		cost_max = INT_MAX;
	      else
		cost_max--;
	      approx = 1;
	      break;
	    case CHAR_COMMA:
	      r++;
	      done = 1;
	      break;
	    default:
	      if (*r >= L'0' && *r <= L'9')
		{
#ifdef TRE_DEBUG
		  const tre_char_t *sr = r;
#endif /* TRE_DEBUG */
		  int cost = tre_parse_int(&r, ctx->re_end);
		  /* XXX - make sure r is not past end. */
		  switch (*r)
		    {
		    case L'i':	/* Insert cost */
		      DPRINT(("tre_parse:    ins cost: '%.*" STRF "'\n",
			      REST(sr)));
		      r++;
		      cost_ins = cost;
		      costs_set = 1;
		      break;
		    case L'd':	/* Delete cost */
		      DPRINT(("tre_parse:    del cost: '%.*" STRF "'\n",
			      REST(sr)));
		      r++;
		      cost_del = cost;
		      costs_set = 1;
		      break;
		    case L's':	/* Substitute cost */
		      DPRINT(("tre_parse:  subst cost: '%.*" STRF "'\n",
			      REST(sr)));
		      r++;
		      cost_subst = cost;
		      costs_set = 1;
		      break;
		    default:
		      return REG_BADBR;
		    }
		}
	      else
		{
		  done = 1;
		  break;
		}
	    }
	}
  } while (start != r);
#endif /* TRE_APPROX */

  /*{*//* Missing }. */
  if (r >= ctx->re_end)
    return REG_EBRACE;

  /* Empty contents of {}. */
  if (r == ctx->re)
    return REG_BADBR;

  /* Parse the ending '}' or '\}'.*/
  if (ctx->cflags & REG_EXTENDED)
    {
      if (r >= ctx->re_end || *r != CHAR_RBRACE)
	return REG_BADBR;
      r++;
      /* Parse trailing '?' marking minimal repetition. */
      if (r < ctx->re_end)
	{
	  if (*r == CHAR_QUESTIONMARK)
	    {
	      /* Process the question mark only in enhanced mode.
		 Otherwise, the question mark is an error in ERE
		 or a literal in BRE */
	      if (ctx->cflags & REG_ENHANCED)
		{
		  minimal = !(ctx->cflags & REG_UNGREEDY);
		  r++;
		}
	      else return REG_BADRPT;
	    }
	  else if (*r == CHAR_STAR || *r == CHAR_PLUS)
	    {
	      /* These are reserved for future extensions. */
	      return REG_BADRPT;
	    }
	}
    }
  else
    {
      if (r + 1 >= ctx->re_end
	  || *r != CHAR_BACKSLASH
	  || *(r + 1) != CHAR_RBRACE)
	return REG_BADBR;
      r += 2;
      if (r < ctx->re_end && *r == CHAR_STAR)
	{
	  /* This is reserved for future extensions. */
	  return REG_BADRPT;
	}
    }

  if (minimal)
    ctx->num_reorder_tags++;

  if (!result) goto parse_bound_exit;
  /* Create the AST node(s). */
  /* Originally, if min == 0 && max == 0, we immediately replace the whole
     iteration with EMPTY.  This unfortunately drops any submatches, and
     messes up setting the pmatch values (we can get tags of -1, and
     tag values in the billions).  So we leave it and process this case as
     usual, and wait until tre_expand_ast() to replace with EMPTY */
#ifdef TRE_APPROX
  if (min < 0 && max < 0)
    /* Only approximate parameters set, no repetitions. */
    min = max = 1;
#endif /* TRE_APPROX */

  *result = tre_ast_new_iter(ctx->mem, *result, min, max, minimal);
  if (!*result)
    return REG_ESPACE;

#ifdef TRE_APPROX
  /* If approximate matching parameters are set, add them to the
     iteration node. */
  if (approx || costs_set || counts_set)
    {
      int *params;
      tre_iteration_t *iter = (*result)->obj;

      if (costs_set || counts_set)
	{
	  if (limit_ins == TRE_PARAM_UNSET)
	    {
	      if (cost_ins == TRE_PARAM_UNSET)
		limit_ins = 0;
	      else
		limit_ins = INT_MAX;
	    }

	  if (limit_del == TRE_PARAM_UNSET)
	    {
	      if (cost_del == TRE_PARAM_UNSET)
		limit_del = 0;
	      else
		limit_del = INT_MAX;
	    }

	  if (limit_subst == TRE_PARAM_UNSET)
	    {
	      if (cost_subst == TRE_PARAM_UNSET)
		limit_subst = 0;
	      else
		limit_subst = INT_MAX;
	    }
	}

      if (cost_max == TRE_PARAM_UNSET)
	cost_max = INT_MAX;
      if (limit_err == TRE_PARAM_UNSET)
	limit_err = INT_MAX;

      ctx->have_approx = 1;
      params = tre_mem_alloc(ctx->mem, sizeof(*params) * TRE_PARAM_LAST);
      if (!params)
	return REG_ESPACE;
      for (i = 0; i < TRE_PARAM_LAST; i++)
	params[i] = TRE_PARAM_UNSET;
      params[TRE_PARAM_COST_INS] = cost_ins;
      params[TRE_PARAM_COST_DEL] = cost_del;
      params[TRE_PARAM_COST_SUBST] = cost_subst;
      params[TRE_PARAM_COST_MAX] = cost_max;
      params[TRE_PARAM_MAX_INS] = limit_ins;
      params[TRE_PARAM_MAX_DEL] = limit_del;
      params[TRE_PARAM_MAX_SUBST] = limit_subst;
      params[TRE_PARAM_MAX_ERR] = limit_err;
      iter->params = params;
    }
#endif /* TRE_APPROX */

parse_bound_exit:
#ifdef TRE_APPROX
  DPRINT(("tre_parse_bound: min %d, max %d, costs [%d,%d,%d, total %d], "
	  "limits [%d,%d,%d, total %d]\n",
	  min, max, cost_ins, cost_del, cost_subst, cost_max,
	  limit_ins, limit_del, limit_subst, limit_err));
#else /* !TRE_APPROX */
  DPRINT(("tre_parse_bound: min %d, max %d\n", min, max));
#endif /* !TRE_APPROX */


  ctx->re = r;
  return REG_OK;
}

/* Previously, we had PARSE_RESTORE_CFLAGS restore the cflags, but for
   non-self-contained options, like (?i), this causes ((?i)fu)bar to be
   treated more like ((?i)fu(?-i)bar), so the pmatch value is incorrect.
   Because we now set up tags for even non-capturing parenthesized
   subexpressions, we always call PARSE_MARK_FOR_SUBMATCH.  So if we
   pass the unmodified version of cflags to PARSE_MARK_FOR_SUBMATCH and
   have it restore cflags after the subexpression, we don't need to have
   a separate PARSE_RESTORE_CFLAGS, and then after processing the
   non-self-contained option, we can call PARSE_ATOM instead of PARSE_RE.
   This has the side-benefit of now matching the perl behavior: the RE
   foo(?i)bar|zap is foo(?i)bar OR (?i)zap instead of TRE previous behavior
   of foo AND (?i) (bar OR zap). */
typedef enum {
  PARSE_RE = 0,
  PARSE_ATOM,
  PARSE_MARK_FOR_SUBMATCH,
  PARSE_BRANCH,
  PARSE_PIECE,
  PARSE_CATENATION,
  PARSE_POST_CATENATION,
  PARSE_UNION,
  PARSE_POST_UNION,
  PARSE_POSTFIX,
} tre_parse_re_stack_symbol_t;


reg_errcode_t
tre_parse(tre_parse_ctx_t *ctx)
{
  tre_ast_node_t *result = NULL;
  tre_parse_re_stack_symbol_t symbol;
  reg_errcode_t status = REG_OK;
  tre_stack_t *stack = ctx->stack;
  int bottom = tre_stack_num_objects(stack);
  int depth = 0;
  int temporary_cflags = 0;
  int bre_branch_begin;
#ifdef TRE_DEBUG
  const tre_char_t *tmp_re;
#endif

  DPRINT(("tre_parse: parsing '%.*" STRF "', len = %d cflags = 0%o\n",
	  ctx->len, ctx->re, ctx->len, ctx->cflags));

  if (ctx->len <= 0) return REG_EMPTY;
  if (!ctx->nofirstsub)
    {
      STACK_PUSH(stack, int, ctx->cflags);
      STACK_PUSH(stack, int, ctx->submatch_id);
      STACK_PUSH(stack, int, PARSE_MARK_FOR_SUBMATCH);
      ctx->submatch_id++;
    }
  STACK_PUSH(stack, int, 0); // bre_branch_begin
  STACK_PUSH(stack, int, PARSE_RE);
  ctx->re_start = ctx->re;
  ctx->re_end = ctx->re + ctx->len;


  /* The following is basically just a recursive descent parser.  I use
     an explicit stack instead of recursive functions mostly because of
     two reasons: compatibility with systems which have an overflowable
     call stack, and efficiency (both in lines of code and speed).  */
  while (tre_stack_num_objects(stack) > bottom)
    {
      symbol = tre_stack_pop_int(stack);
      switch (symbol)
	{
	case PARSE_RE:
	  /* Parse a full regexp.  A regexp is one or more branches,
	     separated by the union operator `|'. */
	  bre_branch_begin = tre_stack_pop_int(stack);
	  if (
#ifdef REG_LITERAL
	      !(ctx->cflags & REG_LITERAL) &&
#endif /* REG_LITERAL */
	      ctx->cflags & (REG_EXTENDED | REG_ENHANCED))
	    STACK_PUSHX(stack, int, PARSE_UNION);
	  STACK_PUSHX(stack, int, bre_branch_begin);
	  STACK_PUSHX(stack, int, PARSE_BRANCH);
	  break;

	case PARSE_BRANCH:
	  /* Parse a branch.  A branch is one or more pieces, concatenated.
	     A piece is an atom possibly followed by a postfix operator. */
	  bre_branch_begin = tre_stack_pop_int(stack);
	  STACK_PUSHX(stack, int, PARSE_CATENATION);
	  STACK_PUSHX(stack, int, bre_branch_begin);
	  STACK_PUSHX(stack, int, PARSE_PIECE);
	  break;

	case PARSE_PIECE:
	  /* Parse a piece.  A piece is an atom possibly followed by one
	     or more postfix operators. */
	  bre_branch_begin = tre_stack_pop_int(stack);
	  STACK_PUSHX(stack, int, PARSE_POSTFIX);
	  STACK_PUSHX(stack, int, bre_branch_begin);
	  STACK_PUSHX(stack, int, PARSE_ATOM);
	  break;

	case PARSE_CATENATION:
	  /* If the expression has not ended, parse another piece. */
	  {
	    tre_char_t c;
	    if (ctx->re >= ctx->re_end)
	      break;
	    c = *ctx->re;
#ifdef REG_LITERAL
	    if (!(ctx->cflags & REG_LITERAL))
	      {
#endif /* REG_LITERAL */
		if ((ctx->cflags & REG_EXTENDED && c == CHAR_PIPE) ||
		    ((ctx->cflags & (REG_EXTENDED | REG_ENHANCED)) == REG_ENHANCED
		    && ctx->re + 1 < ctx->re_end && c == CHAR_BACKSLASH &&
		    *(ctx->re + 1) == CHAR_PIPE))
		  break;
		if ((ctx->cflags & REG_EXTENDED
		     && c == CHAR_RPAREN && depth > 0)
		    || (!(ctx->cflags & REG_EXTENDED)
			&& ctx->re + 1 < ctx->re_end && c == CHAR_BACKSLASH
			    && *(ctx->re + 1) == CHAR_RPAREN))
		  {
		    if (!(ctx->cflags & REG_EXTENDED) && depth == 0)
		      return REG_EPAREN;
		    DPRINT(("tre_parse:	  group end: '%.*" STRF "'\n",
			    REST(ctx->re)));
		    depth--;
		    if (!(ctx->cflags & (REG_EXTENDED | REG_ENHANCED)))
		      ctx->re += 2;
		    break;
		  }
#ifdef REG_LITERAL
	      }
#endif /* REG_LITERAL */

#ifdef REG_LEFT_ASSOC
	    if (ctx->cflags & REG_LEFT_ASSOC)
	      {
		/* Left associative concatenation. */
		STACK_PUSHX(stack, int, PARSE_CATENATION);
		STACK_PUSHX(stack, voidptr, result);
		STACK_PUSHX(stack, int, PARSE_POST_CATENATION);
		STACK_PUSHX(stack, int, 0); // bre_branch_begin
		STACK_PUSHX(stack, int, PARSE_PIECE);
	      }
	    else
#endif /* REG_LEFT_ASSOC */
	      {
		/* Default case, right associative concatenation. */
		STACK_PUSHX(stack, voidptr, result);
		STACK_PUSHX(stack, int, PARSE_POST_CATENATION);
		STACK_PUSHX(stack, int, PARSE_CATENATION);
		STACK_PUSHX(stack, int, 0); // bre_branch_begin
		STACK_PUSHX(stack, int, PARSE_PIECE);
	      }
	    break;
	  }

	case PARSE_POST_CATENATION:
	  {
	    tre_ast_node_t *tree = tre_stack_pop_voidptr(stack);
	    tre_ast_node_t *tmp_node;
	    tmp_node = tre_ast_new_catenation(ctx->mem, tree, result);
	    if (!tmp_node)
	      return REG_ESPACE;
	    result = tmp_node;
	    break;
	  }

	case PARSE_UNION:
	  if (ctx->re >= ctx->re_end)
	    break;
#ifdef REG_LITERAL
	  if (ctx->cflags & REG_LITERAL)
	    break;
#endif /* REG_LITERAL */
	  if (!(ctx->cflags & REG_EXTENDED))
	    {
	      if (*ctx->re != CHAR_BACKSLASH || ctx->re + 1 >= ctx->re_end)
		break;
	      ctx->re++;
	    }
	  switch (*ctx->re)
	    {
	    case CHAR_PIPE:
	      DPRINT(("tre_parse:	union: '%.*" STRF "'\n",
		      REST(ctx->re)));
	      STACK_PUSHX(stack, int, PARSE_UNION);
	      STACK_PUSHX(stack, voidptr, (void *)ctx->re);
	      STACK_PUSHX(stack, voidptr, result);
	      STACK_PUSHX(stack, int, PARSE_POST_UNION);
	      /* We need to pass a boolean (eventually) to PARSE_ATOM to
		 indicate if this is the beginning of a BRE extended branch. */
	      STACK_PUSHX(stack, int, (ctx->cflags & (REG_EXTENDED | REG_ENHANCED)) == REG_ENHANCED); // bre_branch_begin
	      STACK_PUSHX(stack, int, PARSE_BRANCH);
	      ctx->re++;
	      break;

	    case CHAR_RPAREN:
	      ctx->re++;
	      break;

	    default:
	      if (!(ctx->cflags & REG_EXTENDED))
		ctx->re--;
	      break;
	    }
	  break;

	case PARSE_POST_UNION:
	  {
	    tre_ast_node_t *tmp_node;
	    tre_ast_node_t *tree = tre_stack_pop_voidptr(stack);
	    const tre_char_t *pipechar = tre_stack_pop_voidptr(stack);
	    /* error on empty expression at end of union */
	    if (pipechar == ctx->re - 1)
	      {
		return REG_EMPTY;
	      }
	    tmp_node = tre_ast_new_union(ctx->mem, tree, result);
	    if (!tmp_node)
	      return REG_ESPACE;
	    result = tmp_node;
	    break;
	  }

	case PARSE_POSTFIX:
	  /* Parse postfix operators. */
	  if (ctx->re >= ctx->re_end)
	    break;
#ifdef REG_LITERAL
	  if (ctx->cflags & REG_LITERAL)
	    break;
#endif /* REG_LITERAL */
	  int minimal = (ctx->cflags & REG_UNGREEDY) ? 1 : 0;
	  int rep_min = 0;
	  int rep_max = -1;
#ifdef TRE_DEBUG
	  int lbrace_off;
#endif
	  switch (*ctx->re)
	    {
	    case CHAR_PLUS:
	    case CHAR_QUESTIONMARK:
	      if (!(ctx->cflags & REG_EXTENDED))
		break;
		/*FALLTHROUGH*/
	    case CHAR_STAR:
	      {
		tre_ast_node_t *tmp_node;
#ifdef TRE_DEBUG
		const char *tstr = "star";
		tmp_re = ctx->re;
#endif

	handle_plus_or_question:
		/* error on iteration of raw assertion (not in subexpression) */
		if (result->type == LITERAL && result->submatch_id < 0 &&
		    IS_ASSERTION((tre_literal_t *)result->obj))
		  {
		    if (!(ctx->cflags & REG_EXTENDED)) break;
		    return REG_BADRPT;
		  }
		if (*ctx->re == CHAR_PLUS)
		  {
		    rep_min = 1;
#ifdef TRE_DEBUG
		    tstr = "plus";
#endif
		  }
		if (*ctx->re == CHAR_QUESTIONMARK)
		  {
		    rep_max = 1;
#ifdef TRE_DEBUG
		    tstr = "questionmark";
#endif
		  }

		if (ctx->cflags & REG_EXTENDED)
		  {
		    if (ctx->re + 1 < ctx->re_end)
		      {
			if (*(ctx->re + 1) == CHAR_QUESTIONMARK)
			  {
			    /* Process the question mark only in enhanced mode.
			       Otherwise, the question mark is an error in ERE */
			    if (ctx->cflags & REG_ENHANCED)
			      {
				minimal = !(ctx->cflags & REG_UNGREEDY);
				ctx->re++;
			      }
			    else return REG_BADRPT;
			  }
			else if (*(ctx->re + 1) == CHAR_STAR
				 || *(ctx->re + 1) == CHAR_PLUS)
			  {
			    /* These are reserved for future extensions. */
			    return REG_BADRPT;
			  }
		      }
		  }
		else
		  {
		    if (ctx->re + 1 < ctx->re_end && *(ctx->re + 1) == CHAR_STAR)
		      {
			/* This is reserved for future extensions. */
			return REG_BADRPT;
		      }
		    if (ctx->re + 2 < ctx->re_end)
		      {
			if (*(ctx->re + 1) == CHAR_BACKSLASH && *(ctx->re + 1) == CHAR_QUESTIONMARK)
			  {
			    /* Process the question mark only in enhanced mode.
			       Otherwise, the question mark is a literal in BRE */
			    if (ctx->cflags & REG_ENHANCED)
			      {
				minimal = !(ctx->cflags & REG_UNGREEDY);
				ctx->re += 2;
			      }
			  }
			else if (*(ctx->re + 1) == CHAR_BACKSLASH && *(ctx->re + 2) == CHAR_PLUS)
			  {
			    /* This is reserved for future extensions. */
			    return REG_BADRPT;
			  }
		      }
		  }

		if (minimal)
		  ctx->num_reorder_tags++;

		DPRINT(("tre_parse: %s %s: '%.*" STRF "'\n",
			minimal ? "  minimal" : "greedy", tstr, REST(tmp_re)));
		if (result == NULL)
		  {
		    if (ctx->cflags & REG_EXTENDED) return REG_BADRPT;
		    else goto parse_literal;
		  }
		ctx->re++;
		tmp_node = tre_ast_new_iter(ctx->mem, result, rep_min, rep_max,
					    minimal);
		if (tmp_node == NULL)
		  return REG_ESPACE;
		result = tmp_node;

		/* Set the iterator with a submatch id in the invisible range
		 * (which will be overridden if a real submatch is needed) */
		result->submatch_id = ctx->submatch_id_invisible++;

#if 0
		/* We don't allow multiple postfixes, but this might be needed
		   to support approximate matching */
		STACK_PUSHX(stack, int, PARSE_POSTFIX);
#endif
	      }
	      break;

	    case CHAR_BACKSLASH:
	      /* "\{" is special without REG_EXTENDED */
	      /* "\+" and "\?" are special with REG_ENHANCED for BRE */
	      if (!(ctx->cflags & REG_EXTENDED)
		  && ctx->re + 1 < ctx->re_end)
		{
		  switch (*(ctx->re + 1))
		    {
		    case CHAR_LBRACE:
		      ctx->re++;
#ifdef TRE_DEBUG
		      lbrace_off = 2;
#endif
		      goto parse_brace;
		    case CHAR_PLUS:
		    case CHAR_QUESTIONMARK:
		      if (ctx->cflags & REG_ENHANCED)
			{
#ifdef TRE_DEBUG
			  tmp_re = ctx->re;
#endif
			  ctx->re++;
			  goto handle_plus_or_question;
			}
		      break;
		    }
		  break;
		}
	      else
		break;

	    case CHAR_LBRACE:
	      {
		int raw_assertion;

		/* "{" is literal without REG_EXTENDED */
		if (!(ctx->cflags & REG_EXTENDED))
		  break;
#ifdef TRE_DEBUG
		lbrace_off = 1;
#endif

	    parse_brace:
		/* error on iteration of raw assertion (not in subexpression),
		   but wait until after parsing bounds */
		raw_assertion = (result->type == LITERAL
				 && result->submatch_id < 0
				 && IS_ASSERTION((tre_literal_t *)result->obj));
		ctx->re++;

		status = tre_parse_bound(ctx, &result);
#ifdef ERE_LITERAL_LBRACE_ON_NON_NUMERIC_BOUND
		/* For ERE, if status is REG_NOMATCH, this mean the lbrace
		   is to be treated as a literal. */
		if (status == REG_NOMATCH)
		  {
		    ctx->re--;
		    break;
		  }
#endif /* ERE_LITERAL_LBRACE_ON_NON_NUMERIC_BOUND */
		DPRINT(("tre_parse:	bound: '%.*" STRF "'\n",
			REST(ctx->re - lbrace_off)));
		if (status != REG_OK)
		  return status;
		if (raw_assertion) return REG_BADRPT;

		/* Set the iterator with a submatch id in the invisible range
		 * (which will be overridden if a real submatch is needed) */
		if (result->type == ITERATION)
		  result->submatch_id = ctx->submatch_id_invisible++;

#if 0
		/* We don't allow multiple postfixes, but this might be needed
		   to support approximate matching */
		STACK_PUSHX(stack, int, PARSE_POSTFIX);
#endif
		break;
	      }
	    }
	  break;

	case PARSE_ATOM:
	  {
	    /* Parse an atom.  An atom is a regular expression enclosed in `()',
	       an empty set of `()', a bracket expression, `.', `^', `$',
	       a `\' followed by a character, or a single character. */

	    /* The stack contains a boolean value, whether PARSE_ATOM is
	       being called just after the start of a group (left paren)
	       in a BRE */
	    bre_branch_begin = tre_stack_pop_int(stack);

	    /* End of regexp? (empty string). */
	    if (ctx->re >= ctx->re_end)
	      goto parse_literal;

#ifdef REG_LITERAL
	    if (ctx->cflags & REG_LITERAL)
	      goto parse_literal;
#endif /* REG_LITERAL */

	    switch (*ctx->re)
	      {
	      case CHAR_LPAREN:  /* parenthesized subexpression */

		/* Handle "(?...)" extensions.  They work in a way similar
		   to Perls corresponding extensions. */
		if ((ctx->cflags & (REG_EXTENDED|REG_ENHANCED)) ==
		    (REG_EXTENDED|REG_ENHANCED)
		    && *(ctx->re + 1) == CHAR_QUESTIONMARK)
		  {
		    int new_cflags = ctx->cflags;
		    int bit = 1;
		    int invisible_submatch = 0;
		    DPRINT(("tre_parse:	extension: '%.*" STRF "'\n",
			    REST(ctx->re)));
		    ctx->re += 2;
		    while (/*CONSTCOND*/1)
		      {
			if (*ctx->re == L'i')
			  {
			    DPRINT(("tre_parse:	    icase: '%.*" STRF "'\n",
				    REST(ctx->re)));
			    if (bit)
			      new_cflags |= REG_ICASE;
			    else
			      new_cflags &= ~REG_ICASE;
			    ctx->re++;
			  }
			else if (*ctx->re == L'n')
			  {
			    DPRINT(("tre_parse:	  newline: '%.*" STRF "'\n",
				    REST(ctx->re)));
			    if (bit)
			      new_cflags |= REG_NEWLINE;
			    else
			      new_cflags &= ~REG_NEWLINE;
			    ctx->re++;
			  }
#ifdef REG_LEFT_ASSOC
			else if (*ctx->re == L'l')
			  {
			    DPRINT(("tre_parse: left assoc: '%.*" STRF "'\n",
				    REST(ctx->re)));
			    if (bit)
			      new_cflags |= REG_LEFT_ASSOC;
			    else
			      new_cflags &= ~REG_LEFT_ASSOC;
			    ctx->re++;
			  }
#endif /* REG_LEFT_ASSOC */
#ifdef REG_UNGREEDY
			else if (*ctx->re == L'U')
			  {
			    DPRINT(("tre_parse:    ungreedy: '%.*" STRF "'\n",
				    REST(ctx->re)));
			    if (bit)
			      new_cflags |= REG_UNGREEDY;
			    else
			      new_cflags &= ~REG_UNGREEDY;
			    ctx->re++;
			  }
#endif /* REG_UNGREEDY */
			else if (*ctx->re == CHAR_MINUS)
			  {
			    DPRINT(("tre_parse:	 turn off: '%.*" STRF "'\n",
				    REST(ctx->re)));
			    ctx->re++;
			    bit = 0;
			  }
			else if (*ctx->re == CHAR_COLON)
			  {
			    DPRINT(("tre_parse:	 no group: '%.*" STRF
				    "', (invisible submatch %d)\n",
				    REST(ctx->re), ctx->submatch_id_invisible));
			    ctx->re++;
			    depth++;
			    invisible_submatch = 1;
			    break;
			  }
			else if (*ctx->re == CHAR_HASH)
			  {
			    DPRINT(("tre_parse:    comment: '%.*" STRF "'\n",
				    REST(ctx->re)));
			    /* A comment can contain any character except a
			       right parenthesis */
			    while (*ctx->re != CHAR_RPAREN
				   && ctx->re < ctx->re_end)
			      ctx->re++;
			    if (*ctx->re == CHAR_RPAREN && ctx->re < ctx->re_end)
			      {
				ctx->re++;
				break;
			      }
			    else
			      return REG_BADPAT;
			  }
			else if (*ctx->re == CHAR_RPAREN)
			  {
			    ctx->re++;
			    break;
			  }
			else
			  return REG_BADRPT;
		      }

		    /* Turn on the cflags changes for the rest of the
		       enclosing group. */
		    if (invisible_submatch)
		      {
			STACK_PUSHX(stack, int, ctx->cflags);
			STACK_PUSHX(stack, int, ctx->submatch_id_invisible);
			STACK_PUSHX(stack, int, PARSE_MARK_FOR_SUBMATCH);
			ctx->submatch_id_invisible++;
			STACK_PUSHX(stack, int, 0); // bre_branch_begin
			STACK_PUSHX(stack, int, PARSE_RE);
		      }
		    else {
			STACK_PUSHX(stack, int, 0); // bre_branch_begin
			STACK_PUSHX(stack, int, PARSE_ATOM);
		    }
		    ctx->cflags = new_cflags;
		    break;
		  }

		if (ctx->cflags & REG_EXTENDED)
		  {
		parse_bre_lparen:
		    DPRINT(("tre_parse: group begin: '%.*" STRF
			    "', submatch %d\n", REST(ctx->re),
			    ctx->submatch_id));
		    ctx->re++;
		    /* First parse a whole RE, then mark the resulting tree
		       for submatching. */
		    STACK_PUSHX(stack, int, ctx->cflags);
		    STACK_PUSHX(stack, int, ctx->submatch_id);
		    STACK_PUSHX(stack, int, PARSE_MARK_FOR_SUBMATCH);
		    /* We need to pass a boolean (eventually) to PARSE_ATOM to
		       indicate if this is the beginning of a BRE group. */
		    STACK_PUSHX(stack, int, !(ctx->cflags & REG_EXTENDED));
		    STACK_PUSHX(stack, int, PARSE_RE);
		    ctx->submatch_id++;
		    depth++;
		  }
		else
		  goto parse_literal;
		break;

	      case CHAR_RPAREN:  /* end of current subexpression */
		if (ctx->cflags & REG_EXTENDED && depth > 0)
		  {
	      parse_bre_rparen_empty:
		    if (!(ctx->cflags & REG_EXTENDED) && depth == 0)
		      return REG_EPAREN;
		    DPRINT(("tre_parse:	    empty: '%.*" STRF "'\n",
			    REST(ctx->re)));
		    /* We were expecting an atom, but instead the current
		       subexpression was closed.  POSIX leaves the meaning of
		       this to be implementation-defined.  We interpret this as
		       an empty expression (which matches an empty string).  */
		    result = tre_ast_new_literal(ctx->mem, EMPTY, -1, -1);
		    if (result == NULL)
		      return REG_ESPACE;
		    if (!(ctx->cflags & REG_EXTENDED))
		      ctx->re--;
		  }
		else
		  goto parse_literal;
		break;

	      case CHAR_LBRACKET: /* bracket expression */
		DPRINT(("tre_parse:     bracket: '%.*" STRF "'\n",
			REST(ctx->re)));
		ctx->re++;
		status = tre_parse_bracket(ctx, &result);
		if (status != REG_OK)
		  return status;
		break;

	      case CHAR_BACKSLASH:
		/* Deal with "\(", "\)" or "\{" for BREs */
		if (!(ctx->cflags & REG_EXTENDED)
		    && ctx->re + 1 < ctx->re_end)
		  {
		    if (*(ctx->re + 1) == CHAR_LPAREN)
		      {
			ctx->re++;
			goto parse_bre_lparen;
		      }
		    else if (*(ctx->re + 1) == CHAR_RPAREN)
		      {
			ctx->re++;
			goto parse_bre_rparen_empty;
		      }
		    if (*(ctx->re + 1) == CHAR_LBRACE) goto parse_literal;
		  }

		if (ctx->re + 1 >= ctx->re_end)
		  /* Trailing backslash. */
		  return REG_EESCAPE;

		if (!(ctx->cflags & REG_ENHANCED))
		  {
		    DPRINT(("tre_parse:  unenhanced bleep: '%.*" STRF "'\n", REST(ctx->re)));
		    ctx->re++;
		    goto unenhanced_backslash;
		  }

		/* If a macro is used, parse the expanded macro recursively. */
		{
		  tre_char_t buf[64];
		  tre_expand_macro(ctx->re + 1, ctx->re_end,
				   buf, elementsof(buf));
		  if (buf[0] != 0)
		    {
		      tre_parse_ctx_t subctx;
		      memcpy(&subctx, ctx, sizeof(subctx));
		      subctx.re = buf;
		      subctx.len = tre_strlen(buf);
		      subctx.nofirstsub = 1;
		      status = tre_parse(&subctx);
		      if (status != REG_OK)
			return status;
		      ctx->re += 2;
		      ctx->position = subctx.position;
		      result = subctx.result;
		      break;
		    }
		}

#ifdef REG_LITERAL
		if (*(ctx->re + 1) == L'Q')
		  {
		    DPRINT(("tre_parse: tmp literal: '%.*" STRF "'\n",
			    REST(ctx->re)));
		    ctx->cflags |= REG_LITERAL;
		    temporary_cflags |= REG_LITERAL;
		    ctx->re += 2;
		    STACK_PUSHX(stack, int, 0);
		    STACK_PUSHX(stack, int, PARSE_ATOM);
		    break;
		  }
#endif /* REG_LITERAL */

		DPRINT(("tre_parse:  bleep: '%.*" STRF "'\n", REST(ctx->re)));
		ctx->re++;
		switch (*ctx->re)
		  {
		  case L'b':
		    result = tre_ast_new_literal(ctx->mem, ASSERTION,
						 ASSERT_AT_WB, -1);
		    ctx->re++;
		    break;
		  case L'B':
		    result = tre_ast_new_literal(ctx->mem, ASSERTION,
						 ASSERT_AT_WB_NEG, -1);
		    ctx->re++;
		    break;
		  case L'<':
		    result = tre_ast_new_literal(ctx->mem, ASSERTION,
						 ASSERT_AT_BOW, -1);
		    ctx->re++;
		    break;
		  case L'>':
		    result = tre_ast_new_literal(ctx->mem, ASSERTION,
						 ASSERT_AT_EOW, -1);
		    ctx->re++;
		    break;
		  case L'x':
		    ctx->re++;
		    if (ctx->re[0] != CHAR_LBRACE && ctx->re < ctx->re_end)
		      {
			/* 8 bit hex char. */
			char tmp[3] = {0, 0, 0};
			long val;
			DPRINT(("tre_parse:  8 bit hex: '%.*" STRF "'\n",
				REST(ctx->re - 2)));

			if (tre_isxdigit_l(ctx->re[0], ctx->loc) &&
			    ctx->re < ctx->re_end)
			  {
			    tmp[0] = (char)ctx->re[0];
			    ctx->re++;
			  }
			if (tre_isxdigit_l(ctx->re[0], ctx->loc) &&
			    ctx->re < ctx->re_end)
			  {
			    tmp[1] = (char)ctx->re[0];
			    ctx->re++;
			  }
			val = strtol(tmp, NULL, 16);
			result = tre_ast_new_literal(ctx->mem, (int)val,
						     (int)val, ctx->position);
			ctx->position++;
			break;
		      }
		    else if (ctx->re < ctx->re_end)
		      {
			/* Wide char. */
			char tmp[32];
			long val;
			int i = 0;
			ctx->re++;
			while (ctx->re_end - ctx->re >= 0)
			  {
			    if (i == sizeof(tmp))
				return REG_EBRACE;
			    if (ctx->re[0] == CHAR_RBRACE)
			      break;
			    if (tre_isxdigit_l(ctx->re[0], ctx->loc))
			      {
				tmp[i] = (char)ctx->re[0];
				i++;
				ctx->re++;
				continue;
			      }
			    return REG_EBRACE;
			  }
			ctx->re++;
			tmp[i] = 0;
			val = strtol(tmp, NULL, 16);
			result = tre_ast_new_literal(ctx->mem, (int)val, (int)val,
						     ctx->position);
			ctx->position++;
			break;
		      }
		    /*FALLTHROUGH*/

		  default:
		  unenhanced_backslash:
		    if ((ctx->cflags & (REG_EXTENDED | REG_ENHANCED)) !=
			REG_EXTENDED &&
			tre_isdigit_l(*ctx->re, ctx->loc) && *ctx->re != L'0')
		      {
			/* Back reference (only in BRE or enhanced). */
			int val = *ctx->re - L'0';
			DPRINT(("tre_parse:     backref: '%.*" STRF "'\n",
				REST(ctx->re - 1)));
			result = tre_ast_new_literal(ctx->mem, BACKREF, val,
						     ctx->position);
			if (result == NULL)
			  return REG_ESPACE;

			/* Set the backref with a submatch id in the invisible
			 * range (which will be overridden if a real submatch
			 * is needed) */
			result->submatch_id = ctx->submatch_id_invisible++;

			ctx->position++;
			ctx->num_reorder_tags++;
			ctx->max_backref = MAX(val, ctx->max_backref);
			ctx->re++;
		      }
		    else
		      {
			/* Escaped character. */
			DPRINT(("tre_parse:     escaped: '%.*" STRF "'\n",
				REST(ctx->re - 1)));
			result = tre_ast_new_literal(ctx->mem, *ctx->re, *ctx->re,
						     ctx->position);
			ctx->position++;
			ctx->re++;
		      }
		    break;
		  }
		if (result == NULL)
		  return REG_ESPACE;
		break;

	      case CHAR_PERIOD:	 /* the any-symbol */
		DPRINT(("tre_parse:	  any: '%.*" STRF "'\n",
			REST(ctx->re)));
		if (ctx->cflags & REG_NEWLINE)
		  {
		    tre_ast_node_t *tmp1;
		    tre_ast_node_t *tmp2;
		    tmp1 = tre_ast_new_literal(ctx->mem, 0, L'\n' - 1,
					       ctx->position);
		    if (!tmp1)
		      return REG_ESPACE;
		    tmp2 = tre_ast_new_literal(ctx->mem, L'\n' + 1, TRE_CHAR_MAX,
					       ctx->position + 1);
		    if (!tmp2)
		      return REG_ESPACE;
		    result = tre_ast_new_union(ctx->mem, tmp1, tmp2);
		    if (!result)
		      return REG_ESPACE;
		    ctx->position += 2;
		  }
		else
		  {
		    result = tre_ast_new_literal(ctx->mem, 0, TRE_CHAR_MAX,
						 ctx->position);
		    if (!result)
		      return REG_ESPACE;
		    ctx->position++;
		  }
		ctx->re++;
		break;

	      case CHAR_CARET:	 /* beginning of line assertion */
		/* '^' has a special meaning everywhere in EREs, at the
		   beginning of the RE and after \( is BREs.  It is also
		   special in enhanced BREs at the beginning of each branches
		   of a union */
		if (ctx->cflags & REG_EXTENDED
		    || bre_branch_begin
		    || ctx->re == ctx->re_start)
		  {
		    DPRINT(("tre_parse:	      BOL: '%.*" STRF "'\n",
			    REST(ctx->re)));
		    result = tre_ast_new_literal(ctx->mem, ASSERTION,
						 ASSERT_AT_BOL, -1);
		    if (result == NULL)
		      return REG_ESPACE;
		    ctx->re++;
		  }
		else
		  goto parse_literal;
		break;

	      case CHAR_DOLLAR:	 /* end of line assertion. */
		/* '$' is special everywhere in EREs, and in the end of the
		   string and before \) is BREs. */
		if (ctx->cflags & REG_EXTENDED
		    || (ctx->re + 2 < ctx->re_end
			&& *(ctx->re + 1) == CHAR_BACKSLASH
			&& *(ctx->re + 2) == CHAR_RPAREN)
		    || ctx->re + 1 == ctx->re_end)
		  {
		    DPRINT(("tre_parse:	      EOL: '%.*" STRF "'\n",
			    REST(ctx->re)));
		    result = tre_ast_new_literal(ctx->mem, ASSERTION,
						 ASSERT_AT_EOL, -1);
		    if (result == NULL)
		      return REG_ESPACE;
		    ctx->re++;
		  }
		else
		  goto parse_literal;
		break;

	      default:
	      parse_literal:

		if (temporary_cflags && ctx->re + 1 < ctx->re_end
		    && *ctx->re == CHAR_BACKSLASH && *(ctx->re + 1) == L'E')
		  {
		    DPRINT(("tre_parse:	 end tmps: '%.*" STRF "'\n",
			    REST(ctx->re)));
		    ctx->cflags &= ~temporary_cflags;
		    temporary_cflags = 0;
		    ctx->re += 2;
		    if (ctx->re < ctx->re_end)
		      {
			STACK_PUSHX(stack, int, 0);
			STACK_PUSHX(stack, int, PARSE_ATOM);
		      }
		    else
		      {
			result = tre_ast_new_literal(ctx->mem, EMPTY, -1, -1);
			if (!result) return REG_ESPACE;
		      }
		    break;
		  }


		/* We are expecting an atom.  If the subexpression (or the whole
		   regexp ends here, we interpret it as an empty expression
		   (which matches an empty string), which is an error.
		   Iterations of an empty expression is also an error. */
#ifdef REG_LITERAL
		if (!(ctx->cflags & REG_LITERAL))
		  {
#endif /* REG_LITERAL */
		    /* error on end of string */
		    if (ctx->re >= ctx->re_end) return depth > 0 ? REG_EPAREN
						       : REG_EMPTY;
		    /* error on unions and iterations of empty expressions */
		    if (ctx->cflags & REG_EXTENDED)
		      {
			if (ctx->re < ctx->re_end)
			  {
			    if (*ctx->re == CHAR_PIPE) return REG_EMPTY;
			    if (*ctx->re == CHAR_LBRACE)
			      {
				ctx->re++;
		  empty_parse_bound:
				/* We need to parse the bound first and return
				   any error, before returning REG_BADRPT */
				status = tre_parse_bound(ctx, NULL);
#ifdef ERE_LITERAL_LBRACE_ON_NON_NUMERIC_BOUND
				/* For ERE, if REG_NOMATCH is returned, we
				   treat the lbrace as a literal. */
				if (status == REG_NOMATCH)
				  {
				    ctx->re--;
				    /* Drop down to literal-handling code */
				  }
				else
				  {
#endif /* ERE_LITERAL_LBRACE_ON_NON_NUMERIC_BOUND */
				    if (status != REG_OK)
				      return status;
				    return REG_BADRPT;
#ifdef ERE_LITERAL_LBRACE_ON_NON_NUMERIC_BOUND
				  }
#endif /* ERE_LITERAL_LBRACE_ON_NON_NUMERIC_BOUND */
			      }
#ifdef ERE_LITERAL_LBRACE_ON_NON_NUMERIC_BOUND
			    else
#endif /* ERE_LITERAL_LBRACE_ON_NON_NUMERIC_BOUND */
			    if (*ctx->re == CHAR_STAR
				|| *ctx->re == CHAR_PLUS
				|| *ctx->re == CHAR_QUESTIONMARK)
			      {
				return REG_BADRPT;
			      }
			  }
		      }
		    else if (ctx->re + 1 < ctx->re_end
			     && *ctx->re == CHAR_BACKSLASH
			     && *(ctx->re + 1) == CHAR_LBRACE)
		      {
			ctx->re += 2;
			goto empty_parse_bound;
		      }
#ifdef REG_LITERAL
		  }
#endif /* REG_LITERAL */

		DPRINT(("tre_parse:     literal: '%.*" STRF "'\n",
			REST(ctx->re)));
		/* Note that we can't use an tre_isalpha() test here, since there
		   may be characters which are alphabetic but neither upper or
		   lower case. */
		if (ctx->cflags & REG_ICASE
		    && (tre_isupper_l(*ctx->re, ctx->loc) ||
		    tre_islower_l(*ctx->re, ctx->loc)))
		  {
		    tre_ast_node_t *tmp1;
		    tre_ast_node_t *tmp2;

		    /* XXX - Can there be more than one opposite-case
		       counterpoints for some character in some locale?  Or
		       more than two characters which all should be regarded
		       the same character if case is ignored?  If yes, there
		       does not seem to be a portable way to detect it.  I guess
		       that at least for multi-character collating elements there
		       could be several opposite-case counterpoints, but they
		       cannot be supported portably anyway. */
		    tmp1 = tre_ast_new_literal(ctx->mem,
					       tre_toupper_l(*ctx->re, ctx->loc),
					       tre_toupper_l(*ctx->re, ctx->loc),
					       ctx->position);
		    if (!tmp1)
		      return REG_ESPACE;
		    tmp2 = tre_ast_new_literal(ctx->mem,
					       tre_tolower_l(*ctx->re, ctx->loc),
					       tre_tolower_l(*ctx->re, ctx->loc),
					       ctx->position);
		    if (!tmp2)
		      return REG_ESPACE;
		    result = tre_ast_new_union(ctx->mem, tmp1, tmp2);
		    if (!result)
		      return REG_ESPACE;
		  }
		else
		  {
		    result = tre_ast_new_literal(ctx->mem, *ctx->re, *ctx->re,
						 ctx->position);
		    if (!result)
		      return REG_ESPACE;
		  }
		ctx->position++;
		ctx->re++;
		break;
	      }
	    break;
	  }

	case PARSE_MARK_FOR_SUBMATCH:
	  {
	    int submatch_id = tre_stack_pop_int(stack);

	    ctx->cflags = tre_stack_pop_int(stack); /* restore cflags */
	    if (result->submatch_id >= 0 &&
		result->submatch_id < SUBMATCH_ID_INVISIBLE_START)
	      {
		tre_ast_node_t *n, *tmp_node;
		if (submatch_id >= SUBMATCH_ID_INVISIBLE_START)
		  break;
		n = tre_ast_new_literal(ctx->mem, EMPTY, -1, -1);
		if (n == NULL)
		  return REG_ESPACE;
		tmp_node = tre_ast_new_catenation(ctx->mem, n, result);
		if (tmp_node == NULL)
		  return REG_ESPACE;
		tmp_node->num_submatches = result->num_submatches;
		result = tmp_node;
	      }
	    result->submatch_id = submatch_id;
	    if (submatch_id < SUBMATCH_ID_INVISIBLE_START)
	      result->num_submatches++;
	    break;
	  }

	default:
	  assert(0);
	  break;
	}
    }

  /* Check for missing closing parentheses. */
  if (depth > 0)
    return REG_EPAREN;

  ctx->result = result;

  return REG_OK;
}

/* EOF */

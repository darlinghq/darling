/*
  tre_regexec.c - TRE POSIX compatible matching functions (and more).

  This software is released under a BSD-style license.
  See the file LICENSE for details and copyright.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

/* Unset TRE_USE_ALLOCA to avoid using the stack to hold all the state
   info while running */
#undef TRE_USE_ALLOCA

#ifdef TRE_USE_ALLOCA
/* AIX requires this to be the first thing in the file.	 */
#ifndef __GNUC__
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef _AIX
 #pragma alloca
#  else
#   ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#   endif
#  endif
# endif
#endif
#endif /* TRE_USE_ALLOCA */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif /* HAVE_WCHAR_H */
#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif /* HAVE_WCTYPE_H */
#ifndef TRE_WCHAR
#include <ctype.h>
#endif /* !TRE_WCHAR */
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif /* HAVE_MALLOC_H */
#include <limits.h>

#include "tre-internal.h"
#include "tre-match-utils.h"
#include "tre.h"
#include "xmalloc.h"


/* For each tre_last_matched_t in the lm array, find the last matched branch by
   comparing the touch value of the cmp_tag's.  For all other branches, reset
   the corresponding tags.  If reset_all is non-zero, reset all tags in all
   branches.  Recurse into the nested last matched structures, clearing tags as
   apprpriate. */
static void
tre_reset_last_matched_branches(tre_tag_t *tags, const tre_last_matched_t *lm,
			        int n, int start_tag, int reset_all)
{
  int max, i, reset;
  tre_last_matched_branch_t *b;

  DPRINT(("tre_reset_last_matched_branches: n=%d start_tag=%d reset_all=%d\n",
	  n, start_tag, reset_all));
  for (; n-- > 0; lm++)
    {
      if (lm->n_branches == 1)
	{
	  b = lm->branches;
	  if (start_tag > 0)
	    {
	      DPRINT(("  b->cmp_tag=%d %d <? %d\n", b->cmp_tag,
		      tre_tag_touch_get(tags, b->cmp_tag),
		      tre_tag_touch_get(tags, start_tag)));
	      reset = (reset_all || tre_tag_touch_get(tags, b->cmp_tag) <
		       tre_tag_touch_get(tags, start_tag));
	    }
	  else
	    reset = 0;

	  if (reset)
	    {
	      int *t;

	      for (i = b->n_tags, t = b->tags; i > 0; i--, t++)
		{
		  DPRINT(("  Resetting t%d\n", *t));
		  tre_tag_reset(tags, *t);
		}
	    }
	  if (b->n_last_matched > 0)
	    tre_reset_last_matched_branches(tags, b->last_matched,
						b->n_last_matched, 
						lm->start_tag, reset);
	}
      else
	{
	  if (!reset_all)
	    {
#ifdef TRE_DEBUG
	      int last;
#endif /* TRE_DEBUG */
	      max = 0;
	      for (i = lm->n_branches, b = lm->branches; i > 0; i--, b++)
		{
		  int t = b->cmp_tag;
		  int touch = tre_tag_touch_get(tags, t);
		  if (touch > max)
		    {
		      max = touch;
#ifdef TRE_DEBUG
		      last = t;
#endif /* TRE_DEBUG */
		    }
		}
	      DPRINT(("  Last touched end tag t%d=%d\n", last, max));
	    }

	  for (i = lm->n_branches, b = lm->branches; i > 0; i--, b++)
	    {
	      reset = (reset_all || tre_tag_touch_get(tags, b->cmp_tag) < max);
	      if (reset)
		{
		  int j;
		  int *t;

		  for (j = b->n_tags, t = b->tags; j > 0; j--, t++)
		    {
		      DPRINT(("  Resetting t%d\n", *t));
		      tre_tag_reset(tags, *t);
		    }
		}
	      if (b->n_last_matched > 0)
		tre_reset_last_matched_branches(tags, b->last_matched,
						b->n_last_matched,
						lm->start_tag, reset);
	    }
	}
    }
}


/* Fills the POSIX.2 regmatch_t array according to the TNFA tag and match
   endpoint values. */
reg_errcode_t
tre_fill_pmatch(size_t nmatch, regmatch_t pmatch[], int cflags,
		const tre_tnfa_t *tnfa, const tre_tag_t *intags, int match_eo)
{
  unsigned int i;

  if (cflags & REG_NOSUB) return REG_OK;

  i = 0;
  if (match_eo >= 0 && intags)
    {
      const tre_tag_t *tags = intags;
      tre_submatch_data_t *submatch_data;

      if (tnfa->last_matched_branch &&
	  tnfa->last_matched_branch->n_last_matched > 0)
	{
	  tre_tag_t *t;
#ifdef TRE_USE_ALLOCA
	  t = alloca(sizeof(*t) * tnfa->num_tags);
#else /* !TRE_USE_ALLOCA */
	  t = xmalloc(sizeof(*t) * tnfa->num_tags);
#endif /* !TRE_USE_ALLOCA */
	  if (!t) return REG_ESPACE;
	  memcpy(t, intags, tnfa->num_tags * sizeof(tre_tag_t));
	  tre_reset_last_matched_branches(t,
				    tnfa->last_matched_branch->last_matched,
				    tnfa->last_matched_branch->n_last_matched,
				    0, 0);
	  tags = t;
	}
      /* Construct submatch offsets from the tags. */
      DPRINT(("end tag = t%d = %d\n", tnfa->end_tag, match_eo));
      submatch_data = tnfa->submatch_data;
      while (i < tnfa->num_submatches && i < nmatch)
	{
	  if (submatch_data[i].so_tag == tnfa->end_tag)
	    pmatch[i].rm_so = match_eo;
	  else
	    pmatch[i].rm_so = tre_tag_get(tags, submatch_data[i].so_tag);

	  if (submatch_data[i].eo_tag == tnfa->end_tag)
	    pmatch[i].rm_eo = match_eo;
	  else
	    pmatch[i].rm_eo = tre_tag_get(tags, submatch_data[i].eo_tag);

	  /* If either of the endpoints were not used, this submatch
	     was not part of the match. */
	  if (pmatch[i].rm_so == -1 || pmatch[i].rm_eo == -1)
	    pmatch[i].rm_so = pmatch[i].rm_eo = -1;

	  DPRINT(("pmatch[%d] = {t%d = %qd, t%d = %qd}\n", i,
		  submatch_data[i].so_tag, pmatch[i].rm_so,
		  submatch_data[i].eo_tag, pmatch[i].rm_eo));
	  i++;
	}
#ifndef TRE_USE_ALLOCA
	if (tags != intags) xfree((void*)tags);
#endif /* !TRE_USE_ALLOCA */
    }

  while (i < nmatch)
    {
      pmatch[i].rm_so = -1;
      pmatch[i].rm_eo = -1;
      i++;
    }

  return REG_OK;
}


/*
  Wrapper functions for POSIX compatible regexp matching.
*/

#ifndef __LIBC__
int
tre_have_backrefs(const regex_t *preg)
{
  tre_tnfa_t *tnfa = (void *)preg->TRE_REGEX_T_FIELD;
  return tnfa->have_backrefs;
}

#ifdef TRE_APPROX
int
tre_have_approx(const regex_t *preg)
{
  tre_tnfa_t *tnfa = (void *)preg->TRE_REGEX_T_FIELD;
  return tnfa->have_approx;
}
#endif /* TRE_APPROX */
#endif /* !__LIBC__ */

static int
tre_match(const tre_tnfa_t *tnfa, const void *string, size_t len,
	  tre_str_type_t type, size_t nmatch, regmatch_t pmatch[],
	  int eflags)
{
  reg_errcode_t status;
  tre_tag_t *tags = NULL;
  int eo;
  size_t offset = 0, count = 0;

  if (
#ifdef TRE_STR_USER
      type != STR_USER &&
#endif /* TRE_STR_USER */
      (eflags & REG_STARTEND) && pmatch)
    {
      if (pmatch->rm_so < 0)
	return REG_INVARG;
      if (len == (size_t)-1)
	{
	  if (pmatch->rm_eo < 0 || pmatch->rm_so > pmatch->rm_eo)
	    return REG_INVARG;
	  len = pmatch->rm_eo - pmatch->rm_so;
	}
      count = offset = pmatch->rm_so;
      if (type == STR_WIDE) offset *= sizeof(wchar_t);
    }

	if (tnfa->num_tags > 0 && nmatch > 0)
	{
#ifdef TRE_USE_ALLOCA
		tags = alloca(sizeof(*tags) * tnfa->num_tags);
#else /* !TRE_USE_ALLOCA */
		tags = xmalloc(sizeof(*tags) * tnfa->num_tags);
#endif /* !TRE_USE_ALLOCA */
		if (tags == NULL)
			return REG_ESPACE;
	}

  /* Dispatch to the appropriate matcher. */
  if (tnfa->have_backrefs || eflags & REG_BACKTRACKING_MATCHER)
    {
      /* The regex has back references, use the backtracking matcher. */
#ifdef TRE_STR_USER
      if (type == STR_USER)
	{
	  const tre_str_source *source = string;
	  if (source->rewind == NULL || source->compare == NULL)
	    /* The backtracking matcher requires rewind and compare
	       capabilities from the input stream. */
	    status = REG_BADPAT;
	} else
#endif /* TRE_STR_USER */
      status = tre_tnfa_run_backtrack(tnfa, string + offset, (int)len, type,
				      tags, eflags, &eo);
    }
#ifdef TRE_APPROX
  else if (tnfa->have_approx || eflags & REG_APPROX_MATCHER)
    {
      /* The regex uses approximate matching, use the approximate matcher. */
      regamatch_t match;
      regaparams_t params;
      tre_regaparams_default(&params);
      params.max_err = 0;
      params.max_cost = 0;
      status = tre_tnfa_run_approx(tnfa, string + offset, (int)len, type, tags,
				   &match, params, eflags, &eo);
    }
#endif /* TRE_APPROX */
  else
    {
      /* Exact matching, no back references, use the parallel matcher. */
      status = tre_tnfa_run_parallel(tnfa, string + offset, (int)len, type,
				     tags, eflags, &eo);
    }

  if (status == REG_OK)
    {
      /* A match was found, so fill the submatch registers. */
      status = tre_fill_pmatch(nmatch, pmatch, tnfa->cflags, tnfa, tags, eo);
      /* If doing REG_STARTEND, adjust the pmatch array (we can't build
         this into tre_fill_pmatch, because tre_tnfa_run_backtrack calls
	 tre_fill_pmatch itself). */
      if (status == REG_OK && !(tnfa->cflags & REG_NOSUB) &&
#ifdef TRE_STR_USER
	  type != STR_USER &&
#endif /* TRE_STR_USER */
	  (eflags & REG_STARTEND) && pmatch && nmatch > 0)
	{
	  size_t i;
	  regmatch_t *p;
	  for (i = nmatch, p = pmatch; i > 0; p++, i--)
	    {
	      if (p->rm_so >= 0) p->rm_so += count;
	      if (p->rm_eo >= 0) p->rm_eo += count;
	    }
	}
    }
#ifndef TRE_USE_ALLOCA
  if (tags)
    xfree(tags);
#endif /* !TRE_USE_ALLOCA */
  return status;
}

int
tre_regnexec(const regex_t *preg, const char *str, size_t len,
	 size_t nmatch, regmatch_t pmatch[], int eflags)
{
  tre_tnfa_t *tnfa = (void *)preg->TRE_REGEX_T_FIELD;
  tre_str_type_t type = (TRE_MB_CUR_MAX_L(tnfa->loc) == 1) ? STR_BYTE : STR_MBS;

#ifdef TRE_USE_SYSTEM_REGEX_H
  if (preg->re_magic != RE_MAGIC) return REG_BADPAT;
#endif /* TRE_USE_SYSTEM_REGEX_H */

  return tre_match(tnfa, str, len, type, nmatch, pmatch, eflags);
}

int
tre_regexec(const regex_t *preg, const char *str,
	size_t nmatch, regmatch_t pmatch[], int eflags)
{
  return tre_regnexec(preg, str, (size_t)-1, nmatch, pmatch, eflags);
}


#ifdef TRE_WCHAR

int
tre_regwnexec(const regex_t *preg, const wchar_t *str, size_t len,
	  size_t nmatch, regmatch_t pmatch[], int eflags)
{
  tre_tnfa_t *tnfa = (void *)preg->TRE_REGEX_T_FIELD;

#ifdef TRE_USE_SYSTEM_REGEX_H
  if (preg->re_magic != RE_MAGIC) return REG_BADPAT;
#endif /* TRE_USE_SYSTEM_REGEX_H */

  return tre_match(tnfa, str, len, STR_WIDE, nmatch, pmatch, eflags);
}

int
tre_regwexec(const regex_t *preg, const wchar_t *str,
	 size_t nmatch, regmatch_t pmatch[], int eflags)
{
  return tre_regwnexec(preg, str, (size_t)-1, nmatch, pmatch, eflags);
}

#endif /* TRE_WCHAR */

#ifdef TRE_STR_USER
int
tre_reguexec(const regex_t *preg, const tre_str_source *str,
	 size_t nmatch, regmatch_t pmatch[], int eflags)
{
  tre_tnfa_t *tnfa = (void *)preg->TRE_REGEX_T_FIELD;

#ifdef TRE_USE_SYSTEM_REGEX_H
  if (preg->re_magic != RE_MAGIC) return REG_BADPAT;
#endif /* TRE_USE_SYSTEM_REGEX_H */

  return tre_match(tnfa, str, (size_t)-1, STR_USER, nmatch, pmatch, eflags);
}
#endif /* TRE_STR_USER */


#ifdef TRE_APPROX

/*
  Wrapper functions for approximate regexp matching.
*/

static int
tre_match_approx(const tre_tnfa_t *tnfa, const void *string, size_t len,
		 tre_str_type_t type, regamatch_t *match, regaparams_t params,
		 int eflags)
{
  reg_errcode_t status;
  tre_tag_t *tags = NULL;
  int eo;
  size_t offset = 0, count = 0;

  /* If the regexp does not use approximate matching features, the
     maximum cost is zero, and the approximate matcher isn't forced,
     use the exact matcher instead. */
  if (params.max_cost == 0 && !tnfa->have_approx
      && !(eflags & REG_APPROX_MATCHER))
    return tre_match(tnfa, string, len, type, match->nmatch, match->pmatch,
		     eflags);

  /* Back references are not supported by the approximate matcher. */
  if (tnfa->have_backrefs)
    return REG_BADPAT;

  if (tnfa->num_tags > 0 && match->nmatch > 0)
    {
#if TRE_USE_ALLOCA
      tags = alloca(sizeof(*tags) * tnfa->num_tags);
#else /* !TRE_USE_ALLOCA */
      tags = xmalloc(sizeof(*tags) * tnfa->num_tags);
#endif /* !TRE_USE_ALLOCA */
      if (tags == NULL)
	return REG_ESPACE;
    }

  if (
#ifdef TRE_STR_USER
      type != STR_USER &&
#endif /* TRE_STR_USER */
      (eflags & REG_STARTEND) && match->pmatch)
    {
      if (match->pmatch->rm_so < 0)
	return REG_INVARG;
      if (len == (size_t)-1)
	{
	  if (match->pmatch->rm_eo < 0 || match->pmatch->rm_so >
	      match->pmatch->rm_eo)
	    return REG_INVARG;
	  len = match->pmatch->rm_eo - match->pmatch->rm_so;
	}
      count = offset = match->pmatch->rm_so;
      if (type == STR_WIDE) offset *= sizeof(wchar_t);
    }

  status = tre_tnfa_run_approx(tnfa, string, (int)len, type, tags,
			       match, params, eflags, &eo);
  if (status == REG_OK)
    {
      status = tre_fill_pmatch(match->nmatch, match->pmatch, tnfa->cflags,
			       tnfa, tags, eo);
      /* If doing REG_STARTEND, adjust the pmatch array (we can't build
         this into tre_fill_pmatch, because tre_tnfa_run_backtrack call
	 tre_fill_pmatch itself). */
      if (status == REG_OK && !(tnfa->cflags & REG_NOSUB) &&
#ifdef TRE_STR_USER
	  type != STR_USER &&
#endif /* TRE_STR_USER */
	  (eflags & REG_STARTEND) && match->pmatch && match->nmatch > 0)
	{
	  size_t i;
	  regmatch_t *p;
	  for (i = match->nmatch, p = match->pmatch; i > 0; p++, i--)
	    {
	      if (p->rm_so >= 0) p->rm_so += count;
	      if (p->rm_eo >= 0) p->rm_eo += count;
	    }
	}
    }
#ifndef TRE_USE_ALLOCA
  if (tags)
    xfree(tags);
#endif /* !TRE_USE_ALLOCA */
  return status;
}

int
tre_reganexec(const regex_t *preg, const char *str, size_t len,
	  regamatch_t *match, regaparams_t params, int eflags)
{
  tre_tnfa_t *tnfa = (void *)preg->TRE_REGEX_T_FIELD;
  tre_str_type_t type = (TRE_MB_CUR_MAX_L(tnfa->loc) == 1) ? STR_BYTE : STR_MBS;

#ifdef TRE_USE_SYSTEM_REGEX_H
  if (preg->re_magic != RE_MAGIC) return REG_BADPAT;
#endif /* TRE_USE_SYSTEM_REGEX_H */

  return tre_match_approx(tnfa, str, len, type, match, params, eflags);
}

int
tre_regaexec(const regex_t *preg, const char *str,
	 regamatch_t *match, regaparams_t params, int eflags)
{
  return tre_reganexec(preg, str, (size_t)-1, match, params, eflags);
}

#ifdef TRE_WCHAR

int
tre_regawnexec(const regex_t *preg, const wchar_t *str, size_t len,
	   regamatch_t *match, regaparams_t params, int eflags)
{
  tre_tnfa_t *tnfa = (void *)preg->TRE_REGEX_T_FIELD;

#ifdef TRE_USE_SYSTEM_REGEX_H
  if (preg->re_magic != RE_MAGIC) return REG_BADPAT;
#endif /* TRE_USE_SYSTEM_REGEX_H */

  return tre_match_approx(tnfa, str, len, STR_WIDE,
			  match, params, eflags);
}

int
tre_regawexec(const regex_t *preg, const wchar_t *str,
	  regamatch_t *match, regaparams_t params, int eflags)
{
  return tre_regawnexec(preg, str, (size_t)-1, match, params, eflags);
}

#endif /* TRE_WCHAR */

void
tre_regaparams_default(regaparams_t *params)
{
  memset(params, 0, sizeof(*params));
  params->cost_ins = 1;
  params->cost_del = 1;
  params->cost_subst = 1;
  params->max_cost = INT_MAX;
  params->max_ins = INT_MAX;
  params->max_del = INT_MAX;
  params->max_subst = INT_MAX;
  params->max_err = INT_MAX;
}

#endif /* TRE_APPROX */

/* EOF */

/*
  tre-match-backtrack.c - TRE backtracking regex matching engine

  This software is released under a BSD-style license.
  See the file LICENSE for details and copyright.

*/

/*
  This matcher is for regexps that use back referencing.  Regexp matching
  with back referencing is an NP-complete problem on the number of back
  references.  The easiest way to match them is to use a backtracking
  routine which basically goes through all possible paths in the TNFA
  and chooses the one which results in the best (leftmost and longest)
  match.  This can be spectacularly expensive and may run out of stack
  space, but there really is no better known generic algorithm.	 Quoting
  Henry Spencer from comp.compilers:
  <URL: http://compilers.iecc.com/comparch/article/93-03-102>

    POSIX.2 REs require longest match, which is really exciting to
    implement since the obsolete ("basic") variant also includes
    \<digit>.  I haven't found a better way of tackling this than doing
    a preliminary match using a DFA (or simulation) on a modified RE
    that just replicates subREs for \<digit>, and then doing a
    backtracking match to determine whether the subRE matches were
    right.  This can be rather slow, but I console myself with the
    thought that people who use \<digit> deserve very slow execution.
    (Pun unintentional but very appropriate.)

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

#include "tre-internal.h"
#include "tre-mem.h"
#include "tre-match-utils.h"
#include "tre.h"
#include "xmalloc.h"

typedef struct {
  int pos;
  unsigned int pos_add_next;
  const char *str_byte;
#ifdef TRE_WCHAR
  const wchar_t *str_wide;
#endif /* TRE_WCHAR */
  tre_tnfa_transition_t *state;
  int state_id;
  int next_c;
  tre_tag_t *tags;
#ifdef TRE_MBSTATE
  mbstate_t mbstate;
#endif /* TRE_MBSTATE */
} tre_backtrack_item_t;

typedef struct tre_backtrack_struct {
  tre_backtrack_item_t item;
  struct tre_backtrack_struct *prev;
  struct tre_backtrack_struct *next;
} *tre_backtrack_t;

#ifdef TRE_WCHAR
#define BT_STACK_WIDE_IN(_str_wide)	stack->item.str_wide = (_str_wide)
#define BT_STACK_WIDE_OUT		(str_wide) = stack->item.str_wide
#else /* !TRE_WCHAR */
#define BT_STACK_WIDE_IN(_str_wide)
#define BT_STACK_WIDE_OUT
#endif /* !TRE_WCHAR */

#ifdef TRE_MBSTATE
#define BT_STACK_MBSTATE_IN  stack->item.mbstate = (mbstate)
#define BT_STACK_MBSTATE_OUT (mbstate) = stack->item.mbstate
#else /* !TRE_MBSTATE */
#define BT_STACK_MBSTATE_IN
#define BT_STACK_MBSTATE_OUT
#endif /* !TRE_MBSTATE */


#ifdef TRE_USE_ALLOCA
#define tre_bt_mem_new		  tre_mem_newa
#define tre_bt_mem_alloc	  tre_mem_alloca
#define tre_bt_mem_destroy(obj)	  do { } while (0)
#else /* !TRE_USE_ALLOCA */
#define tre_bt_mem_new		  tre_mem_new
#define tre_bt_mem_alloc	  tre_mem_alloc
#define tre_bt_mem_destroy	  tre_mem_destroy
#endif /* !TRE_USE_ALLOCA */


#define BT_STACK_PUSH(_pos, _pos_add_next, _str_byte, _str_wide, _state, _state_id, _next_c, _tags, _mbstate) \
  do									      \
    {									      \
      if (!stack->next)							      \
	{								      \
	  tre_backtrack_t s;						      \
	  s = tre_bt_mem_alloc(mem, sizeof(*s));			      \
	  if (!s)							      \
	    {								      \
	      tre_bt_mem_destroy(mem);					      \
	      if (tags)							      \
		xfree(tags);						      \
	      if (pmatch)						      \
		xfree(pmatch);						      \
	      if (states_seen)						      \
		xfree(states_seen);					      \
	      return REG_ESPACE;					      \
	    }								      \
	  s->prev = stack;						      \
	  s->next = NULL;						      \
	  s->item.tags = tre_bt_mem_alloc(mem,				      \
					  num_tags * sizeof(*tags));          \
	  if (!s->item.tags)						      \
	    {								      \
	      tre_bt_mem_destroy(mem);					      \
	      if (tags)							      \
		xfree(tags);						      \
	      if (pmatch)						      \
		xfree(pmatch);						      \
	      if (states_seen)						      \
		xfree(states_seen);					      \
	      return REG_ESPACE;					      \
	    }								      \
	  stack->next = s;						      \
	  stack = s;							      \
	}								      \
      else								      \
	stack = stack->next;						      \
      stack->item.pos = (_pos);						      \
      stack->item.pos_add_next = (_pos_add_next);			      \
      stack->item.str_byte = (_str_byte);				      \
      BT_STACK_WIDE_IN(_str_wide);					      \
      stack->item.state = (_state);					      \
      stack->item.state_id = (_state_id);				      \
      stack->item.next_c = (_next_c);					      \
      memcpy(stack->item.tags, (_tags), num_tags * sizeof(*(_tags)));         \
      BT_STACK_MBSTATE_IN;						      \
    }									      \
  while (/*CONSTCOND*/0)

#ifdef TRE_STR_USER
#define BT_STACK_POP()							      \
  do									      \
    {									      \
      assert(stack->prev);						      \
      pos = stack->item.pos;						      \
      pos_add_next = stack->item.pos_add_next;				      \
      if (type == STR_USER)                                                   \
        str_source->rewind(pos + pos_add_next, str_source->context);          \
      str_byte = stack->item.str_byte;					      \
      BT_STACK_WIDE_OUT;						      \
      state = stack->item.state;					      \
      next_c = stack->item.next_c;					      \
      memcpy(tags, stack->item.tags, num_tags * sizeof(*tags));               \
      BT_STACK_MBSTATE_OUT;						      \
      stack = stack->prev;						      \
    }									      \
  while (/*CONSTCOND*/0)
#else /* !TRE_STR_USER */
#define BT_STACK_POP()							      \
  do									      \
    {									      \
      assert(stack->prev);						      \
      pos = stack->item.pos;						      \
      pos_add_next = stack->item.pos_add_next;				      \
      str_byte = stack->item.str_byte;					      \
      BT_STACK_WIDE_OUT;						      \
      state = stack->item.state;					      \
      next_c = stack->item.next_c;					      \
      memcpy(tags, stack->item.tags, num_tags * sizeof(*tags));               \
      BT_STACK_MBSTATE_OUT;						      \
      stack = stack->prev;						      \
    }									      \
  while (/*CONSTCOND*/0)
#endif /* !TRE_STR_USER */

#undef MIN
#define MIN(a, b) ((a) <= (b) ? (a) : (b))

reg_errcode_t
tre_tnfa_run_backtrack(const tre_tnfa_t *tnfa, const void *string,
		       int len, tre_str_type_t type, tre_tag_t *match_tags,
		       int eflags, int *match_end_ofs)
{
  /* State variables required by GET_NEXT_WCHAR. */
  tre_char_t prev_c = 0, next_c = 0;
  const char *str_byte = string;
  int pos = 0;
  unsigned int pos_add_next = 1;
#ifdef TRE_WCHAR
  const wchar_t *str_wide = string;
#ifdef TRE_MBSTATE
  mbstate_t mbstate;
#endif /* TRE_MBSTATE */
#endif /* TRE_WCHAR */
  int reg_notbol = eflags & REG_NOTBOL;
  int reg_noteol = eflags & REG_NOTEOL;
  int reg_newline = tnfa->cflags & REG_NEWLINE;
#ifdef TRE_STR_USER
  int str_user_end = 0;
#endif /* TRE_STR_USER */
  int i;

  /* These are used to remember the necessary values of the above
     variables to return to the position where the current search
     started from. */
  int next_c_start;
  const char *str_byte_start;
  int pos_start = -1;
#ifdef TRE_WCHAR
  const wchar_t *str_wide_start;
#endif /* TRE_WCHAR */
#ifdef TRE_MBSTATE
  mbstate_t mbstate_start;
#endif /* TRE_MBSTATE */

  /* End offset of best match so far, or -1 if no match found yet. */
  int match_eo = -1;
  /* Tag arrays. */
  int *next_tags;
  tre_tag_t *tags = NULL;
  /* Current TNFA state. */
  tre_tnfa_transition_t *state;
  int *states_seen = NULL;

  /* Memory allocator to for allocating the backtracking stack. */
  tre_mem_t mem = tre_bt_mem_new();

  /* The backtracking stack. */
  tre_backtrack_t stack;

  tre_tnfa_transition_t *trans_i;
  regmatch_t *pmatch = NULL;
  reg_errcode_t ret;

  int num_tags = tnfa->num_tags;
  int touch = 1;
  char *buf;
  int tbytes;

#ifdef TRE_MBSTATE
  memset(&mbstate, '\0', sizeof(mbstate));
#endif /* TRE_MBSTATE */

  if (!mem)
    return REG_ESPACE;
  stack = tre_bt_mem_alloc(mem, sizeof(*stack));
  if (!stack)
    {
      ret = REG_ESPACE;
      goto error_exit;
    }
  stack->prev = NULL;
  stack->next = NULL;

  DPRINT(("tnfa_execute_backtrack, input type %d\n", type));
  DPRINT(("len = %d\n", len));

  {
    int pbytes, sbytes, total_bytes;
    char *tmp_buf;
    /* Compute the length of the block we need. */
    tbytes = sizeof(*tags) * num_tags;
    pbytes = sizeof(*pmatch) * tnfa->num_submatches;
    sbytes = sizeof(*states_seen) * tnfa->num_states;
    total_bytes =
      (sizeof(long) - 1) * 2 /* for alignment paddings */
      + tbytes + pbytes + sbytes;

    DPRINT(("tre_tnfa_run_backtrack, allocate %d bytes\n", total_bytes));
    /* Allocate the memory. */
#ifdef TRE_USE_ALLOCA
    buf = alloca(total_bytes);
#else /* !TRE_USE_ALLOCA */
    buf = xmalloc((unsigned)total_bytes);
#endif /* !TRE_USE_ALLOCA */
    if (buf == NULL)
      return REG_ESPACE;

    /* Get the various pointers within tmp_buf (properly aligned). */
    tags = (void *)buf;
    tmp_buf = buf + tbytes;
    tmp_buf += ALIGN(tmp_buf, long);
    pmatch = (void *)tmp_buf;
    tmp_buf += pbytes;
    tmp_buf += ALIGN(tmp_buf, long);
    states_seen = (void *)tmp_buf;
  }

 retry:
  {
    memset(tags, 0, num_tags * sizeof(*tags));
    if (match_tags)
      memset(match_tags, 0, num_tags * sizeof(*match_tags));
    for (i = 0; i < tnfa->num_states; i++)
      states_seen[i] = 0;
  }

  state = NULL;
  pos = pos_start;
#ifdef TRE_STR_USER
  if (type == STR_USER)
    str_source->rewind(pos + pos_add_next, str_source->context);
#endif /* TRE_STR_USER */
  GET_NEXT_WCHAR();
  pos_start = pos;
  next_c_start = next_c;
  str_byte_start = str_byte;
#ifdef TRE_WCHAR
  str_wide_start = str_wide;
#endif /* TRE_WCHAR */
#ifdef TRE_MBSTATE
  mbstate_start = mbstate;
#endif /* TRE_MBSTATE */

  /* Handle initial states. */
  next_tags = NULL;
  for (trans_i = tnfa->initial; trans_i->state; trans_i++)
    {
      DPRINT(("> init %p, prev_c %lc\n", trans_i->state, (tre_cint_t)prev_c));
      if (trans_i->assertions && CHECK_ASSERTIONS(trans_i->assertions))
	{
	  DPRINT(("assert failed\n"));
	  continue;
	}
      if (state == NULL)
	{
	  /* Start from this state. */
	  state = trans_i->state;
	  next_tags = trans_i->tags;
	}
      else
	{
	  /* Backtrack to this state. */
	  DPRINT(("saving state %d for backtracking\n", trans_i->state_id));
	  BT_STACK_PUSH(pos, pos_add_next, str_byte, str_wide, trans_i->state,
			trans_i->state_id, next_c, tags, mbstate);
	  {
	    int *tmp = trans_i->tags;
	    if (tmp)
	      {
		while (*tmp >= 0)
		  tre_tag_set(stack->item.tags, *tmp++, pos, touch);
		touch++;
	      }
	  }
	}
    }

  if (next_tags)
    {
      for (; *next_tags >= 0; next_tags++)
	tre_tag_set(tags, *next_tags, pos, touch);
      touch++;
    }


  DPRINT(("entering match loop, pos %d, str_byte %p\n", pos, str_byte));
  DPRINT(("pos:chr/code | state and tags\n"));
  DPRINT(("-------------+------------------------------------------------\n"));

  if (state == NULL)
    goto backtrack;

  while (/*CONSTCOND*/1)
    {
      tre_tnfa_transition_t *next_state;
      int empty_br_match;

      DPRINT(("start loop\n"));

      if (match_eo >= 0 && tnfa->num_minimals)
	{
	  int skip = 0;
#ifdef TRE_DEBUG
	  DPRINT(("Checking minimal conditions: match_eo=%d match_tags=",
		  match_eo));
	  tre_print_tags(match_tags, tnfa->num_tags);
	  DPRINT(("\n"));
#endif /* TRE_DEBUG */
	  for (i = 0; tnfa->minimal_tags[i] >= 0; i += 2)
	    {
	      int end = tnfa->minimal_tags[i];
	      int start = tnfa->minimal_tags[i + 1];
	      DPRINT(("  Minimal start %d, end %d\n", start, end));
	      if (tre_minimal_tag_order(start, end, match_tags, tags) > 0)
		{
		  skip = 1;
		  break;
		}
	    }
	  if (!skip)
	    {
#ifdef TRE_DEBUG
	      DPRINT(("	 Keeping tags="));
	      tre_print_tags(tags, tnfa->num_tags);
	      DPRINT(("\n"));
#endif /* TRE_DEBUG */
	    }
	  else
	    {
#ifdef TRE_DEBUG
	      DPRINT(("	 Throwing out tags="));
	      tre_print_tags(tags, tnfa->num_tags);
	      DPRINT(("\n"));
#endif /* TRE_DEBUG */
	      goto backtrack;
	    }
	}

      if (state == tnfa->final)
	{
	  DPRINT(("  match found, match_eo=%d pos=%d\n", match_eo, pos));

	  if (match_eo >= 0 && tnfa->num_minimals)
	    {
	      int compare = 0;
#ifdef TRE_DEBUG
	      DPRINT(("Checking minimal conditions: match_eo=%d "
		      "match_tags=", match_eo));
	      tre_print_tags(match_tags, tnfa->num_tags);
	      DPRINT(("\n"));
#endif /* TRE_DEBUG */
	      for (i = 0; tnfa->minimal_tags[i] >= 0; i += 2)
		{
		  int end = tnfa->minimal_tags[i];
		  int start = tnfa->minimal_tags[i + 1];
		  DPRINT(("  Minimal start %d, end %d\n", start, end));
		  if ((compare = tre_minimal_tag_order(start, end,
					   match_tags, tags)) != 0)
		    break;
		}
	      if (compare > 0)
		{
#ifdef TRE_DEBUG
		  DPRINT(("	 Throwing out new match, tags="));
		  tre_print_tags(tags, tnfa->num_tags);
		  DPRINT(("\n"));
#endif /* TRE_DEBUG */
		  goto backtrack;
		}
	      else if (compare < 0)
		{
#ifdef TRE_DEBUG
		  DPRINT(("	 Throwing out old match, tags="));
		  tre_print_tags(match_tags, tnfa->num_tags);
		  DPRINT(("\n"));
#endif /* TRE_DEBUG */
		  match_eo = -1;
		}
	    }

	  if (match_eo < pos
	      || (match_eo == pos
		  && match_tags
		  && tre_tag_order(tnfa->num_tags, tnfa->tag_directions,
				   tags, match_tags)))
	    {
	      /* This match wins the previous match. */
#ifdef TRE_DEBUG
	      DPRINT(("	 win previous tags="));
	      tre_print_tags(tags, tnfa->num_tags);
	      DPRINT(("\n"));
#endif /* TRE_DEBUG */
	      match_eo = pos;
	      if (match_tags)
		memcpy(match_tags, tags, num_tags * sizeof(*tags));
	    }
	  /* Our TNFAs never have transitions leaving from the final state,
	     so we jump right to backtracking. */
	  goto backtrack;
	}

#ifdef TRE_DEBUG
      DPRINT(("%3d:%2lc/%05d | %p ", pos, (tre_cint_t)next_c, (int)next_c,
	      state));
      tre_print_tags(tags, tnfa->num_tags);
      DPRINT(("\n"));
#endif /* TRE_DEBUG */

      /* Go to the next character in the input string. */
      empty_br_match = 0;
      trans_i = state;
      if (trans_i->state && trans_i->assertions & ASSERT_BACKREF)
	{
	  /* This is a back reference state.  All transitions leaving from
	     this state have the same back reference "assertion".  Instead
	     of reading the next character, we match the back reference. */
	  int so, eo, bt = trans_i->u.backref;
	  int bt_len;
	  int result;

	  DPRINT(("  should match back reference %d\n", bt));
	  /* Get the substring we need to match against.  Remember to
	     turn off REG_NOSUB temporarily. */
	  ret = tre_fill_pmatch(bt + 1, pmatch, tnfa->cflags & ~REG_NOSUB,
			  tnfa, tags, pos);
	  if (ret != REG_OK) goto error_exit;
	  so = pmatch[bt].rm_so;
	  eo = pmatch[bt].rm_eo;
	  bt_len = eo - so;

#ifdef TRE_DEBUG
	  {
	    int slen;
	    if (len < 0)
	      slen = bt_len;
	    else
	      slen = MIN(bt_len, len - pos);

	    if (type == STR_BYTE)
	      {
		DPRINT(("  substring (len %d) is [%d, %d]: '%.*s'\n",
			bt_len, so, eo, bt_len, (char*)string + so));
		DPRINT(("  current string is '%.*s'\n", slen, str_byte - 1));
	      }
#ifdef TRE_WCHAR
	    else if (type == STR_WIDE)
	      {
		DPRINT(("  substring (len %d) is [%d, %d]: '%.*" STRF "'\n",
			bt_len, so, eo, bt_len, (wchar_t*)string + so));
		DPRINT(("  current string is '%.*" STRF "'\n",
			slen, str_wide - 1));
	      }
#endif /* TRE_WCHAR */
	  }
#endif

	  if (so < 0)
	    {
	      result = 1; /* Back reference of nomatch doesn't match */
	    }
	  else if (len < 0)
	    {
#ifdef TRE_STR_USER
	      if (type == STR_USER)
		result = str_source->compare((unsigned)so, (unsigned)pos,
					     (unsigned)bt_len,
					     str_source->context);
	      else
#endif /* TRE_STR_USER */
#ifdef TRE_WCHAR
	      if (type == STR_WIDE)
		result = wcsncmp((const wchar_t*)string + so, str_wide - 1,
				 (size_t)bt_len);
	      else
#endif /* TRE_WCHAR */
	      result = strncmp((const char*)string + so, str_byte - 1,
				 (size_t)bt_len);
	    }
	  else if (len - pos < bt_len)
	    result = 1;
#ifdef TRE_WCHAR
	  else if (type == STR_WIDE)
	    result = wmemcmp((const wchar_t*)string + so, str_wide - 1,
			     (size_t)bt_len);
#endif /* TRE_WCHAR */
	  else
	    result = memcmp((const char*)string + so, str_byte - 1,
			    (size_t)bt_len);

	  if (result == 0)
	    {
	      /* Back reference matched.  Check for infinite loop. */
	      if (bt_len == 0)
		empty_br_match = 1;
	      if (empty_br_match && states_seen[trans_i->state_id])
		{
		  DPRINT(("  avoid loop\n"));
		  goto backtrack;
		}

	      states_seen[trans_i->state_id] = empty_br_match;

	      /* Advance in input string and resync `prev_c', `next_c'
		 and pos. */
	      DPRINT(("	 back reference matched\n"));
	      str_byte += bt_len - 1;
#ifdef TRE_WCHAR
	      str_wide += bt_len - 1;
#endif /* TRE_WCHAR */
	      pos += bt_len - 1;
	      GET_NEXT_WCHAR();
	      DPRINT(("	 pos now %d\n", pos));
	    }
	  else
	    {
	      DPRINT(("	 back reference did not match\n"));
	      goto backtrack;
	    }
	}
      else
	{
	  /* Check for end of string. */
	  if (len < 0)
	    {
#ifdef TRE_STR_USER
	      if (type == STR_USER)
		{
		  if (str_user_end)
		    goto backtrack;
		}
	      else
#endif /* TRE_STR_USER */
	      if (next_c == L'\0')
		goto backtrack;
	    }
	  else
	    {
	      if (pos >= len)
		goto backtrack;
	    }

	  /* Read the next character. */
	  GET_NEXT_WCHAR();
	}

      next_state = NULL;
      for (trans_i = state; trans_i->state; trans_i++)
	{
	  DPRINT(("  transition %d-%d (%c-%c) %d to %d\n",
		  trans_i->code_min, trans_i->code_max,
		  trans_i->code_min, trans_i->code_max,
		  trans_i->assertions, trans_i->state_id));
	  if (trans_i->code_min <= (tre_cint_t)prev_c
	      && trans_i->code_max >= (tre_cint_t)prev_c)
	    {
	      if (trans_i->assertions
		  && (CHECK_ASSERTIONS(trans_i->assertions)
		      || CHECK_CHAR_CLASSES(trans_i, tnfa, eflags)))
		{
		  DPRINT(("  assertion failed\n"));
		  continue;
		}

	      if (next_state == NULL)
		{
		  /* First matching transition. */
		  DPRINT(("  Next state is %d\n", trans_i->state_id));
		  next_state = trans_i->state;
		  next_tags = trans_i->tags;
		}
	      else
		{
		  /* Second matching transition.  We may need to backtrack here
		     to take this transition instead of the first one, so we
		     push this transition in the backtracking stack so we can
		     jump back here if needed. */
		  DPRINT(("  saving state %d for backtracking\n",
			  trans_i->state_id));
		  BT_STACK_PUSH(pos, pos_add_next, str_byte, str_wide,
				trans_i->state, trans_i->state_id, next_c,
				tags, mbstate);
		  {
		    int *tmp;
		    for (tmp = trans_i->tags; tmp && *tmp >= 0; tmp++)
		      tre_tag_set(stack->item.tags, *tmp, pos, touch);
		    touch++;
		  }
#if 0 /* XXX - it's important not to look at all transitions here to keep
	 the stack small! */
		  break;
#endif
		}
	    }
	}

      if (next_state != NULL)
	{
	  /* Matching transitions were found.  Take the first one. */
	  state = next_state;

	  /* Update the tag values. */
	  if (next_tags)
	    {
	      while (*next_tags >= 0)
		tre_tag_set(tags, *next_tags++, pos, touch);
	      touch++;
	    }
	}
      else
	{
	backtrack:
	  /* A matching transition was not found.  Try to backtrack. */
	  if (stack->prev)
	    {
	      DPRINT(("	 backtracking\n"));
	      if (stack->item.state->assertions & ASSERT_BACKREF)
		{
		  DPRINT(("  states_seen[%d] = 0\n",
			  stack->item.state_id));
		  states_seen[stack->item.state_id] = 0;
		}

	      BT_STACK_POP();
	    }
	  else if (match_eo < 0)
	    {
	      /* Try starting from a later position in the input string. */
	      /* Check for end of string. */
	      if (pos == pos_start)
		{
		  if (len < 0)
		    {
		      if (next_c == L'\0')
			{
			  DPRINT(("end of string.\n"));
			  break;
			}
		    }
		  else
		    {
		      if (pos >= len)
			{
			  DPRINT(("end of string.\n"));
			  break;
			}
		    }
		}
	      DPRINT(("restarting from next start position\n"));
	      next_c = next_c_start;
#ifdef TRE_MBSTATE
	      mbstate = mbstate_start;
#endif /* TRE_MBSTATE */
	      str_byte = str_byte_start;
#ifdef TRE_WCHAR
	      str_wide = str_wide_start;
#endif /* TRE_WCHAR */
	      goto retry;
	    }
	  else
	    {
	      DPRINT(("finished\n"));
	      break;
	    }
	}
    }

  ret = match_eo >= 0 ? REG_OK : REG_NOMATCH;
  *match_end_ofs = match_eo;

 error_exit:
  tre_bt_mem_destroy(mem);
#ifndef TRE_USE_ALLOCA
  if (buf)
    xfree(buf);
#endif /* !TRE_USE_ALLOCA */

  return ret;
}

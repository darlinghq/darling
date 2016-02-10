/*
  tre-match-utils.h - TRE matcher helper definitions

  This software is released under a BSD-style license.
  See the file LICENSE for details and copyright.

*/

#include "tre-internal.h"

#define str_source ((const tre_str_source*)string)

#ifdef TRE_WCHAR

#ifdef TRE_MULTIBYTE

/* Wide character and multibyte support. */

#ifdef TRE_STR_USER
#define GET_NEXT_WCHAR()						      \
  do {									      \
    prev_c = next_c;							      \
    if (type == STR_BYTE)						      \
      {									      \
	pos++;								      \
	if (len >= 0 && pos >= len)					      \
	  next_c = '\0';						      \
	else								      \
	  next_c = (unsigned char)(*str_byte++);			      \
      }									      \
    else if (type == STR_WIDE)						      \
      {									      \
	pos++;								      \
	if (len >= 0 && pos >= len)					      \
	  next_c = L'\0';						      \
	else								      \
	  next_c = *str_wide++;						      \
      }									      \
    else if (type == STR_MBS)						      \
      {									      \
        pos += pos_add_next;					      	      \
	if (str_byte == NULL)						      \
	  next_c = L'\0';						      \
	else								      \
	  {								      \
	    size_t w;							      \
	    int max;							      \
	    if (len >= 0)						      \
	      max = len - pos;						      \
	    else							      \
	      max = 32;							      \
	    if (max <= 0)						      \
	      {								      \
		next_c = L'\0';						      \
		pos_add_next = 1;					      \
	      }								      \
	    else							      \
	      {								      \
		w = tre_mbrtowc_l(&next_c, str_byte, (size_t)max, &mbstate,   \
				  tnfa->loc);                                 \
		if (w == (size_t)-1 || w == (size_t)-2)			      \
		  return REG_ILLSEQ;					      \
		if (w == 0 && len >= 0)					      \
		  {							      \
		    pos_add_next = 1;					      \
		    next_c = 0;						      \
		    str_byte++;						      \
		  }							      \
		else							      \
		  {							      \
		    pos_add_next = w;					      \
		    str_byte += w;					      \
		  }							      \
	      }								      \
	  }								      \
      }									      \
    else if (type == STR_USER)						      \
      {									      \
        pos += pos_add_next;					      	      \
	str_user_end = str_source->get_next_char(&next_c, &pos_add_next,      \
                                                 str_source->context);	      \
      }									      \
  } while(/*CONSTCOND*/0)
#else /* !TRE_STR_USER */
#define GET_NEXT_WCHAR()						      \
  do {									      \
    prev_c = next_c;							      \
    if (type == STR_BYTE)						      \
      {									      \
	pos++;								      \
	if (len >= 0 && pos >= len)					      \
	  next_c = '\0';						      \
	else								      \
	  next_c = (unsigned char)(*str_byte++);			      \
      }									      \
    else if (type == STR_WIDE)						      \
      {									      \
	pos++;								      \
	if (len >= 0 && pos >= len)					      \
	  next_c = L'\0';						      \
	else								      \
	  next_c = *str_wide++;						      \
      }									      \
    else if (type == STR_MBS)						      \
      {									      \
        pos += pos_add_next;					      	      \
	if (str_byte == NULL)						      \
	  next_c = L'\0';						      \
	else								      \
	  {								      \
	    size_t w;							      \
	    int max;							      \
	    if (len >= 0)						      \
	      max = len - pos;						      \
	    else							      \
	      max = 32;							      \
	    if (max <= 0)						      \
	      {								      \
		next_c = L'\0';						      \
		pos_add_next = 1;					      \
	      }								      \
	    else							      \
	      {								      \
		w = tre_mbrtowc_l(&next_c, str_byte, (size_t)max, &mbstate,   \
				  tnfa->loc);                                 \
		if (w == (size_t)-1 || w == (size_t)-2)			      \
		  return REG_ILLSEQ;					      \
		if (w == 0 && len >= 0)					      \
		  {							      \
		    pos_add_next = 1;					      \
		    next_c = 0;						      \
		    str_byte++;						      \
		  }							      \
		else							      \
		  {							      \
		    pos_add_next = w;					      \
		    str_byte += w;					      \
		  }							      \
	      }								      \
	  }								      \
      }									      \
  } while(/*CONSTCOND*/0)
#endif /* !TRE_STR_USER */

#else /* !TRE_MULTIBYTE */

/* Wide character support, no multibyte support. */

#ifdef TRE_STR_USER
#define GET_NEXT_WCHAR()						      \
  do {									      \
    prev_c = next_c;							      \
    if (type == STR_BYTE)						      \
      {									      \
	pos++;								      \
	if (len >= 0 && pos >= len)					      \
	  next_c = '\0';						      \
	else								      \
	  next_c = (unsigned char)(*str_byte++);			      \
      }									      \
    else if (type == STR_WIDE)						      \
      {									      \
	pos++;								      \
	if (len >= 0 && pos >= len)					      \
	  next_c = L'\0';						      \
	else								      \
	  next_c = *str_wide++;						      \
      }									      \
    else if (type == STR_USER)						      \
      {									      \
        pos += pos_add_next;					      	      \
	str_user_end = str_source->get_next_char(&next_c, &pos_add_next,      \
                                                 str_source->context);	      \
      }									      \
  } while(/*CONSTCOND*/0)
#else /* !TRE_STR_USER */
#define GET_NEXT_WCHAR()						      \
  do {									      \
    prev_c = next_c;							      \
    if (type == STR_BYTE)						      \
      {									      \
	pos++;								      \
	if (len >= 0 && pos >= len)					      \
	  next_c = '\0';						      \
	else								      \
	  next_c = (unsigned char)(*str_byte++);			      \
      }									      \
    else if (type == STR_WIDE)						      \
      {									      \
	pos++;								      \
	if (len >= 0 && pos >= len)					      \
	  next_c = L'\0';						      \
	else								      \
	  next_c = *str_wide++;						      \
      }									      \
  } while(/*CONSTCOND*/0)
#endif /* !TRE_STR_USER */

#endif /* !TRE_MULTIBYTE */

#else /* !TRE_WCHAR */

/* No wide character or multibyte support. */

#ifdef TRE_STR_USER
#define GET_NEXT_WCHAR()						      \
  do {									      \
    prev_c = next_c;							      \
    if (type == STR_BYTE)						      \
      {									      \
	pos++;								      \
	if (len >= 0 && pos >= len)					      \
	  next_c = '\0';						      \
	else								      \
	  next_c = (unsigned char)(*str_byte++);			      \
      }									      \
    else if (type == STR_USER)						      \
      {									      \
	pos += pos_add_next;						      \
	str_user_end = str_source->get_next_char(&next_c, &pos_add_next,      \
						 str_source->context);	      \
      }									      \
  } while(/*CONSTCOND*/0)
#else /* !TRE_STR_USER */
#define GET_NEXT_WCHAR()						      \
  do {									      \
    prev_c = next_c;							      \
    if (type == STR_BYTE)						      \
      {									      \
	pos++;								      \
	if (len >= 0 && pos >= len)					      \
	  next_c = '\0';						      \
	else								      \
	  next_c = (unsigned char)(*str_byte++);			      \
      }									      \
  } while(/*CONSTCOND*/0)
#endif /* !TRE_STR_USER */

#endif /* !TRE_WCHAR */



/* Assumes tre_tnfa_t *tnfa in scope */
#define IS_WORD_CHAR(c)	 ((c) == L'_' || tre_isalnum_l(c, tnfa->loc))

#define CHECK_ASSERTIONS(assertions)					      \
  (((assertions & ASSERT_AT_BOL)					      \
    && (pos > 0 || reg_notbol)						      \
    && (prev_c != L'\n' || !reg_newline))				      \
   || ((assertions & ASSERT_AT_EOL)					      \
       && (next_c != L'\0' || reg_noteol)				      \
       && (next_c != L'\n' || !reg_newline))				      \
   || ((assertions & ASSERT_AT_BOW)					      \
       && (IS_WORD_CHAR(prev_c) || !IS_WORD_CHAR(next_c)))	              \
   || ((assertions & ASSERT_AT_EOW)					      \
       && (!IS_WORD_CHAR(prev_c) || IS_WORD_CHAR(next_c)))		      \
   || ((assertions & ASSERT_AT_WB)					      \
       && (pos != 0 && next_c != L'\0'					      \
	   && IS_WORD_CHAR(prev_c) == IS_WORD_CHAR(next_c)))		      \
   || ((assertions & ASSERT_AT_WB_NEG)					      \
       && (pos == 0 || next_c == L'\0'					      \
	   || IS_WORD_CHAR(prev_c) != IS_WORD_CHAR(next_c))))

#define CHECK_CHAR_CLASSES(trans_i, tnfa, eflags)                             \
  ((trans_i->assertions & ASSERT_BRACKET_MATCH)                               \
    && !tre_bracket_match(trans_i->u.bracket_match_list,(tre_cint_t)prev_c,    \
                                      tnfa))




inline static int
tre_tag_get(const tre_tag_t *tags, int i)
{
  tags += i;
  return tags->count > 0 ? tags->value : -1;
}

inline static void
tre_tag_set(tre_tag_t *tags, int i, int val, int touch)
{
  tags += i;
  if (tags->count++ == 0)
    tags->first = val;
  tags->value = val;
  tags->touch = touch;
}

inline static void
tre_tag_reset(tre_tag_t *tags, int i)
{
  tags[i].count = 0;
}

inline static int
tre_tag_touch_get(const tre_tag_t *tags, int i)
{
  return tags[i].touch;
}

#ifdef TRE_DEBUG
inline static void
tre_print_tags(const tre_tag_t *tags, int num_tags)
{
  int i;
  for (i = 0; i < num_tags; i++, tags++)
    {
      switch(tags->count)
	{
	case 0:
	  DPRINT(("%d:(0,-1)", i));
	  break;
	case 1:
	  DPRINT(("%d:(1,%d)", i, tags->first));
	  break;
	default:
	  DPRINT(("%d:(%d,%d,%d)", i, tags->count, tags->first,
		  tags->value));
	  break;
	}
      if (i < (num_tags - 1))
	DPRINT((" "));
    }
}

inline static void
tre_print_tags_all(const tre_tag_t *tags, int num_tags)
{
  int i;
  for (i = 0; i < num_tags; i++, tags++)
    {
      switch(tags->count)
	{
	case 0:
	  DPRINT(("%d:(0,-1)/%d", i, tags->touch));
	  break;
	case 1:
	  DPRINT(("%d:(1,%d)/%d", i, tags->first, tags->touch));
	  break;
	default:
	  DPRINT(("%d:(%d,%d,%d)/%d", i, tags->count, tags->first,
		  tags->value, tags->touch));
	  break;
	}
      if (i < (num_tags - 1))
	DPRINT((" "));
    }
}
#endif /* TRE_DEBUG */

/* Return < 0, = 0 or > 0 depending on how the start/end pairs of a minimal
 * group between t1 and t2 compare (t1 loses if < 0, t1 wins if > 0) */
inline static int
tre_minimal_tag_order(int start, int end, const tre_tag_t *tags1,
		      const tre_tag_t *tags2)
{
  const tre_tag_t *t1, *t2;

  t1 = tags1 + start;
  t2 = tags2 + start;
  /* We need both start tags to be set */
  if (t1->count == 0 || t2->count == 0)
    return 0;

  /* The start tags must be equal */
  if (t1->value != t2->value)
    return 0;

  t1 = tags1 + end;
  t2 = tags2 + end;
  /* For the end tags, we prefer set over unset, because unset means that
   * the end tag is still growing */
  if (t1->count == 0)
    {
      /* if t2 is set, t1 loses since it is unset */
      if (t2->count != 0)
	return -1;
    }
  /* if t2 not set, t1 wins since it is set */
  else if (t2->count == 0)
    return 1;

  /* least current value wins */
  return t2->value - t1->value;
}

/* Return < 0, = 0 or > 0 depending on how the i-th item of t1 and t2 compare
 * (t1 loses if < 0, t1 wins if > 0) */
inline static int
tre_tag_order_1(int i, tre_tag_direction_t dir, const tre_tag_t *t1,
		const tre_tag_t *t2)
{
  int diff;

  t1 += i;
  t2 += i;
  switch (dir)
    {
    case TRE_TAG_MINIMIZE:
      /* least current value wins (because tags are initialized to all zeros,
       * unset wins over set; also, tre_minimal_tag_order() will have already
       * been run, which checks for being unset) */
      return t2->value - t1->value;

    case TRE_TAG_MAXIMIZE:
      /* prefer set */
      if (t1->count == 0)
	{
	  /* if neither t1 and t2 are set, try next tag */
	  if (t2->count == 0)
	    return 0;
	  /* t2 is set, t1 loses since it is unset */
	  return -1;
	}
      /* if t2 not set, t1 wins since it is set */
      else if (t2->count == 0)
	return 1;
      /* greatest initial value wins */
      if ((diff = t1->first - t2->first) != 0)
	return diff;
      /* least number of times the tag was set, wins */
      if ((diff = t2->count - t1->count) != 0)
	return diff;
      /* if the tags were only set once, they only have initial values */
      if (t1->count == 1)
	return 0;
      /* greatest current value wins */
      return t1->value - t2->value;

    case TRE_TAG_LEFT_MAXIMIZE:
      /* prefer set */
      if (t1->count == 0)
	{
	  /* if neither t1 and t2 are set, try next tag */
	  if (t2->count == 0)
	    return 0;
	  /* t2 is set, t1 loses since it is unset */
	  return -1;
	}
      /* if t2 not set, t1 wins since it is set */
      else if (t2->count == 0)
	return 1;
      /* least initial value wins */
      if ((diff = t2->first - t1->first) != 0)
	return diff;
      /* least number of times the tag was set, wins */
      if ((diff = t2->count - t1->count) != 0)
	return diff;
      /* if the tags were only set once, they only have initial values */
      if (t1->count == 1)
	return 0;
      /* greatest current value wins */
      return t1->value - t2->value;

    default:
      /* Shouldn't happen: only assert if TRE_DEBUG defined */
      assert(0);
      break;
    }
  return 0;
}

#ifdef TRE_DEBUG
#define _MORE_DEBUGGING
#endif /* TRE_DEBUG */

/* Returns 1 if `t1' wins `t2', 0 otherwise. */
inline static int
#ifdef _MORE_DEBUGGING
_tre_tag_order(int num_tags, tre_tag_direction_t *tag_directions,
	      const tre_tag_t *t1, const tre_tag_t *t2)
#else /* !_MORE_DEBUGGING */
tre_tag_order(int num_tags, tre_tag_direction_t *tag_directions,
	      const tre_tag_t *t1, const tre_tag_t *t2)
#endif /* !_MORE_DEBUGGING */
{
  int i, ret;

  for (i = 0; i < num_tags; i++)
    {
      if ((ret = tre_tag_order_1(i, tag_directions[i], t1, t2)) != 0)
	return (ret > 0);
    }
  /*  assert(0);*/
  return 0;
}

#ifdef _MORE_DEBUGGING
inline static int
tre_tag_order(int num_tags, tre_tag_direction_t *tag_directions,
	      const tre_tag_t *t1, const tre_tag_t *t2)
{
  int ret = _tre_tag_order(num_tags, tag_directions, t1, t2);
  DPRINT(("tre_tag_order: "));
  tre_print_tags(t1, num_tags);
  DPRINT((" %s ", ret ? "wins" : "doesn't win"));
  tre_print_tags(t2, num_tags);
  DPRINT(("\n"));
  return ret;
}
#endif /* _MORE_DEBUGGING */

#ifdef __LIBC__
#include <xlocale_private.h>
#else /* !__LIBC__ */
#include <xlocale.h>
#endif /* !__LIBC__ */

int __collate_equiv_value(locale_t loc, const wchar_t *str, size_t len);

inline static int
tre_bracket_match(tre_bracket_match_list_t * __restrict list, tre_cint_t wc,
		  const tre_tnfa_t * __restrict tnfa)
{
  int match = 0;
  int i;
  tre_bracket_match_t *b;
  tre_cint_t uc, lc;
  int we, ue, le, got_equiv = 0;
  int icase = ((tnfa->cflags & REG_ICASE) != 0);

  DPRINT(("tre_bracket_match: %p, %d, %d\n", list, wc, icase));
  if (icase)
    {
      if (tre_islower_l(wc, tnfa->loc))
	{
	  lc = wc;
	  uc = tre_toupper_l(wc, tnfa->loc);
	}
      else if (tre_isupper_l(wc, tnfa->loc))
	{
	  uc = wc;
	  lc = tre_tolower_l(wc, tnfa->loc);
	}
      else
	{
	  icase = 0;
	}
    }
  for (i = 0, b = list->bracket_matches; i < list->num_bracket_matches;
       i++, b++)
    {
      switch (b->type)
	{
	case TRE_BRACKET_MATCH_TYPE_CHAR:
	  if (icase)
	    match = (b->value == uc || b->value == lc);
	  else
	    match = (b->value == wc);
	  break;
	case TRE_BRACKET_MATCH_TYPE_RANGE_BEGIN:
	  {
	    tre_cint_t start = b->value, end;
	    if (++i >= list->num_bracket_matches ||
		(++b)->type != TRE_BRACKET_MATCH_TYPE_RANGE_END)
	      {
		DPRINT(("tre_bracket_match: no following range end\n"));
		assert(0);
		goto error;
	      }
	    end = b->value;
	    if (!got_equiv)
	      {
		if (icase)
		  {
		    ue = __collate_equiv_value(tnfa->loc, &uc, 1);
		    le = __collate_equiv_value(tnfa->loc, &lc, 1);
		  }
		else
		    we = __collate_equiv_value(tnfa->loc, &wc, 1);
		got_equiv = 1;
	      }
	    if (icase)
	      match = ((start <= ue && ue <= end) ||
		      (start <= le && le <= end));
	    else
	      match = (start <= we && we <= end);
	    break;
	  }
	case TRE_BRACKET_MATCH_TYPE_RANGE_END:
	  DPRINT(("tre_bracket_match: range end without preceeding start\n"));
	  assert(0);
	  break;
	case TRE_BRACKET_MATCH_TYPE_CLASS:
	  if (icase)
	    match = (tre_isctype_l(uc, b->value, tnfa->loc) ||
		     tre_isctype_l(lc, b->value, tnfa->loc));
	  else
	    match = (tre_isctype_l(wc, b->value, tnfa->loc));
	  break;
	case TRE_BRACKET_MATCH_TYPE_EQUIVALENCE:
	  if (!got_equiv)
	    {
	      if (icase)
		{
		  ue = __collate_equiv_value(tnfa->loc, &uc, 1);
		  le = __collate_equiv_value(tnfa->loc, &lc, 1);
		}
	      else
		  we = __collate_equiv_value(tnfa->loc, &wc, 1);
	      got_equiv = 1;
	    }
	  if (icase)
	    match = (b->value == ue || b->value == le);
	  else
	    match = (b->value == we);
	  break;
	default:
	  DPRINT(("tre_bracket_match: unknown type %d\n", b->type));
	  assert(0);
	  break;
	}
	if (match)
	  break;
    }
error:
  if (list->flags & TRE_BRACKET_MATCH_FLAG_NEGATE)
    match = !match;
  return match;
}

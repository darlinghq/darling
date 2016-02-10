/*
  tre-compile.h: Regex compilation definitions

  This software is released under a BSD-style license.
  See the file LICENSE for details and copyright.

*/


#ifndef TRE_COMPILE_H
#define TRE_COMPILE_H 1

typedef struct {
  int position;
  int code_min;
  int code_max;
  int *tags;
  int assertions;
  tre_bracket_match_list_t *bracket_match_list;
  int backref;
  int *params;
  locale_t loc;
} tre_pos_and_tags_t;

#endif /* TRE_COMPILE_H */

/* EOF */

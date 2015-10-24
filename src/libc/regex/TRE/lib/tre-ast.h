/*
  tre-ast.h - Abstract syntax tree (AST) definitions

  This software is released under a BSD-style license.
  See the file LICENSE for details and copyright.

*/


#ifndef TRE_AST_H
#define TRE_AST_H 1

#include <limits.h>

#include "tre-mem.h"
#include "tre-internal.h"
#include "tre-compile.h"
#include "tre-last-matched.h"

/* The different AST node types. */
typedef enum {
  LITERAL,
  CATENATION,
  ITERATION,
  UNION
} tre_ast_type_t;

/* Special subtypes of TRE_LITERAL. */
#define EMPTY	  -1   /* Empty leaf (denotes empty string). */
#define ASSERTION -2   /* Assertion leaf. */
#define TAG	  -3   /* Tag leaf. */
#define BACKREF	  -4   /* Back reference leaf. */
#define PARAMETER -5   /* Parameter. */

#define IS_SPECIAL(x)	((x)->code_min < 0)
#define IS_EMPTY(x)	((x)->code_min == EMPTY)
#define IS_ASSERTION(x) ((x)->code_min == ASSERTION)
#define IS_TAG(x)	((x)->code_min == TAG)
#define IS_BACKREF(x)	((x)->code_min == BACKREF)
#define IS_PARAMETER(x) ((x)->code_min == PARAMETER)

#define SUBMATCH_ID_INVISIBLE_START	(INT_MAX / 2 + 1)


/* A generic AST node.  All AST nodes consist of this node on the top
   level with `obj' pointing to the actual content. */
typedef struct _tre_ast_node {
  void *obj;             /* Pointer to actual node. */
  tre_last_matched_branch_pre_t *last_matched_branch;
  tre_last_matched_pre_t *last_matched_in_progress;
  tre_pos_and_tags_t *firstpos;
  tre_pos_and_tags_t *lastpos;
  /* The original pointer is used to point to the original node, when a
   * CATENATION and tag are added.  If NULL, this is node is the original */
  struct _tre_ast_node *original;
  tre_ast_type_t type;   /* Type of the node. */
  int submatch_id;
  int num_submatches;
  int num_tags;
  short nullable;
  short make_branches;
} tre_ast_node_t;


/* A "literal" node.  These are created for assertions, back references,
   tags, matching parameter settings, and all expressions that match one
   character. */
typedef struct {
  tre_cint_t code_min;
  tre_cint_t code_max;
  int position;
  union {
    tre_bracket_match_list_t *bracket_match_list;
    int *params;
  } u;
} tre_literal_t;

/* A "catenation" node.	 These are created when two regexps are concatenated.
   If there are more than one subexpressions in sequence, the `left' part
   holds all but the last, and `right' part holds the last subexpression
   (catenation is left associative). */
typedef struct {
  tre_ast_node_t *left;
  tre_ast_node_t *right;
} tre_catenation_t;

/* An "iteration" node.	 These are created for the "*", "+", "?", and "{m,n}"
   operators. */
typedef struct {
  /* Subexpression to match. */
  tre_ast_node_t *arg;
  /* Minimum number of consecutive matches. */
  int min;
  /* Maximum number of consecutive matches. */
  int max;
  /* If 0, match as many characters as possible, if 1 match as few as
     possible.	Note that this does not always mean the same thing as
     matching as many/few repetitions as possible. */
  unsigned int minimal:1;
  /* Approximate matching parameters (or NULL). */
  int *params;
} tre_iteration_t;

/* An "union" node.  These are created for the "|" operator. */
typedef struct {
  tre_ast_node_t *left;
  tre_ast_node_t *right;
  /* The left end right end tags if non-zero */
  int left_tag;
  int right_tag;
} tre_union_t;

__private_extern__ tre_ast_node_t *
tre_ast_new_node(tre_mem_t mem, tre_ast_type_t type, size_t size);

__private_extern__ tre_ast_node_t *
tre_ast_new_literal(tre_mem_t mem, int code_min, int code_max, int position);

__private_extern__ tre_ast_node_t *
tre_ast_new_iter(tre_mem_t mem, tre_ast_node_t *arg, int min, int max,
		 int minimal);

__private_extern__ tre_ast_node_t *
tre_ast_new_union(tre_mem_t mem, tre_ast_node_t *left, tre_ast_node_t *right);

__private_extern__ tre_ast_node_t *
tre_ast_new_catenation(tre_mem_t mem, tre_ast_node_t *left,
		       tre_ast_node_t *right);

#ifdef TRE_DEBUG
void
tre_ast_print(tre_ast_node_t *tree);

/* XXX - rethink AST printing API */
void
tre_print_params(int *params);
#endif /* TRE_DEBUG */

#endif /* TRE_AST_H */

/* EOF */

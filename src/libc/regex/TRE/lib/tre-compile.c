/*
  tre-compile.c - TRE regex compiler

  This software is released under a BSD-style license.
  See the file LICENSE for details and copyright.

*/

/*
  TODO:
   - Fix tre_ast_to_tnfa() to recurse using a stack instead of recursive
     function calls.
*/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>

#include "tre-internal.h"
#include "tre-mem.h"
#include "tre-stack.h"
#include "tre-ast.h"
#include "tre-parse.h"
#include "tre-compile.h"
#include "tre.h"
#include "tre-last-matched.h"
#include "xmalloc.h"

/*
  The bit_ffs() macro in bitstring.h is flawed.  Replace it with a working one.
*/
#undef bit_ffs
#define	bit_ffs(name, nbits, value) { \
	register bitstr_t *_name = name; \
	register int _byte, _nbits = nbits; \
	register int _stopbyte = _bit_byte(_nbits), _value = -1; \
	for (_byte = 0; _byte <= _stopbyte; ++_byte) \
		if (_name[_byte]) { \
			_value = _byte << 3; \
			for (_stopbyte = _name[_byte]; !(_stopbyte&0x1); \
			    ++_value, _stopbyte >>= 1); \
			break; \
		} \
	*(value) = _value; \
}

/*
  Algorithms to setup tags so that submatch addressing can be done.
*/


#ifdef TRE_DEBUG
static const char *tag_dir_str[] = {
  "minimize",
  "maximize",
  "left-maximize"
};

static const char _indent[] = "  ";

static void
print_indent(int indent)
{
  while (indent-- > 0)
    DPRINT((_indent));
}

static void print_last_matched_pre(tre_last_matched_pre_t *lm, int indent,
				   int num_tags);
static void
print_last_match_branch_pre(tre_last_matched_branch_pre_t *branch, int indent,
			    int num_tags)
{
  tre_last_matched_pre_t *u = branch->last_matched;
  int n_last_matched = 0;

  while (u)
    {
      n_last_matched++;
      u = u->next;
    }

  print_indent(indent);
  DPRINT(("BRANCH: tot_branches=%d tot_last_matched=%d tot_tags=%d\n",
	  branch->tot_branches, branch->tot_last_matched, branch->tot_tags));
  print_indent(indent);
  DPRINT(("..n_last_matched=%d last_matched=%d\n", branch->n_last_matched,
	  n_last_matched));
  if (branch->n_last_matched != n_last_matched)
    DPRINT(("*** mismatch between n_last_matched and unions ***\n"));
  if (branch->cmp_tag > 0)
    {
      int i;
      const char *sep = " tags=";
      print_indent(indent);
      DPRINT(("..cmp_tag=%d n_tags=%d", branch->cmp_tag, branch->n_tags));
      for (i = 0; i < num_tags; i++)
	if (bit_test(branch->tags, i))
	  {
	    DPRINT(("%s%d", sep, i));
	    sep = ",";
	  }
      DPRINT(("\n"));
    }

  u = branch->last_matched;
  indent++;
  while (u)
    {
      print_last_matched_pre(u, indent, num_tags);
      u = u->next;
    }
}

static void
print_last_matched_pre(tre_last_matched_pre_t *lm, int indent, int num_tags)
{
  tre_last_matched_branch_pre_t *b = lm->branches;
  int n_branches = 0;

  while (b)
    {
      n_branches++;
      b = b->next;
    }

  print_indent(indent);
  DPRINT(("LAST_MATCHED: tot_branches=%d tot_last_matched=%d tot_tags=%d\n",
	  lm->tot_branches, lm->tot_last_matched, lm->tot_tags));
  print_indent(indent);
  DPRINT(("..start_tag=%d n_branches=%d branches=%d\n", lm->start_tag,
	  lm->n_branches, n_branches));
  if (lm->n_branches != n_branches)
    DPRINT(("*** mismatch between n and branches ***\n"));

  b = lm->branches;
  indent++;
  while (b)
    {
      print_last_match_branch_pre(b, indent, num_tags);
      b = b->next;
    }
}

static void print_last_matched(tre_last_matched_t *lm, int indent);
static void
print_last_match_branch(tre_last_matched_branch_t *branch, int indent)
{
  tre_last_matched_t *u;
  int i;

  print_indent(indent);
  DPRINT(("BRANCH: n_last_matched=%d\n", branch->n_last_matched));
  if (branch->cmp_tag > 0)
    {
      print_indent(indent);
      DPRINT(("..cmp_tag=%d n_tags=%d", branch->cmp_tag, branch->n_tags));
      if (branch->n_tags > 0)
	{
	  const char *sep = " tags=";
	  for (i = 0; i < branch->n_tags; i++)
	    {
	      DPRINT(("%s%d", sep, branch->tags[i]));
	      sep = ",";
	    }
	}
      DPRINT(("\n"));
    }

  u = branch->last_matched;
  indent++;
  for (i = branch->n_last_matched; i > 0; i--, u++)
    print_last_matched(u, indent);
}

static void
print_last_matched(tre_last_matched_t *lm, int indent)
{
  int i;
  tre_last_matched_branch_t *b;

  print_indent(indent);
  DPRINT(("LAST_MATCHED: n_branches=%d start_tag=%d\n", lm->n_branches,
	  lm->start_tag));

  b = lm->branches;
  indent++;
  for (i = lm->n_branches; i > 0; i--, b++)
    print_last_match_branch(b, indent);
}
#endif /* TRE_DEBUG */


/* Merge the tre_last_matched_branch_pre_t of src into dst, creating a new
   one if needed.  If tag_id > 0, add that tag as well (a negative tag_id will
   create an unset tre_last_matched_branch_pre_t. */
static reg_errcode_t
tre_merge_branches(tre_mem_t mem, tre_ast_node_t *dst, tre_ast_node_t *src,
		   int tag_id, int num_tags)
{
  tre_last_matched_branch_pre_t *db = dst->last_matched_branch;
  tre_last_matched_branch_pre_t *sb = (src ? src->last_matched_branch : NULL);

  if (db)
    {
      if (sb)
	{
	  bitstr_t *l = db->tags;
	  bitstr_t *r = sb->tags;
	  int i = bitstr_size(num_tags);

	  while(i-- > 0)
	    *l++ |= *r++;
	  /* db and sb are the info from two parallel sub-trees, so the tags
	     must be mutually exclusive, and we can just add their numbers */
	  db->n_tags += sb->n_tags;
	  db->tot_tags += sb->tot_tags;
	  if (db->last_matched)
	    {
	      if (sb->last_matched)
		{
		  tre_last_matched_pre_t *u = db->last_matched;

		  while(u->next)
		    u = u->next;
		  u->next = sb->last_matched;
		  db->n_last_matched += sb->n_last_matched;
		  db->tot_branches += sb->tot_branches;
		  db->tot_last_matched += sb->tot_last_matched;
		}
	    }
	    else if (sb->last_matched)
	      {
		db->last_matched = sb->last_matched;
		db->n_last_matched = sb->n_last_matched;
		db->tot_branches = sb->tot_branches;
		db->tot_last_matched = sb->tot_last_matched;
	      }
	}
    }
  else
    db = sb;

  if (tag_id != 0)
    {
      if (!db)
	{
	  db = tre_mem_calloc(mem, sizeof(tre_last_matched_branch_pre_t)
			      + bitstr_size(num_tags));
	  if (db == NULL)
	    return REG_ESPACE;
	  db->tot_branches = 1;
	}
      if (tag_id > 0)
	{
	  /* tag_id is a new tag, and shouldn't exist in db's tags,
	     so we can always increment n_tags */
	  bit_set(db->tags, tag_id);
	  db->n_tags++;
	  db->tot_tags++;
	}
    }
  dst->last_matched_branch = db;
  return REG_OK;
}


/* Inserts a catenation node to the root of the tree given in `node'.
   As the left child a new tag with number `tag_id' to `node' is added,
   and the right child is the old root. */
static reg_errcode_t
tre_add_tag_left(tre_mem_t mem, tre_ast_node_t *node, int tag_id)
{
  tre_catenation_t *c;

  DPRINT(("add_tag_left: tag %d\n", tag_id));

  c = tre_mem_alloc(mem, sizeof(*c));
  if (c == NULL)
    return REG_ESPACE;
  c->left = tre_ast_new_literal(mem, TAG, tag_id, -1);
  if (c->left == NULL)
    return REG_ESPACE;
  c->right = tre_mem_calloc(mem, sizeof(tre_ast_node_t));
  if (c->right == NULL)
    return REG_ESPACE;

  c->right->obj = node->obj;
  c->right->type = node->type;
  c->right->last_matched_branch = node->last_matched_branch;
  c->right->nullable = -1;
  c->right->submatch_id = -1;
  node->obj = c;
  node->type = CATENATION;
  node->original = c->right;
  return REG_OK;
}

/* Inserts a catenation node to the root of the tree given in `node'.
   As the right child a new tag with number `tag_id' to `node' is added,
   and the left child is the old root. */
static reg_errcode_t
tre_add_tag_right(tre_mem_t mem, tre_ast_node_t *node, int tag_id)
{
  tre_catenation_t *c;

  DPRINT(("tre_add_tag_right: tag %d\n", tag_id));

  c = tre_mem_alloc(mem, sizeof(*c));
  if (c == NULL)
    return REG_ESPACE;
  c->right = tre_ast_new_literal(mem, TAG, tag_id, -1);
  if (c->right == NULL)
    return REG_ESPACE;
  c->left = tre_mem_calloc(mem, sizeof(tre_ast_node_t));
  if (c->left == NULL)
    return REG_ESPACE;

  c->left->obj = node->obj;
  c->left->type = node->type;
  c->left->last_matched_branch = node->last_matched_branch;
  c->left->nullable = -1;
  c->left->submatch_id = -1;
  node->obj = c;
  node->type = CATENATION;
  node->original = c->left;
  return REG_OK;
}

typedef enum {
  ADDTAGS_RECURSE,
  ADDTAGS_RECURSE_NOT_TOP_UNION,
  ADDTAGS_AFTER_ITERATION,
  ADDTAGS_AFTER_UNION_LEFT,
  ADDTAGS_AFTER_UNION_RIGHT,
  ADDTAGS_AFTER_CAT_LEFT,
  ADDTAGS_AFTER_CAT_RIGHT,
  ADDTAGS_SET_SUBMATCH_END,
  ADDTAGS_UNION_RECURSE,
  ADDTAGS_UNION_RIGHT_RECURSE,
  ADDTAGS_AFTER_UNION_TOP,
} tre_addtags_symbol_t;

enum {
  COPY_LAST_MATCHED_BRANCH,
  COPY_LAST_MATCHED_BRANCH_NEXT,
  COPY_LAST_MATCHED,
  COPY_LAST_MATCHED_NEXT,
};


#define REGSET_UNSET		((unsigned)-1)

/* Go through `regset' and set submatch data for submatches that are
   using this tag. */
static void
tre_purge_regset(unsigned *regset, tre_tnfa_t *tnfa, int tag)
{
  int i;

  for (i = 0; regset[i] != REGSET_UNSET; i++)
    {
      int id = regset[i] / 2;
      int start = !(regset[i] % 2);
      if (id >= SUBMATCH_ID_INVISIBLE_START)
	continue;
      DPRINT(("  Using tag %d for %s offset of "
	      "submatch %d\n", tag,
	      start ? "start" : "end", id));
      if (start)
	tnfa->submatch_data[id].so_tag = tag;
      else
	tnfa->submatch_data[id].eo_tag = tag;
    }
  regset[0] = -1;
}


#define REGSET_HAS_STARTS	0x1
#define REGSET_HAS_ENDS		0x2


/* Adds tags to appropriate locations in the parse tree in `tree', so that
   subexpressions marked for submatch addressing can be traced. */
static reg_errcode_t
tre_add_tags(tre_mem_t mem, tre_stack_t *stack, tre_ast_node_t *tree,
	     tre_tnfa_t *tnfa)
{
  reg_errcode_t status = REG_OK;
  tre_addtags_symbol_t symbol;
  tre_ast_node_t *node = tree; /* Tree node we are currently looking at. */
  int bottom = tre_stack_num_objects(stack);
  /* True for first pass (counting number of needed tags) */
  int first_pass = (mem == NULL || tnfa == NULL);
  unsigned *regset, *orig_regset;
  int regset_contains = 0;
  int num_tags = 0; /* Total number of tags. */
  int num_minimals = 0;	 /* Number of special minimal tags. */
  int tag = 0;	    /* The tag that is to be added next. */
  int next_tag = 1; /* Next tag to use after this one. */
  int minimal_tag = -1; /* Tag that marks the beginning of a minimal match. */
  int *reorder_tags = NULL; /* Tag reorder array: a pair for each reorder,
		             * the first is the tag to reorder, the second
		             * is the tag after which the first is reordered */
  int *rtp;		    /* Pointer used to fill in reorder_tags and
			     * tag_order */
  int *to_reorder;          /* Transform array converting sequential order to
		             * that specified by reorder_tags */
  int id;

  tre_tag_direction_t direction = TRE_TAG_LEFT_MAXIMIZE;
  if (!first_pass)
    {
      DPRINT(("Initializing direction to %s\n", tag_dir_str[direction]));
      tnfa->end_tag = 0;
      tnfa->minimal_tags[0] = -1;
    }

  regset = xmalloc(sizeof(*regset) * ((tnfa->num_submatches
		   + tnfa->num_submatches_invisible + 1) * 2));
  if (regset == NULL)
    {
      status = REG_ESPACE;
      goto error_regset;
    }
  regset[0] = REGSET_UNSET;
  orig_regset = regset;

  if (!first_pass)
    {
      /* Allocate all memory for reorder_tags, tag_order, to_seq_order and
       * to_reorder in one batch (assuming all are the same type) */
      rtp = reorder_tags = xmalloc(sizeof(*reorder_tags) *
				   ((2 * tnfa->num_reorder_tags + 1) +
				   tnfa->num_tags));
      if (reorder_tags == NULL)
	{
	  status = REG_ESPACE;
	  goto error_reorder_tags;
	}
      to_reorder = reorder_tags + (2 * tnfa->num_reorder_tags + 1);
    }

  STACK_PUSH(stack, voidptr, node);
  STACK_PUSH(stack, int, ADDTAGS_RECURSE);

  while (tre_stack_num_objects(stack) > bottom)
    {
      if (status != REG_OK)
	break;

      symbol = (tre_addtags_symbol_t)tre_stack_pop_int(stack);
      switch (symbol)
	{
	  int top_union;

	case ADDTAGS_SET_SUBMATCH_END:
	  {
	    int i;

	    id = tre_stack_pop_int(stack);
	    node = tre_stack_pop_voidptr(stack);
	    /* Add end of this submatch to regset. */
	    for (i = 0; regset[i] != REGSET_UNSET; i++);
	    regset[i] = id * 2 + 1;
	    regset[i + 1] = -1;
	    regset_contains |= REGSET_HAS_ENDS;

	    /* Always put a tag after a minimal iterator. */
	    if (minimal_tag >= 0)
	      {
		if (first_pass)
		  {
		    node->num_tags++;
		    DPRINT(("  ADDTAGS_SET_SUBMATCH_END: node->num_tags = %d\n",
			    node->num_tags));
		  }
		else
		  {
		    int i;
		    status = tre_merge_branches(mem, node, NULL, tag,
						tnfa->num_tags);
		    if (status != REG_OK)
		      break;
		    status = tre_add_tag_right(mem, node, tag);
		    if (status != REG_OK)
		      break;
		    tnfa->tag_directions[tag] = TRE_TAG_MINIMIZE;
		    DPRINT(("Setting t%d direction to %s\n", tag,
			    tag_dir_str[tnfa->tag_directions[tag]]));
		    DPRINT(("Minimal %d, %d\n", minimal_tag, tag));
		    for (i = 0; tnfa->minimal_tags[i] >= 0; i++);
		    tnfa->minimal_tags[i] = tag;
		    tnfa->minimal_tags[i + 1] = minimal_tag;
		    tnfa->minimal_tags[i + 2] = -1;

		    DPRINT(("  Minimal end: t%d reordered to "
			    "after t%d\n", tag, minimal_tag));
		    /* Append to tag_order, move "tag" after
		     * "minimal_tag" */
		    *rtp++ = tag;
		    *rtp++ = minimal_tag;

		    num_minimals++;
		    tre_purge_regset(regset, tnfa, tag);
		  }

		minimal_tag = -1;
		DPRINT(("  ADDTAGS_SET_SUBMATCH_END num_tags++ tag=%d\n", tag));
		regset[0] = REGSET_UNSET;
		regset_contains = 0;
		tag = next_tag;
		num_tags++;
		next_tag++;
	      }
	    break;
	  }

	case ADDTAGS_RECURSE_NOT_TOP_UNION:
	  /* Like ADDTAGS_RECURSE, except that top_union is set to zero,
	   * indicating that if a union is being processed, it is not the
	   * top-most of a series */
	  top_union = 0;
	  goto do_addtags_recurse;

	case ADDTAGS_RECURSE:
	  /* Setting top_union to 1 means that if a union is begin processed,
	   * it is the top-most of a series, and should recurse through the
	   * series to set the left_tag and right_tag values */
	  top_union = 1;

do_addtags_recurse:
	  node = tre_stack_pop_voidptr(stack);

	  id = node->submatch_id;
	  if (id >= 0)
	    {
	      int i;


	      /* Add start of this submatch to regset. */
	      for (i = 0; regset[i] != REGSET_UNSET; i++);
	      regset[i] = id * 2;
	      regset[i + 1] = -1;
	      regset_contains |= REGSET_HAS_STARTS;

	      /* Add end of this submatch to regset after processing this
		 node. */
	      STACK_PUSH(stack, voidptr, node);
	      STACK_PUSHX(stack, int, id);
	      STACK_PUSHX(stack, int, ADDTAGS_SET_SUBMATCH_END);
	    }

	  switch (node->type)
	    {
	    case LITERAL:
	      {
		tre_literal_t *lit = node->obj;

		if (!IS_SPECIAL(lit) || IS_BACKREF(lit) || IS_EMPTY(lit) || IS_ASSERTION(lit))
		  {
		    DPRINT(("Literal %d-%d\n",
			    (int)lit->code_min, (int)lit->code_max));
		    if (regset_contains)
		      {
			/* Regset is not empty, so add a tag before the
			   literal or backref. */
			if (first_pass)
			  {
			    DPRINT(("  ADDTAGS_RECURSE:LITERAL node->num_tags = 1\n"));
			    node->num_tags = 1;
			  }
			else
			  {
			    status = tre_merge_branches(mem, node, NULL, tag,
							tnfa->num_tags);
			    if (status != REG_OK)
			      break;
			    status = tre_add_tag_left(mem, node, tag);
			    if (status != REG_OK)
			      break;
			    if (regset_contains == REGSET_HAS_STARTS)
			      tnfa->tag_directions[tag] = TRE_TAG_LEFT_MAXIMIZE;
			    else
			      tnfa->tag_directions[tag] = direction;
			    DPRINT(("Setting t%d direction to %s\n", tag,
				    tag_dir_str[tnfa->tag_directions[tag]]));
			    tre_purge_regset(regset, tnfa, tag);

			    if (IS_BACKREF(lit))
			      {
				int b = lit->code_max;
				int t = tnfa->submatch_data[b].so_tag;
				/* Fail if the referenced submatch hasn't been
				 * completed yet */
				if (tnfa->submatch_data[b].eo_tag < 0)
				  {
				    status = REG_ESUBREG;
				    break;
				  }
				if (t < tag)
				  {
				    DPRINT(("  Backref %d start: "
					    "t%d reordered to before t%d\n",
					    b, tag, t));
				    if(t > 0)
				      t--;
				    /* Append to tag_order, move "tag" after
				     * "t" */
				    *rtp++ = tag;
				    *rtp++ = t;
				  }
#if TRE_DEBUG
				else
				  DPRINT(("  Backref %d start: "
					  "(t%d already before t%d)\n",
					    b, tag, t));
#endif /* TRE_DEBUG */
			      }
			  }

			DPRINT(("  ADDTAGS_RECURSE:LITERAL num_tags++ tag=%d\n",
				tag));
			regset[0] = REGSET_UNSET;
			regset_contains = 0;
			tag = next_tag;
			num_tags++;
			next_tag++;
		      }
		  }
		else
		  {
		    assert(!IS_TAG(lit));
		  }
		break;
	      }
	    case CATENATION:
	      {
		tre_catenation_t *cat = node->obj;
		tre_ast_node_t *left = cat->left;
		tre_ast_node_t *right = cat->right;
		int reserved_tag = -1;
		DPRINT(("Catenation, next_tag = %d\n", next_tag));


		/* After processing right child. */
		STACK_PUSHX(stack, voidptr, node);
		STACK_PUSHX(stack, int, ADDTAGS_AFTER_CAT_RIGHT);

		/* Process right child. */
		STACK_PUSHX(stack, voidptr, right);
		STACK_PUSHX(stack, int, ADDTAGS_RECURSE);

		/* After processing left child. */
		STACK_PUSHX(stack, int, next_tag + left->num_tags);
		DPRINT(("  Pushing %d for after left\n",
			next_tag + left->num_tags));
		if (left->num_tags > 0 && right->num_tags > 0)
		  {
		    /* Reserve the next tag to the right child. */
		    DPRINT(("  ADDTAGS_RECURSE:CATENATION num_tags++ "
			    "Reserving next_tag %d to right child\n",
			    next_tag));
		    reserved_tag = next_tag;
		    next_tag++;
		  }
		STACK_PUSHX(stack, int, reserved_tag);
		STACK_PUSHX(stack, int, ADDTAGS_AFTER_CAT_LEFT);

		/* Process left child. */
		STACK_PUSHX(stack, voidptr, left);
		STACK_PUSHX(stack, int, ADDTAGS_RECURSE);

		}
	      break;
	    case ITERATION:
	      {
		tre_iteration_t *iter = node->obj;
		DPRINT(("Iteration\n"));

		if (first_pass)
		  STACK_PUSHX(stack, int, regset_contains != 0);
		STACK_PUSHX(stack, int, tag);
		STACK_PUSHX(stack, voidptr, node);
		STACK_PUSHX(stack, int, ADDTAGS_AFTER_ITERATION);

		STACK_PUSHX(stack, voidptr, iter->arg);
		STACK_PUSHX(stack, int, ADDTAGS_RECURSE);

		/* Regset is not empty, so add a tag here (this always happens
		   because iterators always get submatch id, even if in the
		   invisible range) */
		if (regset_contains)
		  {
		    if (!first_pass)
		      {
			status = tre_merge_branches(mem, node, NULL, tag,
						    tnfa->num_tags);
			if (status != REG_OK)
			  break;
			status = tre_add_tag_left(mem, node, tag);
			if (status != REG_OK)
			  break;
			if (regset_contains == REGSET_HAS_STARTS && tag != 0)
			  tnfa->tag_directions[tag] = iter->minimal ?
						      TRE_TAG_MINIMIZE :
						      TRE_TAG_LEFT_MAXIMIZE;
			else
			  tnfa->tag_directions[tag] = direction;
			DPRINT(("Setting t%d direction to %s\n", tag,
				tag_dir_str[tnfa->tag_directions[tag]]));
			tre_purge_regset(regset, tnfa, tag);
		      }

		    DPRINT(("  ADDTAGS_RECURSE:ITERATION num_tags++ tag=%d\n",
			    tag));
		    regset[0] = REGSET_UNSET;
		    regset_contains = 0;
		    tag = next_tag;
		    num_tags++;
		    next_tag++;
		  }
		direction = TRE_TAG_LEFT_MAXIMIZE;
		DPRINT(("  Setting direction to %s\n", tag_dir_str[direction]));
	      }
	      break;
	    case UNION:
	      {
		tre_union_t *uni;
		tre_ast_node_t *left;
		tre_ast_node_t *right;
		int front_tag = -1;

		DPRINT(("Union\n"));

		if (regset_contains)
		  {
		    DPRINT(("  UNION num_tags++ tag=%d\n", tag));
		    front_tag = tag;
		    tag = next_tag;
		    num_tags++;
		    next_tag++;
		  }

		/* For the top union, walk the tree of consecutive unions,
		 * setting the left_tag and right_tag values in increasing
		 * order (left to right priority) */
		if (top_union &&
		    (node->num_submatches -
		    (node->submatch_id >= 0 &&
		    node->submatch_id < SUBMATCH_ID_INVISIBLE_START)) > 0)
		  {
		    tre_ast_node_t *n;
		    int last = tre_stack_num_objects(stack);

		    STACK_PUSH(stack, voidptr, node);
		    STACK_PUSH(stack, int, ADDTAGS_UNION_RECURSE);

		    while (tre_stack_num_objects(stack) > last)
		      {
			symbol = (tre_addtags_symbol_t)tre_stack_pop_int(stack);
			switch (symbol)
			  {
			  case ADDTAGS_UNION_RECURSE:
			    n = tre_stack_pop_voidptr(stack);
			    uni = n->obj;
			    left = uni->left;

			    /* Since the top union has num_submatches > 0,
			     * we set all the consecutive union's
			     * make_branches to 1 to force the generation
			     * of end tags for each union branch. */
			    n->make_branches = 1;

			    STACK_PUSH(stack, voidptr, n);
			    STACK_PUSH(stack, int,
				       ADDTAGS_UNION_RIGHT_RECURSE);

			    if (left->type == UNION)
			      {
				STACK_PUSH(stack, voidptr, left);
				STACK_PUSH(stack, int,
					   ADDTAGS_UNION_RECURSE);
			      }
			    else
			      {
				DPRINT(("  ADDTAGS_UNION_RECURSE "
					"num_tags++ tag=%d\n", tag));
				uni->left_tag = tag;
				tag = next_tag;
				num_tags++;
				next_tag++;
			      }
			    break;

			  case ADDTAGS_UNION_RIGHT_RECURSE:
			    n = tre_stack_pop_voidptr(stack);
			    uni = n->obj;
			    right = uni->right;

			    if (right->type == UNION)
			      {
				STACK_PUSH(stack, voidptr, right);
				STACK_PUSH(stack, int,
					   ADDTAGS_UNION_RECURSE);
			      }
			    else
			      {
				DPRINT(("  ADDTAGS_UNION_RIGHT_RECURSE "
					"num_tags++ tag=%d\n", tag));
				uni->right_tag = tag;
				tag = next_tag;
				num_tags++;
				next_tag++;
			      }

			    break;

			  default:
			    assert(0);
			    break;

			  } /* end switch(symbol) */
		      } /* end while(tre_stack_num_objects(stack) > last */
		    if (!first_pass)
		      {
			STACK_PUSHX(stack, int, front_tag);
			STACK_PUSHX(stack, voidptr, node);
			STACK_PUSHX(stack, int, ADDTAGS_AFTER_UNION_TOP);
		      }
		  } /* end if (top_union && ...) */

		uni = node->obj;
		left = uni->left;
		right = uni->right;

		/* After processing right child. */
		STACK_PUSHX(stack, voidptr, regset);
		STACK_PUSHX(stack, int, regset_contains != 0);
		STACK_PUSHX(stack, voidptr, node);
		STACK_PUSHX(stack, int, ADDTAGS_AFTER_UNION_RIGHT);

		/* Process right child. */
		STACK_PUSHX(stack, voidptr, right);
		STACK_PUSHX(stack, int, ADDTAGS_RECURSE_NOT_TOP_UNION);

		/* After processing left child. */
		STACK_PUSHX(stack, int, ADDTAGS_AFTER_UNION_LEFT);

		/* Process left child. */
		STACK_PUSHX(stack, voidptr, left);
		STACK_PUSHX(stack, int, ADDTAGS_RECURSE_NOT_TOP_UNION);

		/* Regset is not empty, so add a tag here. */
		if (regset_contains)
		  {
		    if (!first_pass)
		      {
			status = tre_merge_branches(mem, node, NULL, front_tag,
						    tnfa->num_tags);
			if (status != REG_OK)
			  break;
			status = tre_add_tag_left(mem, node, front_tag);
			if (status != REG_OK)
			  break;
			if (regset_contains == REGSET_HAS_STARTS)
			  tnfa->tag_directions[front_tag] = TRE_TAG_LEFT_MAXIMIZE;
			else
			  tnfa->tag_directions[front_tag] = direction;
			DPRINT(("Setting t%d direction to %s\n", front_tag,
				tag_dir_str[tnfa->tag_directions[front_tag]]));
			tre_purge_regset(regset, tnfa, front_tag);
		      }

		    regset[0] = REGSET_UNSET;
		    regset_contains = 0;
		  }

		break;
	      }
	    } /* end switch (node->type) */

	  break; /* end case: ADDTAGS_RECURSE */

	case ADDTAGS_AFTER_ITERATION:
	  {
	    tre_iteration_t *iter;
	    tre_ast_node_t *orig;
	    int enter_tag;

	    node = tre_stack_pop_voidptr(stack);
	    orig = node->original ? node->original : node;
	    iter = (tre_iteration_t *)orig->obj;
	    enter_tag = tre_stack_pop_int(stack);
	    if (iter->minimal)
	      minimal_tag = enter_tag;

	    DPRINT(("After iteration\n"));
	    if (first_pass)
	      {
		node->num_tags = iter->arg->num_tags + tre_stack_pop_int(stack);
		DPRINT(("  ADDTAGS_AFTER_ITERATION: node->num_tags = %d\n",
			node->num_tags));
	      }
	    else
	      {
		/* node->last_matched_branch will have the start tag (the tag
		   just *before* the iteration).  iter->arg->last_matched_branch
		   will have the tag(s) inside the iteration, the ones that
		   may need to be reset if the iteration doesn't match.  So
		   before we merge iter->arg into node, we need to set up
		   a new tre_last_matched_t and tre_last_matched_branch_t,
		   using any of the inside tags as cmp_tag (we choose the first
		   tag found by bit_ffs).  If there are no inside tags, we
		   don't bother creating the extra structures. */
		tre_last_matched_branch_pre_t *b =
						iter->arg->last_matched_branch;

		if (b && b->n_tags > 0)
		  {
		    tre_last_matched_pre_t *u;

		    bit_ffs(b->tags, num_tags, &b->cmp_tag);
		    DPRINT(("  ADDTAGS_AFTER_ITERATION: n_tags=%d "
			    "cmp_tag = %d\n", b->n_tags, b->cmp_tag));

		    u = tre_mem_calloc(mem, sizeof(tre_last_matched_pre_t) +
				       sizeof(tre_last_matched_branch_pre_t)
				       + bitstr_size(num_tags));
		    if (!u)
		      {
			status = REG_ESPACE;
			break;
		      }
		    u->branches = b;
		    u->n_branches = 1;
		    u->start_tag = b->cmp_tag;
		    u->tot_branches = b->tot_branches;
		    u->tot_last_matched = 1 + b->tot_last_matched;
		    u->tot_tags = b->tot_tags;

		    b = (tre_last_matched_branch_pre_t *)(u + 1);
		    b->last_matched = u;
		    b->n_last_matched = 1;
		    b->tot_branches = 1 + u->tot_branches;
		    b->tot_last_matched = u->tot_last_matched;
		    b->tot_tags = u->tot_tags;

		    iter->arg->last_matched_branch = b;
		  }
		status = tre_merge_branches(mem, node, iter->arg, 0,
					    tnfa->num_tags);
		if (status != REG_OK)
		  break;

		if (iter->minimal)
		  {
		    /* Add a union with a left EMPTY literal and the right
		       being iter->arg.  This should force the tags inside
		       the minimal iteration to prefer being unset */
		    if (iter->min == 0 && iter->max <= 1)
		      {
			tre_ast_node_t *u, *e;

			e = tre_ast_new_literal(mem, EMPTY, -1, -1);
			if (e == NULL)
			  {
			    status = REG_ESPACE;
			    break;
			  }
			u = tre_ast_new_union(mem, e, iter->arg);
			if (u == NULL)
			  {
			    status = REG_ESPACE;
			    break;
			  }
			iter->arg = u;
		      }

		    direction = TRE_TAG_MINIMIZE;
		  }
		else
		  direction = TRE_TAG_MAXIMIZE;
		DPRINT(("  Setting direction to %s\n", tag_dir_str[direction]));
	      }
	    break;
	  }

	case ADDTAGS_AFTER_CAT_LEFT:
	  {
	    int new_tag = tre_stack_pop_int(stack);
	    next_tag = tre_stack_pop_int(stack);
	    DPRINT(("After cat left, tag = %d, next_tag = %d\n",
		    tag, next_tag));
	    if (new_tag >= 0)
	      {
		DPRINT(("  Setting tag to %d\n", new_tag));
		tag = new_tag;
	      }
	    break;
	  }

	case ADDTAGS_AFTER_CAT_RIGHT:
	  {
	    tre_catenation_t *cat;

	    DPRINT(("After cat right\n"));
	    node = tre_stack_pop_voidptr(stack);
	    cat = node->obj;
	    if (first_pass)
	      {
		node->num_tags = cat->left->num_tags + cat->right->num_tags;
		DPRINT(("  ADDTAGS_AFTER_CAT_RIGHT: node->num_tags = %d\n",
			node->num_tags));
	      }
	    else
	      {
		status = tre_merge_branches(mem, cat->left, cat->right, 0,
					    tnfa->num_tags);
		if (status != REG_OK)
		  break;
		status = tre_merge_branches(mem, node, cat->left, 0,
					    tnfa->num_tags);
	      }
	    break;
	  }

	case ADDTAGS_AFTER_UNION_LEFT:
	  DPRINT(("After union left\n"));
	  /* Lift the bottom of the `regset' array so that when processing
	     the right operand the items currently in the array are
	     invisible.	 The original bottom was saved at ADDTAGS_UNION and
	     will be restored at ADDTAGS_AFTER_UNION_RIGHT below. */
	  while (*regset != REGSET_UNSET)
	    regset++;
	  regset_contains = 0;
	  break;

	case ADDTAGS_AFTER_UNION_RIGHT:
	  {
	    int added_tags;
	    tre_ast_node_t *orig;
	    tre_union_t *uni;
	    /* Note: node may not be a UNION, but a CATENATION with a left
	     * tag.  So that is why we pass the original node->obj on the
	     * stack, to get the union's true values. */

	    DPRINT(("After union right\n"));
	    node = tre_stack_pop_voidptr(stack);
	    orig = node->original ? node->original : node;
	    uni = (tre_union_t *)orig->obj;
	    added_tags = tre_stack_pop_int(stack);
	    if (first_pass)
	      {
		node->num_tags = uni->left->num_tags + uni->right->num_tags
				 + added_tags;
		if (uni->left_tag > 0)
		  node->num_tags++;
		if (uni->right_tag > 0)
		  node->num_tags++;
		DPRINT(("  ADDTAGS_AFTER_UNION_RIGHT: node->num_tags = %d\n",
			node->num_tags));
	      }
	    regset = tre_stack_pop_voidptr(stack);

	    /* Add tags after both children, the left child gets a smaller
	       tag than the right child.  This guarantees that we prefer
	       the left child over the right child. */
	    /* XXX - This is not always necessary (if the children have
	       tags which must be seen for every match of that child). */
	    if (!first_pass && node->make_branches)
	      {
		tre_last_matched_branch_pre_t *lb =
		  uni->left->last_matched_branch;
		tre_last_matched_branch_pre_t *rb =
		  uni->right->last_matched_branch;
		tre_last_matched_pre_t *lu =
		  uni->left->last_matched_in_progress;
		tre_last_matched_pre_t *ru =
		  uni->right->last_matched_in_progress;
		tre_last_matched_pre_t *u;
		/* We don't need to call tre_merge_branches because these
		 * tags don't participate in submatch ranges, so don't need
		 * to be recorded.  But we do set the cmp_tag entry of the
		 * tre_last_matched_branch_pre_t, so we might call
		 * tre_merge_branches if we need to create an empty
		 * tre_last_matched_branch_pre_t. */
		if (uni->left_tag > 0)
		  {
		    DPRINT(("Setting t%d direction to maximize\n",
			    uni->left_tag));
		    status = tre_add_tag_right(mem, uni->left, uni->left_tag);
		    if (status != REG_OK)
		      break;
		    tnfa->tag_directions[uni->left_tag] = TRE_TAG_MAXIMIZE;
		    if (!lb)
		      {
			status = tre_merge_branches(mem, uni->left, NULL, -1,
						    tnfa->num_tags);
			if (status != REG_OK)
			  break;
			lb = uni->left->last_matched_branch;
		      }
		    lb->cmp_tag = uni->left_tag;
		  }
		if (uni->right_tag > 0)
		  {
		    DPRINT(("Setting t%d direction to maximize\n",
			    uni->right_tag));
		    status = tre_add_tag_right(mem, uni->right, uni->right_tag);
		    if (status != REG_OK)
		      break;
		    tnfa->tag_directions[uni->right_tag] = TRE_TAG_MAXIMIZE;
		    if (!rb)
		      {
			status = tre_merge_branches(mem, uni->right, NULL, -1,
						    tnfa->num_tags);
			if (status != REG_OK)
			  break;
			rb = uni->right->last_matched_branch;
		      }
		    rb->cmp_tag = uni->right_tag;
		  }
		/* Now merge the tre_last_matched_branch_pre_t into a
		   tre_last_matched_pre_t */
		if (lu == NULL)
		  {
		    if (ru == NULL)
		      {
			/* Create a new tre_last_matched_pre_t */
			u = tre_mem_calloc(mem, sizeof(tre_last_matched_pre_t));
			if (!u)
			  {
			    status = REG_ESPACE;
			    break;
			  }
			u->tot_last_matched = 1;

			if (lb)
			  {
			    u->branches = lb;
			    u->n_branches = 1;
			    u->tot_branches += lb->tot_branches;
			    u->tot_last_matched += lb->tot_last_matched;
			    u->tot_tags += lb->tot_tags;
			    if (rb)
			      {
				lb->next = rb;
				u->n_branches++;
				u->tot_branches += rb->tot_branches;
				u->tot_last_matched += rb->tot_last_matched;
				u->tot_tags += rb->tot_tags;
			      }
			  }
			else if (rb)
			  {
			    u->branches = rb;
			    u->n_branches = 1;
			    u->tot_branches += rb->tot_branches;
			    u->tot_last_matched += rb->tot_last_matched;
			    u->tot_tags += rb->tot_tags;
			  }
		      }
		    else
		      {
			/* Use ru, and add lb */
			u = ru;
			if (lb)
			  {
			    lb->next = u->branches;
			    u->branches = lb;
			    u->n_branches++;
			    u->tot_branches += lb->tot_branches;
			    u->tot_last_matched += lb->tot_last_matched;
			    u->tot_tags += lb->tot_tags;
			  }
		      }
		  }
		else if (ru == NULL)
		  {
		    /* Use lu, and add rb */
		    u = lu;
		    if (rb)
		      {
			rb->next = u->branches;
			u->branches = rb;
			u->n_branches++;
			u->tot_branches += rb->tot_branches;
			u->tot_last_matched += rb->tot_last_matched;
			u->tot_tags += rb->tot_tags;
		      }
		  }
		else
		  {
		    /* Merge lu and ru into lu */
		    if (lu->branches)
		      {
			if (ru->branches)
			  {
			    tre_last_matched_branch_pre_t *b = lu->branches;
			    while (b->next) b = b->next;
			    b->next = ru->branches;
			    lu->n_branches += ru->n_branches;
			  }
		      }
		    else if (ru->branches)
		      {
			lu->branches = ru->branches;
			lu->n_branches = ru->n_branches;
		      }
		    lu->tot_branches += ru->tot_branches;
		    lu->tot_last_matched += ru->tot_last_matched - 1;
		    lu->tot_tags += ru->tot_tags;
		    u = lu;
		  }
		node->last_matched_in_progress = u;
	      }
	    direction = TRE_TAG_MAXIMIZE;
	    break;
	  }

	case ADDTAGS_AFTER_UNION_TOP: /* only called when not first_pass */
	  {
	    tre_last_matched_branch_pre_t *b;
	    tre_last_matched_pre_t *u;
	    int start_tag;

	    DPRINT(("After union top\n"));
	    node = tre_stack_pop_voidptr(stack);
	    start_tag = tre_stack_pop_int(stack);
	    b = tre_mem_calloc(mem, sizeof(tre_last_matched_branch_pre_t)
			       + bitstr_size(num_tags));
	    if (!b)
	      {
		status = REG_ESPACE;
		break;
	      }

	    u = node->last_matched_in_progress;
	    u->start_tag = start_tag;
	    b->tot_branches = 1 + u->tot_branches;
	    b->tot_last_matched = u->tot_last_matched;
	    b->tot_tags = u->tot_tags;
	    b->last_matched = u;
	    b->n_last_matched = 1;
	    node->last_matched_branch = b;
	    node->last_matched_in_progress = NULL;
	    break;
	  }

	default:
	  assert(0);
	  break;

	} /* end switch(symbol) */
    } /* end while(tre_stack_num_objects(stack) > bottom) */

  if (status != REG_OK)
    {
      DPRINT(("Error during %s pass\n", first_pass ? "first" : "second"));
      goto error_post_compile;
    }

  if (!first_pass)
    {
      int i;
      if (num_tags != tnfa->num_tags)
	{
	  DPRINT(("num_tags(%d) != tnfa->num_tags(%d)\n", num_tags,
		  tnfa->num_tags));
	  status = REG_BADPAT;
	  goto error_post_compile;
	}

      tre_purge_regset(regset, tnfa, tag);
      DPRINT(("Setting t%d to %s\n", num_tags,
	      tag_dir_str[direction]));
      tnfa->tag_directions[num_tags] = direction;

      if (rtp > reorder_tags + 2 * tnfa->num_reorder_tags)
	{
	  DPRINT(("Processed %d reorder tags instead of %d\n",
		  (int)(rtp - reorder_tags) / 2, tnfa->num_reorder_tags));
	  status = REG_BADPAT;
	  goto error_post_compile;
	}
    *rtp = -1;
#if TRE_DEBUG
      if (reorder_tags[0] >= 0)
	{
	  DPRINT(("reorder_tags:\n"));
	  for (rtp = reorder_tags; *rtp >= 0;)
	    {
	      DPRINT(("%d after ", *rtp++));
	      DPRINT(("%d\n", *rtp++));
	    }
	}
	else
	  DPRINT(("No reorder_tags\n"));
#endif /* TRE_DEBUG */

      /* Initialize to_reorder */
      for (i = 0; i < num_tags; i++)
	to_reorder[i] = i;
      /* Use to_seq_order to convert reorder_tags values, and use those to
       * reorder to_reorder */
      for (rtp = reorder_tags; *rtp >= 0;)
	{
	  int j, high, low;
	  int ti = *rtp++;

	  /* Skip reordering the final tag */
	  if (ti >= num_tags)
	    {
	      DPRINT(("Skipping reorder of %d\n", ti));
	      rtp++;
	      continue;
	    }
	  /* The number of the tag to reorder */
	  high = to_reorder[ti];
	  /* Reorder after this tag */
	  low = to_reorder[*rtp++];

	  DPRINT(("ti=%d high=%d low=%d\n", ti, high, low));
	  if (low > high)
	    {
	      DPRINT(("Tag %d already before %d\n", high, low));
	      continue;
	    }
	  for (j = 0; j < num_tags; j++)
	    if (to_reorder[j] > low && to_reorder[j] < high)
	      to_reorder[j]++;
	  to_reorder[ti] = low + 1;
#ifdef TRE_DEBUG
	  DPRINT(("to_reorder=("));
	  for (j = 0; j < num_tags; j++)
	    {
	      DPRINT(("%d", to_reorder[j]));
	      if (j < num_tags - 1)
		DPRINT((","));
	    }
	  DPRINT((")\n"));
#endif /* TRE_DEBUG */
	}
      /* Determine if reordering in really necessary */
      {
	int need_reorder = 0;
	for (i = 0; i < num_tags; i++)
	  if(to_reorder[i] != i)
	    {
	      need_reorder = 1;
	      break;
	    }
	/* If need_reorder is not set, free reorder_tags, and set to NULL,
	 * indicating no reordering is needed */
	if (!need_reorder)
	  {
	    DPRINT(("Don't need to reorder\n"));
	    xfree(reorder_tags);
	    reorder_tags = NULL;
	  }
      }
    }

  if (reorder_tags)
    {
      int i;
      tre_tag_direction_t *new_tag_directions;
#if TRE_DEBUG
      DPRINT(("to_reorder:"));
      for (i = 0; i < num_tags; i++)
	DPRINT((" %d->%d", i, to_reorder[i]));
      DPRINT(("\n"));
#endif /* TRE_DEBUG */

      DPRINT(("Reordering submatch_data\n"));
      for (i = 0; i < tnfa->num_submatches; i++)
	{
#if TRE_DEBUG
	  int so = tnfa->submatch_data[i].so_tag;
	  int eo = tnfa->submatch_data[i].eo_tag;
#endif /* TRE_DEBUG */
	  tnfa->submatch_data[i].so_tag =
	    to_reorder[tnfa->submatch_data[i].so_tag];
	  tnfa->submatch_data[i].eo_tag =
	    tnfa->submatch_data[i].eo_tag < num_tags ?
	    to_reorder[tnfa->submatch_data[i].eo_tag] :
	    tnfa->submatch_data[i].eo_tag;
	  DPRINT(("pmatch[%d]: {%d, %d}->{%d, %d}\n", i, so, eo,
		  tnfa->submatch_data[i].so_tag,
		  tnfa->submatch_data[i].eo_tag));
	}

      DPRINT(("Reordering tag_directions\n"));
      /* We only allocate num_tags directions and reorder them.  The
       * num_tags-th direction (end tag) is left unchanged. */
      new_tag_directions = xmalloc(sizeof(*new_tag_directions) * num_tags);
      if (new_tag_directions == NULL)
	{
	  status = REG_ESPACE;
	  goto error_post_compile;
	}
      for (i = 0; i < num_tags; i++)
	{
	  new_tag_directions[to_reorder[i]] = tnfa->tag_directions[i];
	}
#if TRE_DEBUG
      for (i = 0; i < num_tags; i++)
	{
	  DPRINT(("t%d %s->%s\n", i,
		  tag_dir_str[tnfa->tag_directions[i]],
		  tag_dir_str[new_tag_directions[i]]));
	}
	DPRINT(("t%d %s->%s\n", num_tags,
		tag_dir_str[tnfa->tag_directions[num_tags]],
		tag_dir_str[tnfa->tag_directions[num_tags]]));
#endif /* TRE_DEBUG */
      memcpy(tnfa->tag_directions, new_tag_directions, sizeof(*new_tag_directions) * num_tags);
      xfree(new_tag_directions);

      DPRINT(("Reordering minimal_tags\n"));
      for (i = 0; tnfa->minimal_tags[i] >= 0; i++)
	tnfa->minimal_tags[i] = tnfa->minimal_tags[i] < num_tags ?
				to_reorder[tnfa->minimal_tags[i]] :
				tnfa->minimal_tags[i];

      DPRINT(("Reordering AST tags\n"));
      STACK_PUSH(stack, voidptr, tree);
      while (status == REG_OK && tre_stack_num_objects(stack) > bottom)
	{
	  node = tre_stack_pop_voidptr(stack);

	  switch (node->type)
	    {
	    case LITERAL:
	      {
		tre_literal_t *lit = (tre_literal_t *)node->obj;
		if (IS_TAG(lit))
		  lit->code_max = to_reorder[lit->code_max];
		break;
	      }

	    case UNION:
	      {
		tre_union_t *uni = (tre_union_t *)node->obj;
		STACK_PUSHX(stack, voidptr, uni->right);
		STACK_PUSHX(stack, voidptr, uni->left);
		break;
	      }

	    case CATENATION:
	      {
		tre_catenation_t *cat = (tre_catenation_t *)node->obj;
		STACK_PUSHX(stack, voidptr, cat->right);
		STACK_PUSHX(stack, voidptr, cat->left);
		break;
	      }

	    case ITERATION:
	      {
		tre_iteration_t *iter = (tre_iteration_t *)node->obj;
		STACK_PUSHX(stack, voidptr, iter->arg);
		break;
	      }

	    default:
	      assert(0);
	      break;
	    }
	}
	if (status != REG_OK)
	  {
	    DPRINT(("Error while reordering tags\n"));
	    goto error_post_compile;
	  }
    }


  if (!first_pass)
    {
      if (tree->last_matched_branch)
	{
	  tre_last_matched_branch_t *buf, *b, *bb;
	  tre_last_matched_branch_pre_t *bp;
	  tre_last_matched_t *u, *uu;
	  tre_last_matched_pre_t *up;
	  int *t;
	  int i;
#ifdef TRE_DEBUG
	  tre_last_matched_branch_t *_b;
	  tre_last_matched_t *_u;
	  int *_t;

	  DPRINT(("last_match_branch_pre:\n"));
	  print_last_match_branch_pre(tree->last_matched_branch, 0, num_tags);
#endif /* TRE_DEBUG */
	  buf = (tre_last_matched_branch_t *)xcalloc(1,
				     tree->last_matched_branch->tot_branches
				     * sizeof(tre_last_matched_branch_t) +
				     tree->last_matched_branch->tot_last_matched
				     * sizeof(tre_last_matched_t) +
				     tree->last_matched_branch->tot_tags *
				     sizeof(int));
	  if (!buf)
	    {
	      status = REG_ESPACE;
	      goto error_post_compile;
	    }

	  b = buf;
	  u = (tre_last_matched_t *)(b +
	      tree->last_matched_branch->tot_branches);
	  t = (int *)(u + tree->last_matched_branch->tot_last_matched);
#ifdef TRE_DEBUG
	  _b = b;
	  _u = u;
	  _t = t;
#endif /* TRE_DEBUG */
	  DPRINT(("Copying info_pre to info\n"));
	  STACK_PUSH(stack, voidptr, tree->last_matched_branch);
	  STACK_PUSH(stack, int, 1);
	  STACK_PUSH(stack, int, COPY_LAST_MATCHED_BRANCH);

	  while (status == REG_OK && tre_stack_num_objects(stack) > bottom)
	    {
	      switch (tre_stack_pop_int(stack))
		{
		case COPY_LAST_MATCHED_BRANCH:
		  i = tre_stack_pop_int(stack);
		  /* The tre_last_matched_branch_pre_t * is still on the
		     stack */
		  STACK_PUSHX(stack, voidptr, b);
		  STACK_PUSHX(stack, int, COPY_LAST_MATCHED_BRANCH_NEXT);
		  b += i;
		  break;

		case COPY_LAST_MATCHED_BRANCH_NEXT:
		  bb = tre_stack_pop_voidptr(stack);
		  bp = tre_stack_pop_voidptr(stack);
		  bb->n_last_matched = bp->n_last_matched;
		  bb->cmp_tag = bp->cmp_tag;
		  if (bp->n_tags > 0)
		    {
		      int n;
		      n = bb->n_tags = bp->n_tags;
		      bb->tags = t;
		      for (i = 0; i < num_tags; i++)
			if (bit_test(bp->tags, i))
			  {
			    *t++ = i;
			    if (--n <= 0)
			      break;
			  }
		    }
		  if (bp->next)
		    {
		      STACK_PUSHX(stack, voidptr, bp->next);
		      STACK_PUSHX(stack, voidptr, bb + 1);
		      STACK_PUSHX(stack, int, COPY_LAST_MATCHED_BRANCH_NEXT);
		    }
		  if (bp->n_last_matched > 0)
		    {
		      bb->last_matched = u;
		      STACK_PUSHX(stack, voidptr, bp->last_matched);
		      STACK_PUSHX(stack, int, bp->n_last_matched);
		      STACK_PUSHX(stack, int, COPY_LAST_MATCHED);
		    }
		  break;

		case COPY_LAST_MATCHED:
		  i = tre_stack_pop_int(stack);
		  /* The tre_last_matched_pre_t * is still on the stack */
		  STACK_PUSHX(stack, voidptr, u);
		  STACK_PUSHX(stack, int, COPY_LAST_MATCHED_NEXT);
		  u += i;
		  break;

		case COPY_LAST_MATCHED_NEXT:
		  uu = tre_stack_pop_voidptr(stack);
		  up = tre_stack_pop_voidptr(stack);
		  uu->n_branches = up->n_branches;
		  uu->branches = b;
		  uu->start_tag = up->start_tag;
		  if (up->next)
		    {
		      STACK_PUSHX(stack, voidptr, up->next);
		      STACK_PUSHX(stack, voidptr, uu + 1);
		      STACK_PUSHX(stack, int, COPY_LAST_MATCHED_NEXT);
		    }
		  STACK_PUSHX(stack, voidptr, up->branches);
		  STACK_PUSHX(stack, int, up->n_branches);
		  STACK_PUSHX(stack, int, COPY_LAST_MATCHED_BRANCH);
		  break;
		}
	    }
	  if (status != REG_OK)
	    goto error_post_compile;
#ifdef TRE_DEBUG
	  DPRINT(("last_matched_branch:\n"));
	  print_last_match_branch(buf, 0);
	  if (b != _b + tree->last_matched_branch->tot_branches)
	    DPRINT(("b/%p != _b + tree->last_matched_branch->tot_branches/%p\n",
		    b, _b + tree->last_matched_branch->tot_branches));
	  if (u != _u + tree->last_matched_branch->tot_last_matched)
	    DPRINT(("u/%p != _u + "
		    "tree->last_matched_branch->tot_last_matched/%p\n",
		    u, _u + tree->last_matched_branch->tot_last_matched));
	  if (t != _t + tree->last_matched_branch->tot_tags)
	    DPRINT(("t/%p != _t + tree->last_matched_branch->tot_tags/%p\n",
		    t, _t + tree->last_matched_branch->tot_tags));
#endif /* TRE_DEBUG */
	  tnfa->last_matched_branch = buf;
	}
#ifdef TRE_DEBUG
      else
	DPRINT(("No last_match_branch_pre\n"));
#endif /* TRE_DEBUG */
    }

  DPRINT(("tre_add_tags: %s complete.  Number of tags %d.\n",
	  first_pass? "First pass" : "Second pass", num_tags));
#ifdef TRE_DEBUG
  tre_ast_print(tree);
#endif /* TRE_DEBUG */
  DPRINT(("tre_add_tags: tree->num_tags=%d num_tags=%d\n", tree->num_tags,
	  num_tags));
  assert(tree->num_tags == num_tags);
  tnfa->end_tag = num_tags;
  tnfa->num_tags = num_tags;
  tnfa->num_minimals = num_minimals;
error_post_compile:
  xfree(reorder_tags);
error_reorder_tags:
  xfree(orig_regset);
error_regset:
  return status;
}



/*
  AST to TNFA compilation routines.
*/

typedef enum {
  COPY_RECURSE,
  COPY_SET_RESULT_PTR
} tre_copyast_symbol_t;

/* Flags for tre_copy_ast(). */
#define COPY_REMOVE_TAGS	 1
#define COPY_MAXIMIZE_FIRST_TAG	 2

static reg_errcode_t
tre_copy_ast(tre_mem_t mem, tre_stack_t *stack, tre_ast_node_t *ast,
	     int flags, int *pos_add, tre_tag_direction_t *tag_directions,
	     tre_ast_node_t **copy, int *max_pos)
{
  reg_errcode_t status = REG_OK;
  int bottom = tre_stack_num_objects(stack);
  int num_copied = 0;
  int first_tag = 1;
  tre_ast_node_t **result = copy;
  tre_copyast_symbol_t symbol;

  STACK_PUSH(stack, voidptr, ast);
  STACK_PUSH(stack, int, COPY_RECURSE);

  while (status == REG_OK && tre_stack_num_objects(stack) > bottom)
    {
      tre_ast_node_t *node;
      if (status != REG_OK)
	break;

      symbol = (tre_copyast_symbol_t)tre_stack_pop_int(stack);
      switch (symbol)
	{
	case COPY_SET_RESULT_PTR:
	  result = tre_stack_pop_voidptr(stack);
	  break;
	case COPY_RECURSE:
	  node = tre_stack_pop_voidptr(stack);
	  switch (node->type)
	    {
	    case LITERAL:
	      {
		tre_literal_t *lit = node->obj;
		int pos = lit->position;
		int min = lit->code_min;
		int max = lit->code_max;
		tre_bracket_match_list_t *list = !IS_SPECIAL(lit) ?
						  lit->u.bracket_match_list :
						  NULL;
		if (!IS_SPECIAL(lit) || IS_BACKREF(lit))
		  {
		    /* XXX - e.g. [ab] has only one position but two
		       nodes, so we are creating holes in the state space
		       here.  Not fatal, just wastes memory. */
		    pos += *pos_add;
		    num_copied++;
		  }
		else if (IS_TAG(lit) && (flags & COPY_REMOVE_TAGS))
		  {
		    /* Change this tag to empty. */
		    min = EMPTY;
		    max = pos = -1;
		  }
		else if (IS_TAG(lit) && (flags & COPY_MAXIMIZE_FIRST_TAG)
			 && first_tag)
		  {
		    /* Maximize the first tag. */
		    if (tag_directions[max] == TRE_TAG_LEFT_MAXIMIZE)
		      tag_directions[max] = TRE_TAG_MAXIMIZE;
		    first_tag = 0;
		  }
		*result = tre_ast_new_literal(mem, min, max, pos);
		if (*result == NULL)
		  status = REG_ESPACE;

		if (pos > *max_pos)
		  *max_pos = pos;

		if (!IS_SPECIAL(lit))
		  ((tre_literal_t *)(*result)->obj)->u.bracket_match_list
		      = list;
		break;
	      }
	    case UNION:
	      {
		tre_union_t *uni = node->obj;
		tre_union_t *tmp;
		*result = tre_ast_new_union(mem, uni->left, uni->right);
		if (*result == NULL)
		  {
		    status = REG_ESPACE;
		    break;
		  }
		tmp = (*result)->obj;
		result = &tmp->left;
		STACK_PUSHX(stack, voidptr, uni->right);
		STACK_PUSHX(stack, int, COPY_RECURSE);
		STACK_PUSHX(stack, voidptr, &tmp->right);
		STACK_PUSHX(stack, int, COPY_SET_RESULT_PTR);
		STACK_PUSHX(stack, voidptr, uni->left);
		STACK_PUSHX(stack, int, COPY_RECURSE);
		break;
	      }
	    case CATENATION:
	      {
		tre_catenation_t *cat = node->obj;
		tre_catenation_t *tmp;
		*result = tre_ast_new_catenation(mem, cat->left, cat->right);
		if (*result == NULL)
		  {
		    status = REG_ESPACE;
		    break;
		  }
		tmp = (*result)->obj;
		tmp->left = NULL;
		tmp->right = NULL;
		result = &tmp->left;

		STACK_PUSHX(stack, voidptr, cat->right);
		STACK_PUSHX(stack, int, COPY_RECURSE);
		STACK_PUSHX(stack, voidptr, &tmp->right);
		STACK_PUSHX(stack, int, COPY_SET_RESULT_PTR);
		STACK_PUSHX(stack, voidptr, cat->left);
		STACK_PUSHX(stack, int, COPY_RECURSE);
		break;
	      }
	    case ITERATION:
	      {
		tre_iteration_t *iter = node->obj;
		STACK_PUSHX(stack, voidptr, iter->arg);
		STACK_PUSHX(stack, int, COPY_RECURSE);
		*result = tre_ast_new_iter(mem, iter->arg, iter->min,
					   iter->max, iter->minimal);
		if (*result == NULL)
		  {
		    status = REG_ESPACE;
		    break;
		  }
		iter = (*result)->obj;
		result = &iter->arg;
		break;
	      }
	    default:
	      assert(0);
	      break;
	    }
	  break;
	}
    }
  *pos_add += num_copied;
  return status;
}

typedef enum {
  EXPAND_RECURSE,
  EXPAND_AFTER_ITER
} tre_expand_ast_symbol_t;

/* Expands each iteration node that has a finite nonzero minimum or maximum
   iteration count to a catenated sequence of copies of the node. */
static reg_errcode_t
tre_expand_ast(tre_mem_t mem, tre_stack_t *stack, tre_ast_node_t *ast,
	       int *position, tre_tag_direction_t *tag_directions,
	       int *max_depth)
{
  reg_errcode_t status = REG_OK;
  int bottom = tre_stack_num_objects(stack);
  int pos_add = 0;
  int pos_add_total = 0;
  int max_pos = 0;
#ifdef TRE_APPROX
  /* Current approximate matching parameters. */
  int params[TRE_PARAM_LAST];
  /* Approximate parameter nesting level. */
  int params_depth = 0;
#endif /* TRE_APPROX */
  int iter_depth = 0;
#ifdef TRE_APPROX
  int i;
#endif /* TRE_APPROX */

#ifdef TRE_APPROX
  for (i = 0; i < TRE_PARAM_LAST; i++)
    params[i] = TRE_PARAM_DEFAULT;
#endif /* TRE_APPROX */

  STACK_PUSHR(stack, voidptr, ast);
  STACK_PUSHR(stack, int, EXPAND_RECURSE);
  while (status == REG_OK && tre_stack_num_objects(stack) > bottom)
    {
      tre_ast_node_t *node;
      tre_expand_ast_symbol_t symbol;

      if (status != REG_OK)
	break;

      DPRINT(("pos_add %d\n", pos_add));

      symbol = (tre_expand_ast_symbol_t)tre_stack_pop_int(stack);
      node = tre_stack_pop_voidptr(stack);
      switch (symbol)
	{
	case EXPAND_RECURSE:
	  switch (node->type)
	    {
	    case LITERAL:
	      {
		tre_literal_t *lit= node->obj;
		if (!IS_SPECIAL(lit) || IS_BACKREF(lit))
		  {
		    lit->position += pos_add;
		    if (lit->position > max_pos)
		      max_pos = lit->position;
		  }
		break;
	      }
	    case UNION:
	      {
		tre_union_t *uni = node->obj;
		STACK_PUSHX(stack, voidptr, uni->right);
		STACK_PUSHX(stack, int, EXPAND_RECURSE);
		STACK_PUSHX(stack, voidptr, uni->left);
		STACK_PUSHX(stack, int, EXPAND_RECURSE);
		break;
	      }
	    case CATENATION:
	      {
		tre_catenation_t *cat = node->obj;
		STACK_PUSHX(stack, voidptr, cat->right);
		STACK_PUSHX(stack, int, EXPAND_RECURSE);
		STACK_PUSHX(stack, voidptr, cat->left);
		STACK_PUSHX(stack, int, EXPAND_RECURSE);
		break;
	      }
	    case ITERATION:
	      {
		tre_iteration_t *iter = node->obj;
		STACK_PUSHX(stack, int, pos_add);
		STACK_PUSHX(stack, voidptr, node);
		STACK_PUSHX(stack, int, EXPAND_AFTER_ITER);
		STACK_PUSHX(stack, voidptr, iter->arg);
		STACK_PUSHX(stack, int, EXPAND_RECURSE);
		/* If we are going to expand this node at EXPAND_AFTER_ITER
		   then don't increase the `pos' fields of the nodes now, it
		   will get done when expanding. */
		if (iter->min > 1 || iter->max > 1)
		  pos_add = 0;
		iter_depth++;
		DPRINT(("iter\n"));
		break;
	      }
	    default:
	      assert(0);
	      break;
	    }
	  break;
	case EXPAND_AFTER_ITER:
	  {
	    tre_iteration_t *iter = node->obj;
	    int pos_add_last;
	    pos_add = tre_stack_pop_int(stack);
	    pos_add_last = pos_add;
	    /* Originally (in tre_parse_bound), if min == 0 && max == 0, we
	       immediate replace the whole iteration with EMPTY.  This
	       unfortunately drops any submatches, and messes up setting the
	       pmatch values (we can get tags of -1, and tag values in the
	       billions).  So we left it there and replace with EMPTY here. */
	    if (iter->min == 0 && iter->max == 0)
	      {
		tre_ast_node_t *empty = tre_ast_new_literal(mem, EMPTY, -1, -1);
		if (empty == NULL)
		  return REG_ESPACE;
		node->obj = empty->obj;
		node->type = empty->type;
	      }
	    else if (iter->min > 1 || iter->max > 1)
	      {
		tre_ast_node_t *seq1 = NULL, *seq2 = NULL;
		int j;
		int pos_add_save = pos_add;

		/* Create a catenated sequence of copies of the node. */
		for (j = 0; j < iter->min; j++)
		  {
		    tre_ast_node_t *copy;
		    /* Remove tags from all but the last copy. */
		    int flags = ((j + 1 < iter->min)
				 ? COPY_REMOVE_TAGS
				 : COPY_MAXIMIZE_FIRST_TAG);
		    DPRINT(("  pos_add %d\n", pos_add));
		    pos_add_save = pos_add;
		    status = tre_copy_ast(mem, stack, iter->arg, flags,
					  &pos_add, tag_directions, &copy,
					  &max_pos);
		    if (status != REG_OK)
		      return status;
		    if (seq1 != NULL)
		      seq1 = tre_ast_new_catenation(mem, seq1, copy);
		    else
		      seq1 = copy;
		    if (seq1 == NULL)
		      return REG_ESPACE;
		  }

		if (iter->max == -1)
		  {
		    /* No upper limit. */
		    pos_add_save = pos_add;
		    status = tre_copy_ast(mem, stack, iter->arg, 0,
					  &pos_add, NULL, &seq2, &max_pos);
		    if (status != REG_OK)
		      return status;
		    seq2 = tre_ast_new_iter(mem, seq2, 0, -1, 0);
		    if (seq2 == NULL)
		      return REG_ESPACE;
		  }
		else
		  {
		    for (j = iter->min; j < iter->max; j++)
		      {
			tre_ast_node_t *tmp, *copy;
			pos_add_save = pos_add;
			status = tre_copy_ast(mem, stack, iter->arg, 0,
					      &pos_add, NULL, &copy, &max_pos);
			if (status != REG_OK)
			  return status;
			if (seq2 != NULL)
			  seq2 = tre_ast_new_catenation(mem, copy, seq2);
			else
			  seq2 = copy;
			if (seq2 == NULL)
			  return REG_ESPACE;
			tmp = tre_ast_new_literal(mem, EMPTY, -1, -1);
			if (tmp == NULL)
			  return REG_ESPACE;
			seq2 = tre_ast_new_union(mem, tmp, seq2);
			if (seq2 == NULL)
			  return REG_ESPACE;
		      }
		  }

		pos_add = pos_add_save;
		if (seq1 == NULL)
		  seq1 = seq2;
		else if (seq2 != NULL)
		  seq1 = tre_ast_new_catenation(mem, seq1, seq2);
		if (seq1 == NULL)
		  return REG_ESPACE;
		node->obj = seq1->obj;
		node->type = seq1->type;
	      }

	    iter_depth--;
	    pos_add_total += pos_add - pos_add_last;
	    if (iter_depth == 0)
	      pos_add = pos_add_total;

#ifdef TRE_APPROX
	    /* If approximate parameters are specified, surround the result
	       with two parameter setting nodes.  The one on the left sets
	       the specified parameters, and the one on the right restores
	       the old parameters. */
	    if (iter->params)
	      {
		tre_ast_node_t *tmp_l, *tmp_r, *tmp_node, *node_copy;
		int *old_params;

		tmp_l = tre_ast_new_literal(mem, PARAMETER, 0, -1);
		if (!tmp_l)
		  return REG_ESPACE;
		((tre_literal_t *)tmp_l->obj)->u.params = iter->params;
		iter->params[TRE_PARAM_DEPTH] = params_depth + 1;
		tmp_r = tre_ast_new_literal(mem, PARAMETER, 0, -1);
		if (!tmp_r)
		  return REG_ESPACE;
		old_params = tre_mem_alloc(mem, sizeof(*old_params)
					   * TRE_PARAM_LAST);
		if (!old_params)
		  return REG_ESPACE;
		for (i = 0; i < TRE_PARAM_LAST; i++)
		  old_params[i] = params[i];
		((tre_literal_t *)tmp_r->obj)->u.params = old_params;
		old_params[TRE_PARAM_DEPTH] = params_depth;
		/* XXX - this is the only place where ast_new_node is
		   needed -- should be moved inside AST module. */
		node_copy = tre_ast_new_node(mem, ITERATION,
					     sizeof(tre_iteration_t));
		if (!node_copy)
		  return REG_ESPACE;
		node_copy->obj = node->obj;
		tmp_node = tre_ast_new_catenation(mem, tmp_l, node_copy);
		if (!tmp_node)
		  return REG_ESPACE;
		tmp_node = tre_ast_new_catenation(mem, tmp_node, tmp_r);
		if (!tmp_node)
		  return REG_ESPACE;
		/* Replace the contents of `node' with `tmp_node'. */
		memcpy(node, tmp_node, sizeof(*node));
		node->obj = tmp_node->obj;
		node->type = tmp_node->type;
		params_depth++;
		if (params_depth > *max_depth)
		  *max_depth = params_depth;
	      }
#endif /* TRE_APPROX */
	    break;
	  }
	default:
	  assert(0);
	  break;
	}
    }

  *position += pos_add_total;

  /* `max_pos' should never be larger than `*position' if the above
     code works, but just an extra safeguard let's make sure
     `*position' is set large enough so enough memory will be
     allocated for the transition table. */
  if (max_pos > *position)
    *position = max_pos;

#ifdef TRE_DEBUG
  DPRINT(("Expanded AST:\n"));
  tre_ast_print(ast);
  DPRINT(("*position %d, max_pos %d\n", *position, max_pos));
#endif

  return status;
}

static tre_pos_and_tags_t *
tre_set_empty(tre_mem_t mem)
{
  tre_pos_and_tags_t *new_set;

  new_set = tre_mem_calloc(mem, sizeof(*new_set));
  if (new_set == NULL)
    return NULL;

  new_set[0].position = -1;
  new_set[0].code_min = -1;
  new_set[0].code_max = -1;

  return new_set;
}

static tre_pos_and_tags_t *
tre_set_one(tre_mem_t mem, int position, int code_min, int code_max,
	    tre_bracket_match_list_t *bracket_match_list, int backref)
{
  tre_pos_and_tags_t *new_set;

  new_set = tre_mem_calloc(mem, sizeof(*new_set) * 2);
  if (new_set == NULL)
    return NULL;

  new_set[0].position = position;
  new_set[0].code_min = code_min;
  new_set[0].code_max = code_max;
  new_set[0].bracket_match_list = bracket_match_list;
  new_set[0].backref = backref;
  new_set[1].position = -1;
  new_set[1].code_min = -1;
  new_set[1].code_max = -1;

  return new_set;
}

static tre_pos_and_tags_t *
tre_set_union(tre_mem_t mem, tre_pos_and_tags_t *set1, tre_pos_and_tags_t *set2,
	      int *tags, int assertions, int *params)
{
  int s1, s2, i, j;
  tre_pos_and_tags_t *new_set;
  int *new_tags;
  int num_tags;

  for (num_tags = 0; tags != NULL && tags[num_tags] >= 0; num_tags++);
  for (s1 = 0; set1[s1].position >= 0; s1++);
  for (s2 = 0; set2[s2].position >= 0; s2++);
  new_set = tre_mem_calloc(mem, sizeof(*new_set) * (s1 + s2 + 1));
  if (!new_set )
    return NULL;

  for (s1 = 0; set1[s1].position >= 0; s1++)
    {
      new_set[s1].position = set1[s1].position;
      new_set[s1].code_min = set1[s1].code_min;
      new_set[s1].code_max = set1[s1].code_max;
      new_set[s1].assertions = set1[s1].assertions | assertions;
      new_set[s1].bracket_match_list = set1[s1].bracket_match_list;
      new_set[s1].backref = set1[s1].backref;
      if (set1[s1].tags == NULL && tags == NULL)
	new_set[s1].tags = NULL;
      else
	{
	  for (i = 0; set1[s1].tags != NULL && set1[s1].tags[i] >= 0; i++);
	  new_tags = tre_mem_alloc(mem, (sizeof(*new_tags)
					 * (i + num_tags + 1)));
	  if (new_tags == NULL)
	    return NULL;
	  for (j = 0; j < i; j++)
	    new_tags[j] = set1[s1].tags[j];
	  for (i = 0; i < num_tags; i++)
	    new_tags[j + i] = tags[i];
	  new_tags[j + i] = -1;
	  new_set[s1].tags = new_tags;
	}
      if (set1[s1].params)
	new_set[s1].params = set1[s1].params;
      if (params)
	{
	  if (!new_set[s1].params)
	    new_set[s1].params = params;
	  else
	    {
	      new_set[s1].params = tre_mem_alloc(mem, sizeof(*params) *
						 TRE_PARAM_LAST);
	      if (!new_set[s1].params)
		return NULL;
	      for (i = 0; i < TRE_PARAM_LAST; i++)
		if (params[i] != TRE_PARAM_UNSET)
		  new_set[s1].params[i] = params[i];
	    }
	}
    }

  for (s2 = 0; set2[s2].position >= 0; s2++)
    {
      new_set[s1 + s2].position = set2[s2].position;
      new_set[s1 + s2].code_min = set2[s2].code_min;
      new_set[s1 + s2].code_max = set2[s2].code_max;
      /* XXX - why not | assertions here as well? */
      new_set[s1 + s2].assertions = set2[s2].assertions;
      new_set[s1 + s2].bracket_match_list = set2[s2].bracket_match_list;
      new_set[s1 + s2].backref = set2[s2].backref;
      if (set2[s2].tags == NULL)
	new_set[s1 + s2].tags = NULL;
      else
	{
	  for (i = 0; set2[s2].tags[i] >= 0; i++);
	  new_tags = tre_mem_alloc(mem, sizeof(*new_tags) * (i + 1));
	  if (new_tags == NULL)
	    return NULL;
	  for (j = 0; j < i; j++)
	    new_tags[j] = set2[s2].tags[j];
	  new_tags[j] = -1;
	  new_set[s1 + s2].tags = new_tags;
	}
      if (set2[s2].params)
	new_set[s1 + s2].params = set2[s2].params;
      if (params)
	{
	  if (!new_set[s1 + s2].params)
	    new_set[s1 + s2].params = params;
	  else
	    {
	      new_set[s1 + s2].params = tre_mem_alloc(mem, sizeof(*params) *
						      TRE_PARAM_LAST);
	      if (!new_set[s1 + s2].params)
		return NULL;
	      for (i = 0; i < TRE_PARAM_LAST; i++)
		if (params[i] != TRE_PARAM_UNSET)
		  new_set[s1 + s2].params[i] = params[i];
	    }
	}
    }
  new_set[s1 + s2].position = -1;
  return new_set;
}

/* Finds the empty path through `node' which is the one that should be
   taken according to POSIX.2 rules, and adds the tags on that path to
   `tags'.   `tags' may be NULL.  If `num_tags_seen' is not NULL, it is
   set to the number of tags seen on the path. */
static reg_errcode_t
tre_match_empty(tre_stack_t *stack, tre_ast_node_t *node, int *tags,
		int *assertions, int *params, int *num_tags_seen,
		int *params_seen)
{
  tre_literal_t *lit;
  tre_union_t *uni;
  tre_catenation_t *cat;
  tre_iteration_t *iter;
  int i;
  int bottom = tre_stack_num_objects(stack);
  reg_errcode_t status = REG_OK;
  if (num_tags_seen)
    *num_tags_seen = 0;
  if (params_seen)
    *params_seen = 0;

  status = tre_stack_push_voidptr(stack, node);

  /* Walk through the tree recursively. */
  while (status == REG_OK && tre_stack_num_objects(stack) > bottom)
    {
      node = tre_stack_pop_voidptr(stack);

      switch (node->type)
	{
	case LITERAL:
	  lit = (tre_literal_t *)node->obj;
	  switch (lit->code_min)
	    {
	    case TAG:
	      if (lit->code_max >= 0)
		{
		  if (tags != NULL)
		    {
		      /* Add the tag to `tags'. */
		      for (i = 0; tags[i] >= 0; i++)
			if (tags[i] == lit->code_max)
			  break;
		      if (tags[i] < 0)
			{
			  tags[i] = lit->code_max;
			  tags[i + 1] = -1;
			}
		    }
		  if (num_tags_seen)
		    (*num_tags_seen)++;
		}
	      break;
	    case ASSERTION:
	      assert(lit->code_max >= 1
		     || lit->code_max <= ASSERT_LAST);
	      if (assertions != NULL)
		*assertions |= lit->code_max;
	      break;
	    case PARAMETER:
	      if (params != NULL)
		for (i = 0; i < TRE_PARAM_LAST; i++)
		  params[i] = lit->u.params[i];
	      if (params_seen != NULL)
		*params_seen = 1;
	      break;
	    case EMPTY:
	      break;
	    default:
	      assert(0);
	      break;
	    }
	  break;

	case UNION:
	  /* Subexpressions starting earlier take priority over ones
	     starting later, so we prefer the left subexpression over the
	     right subexpression. */
	  uni = (tre_union_t *)node->obj;
	  if (uni->left->nullable)
	    STACK_PUSHX(stack, voidptr, uni->left)
	  else if (uni->right->nullable)
	    STACK_PUSHX(stack, voidptr, uni->right)
	  else
	    assert(0);
	  break;

	case CATENATION:
	  /* The path must go through both children. */
	  cat = (tre_catenation_t *)node->obj;
	  assert(cat->left->nullable);
	  assert(cat->right->nullable);
	  STACK_PUSHX(stack, voidptr, cat->left);
	  STACK_PUSHX(stack, voidptr, cat->right);
	  break;

	case ITERATION:
	  /* A match with an empty string is preferred over no match at
	     all, so we go through the argument if possible. */
	  iter = (tre_iteration_t *)node->obj;
	  if (iter->arg->nullable)
	    STACK_PUSHX(stack, voidptr, iter->arg);
	  break;

	default:
	  assert(0);
	  break;
	}
    }

  return status;
}


typedef enum {
  NFL_RECURSE,
  NFL_POST_UNION,
  NFL_POST_CATENATION,
  NFL_POST_ITERATION
} tre_nfl_stack_symbol_t;


/* Computes and fills in the fields `nullable', `firstpos', and `lastpos' for
   the nodes of the AST `tree'. */
static reg_errcode_t
tre_compute_nfl(tre_mem_t mem, tre_stack_t *stack, tre_ast_node_t *tree)
{
  int bottom = tre_stack_num_objects(stack);

  STACK_PUSHR(stack, voidptr, tree);
  STACK_PUSHR(stack, int, NFL_RECURSE);

  while (tre_stack_num_objects(stack) > bottom)
    {
      tre_nfl_stack_symbol_t symbol;
      tre_ast_node_t *node;

      symbol = (tre_nfl_stack_symbol_t)tre_stack_pop_int(stack);
      node = tre_stack_pop_voidptr(stack);
      switch (symbol)
	{
	case NFL_RECURSE:
	  switch (node->type)
	    {
	    case LITERAL:
	      {
		tre_literal_t *lit = (tre_literal_t *)node->obj;
		if (IS_BACKREF(lit))
		  {
		    /* Back references: nullable = false, firstpos = {i},
		       lastpos = {i}. */
		    node->nullable = 0;
		    node->firstpos = tre_set_one(mem, lit->position, 0,
					     TRE_CHAR_MAX, NULL, -1);
		    if (!node->firstpos)
		      return REG_ESPACE;
		    node->lastpos = tre_set_one(mem, lit->position, 0,
						TRE_CHAR_MAX, NULL,
						(int)lit->code_max);
		    if (!node->lastpos)
		      return REG_ESPACE;
		  }
		else if (lit->code_min < 0)
		  {
		    /* Tags, empty strings, params, and zero width assertions:
		       nullable = true, firstpos = {}, and lastpos = {}. */
		    node->nullable = 1;
		    node->firstpos = tre_set_empty(mem);
		    if (!node->firstpos)
		      return REG_ESPACE;
		    node->lastpos = tre_set_empty(mem);
		    if (!node->lastpos)
		      return REG_ESPACE;
		  }
		else
		  {
		    /* Literal at position i: nullable = false, firstpos = {i},
		       lastpos = {i}. */
		    node->nullable = 0;
		    node->firstpos =
		      tre_set_one(mem, lit->position, (int)lit->code_min,
				  (int)lit->code_max, NULL, -1);
		    if (!node->firstpos)
		      return REG_ESPACE;
		    node->lastpos = tre_set_one(mem, lit->position,
						(int)lit->code_min,
						(int)lit->code_max,
						lit->u.bracket_match_list,
						-1);
		    if (!node->lastpos)
		      return REG_ESPACE;
		  }
		break;
	      }

	    case UNION:
	      /* Compute the attributes for the two subtrees, and after that
		 for this node. */
	      STACK_PUSHR(stack, voidptr, node);
	      STACK_PUSHR(stack, int, NFL_POST_UNION);
	      STACK_PUSHR(stack, voidptr, ((tre_union_t *)node->obj)->right);
	      STACK_PUSHR(stack, int, NFL_RECURSE);
	      STACK_PUSHR(stack, voidptr, ((tre_union_t *)node->obj)->left);
	      STACK_PUSHR(stack, int, NFL_RECURSE);
	      break;

	    case CATENATION:
	      /* Compute the attributes for the two subtrees, and after that
		 for this node. */
	      STACK_PUSHR(stack, voidptr, node);
	      STACK_PUSHR(stack, int, NFL_POST_CATENATION);
	      STACK_PUSHR(stack, voidptr, ((tre_catenation_t *)node->obj)->right);
	      STACK_PUSHR(stack, int, NFL_RECURSE);
	      STACK_PUSHR(stack, voidptr, ((tre_catenation_t *)node->obj)->left);
	      STACK_PUSHR(stack, int, NFL_RECURSE);
	      break;

	    case ITERATION:
	      /* Compute the attributes for the subtree, and after that for
		 this node. */
	      STACK_PUSHR(stack, voidptr, node);
	      STACK_PUSHR(stack, int, NFL_POST_ITERATION);
	      STACK_PUSHR(stack, voidptr, ((tre_iteration_t *)node->obj)->arg);
	      STACK_PUSHR(stack, int, NFL_RECURSE);
	      break;
	    }
	  break; /* end case: NFL_RECURSE */

	case NFL_POST_UNION:
	  {
	    tre_union_t *uni = (tre_union_t *)node->obj;
	    node->nullable = uni->left->nullable || uni->right->nullable;
	    node->firstpos = tre_set_union(mem, uni->left->firstpos,
					   uni->right->firstpos, NULL, 0, NULL);
	    if (!node->firstpos)
	      return REG_ESPACE;
	    node->lastpos = tre_set_union(mem, uni->left->lastpos,
					  uni->right->lastpos, NULL, 0, NULL);
	    if (!node->lastpos)
	      return REG_ESPACE;
	    break;
	  }

	case NFL_POST_ITERATION:
	  {
	    int num_tags, *tags, assertions, params_seen;
	    int *params;
	    reg_errcode_t status;
	    tre_iteration_t *iter = (tre_iteration_t *)node->obj;

	    /* From Ville Laurikari's original 2001 Master's thesis, the
	       firstpos(n) and lastpos(n) of an iteration is just the
	       corresponding values of the iteration's argument.  Unfortunately,
	       this isn't sufficient for the following BRE:

	           \(a*\)*b\(\1\)    matched against    ab

	       The backreference wants to force the first subexpression to
	       be the empty string, but there is no transition for this.  So
	       we need to modify the lastpos(n) of an iteration to be the
	       equivalent of that of catentation.  Using the same notation as
	       in the thesis, lastpos(n) is redefined as:

	           if nullable(c1) then
		       lastpos(c1) U
		       addtags(lastpos(c1),
		               emptymatch(c1))
		   else
		       lastpos(c1)

	       where c1 is the argument node.  firstpos(n) remains the same. */

	    /* Compute lastpos. */
	    if (iter->min == 0 || iter->arg->nullable)
	      {
		node->nullable = 1;
		if (iter->arg->nullable)
		  {
		    /* The arg matches the empty string.  Make a first pass
		       with tre_match_empty() to get the number of tags and
		       parameters. */
		    status = tre_match_empty(stack, iter->arg,
					     NULL, NULL, NULL, &num_tags,
					     &params_seen);
		    if (status != REG_OK)
		      return status;
		    /* Allocate arrays for the tags and parameters. */
		    tags = xmalloc(sizeof(int) * (num_tags + 1));
		    if (!tags)
		      return REG_ESPACE;
		    tags[0] = -1;
		    assertions = 0;
		    params = NULL;
		    if (params_seen)
		      {
			params = tre_mem_alloc(mem, sizeof(*params)
					       * TRE_PARAM_LAST);
			if (!params)
			  {
			    xfree(tags);
			    return REG_ESPACE;
			  }
		      }
		    /* Second pass with tre_mach_empty() to get the list of
		       tags and parameters. */
		    status = tre_match_empty(stack, iter->arg, tags,
					     &assertions, params, NULL, NULL);
		    if (status != REG_OK)
		      {
			xfree(tags);
			return status;
		      }
		    node->lastpos =
		      tre_set_union(mem, iter->arg->lastpos, iter->arg->lastpos,
				    tags, assertions, params);
		    xfree(tags);
		    if (!node->lastpos)
		      return REG_ESPACE;
		  }
		else
		  node->lastpos = iter->arg->lastpos;
	      }
	    else
	      {
		node->nullable = 0;
		node->lastpos = iter->arg->lastpos;
	      }
	    node->firstpos = iter->arg->firstpos;
	    break;
	  }

	case NFL_POST_CATENATION:
	  {
	    int num_tags, *tags, assertions, params_seen;
	    int *params;
	    reg_errcode_t status;
	    tre_catenation_t *cat = node->obj;
	    node->nullable = cat->left->nullable && cat->right->nullable;

	    /* Compute firstpos. */
	    if (cat->left->nullable)
	      {
		/* The left side matches the empty string.  Make a first pass
		   with tre_match_empty() to get the number of tags and
		   parameters. */
		status = tre_match_empty(stack, cat->left,
					 NULL, NULL, NULL, &num_tags,
					 &params_seen);
		if (status != REG_OK)
		  return status;
		/* Allocate arrays for the tags and parameters. */
		tags = xmalloc(sizeof(*tags) * (num_tags + 1));
		if (!tags)
		  return REG_ESPACE;
		tags[0] = -1;
		assertions = 0;
		params = NULL;
		if (params_seen)
		  {
		    params = tre_mem_alloc(mem, sizeof(*params)
					   * TRE_PARAM_LAST);
		    if (!params)
		      {
			xfree(tags);
			return REG_ESPACE;
		      }
		  }
		/* Second pass with tre_mach_empty() to get the list of
		   tags and parameters. */
		status = tre_match_empty(stack, cat->left, tags,
					 &assertions, params, NULL, NULL);
		if (status != REG_OK)
		  {
		    xfree(tags);
		    return status;
		  }
		node->firstpos =
		  tre_set_union(mem, cat->right->firstpos, cat->left->firstpos,
				tags, assertions, params);
		xfree(tags);
		if (!node->firstpos)
		  return REG_ESPACE;
	      }
	    else
	      {
		node->firstpos = cat->left->firstpos;
	      }

	    /* Compute lastpos. */
	    if (cat->right->nullable)
	      {
		/* The right side matches the empty string.  Make a first pass
		   with tre_match_empty() to get the number of tags and
		   parameters. */
		status = tre_match_empty(stack, cat->right,
					 NULL, NULL, NULL, &num_tags,
					 &params_seen);
		if (status != REG_OK)
		  return status;
		/* Allocate arrays for the tags and parameters. */
		tags = xmalloc(sizeof(int) * (num_tags + 1));
		if (!tags)
		  return REG_ESPACE;
		tags[0] = -1;
		assertions = 0;
		params = NULL;
		if (params_seen)
		  {
		    params = tre_mem_alloc(mem, sizeof(*params)
					   * TRE_PARAM_LAST);
		    if (!params)
		      {
			xfree(tags);
			return REG_ESPACE;
		      }
		  }
		/* Second pass with tre_mach_empty() to get the list of
		   tags and parameters. */
		status = tre_match_empty(stack, cat->right, tags,
					 &assertions, params, NULL, NULL);
		if (status != REG_OK)
		  {
		    xfree(tags);
		    return status;
		  }
		node->lastpos =
		  tre_set_union(mem, cat->left->lastpos, cat->right->lastpos,
				tags, assertions, params);
		xfree(tags);
		if (!node->lastpos)
		  return REG_ESPACE;
	      }
	    else
	      {
		node->lastpos = cat->right->lastpos;
	      }
	    break;
	  }

	default:
	  assert(0);
	  break;
	}
    }

  return REG_OK;
}


/* Adds a transition from each position in `p1' to each position in `p2'. */
static reg_errcode_t
tre_make_trans(tre_pos_and_tags_t *p1, tre_pos_and_tags_t *p2,
	       tre_tnfa_transition_t *transitions,
	       int *counts, int *offs)
{
  tre_pos_and_tags_t *orig_p2 = p2;
  tre_tnfa_transition_t *trans;
  int i, j, k, l, dup, prev_p2_pos;

  if (transitions != NULL)
    while (p1->position >= 0)
      {
	p2 = orig_p2;
	prev_p2_pos = -1;
	while (p2->position >= 0)
	  {
	    /* Optimization: if this position was already handled, skip it. */
	    if (p2->position == prev_p2_pos)
	      {
		p2++;
		continue;
	      }
	    prev_p2_pos = p2->position;
	    /* Set `trans' to point to the next unused transition from
	       position `p1->position'. */
	    trans = transitions + offs[p1->position];
	    while (trans->state != NULL)
	      {
#if 0
		/* If we find a previous transition from `p1->position' to
		   `p2->position', it is overwritten.  This can happen only
		   if there are nested loops in the regexp, like in "((a)*)*".
		   In POSIX.2 repetition using the outer loop is always
		   preferred over using the inner loop.	 Therefore the
		   transition for the inner loop is useless and can be thrown
		   away. */
		/* XXX - The same position is used for all nodes in a bracket
		   expression, so this optimization cannot be used (it will
		   break bracket expressions) unless I figure out a way to
		   detect it here. */
		if (trans->state_id == p2->position)
		  {
		    DPRINT(("*"));
		    break;
		  }
#endif
		trans++;
	      }

	    if (trans->state == NULL)
	      (trans + 1)->state = NULL;
	    /* Use the character ranges, assertions, etc. from `p1' for
	       the transition from `p1' to `p2'. */
	    trans->code_min = p1->code_min;
	    trans->code_max = p1->code_max;
	    trans->state = transitions + offs[p2->position];
	    trans->state_id = p2->position;
	    trans->assertions = p1->assertions | p2->assertions
	      | (p1->bracket_match_list != NULL ? ASSERT_BRACKET_MATCH : 0);
	    if (p1->backref >= 0)
	      {
		assert((trans->assertions & ASSERT_BRACKET_MATCH) == 0);
		assert(p2->backref < 0);
		trans->u.backref = p1->backref;
		trans->assertions |= ASSERT_BACKREF;
	      }
	    if (p1->bracket_match_list != NULL)
	      {
		trans->u.bracket_match_list =
		  xmalloc(SIZEOF_BRACKET_MATCH_LIST(p1->bracket_match_list));
		if (trans->u.bracket_match_list == NULL)
		  return REG_ESPACE;
		memcpy(trans->u.bracket_match_list, p1->bracket_match_list,
		       SIZEOF_BRACKET_MATCH_LIST(p1->bracket_match_list));
	      }

	    /* Find out how many tags this transition has. */
	    i = 0;
	    if (p1->tags != NULL)
	      while(p1->tags[i] >= 0)
		i++;
	    j = 0;
	    if (p2->tags != NULL)
	      while(p2->tags[j] >= 0)
		j++;

	    /* If we are overwriting a transition, free the old tag array. */
	    if (trans->tags != NULL)
	      xfree(trans->tags);
	    trans->tags = NULL;

	    /* If there were any tags, allocate an array and fill it. */
	    if (i + j > 0)
	      {
		trans->tags = xmalloc(sizeof(*trans->tags) * (i + j + 1));
		if (!trans->tags)
		  return REG_ESPACE;
		i = 0;
		if (p1->tags != NULL)
		  while(p1->tags[i] >= 0)
		    {
		      trans->tags[i] = p1->tags[i];
		      i++;
		    }
		l = i;
		j = 0;
		if (p2->tags != NULL)
		  while (p2->tags[j] >= 0)
		    {
		      /* Don't add duplicates. */
		      dup = 0;
		      for (k = 0; k < i; k++)
			if (trans->tags[k] == p2->tags[j])
			  {
			    dup = 1;
			    break;
			  }
		      if (!dup)
			trans->tags[l++] = p2->tags[j];
		      j++;
		    }
		trans->tags[l] = -1;
	      }

	    /* Set the parameter array.	 If both `p2' and `p1' have same
	       parameters, the values in `p2' override those in `p1'. */
	    if (p1->params || p2->params)
	      {
		if (!trans->params)
		  trans->params = xmalloc(sizeof(*trans->params)
					  * TRE_PARAM_LAST);
		if (!trans->params)
		  return REG_ESPACE;
		for (i = 0; i < TRE_PARAM_LAST; i++)
		  {
		    trans->params[i] = TRE_PARAM_UNSET;
		    if (p1->params && p1->params[i] != TRE_PARAM_UNSET)
		      trans->params[i] = p1->params[i];
		    if (p2->params && p2->params[i] != TRE_PARAM_UNSET)
		      trans->params[i] = p2->params[i];
		  }
	      }
	    else
	      {
		if (trans->params)
		  xfree(trans->params);
		trans->params = NULL;
	      }


#ifdef TRE_DEBUG
	    {
	      int *tags;

	      DPRINT(("	 %2d -> %2d on %3d", p1->position, p2->position,
		      p1->code_min));
	      if (p1->code_max != p1->code_min)
		DPRINT(("-%3d", p1->code_max));
	      tags = trans->tags;
	      if (tags)
		{
		  DPRINT((", tags ["));
		  while (*tags >= 0)
		    {
		      DPRINT(("%d", *tags));
		      tags++;
		      if (*tags >= 0)
			DPRINT((","));
		    }
		  DPRINT(("]"));
		}
	      if (trans->assertions)
		DPRINT((", assert %d", trans->assertions));
	      if (trans->assertions & ASSERT_BACKREF)
		DPRINT((", backref %d", trans->u.backref));
	      else if (trans->assertions & ASSERT_BRACKET_MATCH)
		DPRINT((", bracket_match_list %p",
			trans->u.bracket_match_list));
	      if (trans->params)
		{
		  DPRINT((", "));
		  tre_print_params(trans->params);
		}
	      DPRINT(("\n"));
	    }
#endif /* TRE_DEBUG */
	    p2++;
	  }
	p1++;
      }
  else
    /* Compute a maximum limit for the number of transitions leaving
       from each state. */
    while (p1->position >= 0)
      {
	p2 = orig_p2;
	while (p2->position >= 0)
	  {
	    counts[p1->position]++;
	    p2++;
	  }
	p1++;
      }
  return REG_OK;
}

/* Converts the syntax tree to a TNFA.	All the transitions in the TNFA are
   labelled with one character range (there are no transitions on empty
   strings).  The TNFA takes O(n^2) space in the worst case, `n' is size of
   the regexp. */
static reg_errcode_t
tre_ast_to_tnfa(tre_ast_node_t *node, tre_tnfa_transition_t *transitions,
		int *counts, int *offs)
{
  tre_union_t *uni;
  tre_catenation_t *cat;
  tre_iteration_t *iter;
  reg_errcode_t errcode = REG_OK;

  /* XXX - recurse using a stack!. */
  switch (node->type)
    {
    case LITERAL:
      break;
    case UNION:
      uni = (tre_union_t *)node->obj;
      errcode = tre_ast_to_tnfa(uni->left, transitions, counts, offs);
      if (errcode != REG_OK)
	return errcode;
      errcode = tre_ast_to_tnfa(uni->right, transitions, counts, offs);
      break;

    case CATENATION:
      cat = (tre_catenation_t *)node->obj;
      /* Add a transition from each position in cat->left->lastpos
	 to each position in cat->right->firstpos. */
      errcode = tre_make_trans(cat->left->lastpos, cat->right->firstpos,
			       transitions, counts, offs);
      if (errcode != REG_OK)
	return errcode;
      errcode = tre_ast_to_tnfa(cat->left, transitions, counts, offs);
      if (errcode != REG_OK)
	return errcode;
      errcode = tre_ast_to_tnfa(cat->right, transitions, counts, offs);
      break;

    case ITERATION:
      iter = (tre_iteration_t *)node->obj;
      assert(iter->max == -1 || iter->max == 1);

      if (iter->max == -1)
	{
	  assert(iter->min == 0 || iter->min == 1);
	  /* Add a transition from each last position in the iterated
	     expression to each first position. */
	  errcode = tre_make_trans(iter->arg->lastpos, iter->arg->firstpos,
				   transitions, counts, offs);
	  if (errcode != REG_OK)
	    return errcode;
	}
      errcode = tre_ast_to_tnfa(iter->arg, transitions, counts, offs);
      break;
    }
  return errcode;
}


#define ERROR_EXIT(err)		  \
  do				  \
    {				  \
      errcode = err;		  \
      if (/*CONSTCOND*/1)	  \
      	goto error_exit;	  \
    }				  \
 while (/*CONSTCOND*/0)


int
tre_compile(regex_t *preg, const tre_char_t *regex, size_t n, int cflags,
	    locale_t loc)
{
  tre_stack_t *stack;
  tre_ast_node_t *tree, *tmp_ast_l, *tmp_ast_r;
  tre_pos_and_tags_t *p;
  int *counts = NULL, *offs = NULL;
  int i, add = 0;
  tre_tnfa_transition_t *transitions, *initial;
  tre_tnfa_t *tnfa = NULL;
  tre_submatch_data_t *submatch_data = NULL;
  tre_tag_direction_t *tag_directions = NULL;
  reg_errcode_t errcode;
  tre_mem_t mem;

  /* Parse context. */
  tre_parse_ctx_t parse_ctx;

  /* Allocate a stack used throughout the compilation process for various
     purposes. */
  stack = tre_stack_new(512, 10240, 128);
  if (!stack)
    return REG_ESPACE;
  /* Allocate a fast memory allocator. */
  mem = tre_mem_new();
  if (!mem)
    {
      tre_stack_destroy(stack);
      return REG_ESPACE;
    }

  /* Parse the regexp. */
  memset(&parse_ctx, 0, sizeof(parse_ctx));
  parse_ctx.mem = mem;
  parse_ctx.stack = stack;
  parse_ctx.re = regex;
  parse_ctx.len = n;
  /* Only allow REG_UNGREEDY to be set if both REG_ENHANCED and REG_EXTENDED
     are also set */
  if ((cflags & (REG_ENHANCED | REG_EXTENDED)) != (REG_ENHANCED | REG_EXTENDED))
    cflags &= ~REG_UNGREEDY;
  parse_ctx.cflags = cflags;
  parse_ctx.max_backref = -1;
  parse_ctx.loc = loc;
  parse_ctx.submatch_id_invisible = SUBMATCH_ID_INVISIBLE_START;

  DPRINT(("tre_compile: parsing '%.*" STRF "'\n", (int)n, regex));
  errcode = tre_parse(&parse_ctx);
  if (errcode != REG_OK)
    ERROR_EXIT(errcode);
  preg->re_nsub = parse_ctx.submatch_id - 1;
  tree = parse_ctx.result;

  /* Back references and approximate matching cannot currently be used
     in the same regexp. */
  if (parse_ctx.max_backref >= 0 && parse_ctx.have_approx)
    ERROR_EXIT(REG_BADPAT);

#ifdef TRE_DEBUG
  tre_ast_print(tree);
#endif /* TRE_DEBUG */

  /* Referring to nonexistent subexpressions is illegal. */
  if (parse_ctx.max_backref > (int)preg->re_nsub)
    ERROR_EXIT(REG_ESUBREG);

  /* Allocate the TNFA struct. */
  tnfa = xcalloc(1, sizeof(tre_tnfa_t));
  if (tnfa == NULL)
    ERROR_EXIT(REG_ESPACE);
  tnfa->have_backrefs = parse_ctx.max_backref >= 0;
  tnfa->have_approx = parse_ctx.have_approx;
  tnfa->num_submatches = parse_ctx.submatch_id;
  tnfa->num_submatches_invisible = parse_ctx.submatch_id_invisible
				   - SUBMATCH_ID_INVISIBLE_START;
  tnfa->num_reorder_tags = parse_ctx.num_reorder_tags;
  tnfa->loc = parse_ctx.loc;

  /* Set up tags for submatch addressing.  If REG_NOSUB is set and the
     regexp does not have back references, this can be skipped. */
  if (tnfa->num_reorder_tags > 0 || !(cflags & REG_NOSUB))
    {
      DPRINT(("tre_compile: setting up tags\n"));

      /* Figure out how many tags we will need. */
      errcode = tre_add_tags(NULL, stack, tree, tnfa);
      if (errcode != REG_OK)
	ERROR_EXIT(errcode);
#ifdef TRE_DEBUG
      tre_ast_print(tree);
#endif /* TRE_DEBUG */

      if (tnfa->num_tags > 0)
	{
	  tag_directions = xmalloc(sizeof(*tag_directions)
				   * (tnfa->num_tags + 1));
	  if (tag_directions == NULL)
	    ERROR_EXIT(REG_ESPACE);
	  tnfa->tag_directions = tag_directions;
	  memset(tag_directions, -1,
		 sizeof(*tag_directions) * (tnfa->num_tags + 1));
	}
      tnfa->minimal_tags = xcalloc((unsigned)tnfa->num_tags * 2 + 3,
				   sizeof(tnfa->minimal_tags));
      if (tnfa->minimal_tags == NULL)
	ERROR_EXIT(REG_ESPACE);

      submatch_data = xcalloc((unsigned)parse_ctx.submatch_id,
			      sizeof(*submatch_data));
      if (submatch_data == NULL)
	ERROR_EXIT(REG_ESPACE);
      /* Set the eo_tag value to -1 to indicate that that corresponding
       * submatch has not be completed yet */
      for (i = 0; i < parse_ctx.submatch_id; i++)
	{
	  submatch_data[i].eo_tag = -1;
	}
      tnfa->submatch_data = submatch_data;

      errcode = tre_add_tags(mem, stack, tree, tnfa);
      if (errcode != REG_OK)
	ERROR_EXIT(errcode);

#ifdef TRE_DEBUG
      for (i = 0; i < parse_ctx.submatch_id; i++)
	DPRINT(("pmatch[%d] = {t%d, t%d}\n",
		i, submatch_data[i].so_tag, submatch_data[i].eo_tag));
      for (i = 0; i <= tnfa->num_tags; i++)
	DPRINT(("t%d is %s\n", i, tag_dir_str[tag_directions[i]]));
#endif /* TRE_DEBUG */
    }

  /* Expand iteration nodes. */
  errcode = tre_expand_ast(mem, stack, tree, &parse_ctx.position,
			   tag_directions, &tnfa->params_depth);
  if (errcode != REG_OK)
    ERROR_EXIT(errcode);

  /* Add a dummy node for the final state.
     XXX - For certain patterns this dummy node can be optimized away,
	   for example "a*" or "ab*".	Figure out a simple way to detect
	   this possibility. */
  tmp_ast_l = tree;
  tmp_ast_r = tre_ast_new_literal(mem, 0, 0, parse_ctx.position++);
  if (tmp_ast_r == NULL)
    ERROR_EXIT(REG_ESPACE);

  tree = tre_ast_new_catenation(mem, tmp_ast_l, tmp_ast_r);
  if (tree == NULL)
    ERROR_EXIT(REG_ESPACE);

#ifdef TRE_DEBUG
  tre_ast_print(tree);
  DPRINT(("Number of states: %d\n", parse_ctx.position));
  if (submatch_data)
    for (i = 0; i < parse_ctx.submatch_id; i++)
      DPRINT(("pmatch[%d] = {t%d, t%d}\n",
	      i, submatch_data[i].so_tag, submatch_data[i].eo_tag));
  if (tag_directions)
    for (i = 0; i <= tnfa->num_tags; i++)
      DPRINT(("t%d is %s\n", i, tag_dir_str[tag_directions[i]]));
#endif /* TRE_DEBUG */

  errcode = tre_compute_nfl(mem, stack, tree);
  if (errcode != REG_OK)
    ERROR_EXIT(errcode);

  counts = xmalloc(sizeof(int) * parse_ctx.position);
  if (counts == NULL)
    ERROR_EXIT(REG_ESPACE);

  offs = xmalloc(sizeof(int) * parse_ctx.position);
  if (offs == NULL)
    ERROR_EXIT(REG_ESPACE);

  for (i = 0; i < parse_ctx.position; i++)
    counts[i] = 0;
  tre_ast_to_tnfa(tree, NULL, counts, NULL);

  add = 0;
  for (i = 0; i < parse_ctx.position; i++)
    {
      offs[i] = add;
      add += counts[i] + 1;
      counts[i] = 0;
    }
  transitions = xcalloc((unsigned)add + 1, sizeof(*transitions));
  if (transitions == NULL)
    ERROR_EXIT(REG_ESPACE);
  tnfa->transitions = transitions;
  tnfa->num_transitions = add;

  DPRINT(("Converting to TNFA:\n"));
  errcode = tre_ast_to_tnfa(tree, transitions, counts, offs);
  if (errcode != REG_OK)
    ERROR_EXIT(errcode);

  /* If in eight bit mode, compute a table of characters that can be the
     first character of a match. */
  tnfa->first_char = -1;
  if (TRE_MB_CUR_MAX_L(tnfa->loc) == 1 && !tmp_ast_l->nullable)
    {
      int count = 0;
      tre_cint_t k;
      DPRINT(("Characters that can start a match:"));
      tnfa->firstpos_chars = xcalloc(256, sizeof(char));
      if (tnfa->firstpos_chars == NULL)
	ERROR_EXIT(REG_ESPACE);
      for (p = tree->firstpos; p->position >= 0; p++)
	{
	  tre_tnfa_transition_t *j = transitions + offs[p->position];
	  while (j->state != NULL)
	    {
	      for (k = j->code_min; k <= j->code_max && k < 256; k++)
		{
		  DPRINT((" %d", k));
		  tnfa->firstpos_chars[k] = 1;
		  count++;
		}
	      j++;
	    }
	}
      DPRINT(("\n"));
#define TRE_OPTIMIZE_FIRST_CHAR 1
#if TRE_OPTIMIZE_FIRST_CHAR
      if (count == 1)
	{
	  for (k = 0; k < 256; k++)
	    if (tnfa->firstpos_chars[k])
	      {
		DPRINT(("first char must be %d\n", k));
		tnfa->first_char = k;
		xfree(tnfa->firstpos_chars);
		tnfa->firstpos_chars = NULL;
		break;
	      }
	}
#endif

    }
  else
    tnfa->firstpos_chars = NULL;


  p = tree->firstpos;
  i = 0;
  while (p->position >= 0)
    {
      i++;

#ifdef TRE_DEBUG
      {
	int *tags;
	DPRINT(("initial: %d", p->position));
	tags = p->tags;
	if (tags != NULL)
	  {
	    if (*tags >= 0)
	      DPRINT(("/"));
	    while (*tags >= 0)
	      {
		DPRINT(("%d", *tags));
		tags++;
		if (*tags >= 0)
		  DPRINT((","));
	      }
	  }
	DPRINT((", assert %d", p->assertions));
	if (p->params)
	  {
	    DPRINT((", "));
	    tre_print_params(p->params);
	  }
	DPRINT(("\n"));
      }
#endif /* TRE_DEBUG */

      p++;
    }

  initial = xcalloc((unsigned)i + 1, sizeof(tre_tnfa_transition_t));
  if (initial == NULL)
    ERROR_EXIT(REG_ESPACE);
  tnfa->initial = initial;

  i = 0;
  for (p = tree->firstpos; p->position >= 0; p++)
    {
      initial[i].state = transitions + offs[p->position];
      initial[i].state_id = p->position;
      initial[i].tags = NULL;
      /* Copy the arrays p->tags, and p->params, they are allocated
	 from a tre_mem object. */
      if (p->tags)
	{
	  int j;
	  for (j = 0; p->tags[j] >= 0; j++);
	  initial[i].tags = xmalloc(sizeof(*p->tags) * (j + 1));
	  if (!initial[i].tags)
	    ERROR_EXIT(REG_ESPACE);
	  memcpy(initial[i].tags, p->tags, sizeof(*p->tags) * (j + 1));
	}
      initial[i].params = NULL;
      if (p->params)
	{
	  initial[i].params = xmalloc(sizeof(*p->params) * TRE_PARAM_LAST);
	  if (!initial[i].params)
	    ERROR_EXIT(REG_ESPACE);
	  memcpy(initial[i].params, p->params,
		 sizeof(*p->params) * TRE_PARAM_LAST);
	}
      initial[i].assertions = p->assertions;
      i++;
    }
  initial[i].state = NULL;

  tnfa->num_transitions = add;
  tnfa->final = transitions + offs[tree->lastpos[0].position];
  tnfa->num_states = parse_ctx.position;
  tnfa->cflags = cflags;

  DPRINT(("final state %d (%p)\n", tree->lastpos[0].position,
	  (void *)tnfa->final));

  tre_mem_destroy(mem);
  tre_stack_destroy(stack);
  xfree(counts);
  xfree(offs);

#ifdef TRE_USE_SYSTEM_REGEX_H
  preg->re_magic = RE_MAGIC;
#endif /* TRE_USE_SYSTEM_REGEX_H */
  preg->TRE_REGEX_T_FIELD = (void *)tnfa;
#ifdef __LIBC__
  /* In Libc, we need to retain the locale.  Outside Libc, we already called
     duplocale() which does the retaining. */
  XL_RETAIN(tnfa->loc);
#endif /* __LIBC__ */
  return REG_OK;

 error_exit:
  /* Free everything that was allocated and return the error code. */
  tre_mem_destroy(mem);
  if (stack != NULL)
    tre_stack_destroy(stack);
  if (counts != NULL)
    xfree(counts);
  if (offs != NULL)
    xfree(offs);

  /* Set tnfa into preg, so that calling tre_free() will free the contents
     of tnfa.  But in Libc, NULL out the loc field since we never retained
     the locale.  Outside Libc, we let tre_free() call freelocale(). */
  preg->TRE_REGEX_T_FIELD = (void *)tnfa;
#ifdef __LIBC__
  if(tnfa) tnfa->loc = NULL;
#endif /* __LIBC__ */

  tre_free(preg);
  return errcode;
}




void
tre_free(regex_t *preg)
{
  tre_tnfa_t *tnfa;
  unsigned int i;
  tre_tnfa_transition_t *trans;

#ifdef TRE_USE_SYSTEM_REGEX_H
  preg->re_magic = 0;
#endif /* TRE_USE_SYSTEM_REGEX_H */
  tnfa = (void *)preg->TRE_REGEX_T_FIELD;
  if (!tnfa)
    return;
  preg->TRE_REGEX_T_FIELD = NULL;

  for (i = 0; i < tnfa->num_transitions; i++)
    if (tnfa->transitions[i].state)
      {
	if (tnfa->transitions[i].tags)
	  xfree(tnfa->transitions[i].tags);
	if (tnfa->transitions[i].assertions & ASSERT_BRACKET_MATCH)
	  xfree(tnfa->transitions[i].u.bracket_match_list);
	if (tnfa->transitions[i].params)
	  xfree(tnfa->transitions[i].params);
      }
  if (tnfa->transitions)
    xfree(tnfa->transitions);

  if (tnfa->initial)
    {
      for (trans = tnfa->initial; trans->state; trans++)
	{
	  if (trans->tags)
	    xfree(trans->tags);
	  if (trans->params)
	    xfree(trans->params);
	}
      xfree(tnfa->initial);
    }

  if (tnfa->submatch_data)
    {
      xfree(tnfa->submatch_data);
    }

  if (tnfa->tag_directions)
    xfree(tnfa->tag_directions);
  if (tnfa->firstpos_chars)
    xfree(tnfa->firstpos_chars);
  if (tnfa->minimal_tags)
    xfree(tnfa->minimal_tags);

  if (tnfa->loc)
#ifdef __LIBC__
    XL_RELEASE(tnfa->loc);
#else /* !__LIBC__ */
    freelocale(tnfa->loc);
#endif /* !__LIBC__ */

  if (tnfa->last_matched_branch)
    xfree(tnfa->last_matched_branch);

  xfree(tnfa);
}

#ifndef __LIBC__
char *
tre_version(void)
{
  static char str[256];
  char *version;

  if (str[0] == 0)
    {
      (void) tre_config(TRE_CONFIG_VERSION, &version);
      (void) snprintf(str, sizeof(str), "TRE %s (BSD)", version);
    }
  return str;
}

int
tre_config(int query, void *result)
{
  int *int_result = result;
  const char **string_result = result;

  switch (query)
    {
    case TRE_CONFIG_APPROX:
#ifdef TRE_APPROX
      *int_result = 1;
#else /* !TRE_APPROX */
      *int_result = 0;
#endif /* !TRE_APPROX */
      return REG_OK;

    case TRE_CONFIG_WCHAR:
#ifdef TRE_WCHAR
      *int_result = 1;
#else /* !TRE_WCHAR */
      *int_result = 0;
#endif /* !TRE_WCHAR */
      return REG_OK;

    case TRE_CONFIG_MULTIBYTE:
#ifdef TRE_MULTIBYTE
      *int_result = 1;
#else /* !TRE_MULTIBYTE */
      *int_result = 0;
#endif /* !TRE_MULTIBYTE */
      return REG_OK;

    case TRE_CONFIG_SYSTEM_ABI:
#ifdef TRE_CONFIG_SYSTEM_ABI
      *int_result = 1;
#else /* !TRE_CONFIG_SYSTEM_ABI */
      *int_result = 0;
#endif /* !TRE_CONFIG_SYSTEM_ABI */
      return REG_OK;

    case TRE_CONFIG_VERSION:
      *string_result = TRE_VERSION;
      return REG_OK;
    }

  return REG_NOMATCH;
}
#endif /* !__LIBC__ */


/* EOF */

/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#include <bitstring.h>

#ifndef __TRE_LAST_MATCHED_H__
#define __TRE_LAST_MATCHED_H__

#define __TRE_LAST_MATCHED_BRANCH_SHARED(_i)	\
  struct _tre_ ## _i *last_matched;		\
  int n_last_matched;				\
  int cmp_tag;					\
  int n_tags;
#define __TRE_LAST_MATCHED_SHARED(_i)		\
  tre_ ## _i ## _t *branches;			\
  int n_branches;				\
  int start_tag;

/* These structures record the relationship between each union branch and
 * its tags.  The end_tag is a special tag, created at the end of each branch
 * that can be used to detect which branch was last matched.  Then the tags
 * of the other branches can be set to unmatched.  For example:
 *
 *     ((a)|(b))*
 * 
 * when matched against "ab", the tags associated with the first branch, need
 * to be unset, because the last match was in the second branch.
 *
 * There are two sets of two structures.  The first structure records the
 * branch info, while the second records union info; what branches form that
 * union.  Because a branch may have nested unions, we need to record that
 * as well.  The "n" field of the branch info structure records the number
 * of unions at the top level of the branch (a union may itself have branches
 * with nested unions, but those union are only counted with the immediate
 * branch that contains them).  The "n" field of the union info structure is
 * the count of branches in that union.
 *
 * The "end_tag" field of a branch info structure is the number of the special
 * tag that is created at the end of each branch.  It can be used to determine
 * which branch was last matched.
 *
 * The first set (the info_pre structures) are used during tre_add_tags() to
 * record the tag info while tags are being added to the AST.  They use link
 * lists, and the total number of branch and union structures used are
 * recorded in n_branches and n_unions.  The second set (the info structures)
 * are created from the first, leaving out the link pointers (these structures
 * use arrays of structures, rather than link lists), and the n_branches and
 * n_unions fields are no longer needed.  The info_pre structures are allocated
 * using the tre_mem mechanism, while the info structure are allocated in
 * one chuck with xmalloc (so it can be easily deallocated).
 *
 * The above macro are used for the shared fields of the structures. */

struct _tre_last_matched_pre; /* forward reference */

typedef struct _tre_last_matched_branch_pre {
  struct _tre_last_matched_branch_pre *next;
  __TRE_LAST_MATCHED_BRANCH_SHARED(last_matched_pre)
  int tot_branches;
  int tot_last_matched;
  int tot_tags;
  bitstr_t tags[0];
} tre_last_matched_branch_pre_t;

typedef struct _tre_last_matched_pre {
  struct _tre_last_matched_pre *next;
  __TRE_LAST_MATCHED_SHARED(last_matched_branch_pre)
  int tot_branches;
  int tot_last_matched;
  int tot_tags;
} tre_last_matched_pre_t;

struct _tre_last_matched; /* forward reference */

typedef struct _tre_last_matched_branch {
  int *tags;
  __TRE_LAST_MATCHED_BRANCH_SHARED(last_matched)
} tre_last_matched_branch_t;

typedef struct _tre_last_matched {
  __TRE_LAST_MATCHED_SHARED(last_matched_branch)
} tre_last_matched_t;

#endif /* __TRE_LAST_MATCHED_H__ */

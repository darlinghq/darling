/*	$NetBSD: tdelete.c,v 1.2 1999/09/16 11:45:37 lukem Exp $	*/

/*
 * Tree search generalized from Knuth (6.2.2) Algorithm T just like
 * the AT&T man page says.
 *
 * The node_t structure is for internal use only, lint doesn't grok it.
 *
 * Written by reading the System V Interface Definition, not the code.
 *
 * Totally public domain.
 */

#include <sys/cdefs.h>
#if 0
#if defined(LIBC_SCCS) && !defined(lint)
__RCSID("$NetBSD: tdelete.c,v 1.2 1999/09/16 11:45:37 lukem Exp $");
#endif /* LIBC_SCCS and not lint */
#endif
__FBSDID("$FreeBSD: src/lib/libc/stdlib/tdelete.c,v 1.6 2003/01/05 02:43:18 tjr Exp $");

#define _SEARCH_PRIVATE
#include <search.h>
#include <stdlib.h>


/*
 * delete node with given key
 *
 * vkey:   key to be deleted
 * vrootp: address of the root of the tree
 * compar: function to carry out node comparisons
 */
void *
tdelete(const void * __restrict vkey, void ** __restrict vrootp,
    int (*compar)(const void *, const void *))
{
	node_t **rootp = (node_t **)vrootp;
	node_t *p, *q, *r;
	int cmp;

	if (rootp == NULL || (p = *rootp) == NULL)
		return NULL;

	while ((cmp = (*compar)(vkey, (*rootp)->key)) != 0) {
		p = *rootp;
		rootp = (cmp < 0) ?
		    &(*rootp)->llink :		/* follow llink branch */
		    &(*rootp)->rlink;		/* follow rlink branch */
		if (*rootp == NULL)
			return NULL;		/* key not found */
	}
	r = (*rootp)->rlink;			/* D1: */
	if ((q = (*rootp)->llink) == NULL)	/* Left NULL? */
		q = r;
	else if (r != NULL) {			/* Right link is NULL? */
		if (r->llink == NULL) {		/* D2: Find successor */
			r->llink = q;
			q = r;
		} else {			/* D3: Find NULL link */
			for (q = r->llink; q->llink != NULL; q = r->llink)
				r = q;
			r->llink = q->rlink;
			q->llink = (*rootp)->llink;
			q->rlink = (*rootp)->rlink;
		}
	}
	free(*rootp);				/* D4: Free node */
	*rootp = q;				/* link parent to new node */
	return p;
}

/*	$NetBSD: tfind.c,v 1.2 1999/09/16 11:45:37 lukem Exp $	*/

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
__RCSID("$NetBSD: tfind.c,v 1.2 1999/09/16 11:45:37 lukem Exp $");
#endif /* LIBC_SCCS and not lint */
#endif
__FBSDID("$FreeBSD: src/lib/libc/stdlib/tfind.c,v 1.5 2003/01/05 02:43:18 tjr Exp $");

#define _SEARCH_PRIVATE
#include <stdlib.h>
#include <search.h>

/* find a node, or return 0 */
void *
tfind(vkey, vrootp, compar)
	const void *vkey;		/* key to be found */
	void * const *vrootp;		/* address of the tree root */
	int (*compar)(const void *, const void *);
{
	node_t **rootp = (node_t **)vrootp;

	if (rootp == NULL)
		return NULL;

	while (*rootp != NULL) {		/* T1: */
		int r;

		if ((r = (*compar)(vkey, (*rootp)->key)) == 0)	/* T2: */
			return *rootp;		/* key found */
		rootp = (r < 0) ?
		    &(*rootp)->llink :		/* T3: follow left branch */
		    &(*rootp)->rlink;		/* T4: follow right branch */
	}
	return NULL;
}

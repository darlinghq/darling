/*
 * Copyright (c) 1989, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Newcomb.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * Taken from file_cmds:du.c
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include "dirstat_collection.h"

struct fileid_entry {
    struct fileid_entry *next;
    struct fileid_entry *previous;
    //int links;
    uint64_t fileid;
};


struct dirstat_fileid_set_s {
	struct fileid_entry **buckets;
	struct fileid_entry *free_list;
	size_t number_buckets;
	unsigned long number_entries;
	bool stop_allocating;
};

static const size_t links_hash_initial_size = 8192;

dirstat_fileid_set_t
_dirstat_fileid_set_create(void)
{
    dirstat_fileid_set_t ds = calloc(1, sizeof(dirstat_fileid_set_s));
    ds->number_buckets = links_hash_initial_size;
	ds->buckets = calloc(ds->number_buckets, sizeof(ds->buckets[0]));
    if (ds->buckets == NULL) {
        free(ds);
        return NULL;
    }
    return ds;
}

void
_dirstat_fileid_set_destroy(dirstat_fileid_set_t ds)
{
	struct fileid_entry *le;
    for (size_t i = 0; i < ds->number_buckets; i++) {
        while (ds->buckets[i] != NULL) {
            le = ds->buckets[i];
            ds->buckets[i] = le->next;
            free(le);
        }
    }
    free(ds->buckets);
    free(ds);
}

bool
_dirstat_fileid_set_add(dirstat_fileid_set_t ds, uint64_t fileid)
{
	struct fileid_entry *le, **new_buckets;
	size_t i, new_size;
	int hash;

	/* If the hash table is getting too full, enlarge it. */
	if (ds->number_entries > ds->number_buckets * 10 && !ds->stop_allocating) {
		new_size = ds->number_buckets * 2;
		new_buckets = calloc(new_size, sizeof(struct fileid_entry *));

		/* Try releasing the free list to see if that helps. */
		if (new_buckets == NULL && ds->free_list != NULL) {
			while (ds->free_list != NULL) {
				le = ds->free_list;
				ds->free_list = le->next;
				free(le);
			}
			new_buckets = calloc(new_size, sizeof(new_buckets[0]));
		}

		if (new_buckets == NULL) {
			ds->stop_allocating = 1;
		} else {
			for (i = 0; i < ds->number_buckets; i++) {
				while (ds->buckets[i] != NULL) {
					/* Remove entry from old bucket. */
					le = ds->buckets[i];
					ds->buckets[i] = le->next;

					/* Add entry to new bucket. */
					hash = (int)(fileid % new_size);

					if (new_buckets[hash] != NULL)
						new_buckets[hash]->previous =
						    le;
					le->next = new_buckets[hash];
					le->previous = NULL;
					new_buckets[hash] = le;
				}
			}
			free(ds->buckets);
			ds->buckets = new_buckets;
			ds->number_buckets = new_size;
		}
	}

	/* Try to locate this entry in the hash table. */
	hash = (int)(fileid % ds->number_buckets);
	for (le = ds->buckets[hash]; le != NULL; le = le->next) {
		if (le->fileid == fileid) {
#if 0 // TODO
			/*
			 * Save memory by releasing an entry when we've seen
			 * all of its links.
			 */
			if (--le->links <= 0) {
				if (le->previous != NULL)
					le->previous->next = le->next;
				if (le->next != NULL)
					le->next->previous = le->previous;
				if (buckets[hash] == le)
					buckets[hash] = le->next;
				number_entries--;
				/* Recycle this node through the free list */
				if (stop_allocating) {
					free(le);
				} else {
					le->next = free_list;
					free_list = le;
				}
			}
#endif
			return true;
		}
	}

	if (ds->stop_allocating)
		return false;

	/* Add this entry to the links cache. */
	if (ds->free_list != NULL) {
		/* Pull a node from the free list if we can. */
		le = ds->free_list;
		ds->free_list = le->next;
	} else {
		/* Malloc one if we have to. */
		le = malloc(sizeof(struct fileid_entry));
    }
	if (le == NULL) {
		ds->stop_allocating = 1;
		return false;
	}
	le->fileid = fileid;
	ds->number_entries++;
	le->next = ds->buckets[hash];
	le->previous = NULL;
	if (ds->buckets[hash] != NULL)
		ds->buckets[hash]->previous = le;
	ds->buckets[hash] = le;
	return false;
}

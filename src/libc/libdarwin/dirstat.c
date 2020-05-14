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

#include <strings.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/fsctl.h>
#include <sys/vnode.h>
#include <sys/errno.h>

#include <os/assumes.h>

#include <TargetConditionals.h>

#include "dirstat.h"
#include "dirstat_collection.h"

#if !TARGET_OS_SIMULATOR
#define HAS_APFS
#endif

#ifdef HAS_APFS
#include <apfs/apfs_fsctl.h>
#endif

#if DEBUG
#define DEBUGPRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUGPRINT(...) do { } while(0)
#endif

static int
fdirstat_fallback(int fd, int flags, struct dirstat *ds);

#ifdef HAS_APFS
static int
fdirstat(int fd, int flags, struct dirstat *ds)
{
	struct apfs_dir_stats_ext dstats = {0};
	if (flags & DIRSTAT_FAST_ONLY) {
		dstats.flags |= APFS_DIR_STATS_FAST_PATH;
	}

	int err = ffsctl(fd, APFSIOC_GET_DIR_STATS_EXT, &dstats, 0);
	if (err == -1) {
		if (errno == ENOENT) {
			// <rdar://problem/31696225>
			errno = ENOTSUP;
		}
		return -1;
	}

	ds->total_size = dstats.total_size;
	ds->descendants = dstats.num_children;

	return 0;
}
#endif

int
dirstatat_np(int dfd, const char *path, int flags, struct dirstat *ds_out, size_t ds_size)
{
#ifdef HAS_APFS
	// <rdar://problem/32794924>
	// Until APFS directory sizing is fixed, only the fallback path is
	// available.
	flags |= DIRSTAT_FORCE_FALLBACK;

	// FORCE_FALLBACK trumps FAST_ONLY.  Make sure to set errno accordingly in
	// the case that a confused caller asks for both.
	if ((flags & (DIRSTAT_FAST_ONLY)) && (flags & DIRSTAT_FORCE_FALLBACK)) {
		errno = ENOTSUP;
		return -1;
	}
#endif

	int fd = openat(dfd, path, O_RDONLY | O_DIRECTORY);
	DEBUGPRINT("Opened %d:%s as %d\n", dfd, path, fd);
	if (fd == -1) return -1;

	struct dirstat ds = {};
	int ret = -1;

#ifdef HAS_APFS
	if (!(flags & DIRSTAT_FORCE_FALLBACK)) {
		ret = fdirstat(fd, flags, &ds);
	}

	if (ret == -1 && ((flags & DIRSTAT_FORCE_FALLBACK) || ((errno == ENOTTY) && !(flags & DIRSTAT_FAST_ONLY)))) {
		ret = fdirstat_fallback(fd, flags, &ds);
	}
#else
	ret = fdirstat_fallback(fd, flags, &ds);
#endif
	int saved_errno = errno;

	if (ds_size >= sizeof(ds)) {
		memcpy(ds_out, &ds, sizeof(ds));
	} else {
		memcpy(ds_out, &ds, ds_size);
	}

	close(fd);

	errno = saved_errno;
	return ret;
}

int
dirstat_np(const char *path, int flags, struct dirstat *ds, size_t ds_size)
{
	return dirstatat_np(AT_FDCWD, path, flags, ds, ds_size);
}

#pragma mark Fallback

struct dirqueue_entry {
	STAILQ_ENTRY(dirqueue_entry) entries;
	char *path;
};

static int
fdirstat_fallback(int parent_fd, int flags, struct dirstat *ds)
{
	int reterror = 0;

	/*
	 * This method of gathering disk usage is the fastest by far over other
	 * methods using fts or opendir/readdir + getattrlist or stat to gather
	 * information about filesystem usage.  That's because this method avoids
	 * creating vnodes for each item in a directory.  We implement a recursive
	 * filesystem search by appending each directory child found to a
	 * processing queue, and then process each child directory in that queue on
	 * a FIFO basis resulting in a breadth-first traversal of the filesystem.
	 * This keeps our actual implementation iterative to avoid deep filesystem
	 * hierarchies overflowing our stack.
	 */

	dirstat_fileid_set_t fileid_seen = _dirstat_fileid_set_create();
	STAILQ_HEAD(, dirqueue_entry) dirqueue_head = STAILQ_HEAD_INITIALIZER(dirqueue_head);

	struct attrlist attrlist = {
		.bitmapcount = ATTR_BIT_MAP_COUNT,
		.commonattr =  ATTR_CMN_RETURNED_ATTRS | ATTR_CMN_ERROR |
				ATTR_CMN_NAME | ATTR_CMN_OBJTYPE | ATTR_CMN_FILEID,
		.dirattr = ATTR_DIR_ENTRYCOUNT,
		.fileattr = ATTR_FILE_LINKCOUNT | ATTR_FILE_ALLOCSIZE |
				ATTR_FILE_DATAALLOCSIZE,
	};

	typedef struct {
		/*
		 * fields are in order of possible return in buffer (but note that data
		 * is packed in the actual buffer, and only relevant fields are
		 * returned)
		 */
		uint32_t length;
		attribute_set_t returned;       //ATTR_CMN_RETURNED_ATTRS
		uint32_t error;                 //ATTR_CMN_ERROR
		attrreference_t item_name_info; //ATTR_CMN_NAME
		fsobj_type_t type;              //ATTR_CMN_OBJTYPE
		uint64_t fileid;                //ATTR_CMN_FILEID
		union {
			struct {
				u_int32_t entry_count;  //ATTR_DIR_ENTRYCOUNT
			};
			struct {
				u_int32_t link_count;   //ATTR_FILE_LINKCOUNT
				off_t alloc_size;       //ATTR_FILE_ALLOCSIZE
				off_t data_alloc_size;  //ATTR_FILE_DATAALLOCSIZE
			};
		};
	} max_attr_entry_t;

	size_t attrbuf_len = (32 * 1024);
	char *attrbuf = alloca(attrbuf_len);

#ifdef HAS_APFS
	os_assert(!(flags & DIRSTAT_FAST_ONLY));
#endif

	do {
		int fd = -1;
		char *path;

		if (STAILQ_EMPTY(&dirqueue_head)) {
			fd = parent_fd;
			path = NULL;
		} else {
			struct dirqueue_entry *dqe = STAILQ_FIRST(&dirqueue_head);
			STAILQ_REMOVE_HEAD(&dirqueue_head, entries);
			path = dqe->path;
			free(dqe);

			fd = openat(parent_fd, path, O_RDONLY | O_DIRECTORY);

			if (fd < 0) {
				DEBUGPRINT( "Unable to open directory %d:%s => %s\n", parent_fd, path, strerror(errno));
				continue;
			}
		}

		while (1) {
			int ret_entry_count = getattrlistbulk(fd, &attrlist, attrbuf, attrbuf_len, 0);
			if (-1 == ret_entry_count) {
				if (fd == parent_fd) {
					reterror = errno;
				}
				DEBUGPRINT( "getattrlistbulk on in %s returned error %s\n", path, strerror(errno));
				break;
			} else if (0 == ret_entry_count) {
				break;
			} else {
				char *cursor = NULL; //pointer into attrbuf
				char *entry_start = attrbuf;

				for (int index = 0; index < ret_entry_count; index++) {
					max_attr_entry_t attrs = {0};
					char *name = NULL;

					cursor = entry_start;

					memcpy(&attrs.length, cursor, sizeof(attrs.length));
					cursor += sizeof(attrs.length);

					/* set starting point for next entry */
					entry_start += attrs.length;

					memcpy(&attrs.returned, cursor, sizeof(attrs.returned));
					cursor += sizeof(attrs.returned);

					if (attrs.returned.commonattr & ATTR_CMN_ERROR) {
						memcpy(&attrs.error, cursor, sizeof(attrs.error));
						cursor += sizeof(attrs.error);
					}

					if (attrs.error) {
						DEBUGPRINT( "Got error %s while processing in %s\n", strerror(errno), path);
						continue;
					}

					if (attrs.returned.commonattr & ATTR_CMN_NAME) {
						memcpy(&attrs.item_name_info, cursor, sizeof(attrs.item_name_info));
						name = cursor + attrs.item_name_info.attr_dataoffset;
						if (name + attrs.item_name_info.attr_length > entry_start) {
							name = NULL;
						}
						cursor += sizeof(attrs.item_name_info);
					}

					if (attrs.returned.commonattr & ATTR_CMN_OBJTYPE) {
						memcpy(&attrs.type, cursor, sizeof(attrs.type));
						cursor += sizeof(attrs.type);
					}

					if (attrs.returned.commonattr & ATTR_CMN_FILEID) {
						memcpy(&attrs.fileid, cursor, sizeof(attrs.fileid));
						cursor += sizeof(attrs.fileid);
					}

					if (VDIR == attrs.type) {
						if (attrs.returned.dirattr & ATTR_DIR_ENTRYCOUNT) {
							memcpy(&attrs.entry_count, cursor, sizeof(attrs.entry_count));
							cursor += sizeof(attrs.entry_count);
						} else {
							// Fake it so we go down the right path below
							attrs.entry_count = -1;
						}

						// avoid descending into empty directories
						if (attrs.entry_count && name) {
							struct dirqueue_entry *dqe = malloc(sizeof(struct dirqueue_entry));
							if (path == NULL) {
								dqe->path = strdup(name);
							} else {
								asprintf(&dqe->path, "%s/%s", path, name);
							}

							if (dqe->path != NULL) {
								STAILQ_INSERT_TAIL(&dirqueue_head, dqe, entries);
							} else {
								DEBUGPRINT( "Unable to create dqe\n");
								free(dqe);
							}
						} else if (attrs.entry_count != 0) {
							DEBUGPRINT( "Failed to get name for item in %s\n", path);
						} else if (attrs.entry_count == 0) {
							// Empty directory, nothing to do
						}
					} else {
						off_t object_size = 0;

						if (attrs.returned.fileattr & ATTR_FILE_LINKCOUNT) {
							memcpy(&attrs.link_count, cursor, sizeof(attrs.link_count));
							cursor += sizeof(attrs.link_count);
						}

						if (attrs.returned.fileattr & ATTR_FILE_ALLOCSIZE) {
							memcpy(&attrs.alloc_size, cursor, sizeof(attrs.alloc_size));
							cursor += sizeof(attrs.alloc_size);
							object_size = attrs.alloc_size;
						}

						if (attrs.returned.fileattr & ATTR_FILE_DATAALLOCSIZE) {
							memcpy(&attrs.data_alloc_size, cursor, sizeof(attrs.data_alloc_size));
							cursor += sizeof(attrs.data_alloc_size);
							if (0 == object_size) {
								object_size = attrs.data_alloc_size;
							}
						}

						if (1 == attrs.link_count) {
							ds->total_size += object_size;
						} else {
							bool seen_fileid = _dirstat_fileid_set_add(fileid_seen, attrs.fileid);
							if (!seen_fileid) {
								ds->total_size += object_size;
							} else {
								DEBUGPRINT( "Skipping hardlinked file at %s/%s\n", path, name);
							}
						}
					}
					ds->descendants++;
				}
			}
		}

		if (path) {
			close(fd);
			free(path);
		}
	} while (!STAILQ_EMPTY(&dirqueue_head));

	_dirstat_fileid_set_destroy(fileid_seen);

	if (reterror) {
		errno = reterror;
		return -1;
	} else {
		return 0;
	}
}

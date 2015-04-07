/*
 * Copyright (c) 2009-2010 Apple Inc. All rights reserved.
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

#ifndef _PATHWATCH_H_
#define _PATHWATCH_H_

#include <dispatch/dispatch.h>
/*
 * types for virtual path nodes (path_node_t)
 */
#define PATH_NODE_TYPE_GHOST 0
#define PATH_NODE_TYPE_FILE  1
#define PATH_NODE_TYPE_LINK  2
#define PATH_NODE_TYPE_DIR   3
#define PATH_NODE_TYPE_OTHER 4


enum
{
	PATH_NODE_DELETE = 0x0001, /* node or path deleted */
	PATH_NODE_WRITE  = 0x0002, /* node written */
	PATH_NODE_EXTEND = 0x0004, /* node extended */
	PATH_NODE_ATTRIB = 0x0008, /* node attributes changed (mtime or ctime) */
	PATH_NODE_LINK	 = 0x0010, /* node link count changed */
	PATH_NODE_RENAME = 0x0020, /* node renamed, always accompanied by PATH_NODE_DELETE */
	PATH_NODE_REVOKE = 0x0040, /* access revoked, always accompanied by PATH_NODE_DELETE */
	PATH_NODE_CREATE = 0x0080, /* path created or access re-acquired */
	PATH_NODE_MTIME  = 0x0100, /* path mtime changed, always accompanied by PATH_NODE_ATTRIB */
	PATH_NODE_CTIME  = 0x0200  /* path ctime changed, always accompanied by PATH_NODE_ATTRIB */
};

/* all bits mask */
#define PATH_NODE_ALL 0x000003ff
/* src is suspended */
#define PATH_SRC_SUSPENDED 0x10000000

/* Path changes coalesce for 100 milliseconds */
#define PNODE_COALESCE_TIME 100000000

/*
 * path_node_t represents a virtual path
 */
typedef struct
{
	char *path;
	size_t plen;
	uid_t uid;
	gid_t gid;
	uint32_t pname_count;
	char **pname;
	uint32_t type;
	uint32_t flags;
	dispatch_source_t src;
	dispatch_queue_t src_queue;
	void *contextp;
	uint32_t context32;
	uint64_t context64;
	uint32_t refcount;
} path_node_t;

path_node_t *path_node_create(const char *path, uid_t uid, gid_t gid, uint32_t mask, dispatch_queue_t queue);
void path_node_close(path_node_t *pnode);

#endif /* _PATHWATCH_H_ */

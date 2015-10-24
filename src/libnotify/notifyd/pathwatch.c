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

/*
 * These routines, accessed through path_node_create() and path_node_release(),
 * provide an API for monitoring a path in the filesystem.  The path may contain
 * directories and symbolic links.  The path may not even exist!  If the path
 * does exist, this code will respond to path deletions, component renaming, and
 * access control changes that either delete the path or make it inaccessible to a
 * target user/group.  A notification will be provided if the path comes back into
 * existance or again becomes accessible.
 * 
 * path_node_create() returns a path_node_t object, which contains a dispatch_source_t.
 * This source behaves very much like a DISPATCH_SOURCE_TYPE_VNODE, except that it also
 * triggers on the creation of a path.
 * 
 * Internally, the work of monitoring a path is done by a set of helper vnode_t
 * objects. A vnode_t contains a dispatch_source_t (of type DISPATCH_SOURCE_TYPE_VNODE)
 * for a particular vnode.  When a path_node_t is created, it creates (or shares)
 * vnode_t objects for each component of the desired path.  For example, a path_node_t
 * for "/a/b/c" will create (or share, if some other path_node_t has already created) a
 * dispatch_source_t for "/", "/a", "/a/b", and "/a/b/c".  If any of these sources is
 * notified of a change, the vnode_t will trigger an update for all path_node_t
 * objects that contain that path component.
 * 
 * When a path_node_t update is triggered by a vnode_t component, the node re-evaluates
 * the target path that it is charged with monitoring.  If the path exists and the end-point
 * vnode changed, then the update operation will trigger its dispatch_source_t to notify the
 * end-user of the change.  If an intermediate path component is removed, renamed, or becomes
 * blocked by an access-control change, then the end-point dispatch_source_t is triggered to
 * indicate that the path has been deleted.  However, the path_node_t remains active and
 * monitors the path components that still exist.  Eventually, if the path is recreated or
 * if access controls change so that the path becomes visible to the target user, then the
 * end-point dispatch_source_t is triggered with a PATH_NODE_CREATE bit set in its data flags.
 *
 * path_node_releases() releases a path_node_t object and all of the vnode_t objects
 * that were monitoring components of its target path.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/syscall.h>
#include <sys/kauth.h>
#include <pwd.h>
#include <fcntl.h>
#include <assert.h>
#include <tzfile.h>
#include "pathwatch.h"

#define forever for(;;)
#define streq(A,B) (strcmp(A,B)==0)
#define DISPATCH_VNODE_ALL 0x7f

#define PATH_STAT_OK 0
#define PATH_STAT_FAILED 1
#define PATH_STAT_ACCESS 2

#define VPATH_NODE_TYPE_REG 0
#define VPATH_NODE_TYPE_LINK 1
#define VPATH_NODE_TYPE_DELETED 2

#define DISPATCH_VNODE_UNAVAIL (DISPATCH_VNODE_DELETE | DISPATCH_VNODE_RENAME | DISPATCH_VNODE_REVOKE)

/* Libinfo global */
extern uint32_t gL1CacheEnabled;

/*
 * vnode_t represents a vnode.
 * 
 * The dispatch source is of type DISPATCH_SOURCE_TYPE_VNODE for file descriptor fd.
 * The handler for the source triggers an update routine for all the path_node_t
 * objects in the path_node list.
 */
typedef struct
{
	char *path;
	uint32_t type;
	int fd;
	struct timespec mtime;
	struct timespec ctime;
	dispatch_source_t src;
	uint32_t path_node_count;
	path_node_t **path_node;
} vnode_t;

static struct
{
	dispatch_once_t pathwatch_init;
	dispatch_queue_t pathwatch_queue;
	uint32_t vnode_count;
	vnode_t **vnode;
	char *tzdir;
	size_t tzdir_len;
} _global = {0};

/* forward */
static void _path_node_update(path_node_t *pnode, uint32_t flags, vnode_t *vnode);

/*
 * stat() or lstat() a path as a particular user/group.
 */
static int
_path_stat(const char *path, int link, uid_t uid, gid_t gid)
{
	struct stat sb;
	gid_t orig_gidset[NGROUPS_MAX];
	int ngroups, status, stat_status;
	struct passwd *p;
	uint32_t orig_cache_enabled;

	/* disable L1 cache to avoid notification deadlock */
	orig_cache_enabled = gL1CacheEnabled;
	gL1CacheEnabled = 0;

	/* get my group list */
	memset(orig_gidset, 0, sizeof(orig_gidset));
	ngroups = getgroups(NGROUPS_MAX, orig_gidset);
	if (ngroups < 0)
	{
		return PATH_STAT_FAILED;
	}

	/* look up user name */
	p = getpwuid(uid);
	if (p == NULL)
	{
		gL1CacheEnabled = orig_cache_enabled;
		return PATH_STAT_FAILED;
	}

	/* switch to user's grouplist */
	status = initgroups(p->pw_name, gid);
	if (status < 0)
	{
		gL1CacheEnabled = orig_cache_enabled;
		return PATH_STAT_FAILED;
	}

	/* reset gL1CacheEnabled */
	gL1CacheEnabled = orig_cache_enabled;

	/* set thread credentials */
	pthread_setugid_np(uid, gid);

	/* stat the file */
	stat_status = -1;
	if (link != 0)
	{
		stat_status = lstat(path, &sb);
	}
	else
	{
		stat_status = stat(path, &sb);
	}

	/* unset thread credentials */
	pthread_setugid_np(KAUTH_UID_NONE, KAUTH_GID_NONE);

	/* restore original grouplist for UID 0 */
	status = syscall(SYS_initgroups, ngroups, orig_gidset, 0);
	if (status < 0)
	{
		return PATH_STAT_FAILED;
	}

	/* return status */
	if (stat_status == 0)
	{
		return PATH_STAT_OK;
	}

	if (errno == EACCES)
	{
		return PATH_STAT_ACCESS;
	}

	return PATH_STAT_FAILED;
}

/*
 * Check access to a path by a particular user/group.
 * Sets ftype output parameter if it is non-NULL.
 */
static int
_path_stat_check_access(const char *path, uid_t uid, gid_t gid, uint32_t *ftype)
{
	struct stat sb;
	char buf[MAXPATHLEN + 1];
	int status, t;

	if (path == NULL) return PATH_STAT_FAILED;

	if (ftype != NULL) *ftype = PATH_NODE_TYPE_GHOST;

	/* Paths must be absolute */
	if (path[0] != '/') return PATH_STAT_FAILED;

	/* Root dir is readable */
	if (path[1] == '\0')
	{
		if (ftype != NULL) *ftype = PATH_NODE_TYPE_DIR;
		return PATH_STAT_OK;
	}

	memset(&sb, 0, sizeof(struct stat));
	status = lstat(path, &sb);

	if (status != 0) return PATH_STAT_FAILED;
	else if ((sb.st_mode & S_IFMT) == S_IFDIR) t = PATH_NODE_TYPE_DIR;
	else if ((sb.st_mode & S_IFMT) == S_IFREG) t = PATH_NODE_TYPE_FILE;
	else if ((sb.st_mode & S_IFMT) == S_IFLNK) t = PATH_NODE_TYPE_LINK;
	else t = PATH_NODE_TYPE_OTHER;

	if (ftype != NULL) *ftype = t;

	if (t == PATH_NODE_TYPE_OTHER) return PATH_STAT_FAILED;

	/* skip access control check if uid is zero */
	if (uid == 0) return 0;

	/* special case: anything in the timezone directory is OK */
	memset(buf, 0, sizeof(buf));
	if (realpath(path, buf) == NULL) return PATH_STAT_FAILED;
	if ((_global.tzdir != NULL) && (!strncasecmp(buf, _global.tzdir, _global.tzdir_len)))
	{
		return PATH_STAT_OK;
	}

	/* call _path_stat to check access as the user/group provided */
	if (t == PATH_NODE_TYPE_FILE)
	{
		status = _path_stat(path, 0, uid, gid);
		if ((status == PATH_STAT_ACCESS) && (ftype != NULL)) *ftype = PATH_NODE_TYPE_GHOST;
		return status;
	}
	else if (t == PATH_NODE_TYPE_LINK)
	{
		status = _path_stat(path, 1, uid, gid);
		if ((status == PATH_STAT_ACCESS) && (ftype != NULL)) *ftype = PATH_NODE_TYPE_GHOST;
		return status;
	}
	else if (t == PATH_NODE_TYPE_DIR)
	{
		snprintf(buf, MAXPATHLEN, "%s/.", path);
		status = _path_stat(buf, 0, uid, gid);
		if ((status == PATH_STAT_ACCESS) && (ftype != NULL)) *ftype = PATH_NODE_TYPE_GHOST;
		return status;
	}

	/* we don't ever get here, but... */
	return PATH_STAT_FAILED;
}

/*
 * Uniquely add a pnode to a vnode's list of path nodes.
 */
static void
_vnode_add_pnode(vnode_t *vnode, path_node_t *pnode)
{
	uint32_t i;

	for (i = 0; i < vnode->path_node_count; i++)
	{
		if (vnode->path_node[i] == pnode) return;
	}

	for (i = 0; i < vnode->path_node_count; i++)
	{
		if (vnode->path_node[i] == NULL)
		{
			vnode->path_node[i] = pnode;
			return;
		}
	}

	if (vnode->path_node_count == 0)
	{
		vnode->path_node = (path_node_t **)calloc(1, sizeof(path_node_t *));
	}
	else
	{
		vnode->path_node = (path_node_t **)reallocf(vnode->path_node, (vnode->path_node_count + 1) * sizeof(path_node_t *));
	}

	assert(vnode->path_node != NULL);

	vnode->path_node[vnode->path_node_count++] = pnode;
}

/*
 * Free a vnode_t and cancel/release its dispatch source.
 */
static void
_vnode_free(vnode_t *vnode)
{
	dispatch_source_cancel(vnode->src);

	/*
	 * Actually free the vnode on the pathwatch queue.  This allows any
	 * enqueued _vnode_event operations to complete before the vnode disappears.
	 * _vnode_event() quietly returns if the source has been cancelled.
	 */
	dispatch_async(_global.pathwatch_queue, ^{
		dispatch_release(vnode->src);
		free(vnode->path);
		free(vnode->path_node);
		free(vnode);
	});
}

/*
 * Handler routine for vnode_t objects.
 * Invokes the _path_node_update routine for all of the vnode's pnodes.
 */
static void
_vnode_event(vnode_t *vnode)
{
	uint32_t i, flags;
	unsigned long ulf;
	struct stat sb;

	if (vnode == NULL) return;
	if ((vnode->src != NULL) && (dispatch_source_testcancel(vnode->src))) return;

	ulf = dispatch_source_get_data(vnode->src);
	flags = ulf;

	memset(&sb, 0, sizeof(struct stat));
	if (fstat(vnode->fd, &sb) == 0)
	{
		if ((vnode->mtime.tv_sec != sb.st_mtimespec.tv_sec) || (vnode->mtime.tv_nsec != sb.st_mtimespec.tv_nsec))
		{
			flags |= PATH_NODE_MTIME;
			vnode->mtime = sb.st_mtimespec;
		}

		if ((vnode->ctime.tv_sec != sb.st_ctimespec.tv_sec) || (vnode->ctime.tv_nsec != sb.st_ctimespec.tv_nsec))
		{
			flags |= PATH_NODE_CTIME;
			vnode->ctime = sb.st_ctimespec;
		}
	}

	/*
	 * Flag deleted sources.
	 * We can't delete them here, since _path_node_update may need them.
	 * However, _path_node_update will release them and they will get cleaned
	 * up in a _vnode_sweep later on.
	 */
	if (flags & DISPATCH_VNODE_DELETE) vnode->type = VPATH_NODE_TYPE_DELETED;

	for (i = 0; i < vnode->path_node_count; i++)
	{
		_path_node_update(vnode->path_node[i], flags, vnode);
	}
}

/*
 * Creates a vnode_t object.
 */
static vnode_t *
_vnode_create(const char *path, uint32_t type, path_node_t *pnode)
{
	int fd, flags;
	uint32_t i;
	vnode_t *vnode;
	dispatch_source_t src;
	struct stat sb;

	if (path == NULL) path = "/";
	if (path[0] == '\0') path = "/";

	for (i = 0; i < _global.vnode_count; i++)
	{
		vnode = _global.vnode[i];
		if (vnode == NULL) continue;

		if ((vnode->type == type) && (streq(path, vnode->path)))
		{
			_vnode_add_pnode(vnode, pnode);
			return vnode;
		}
	}

	vnode = NULL;

	flags = O_EVTONLY;
	if (type == VPATH_NODE_TYPE_LINK) flags |= O_SYMLINK;

	fd = open(path, flags, 0);
	if (fd < 0) return NULL;

	src = dispatch_source_create(DISPATCH_SOURCE_TYPE_VNODE, (uintptr_t)fd, DISPATCH_VNODE_ALL, _global.pathwatch_queue);
	if (src == NULL)
	{
		close(fd);
		return NULL;
	}

	vnode = (vnode_t *)calloc(1, sizeof(vnode_t));
	assert(vnode != NULL);

	vnode->type = type;
	vnode->path = strdup(path);
	assert(vnode->path != NULL);

	vnode->fd = fd;
	vnode->src = src;

	memset(&sb, 0, sizeof(struct stat));
	if (fstat(fd, &sb) == 0)
	{
		vnode->mtime = sb.st_mtimespec;
		vnode->ctime = sb.st_ctimespec;
	}

	_vnode_add_pnode(vnode, pnode);

	dispatch_source_set_event_handler(src, ^{ _vnode_event(vnode); });
	dispatch_source_set_cancel_handler(src, ^{ close(fd); });

	if (_global.vnode_count == 0)
	{
		_global.vnode = (vnode_t **)calloc(1, sizeof(vnode_t *));
	}
	else
	{
		_global.vnode = (vnode_t **)reallocf(_global.vnode, (_global.vnode_count + 1) * sizeof(vnode_t *));
	}

	assert(_global.vnode != NULL);

	_global.vnode[_global.vnode_count++] = vnode;

	dispatch_resume(src);

	return vnode;
}

static vnode_t *
_vnode_create_real_path(const char *path, uint32_t type, path_node_t *pnode)
{
	char real[MAXPATHLEN + 1];

	if (path == NULL) return _vnode_create(path, type, pnode);

	if (NULL != realpath(path, real)) return _vnode_create(real, type, pnode);

	return NULL;
}

/*
 * Examines all the vnode_t objects (held in the _global data),
 * frees any that have no path nodes.
 */
static void
_vnode_sweep()
{
	uint32_t i, j, new_vnode_count, new_path_node_count;
	vnode_t **new_source, *vnode;
	path_node_t **new_path_node;

	new_source = NULL;

	for (i = 0; i < _global.vnode_count; i++)
	{
		vnode = _global.vnode[i];
		if (vnode == NULL) continue;

		new_path_node_count = 0;
		new_path_node = NULL;

		for (j = 0; j < vnode->path_node_count; j++)
		{
			if (vnode->path_node[j] != NULL) new_path_node_count++;
		}

		if (new_path_node_count == vnode->path_node_count)
		{
			/* no change */
			continue;
		}
		else if (new_path_node_count > 0)
		{
			new_path_node = (path_node_t **)calloc(new_path_node_count, sizeof(path_node_t *));
			assert(new_path_node != NULL);

			new_path_node_count = 0;
			for (j = 0; j < vnode->path_node_count; j++)
			{
				if (vnode->path_node[j] != NULL) 
				{
					new_path_node[new_path_node_count++] = vnode->path_node[j];
				}
			}
		}

		free(vnode->path_node);
		vnode->path_node = new_path_node;
		vnode->path_node_count = new_path_node_count;
	}

	new_vnode_count = 0;
	for (i = 0; i < _global.vnode_count; i++)
	{
		vnode = _global.vnode[i];
		if (vnode == NULL) continue;
		if (vnode->path_node_count > 0) new_vnode_count++;
	}

	if (new_vnode_count == _global.vnode_count)
	{
		/* no change */
		return;
	}
	else if (new_vnode_count > 0)
	{
		new_source = (vnode_t **)calloc(new_vnode_count, sizeof(vnode_t *));
		assert(new_source != NULL);

		new_vnode_count = 0;
		for (i = 0; i < _global.vnode_count; i++)
		{
			vnode = _global.vnode[i];
			if (vnode == NULL) continue;

			if (vnode->path_node_count > 0) 
			{
				new_source[new_vnode_count++] = vnode;
			}
			else
			{
				_vnode_free(vnode);
			}
		}
	}

	free(_global.vnode);
	_global.vnode = new_source;
	_global.vnode_count = new_vnode_count;
}

/*
 * Releases sources that have a particular node on their list.
 * This is a deferred release mechanism for vnode_t objects.
 * The calling routine must call _vnode_sweep subsequent to
 * calling this routine.
 * _vnode_sweep will actually free any vnode_t objects
 * that have a no path nodes.
 */
static void
_vnode_release_for_node(path_node_t *pnode)
{
	uint32_t i, j;
	vnode_t *vnode;

	for (i = 0; i < _global.vnode_count; i++)
	{
		vnode = _global.vnode[i];
		if (vnode == NULL) continue;

		for (j = 0; j < vnode->path_node_count; j++)
		{
			if (vnode->path_node[j] == pnode)
			{
				vnode->path_node[j] = NULL;
				break;
			}
		}
	}
}

/*
 * Retain a path_node_t object.
 * Dispatched on _global.pathwatch_queue.
 */
static void
_path_node_retain(path_node_t *pnode)
{	
	if (pnode == NULL) return;
	pnode->refcount++;
}

/*
 * Free a path_node_t object.
 * Dispatched on _global.pathwatch_queue.
 */
static void
_path_node_free(path_node_t *pnode)
{
	uint32_t i, n;

	if (pnode == NULL) return;

	/*
	 * Remove this path node from all vnodes.
	 */
	_vnode_release_for_node(pnode);
	_vnode_sweep();

	free(pnode->path);

	if (pnode->pname != NULL)
	{
		n = pnode->pname_count;
		pnode->pname_count = 0;

		for (i = 0; i < n; i++)
		{
			free(pnode->pname[i]);
			pnode->pname[i] = NULL;
		}

		free(pnode->pname);
	}

	free(pnode->contextp);

	dispatch_release(pnode->src);
	dispatch_release(pnode->src_queue);

	memset(pnode, 0, sizeof(path_node_t));
	free(pnode);
}

/*
 * Release a path_node_t object.
 */
static void
_path_node_release(path_node_t *pnode)
{
	if (pnode == NULL) return;

	/*
	 * We need to make sure that the node's event handler isn't currently
	 * executing before freeing the node.  We dispatch on the src_queue, so
	 * that when the block executes there will be no more events in the queue.
	 * From there, we dispatch async back to the pathwatch_queue to do the
	 * data structure cleanup.
	 */
	dispatch_async(pnode->src_queue, ^{
		dispatch_async(_global.pathwatch_queue, ^{
			if (pnode->refcount > 0) pnode->refcount--;
			if (pnode->refcount == 0) _path_node_free(pnode);
		});
	});
}

/*
 * Frees a path_node_t object.
 * The work is actually done on the global pathwatch_queue to make this safe.
 */
void
path_node_close(path_node_t *pnode)
{
	if (pnode == NULL) return;

	if (pnode->src != NULL) dispatch_source_cancel(pnode->src);
	_path_node_release(pnode);
}

static void
_pathwatch_init()
{
	char buf[MAXPATHLEN];
	
	/* Create serial queue for node creation / deletion operations */
	_global.pathwatch_queue = dispatch_queue_create("pathwatch", NULL);

	_global.tzdir = NULL;
	_global.tzdir_len = 0;

	/* Get the real path to TZDIR */
	if (realpath(TZDIR, buf) != NULL)
	{
		_global.tzdir_len = strlen(buf);
		_global.tzdir = strdup(buf);
		if (_global.tzdir == NULL) _global.tzdir_len = 0;
	}
}

/*
 * _path_node_init is responsible for allocating a path_node_t structure,
 * and for creating the pname array and setting the path component.
 * The path is a sanatized version of the caller's path with redundant "/"
 * characters stripped out. The pname array contains each "/" separated
 * component of the path.
 * 
 * For example, _path_node_init("///foo////bar//baz/") creates:
 * pnode->path = "/foo/bar/baz"
 * pnode->pname_count = 3
 * pnode->pname[0] = "foo"
 * pnode->pname[1] = "bar"
 * pnode->pname[2] = "baz"
 */
static path_node_t *
_path_node_init(const char *path)
{
	size_t len;
	uint32_t i;
	path_node_t *pnode;
	const char *start, *end;
	char *name;

	if (path == NULL) path = "/";
	if (path[0] != '/') return NULL;

	pnode = (path_node_t *)calloc(1, sizeof(path_node_t));
	assert(pnode != NULL);

	pnode->plen = 1;
	start = path;
	while (*start == '/') start++;

	forever
	{
		end = strchr(start, '/');
		if (end == NULL) end = strchr(start, '\0');

		len = end - start;
		if (len == 0) break;

		pnode->plen += (len + 1);

		name = NULL;
		if (end == NULL)
		{
			name = strdup(start);
		}
		else
		{
			name = malloc(len + 1);
			assert(name != NULL);
			strncpy(name, start, len);
			name[len] = '\0';
		}

		if (pnode->pname_count == 0)
		{
			pnode->pname = (char **)calloc(1, sizeof(char *));
		}
		else
		{
			pnode->pname = (char **)reallocf(pnode->pname, (pnode->pname_count + 1) * sizeof(char *));
		}

		assert(pnode->pname != NULL);
		pnode->pname[pnode->pname_count] = name;
		pnode->pname_count++;

		start = end;
		if (start != NULL)
		{
			/* skip '/' chars */
			while (*start == '/') start++;
		}
	}

	pnode->path = calloc(1, pnode->plen);
	assert(pnode->path != NULL);
	/*
	 * Reconstruct the path here to strip out excess "/" chars.
	 * This ensures that path comparisons in _path_node_update are correct.
	 */
	for (i = 0; i < pnode->pname_count; i++)
	{
		strlcat(pnode->path, "/", pnode->plen);
		strlcat(pnode->path, pnode->pname[i], pnode->plen);
	}

	return pnode;
}

/* dispatched on _global.pathwatch_queue */
static void
_path_node_update(path_node_t *pnode, uint32_t flags, vnode_t *vnode)
{
	char *buf, fixed[MAXPATHLEN + 1];
	uint32_t i, old_type;
	int status;
	unsigned long data;
	struct stat sb;

	if (pnode == NULL) return;
	if ((pnode->src != NULL) && (dispatch_source_testcancel(pnode->src))) return;

	old_type = pnode->type;

	status = _path_stat_check_access(pnode->path, pnode->uid, pnode->gid, &(pnode->type));
	if (status == PATH_STAT_ACCESS) flags |= DISPATCH_VNODE_REVOKE;

	data = 0;

	if (vnode != NULL)
	{
		/* update status */

		if (flags & DISPATCH_VNODE_UNAVAIL)
		{
			pnode->type = PATH_NODE_TYPE_GHOST;
			data |= (flags & DISPATCH_VNODE_UNAVAIL);
			data |= DISPATCH_VNODE_DELETE;
		}

		if ((vnode->path != NULL) && (pnode->path != NULL) && streq(vnode->path, pnode->path))
		{
			/* this is the target VNODE - transfer flags to src data */
			data |= flags;
		}

		if (old_type == PATH_NODE_TYPE_GHOST)
		{
			/* transition from ghost to non-ghost */
			if (pnode->type != PATH_NODE_TYPE_GHOST)
			{
				data |= PATH_NODE_CREATE;
			}
			else
			{
				data = 0;
			}
		}
		else if (pnode->type == PATH_NODE_TYPE_GHOST)
		{
			/* transition from non-ghost to ghost */
			data |= PATH_NODE_DELETE;
		}

		data &= (pnode->flags & PATH_NODE_ALL);
		if (data != 0)
		{
			if ((pnode->flags & PATH_SRC_SUSPENDED) == 0)
			{
				/* suspend pnode->src, and fire it after PNODE_COALESCE_TIME */
				pnode->flags |= PATH_SRC_SUSPENDED;
				dispatch_suspend(pnode->src);

				dispatch_time_t delay = dispatch_time(DISPATCH_TIME_NOW, PNODE_COALESCE_TIME);
				_path_node_retain(pnode);

				dispatch_after(delay, _global.pathwatch_queue, ^{
					pnode->flags &= ~PATH_SRC_SUSPENDED;
					dispatch_resume(pnode->src);
					_path_node_release(pnode);
				});
			}

			dispatch_source_merge_data(pnode->src, data);
		}
	}

	buf = NULL;
	if (pnode->plen < MAXPATHLEN) buf = fixed;
	else buf = malloc(pnode->plen);
	assert(buf != NULL);

	/* "autorelease" current sources (_vnode_sweep() will delete those with zero refcount) */
	_vnode_release_for_node(pnode);

	/* create new sources (may re-use existing sources) */
	_vnode_create(NULL, 0, pnode);

	memset(buf, 0, pnode->plen);
	for (i = 0; i < pnode->pname_count; i++)
	{
		assert((strlen(buf) + 1) <= pnode->plen);
		strlcat(buf, "/", pnode->plen);

		assert(pnode->pname[i] != NULL);
		assert((strlen(buf) + strlen(pnode->pname[i])) <= pnode->plen);
		strlcat(buf, pnode->pname[i], pnode->plen);

		memset(&sb, 0, sizeof(struct stat));
		if (lstat(buf, &sb) < 0)
		{
			/* the path stops existing here */
			break;
		}

		if ((sb.st_mode & S_IFMT) == S_IFLNK)
		{
			/* open the symlink itself */
			_vnode_create(buf, VPATH_NODE_TYPE_LINK, pnode);

			/* open the symlink target */
			_vnode_create_real_path(buf, 0, pnode);
		}
		else
		{
			_vnode_create(buf, 0, pnode);
		}
	}

	/* sweep source list (deletes those with zero refcount) */
	_vnode_sweep();

	if (buf != fixed) free(buf);
}

/*
 * Creates a dispatch source that activates when a path changes.
 * Internally, creates a data structure (path_node_t) that represents the entire path.
 * Also creates dispatch sources (vnode_t) for each path component.  These vnodes may
 * be shared with other path_node_t structures.
 */
path_node_t *
path_node_create(const char *path, uid_t uid, gid_t gid, uint32_t mask, dispatch_queue_t queue)
{
	path_node_t *pnode;

	dispatch_once(&(_global.pathwatch_init), ^{ _pathwatch_init(); });

	pnode = _path_node_init(path);
	if (pnode == NULL) return NULL;

	pnode->refcount = 1;
	pnode->uid = uid;
	pnode->gid = gid;

	dispatch_sync(_global.pathwatch_queue, ^{ _path_node_update(pnode, 0, NULL); });

	dispatch_retain(queue);

	pnode->src = dispatch_source_create(DISPATCH_SOURCE_TYPE_DATA_OR, 0, 0, queue);
	pnode->src_queue = queue;
	pnode->flags = mask & PATH_NODE_ALL;

	return pnode;
}

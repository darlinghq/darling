/*
 * Copyright (c) 2006, 2007, 2010 Apple Inc. All rights reserved.
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

#include <mach/mach.h>
#include <mach/mach_error.h>
#include <servers/bootstrap.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <membership.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <uuid/uuid.h>
#include <string.h>
#include <libkern/OSByteOrder.h>
#include <TargetConditionals.h>
#include <xpc/xpc.h>
#include <xpc/private.h>

#include "dirhelper.h"
#include "dirhelper_priv.h"

#define BUCKETLEN	2

#define MUTEX_LOCK(x)	if(__is_threaded) pthread_mutex_lock(x)
#define MUTEX_UNLOCK(x)	if(__is_threaded) pthread_mutex_unlock(x)

// Use 5 bits per character, to avoid uppercase and shell magic characters
#define ENCODEBITS	5
#define ENCODEDSIZE	((8 * UUID_UID_SIZE + ENCODEBITS - 1) / ENCODEBITS)
#define MASK(x)		((1 << (x)) - 1)
#define UUID_UID_SIZE	(sizeof(uuid_t) + sizeof(uid_t))

extern int __is_threaded;

static const mode_t modes[] = {
    0755,	/* user */
    0700,	/* temp */
    0700,	/* cache */
};

static const char *subdirs[] = {
    DIRHELPER_TOP_STR,
    DIRHELPER_TEMP_STR,
    DIRHELPER_CACHE_STR,
};

static pthread_once_t userdir_control = PTHREAD_ONCE_INIT;
static char *userdir = NULL;

// lower case letter (minus vowels), plus numbers and _, making
// 32 characters.
static const char encode[] = "0123456789_bcdfghjklmnpqrstvwxyz";

static void
encode_uuid_uid(const uuid_t uuid, uid_t uid, char *str)
{
    unsigned char buf[UUID_UID_SIZE + 1];
    unsigned char *bp = buf;
    int i;
    unsigned int n;

    memcpy(bp, uuid, sizeof(uuid_t));
    uid = OSSwapHostToBigInt32(uid);
    memcpy(bp + sizeof(uuid_t), &uid, sizeof(uid_t));
    bp[UUID_UID_SIZE] = 0; // this ensures the last encoded byte will have trailing zeros
    for(i = 0; i < ENCODEDSIZE; i++) {
	// 5 bits has 8 states
	switch(i % 8) {
	case 0:
	    n = *bp++;
	    *str++ = encode[n >> 3];
	    break;
	case 1:
	    n = ((n & MASK(3)) << 8) | *bp++;
	    *str++ = encode[n >> 6];
	    break;
	case 2:
	    n &= MASK(6);
	    *str++ = encode[n >> 1];
	    break;
	case 3:
	    n = ((n & MASK(1)) << 8) | *bp++;
	    *str++ = encode[n >> 4];
	    break;
	case 4:
	    n = ((n & MASK(4)) << 8) | *bp++;
	    *str++ = encode[n >> 7];
	    break;
	case 5:
	    n &= MASK(7);
	    *str++ = encode[n >> 2];
	    break;
	case 6:
	    n = ((n & MASK(2)) << 8) | *bp++;
	    *str++ = encode[n >> 5];
	    break;
	case 7:
	    *str++ = encode[n & MASK(5)];
	    break;
	}
    }
    *str = 0;
}

char *
__user_local_dirname(uid_t uid, dirhelper_which_t which, char *path, size_t pathlen)
{
#if TARGET_OS_IPHONE
    char *tmpdir;
#else
    uuid_t uuid;
    char str[ENCODEDSIZE + 1];
#endif
    int res;

    if(which < 0 || which > DIRHELPER_USER_LOCAL_LAST) {
	errno = EINVAL;
	return NULL;
    }

#if TARGET_OS_IPHONE
    /* We only support DIRHELPER_USER_LOCAL_TEMP on embedded.
     * This interface really doesn't map from OSX to embedded,
     * and clients of this interface will need to adapt when
     * porting their applications to embedded.
     * See: <rdar://problem/7515613> 
     */
    if(which == DIRHELPER_USER_LOCAL_TEMP) {
        tmpdir = getenv("TMPDIR");
        if(!tmpdir) {
            errno = EINVAL;
            return NULL;
        }
        res = snprintf(path, pathlen, "%s", tmpdir);
    } else {
        errno = EINVAL;
        return NULL;
    }
#else
    res = mbr_uid_to_uuid(uid, uuid);
    if(res != 0) {
        errno = res;
        return NULL;
    }
    
    //
    // We partition the namespace so that we don't end up with too
    // many users in a single directory.  With 1024 buckets, we
    // could scale to 1,000,000 users while keeping the average
    // number of files in a single directory around 1000
    //
    encode_uuid_uid(uuid, uid, str);
    res = snprintf(path, pathlen,
	"%s%.*s/%s/%s",
	VAR_FOLDERS_PATH, BUCKETLEN, str, str + BUCKETLEN, subdirs[which]);
#endif
    if(res >= pathlen) {
	errno = EINVAL;
	return NULL; /* buffer too small */
    }
    return path;
}

char *
__user_local_mkdir_p(char *path)
{
    char *next;
    int res;
    
    next = path + strlen(VAR_FOLDERS_PATH);
    while ((next = strchr(next, '/')) != NULL) {
	*next = 0; // temporarily truncate
	res = mkdir(path, 0755);
	if (res != 0 && errno != EEXIST)
	    return NULL;
	*next++ = '/'; // restore the slash and increment
    }
    return path;
}

static void userdir_allocate(void)
{
    userdir = calloc(PATH_MAX, sizeof(char));
}

/*
 * 9407258: Invalidate the dirhelper cache (userdir) of the child after fork.
 * There is a rare case when launchd will have userdir set, and child process
 * will sometimes inherit this cached value.
 */
__private_extern__ void
_dirhelper_fork_child(void)
{
    if(userdir) *userdir = 0;
}

__private_extern__ char *
_dirhelper(dirhelper_which_t which, char *path, size_t pathlen)
{
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    struct stat sb;

    if(which < 0 || which > DIRHELPER_USER_LOCAL_LAST) {
	errno = EINVAL;
	return NULL;
    }

    if (pthread_once(&userdir_control, userdir_allocate)
        || !userdir) {
        errno = ENOMEM;
        return NULL;
    }

    if(!*userdir) {
	MUTEX_LOCK(&lock);
	if (!*userdir) {
	    
	    if(__user_local_dirname(geteuid(), DIRHELPER_USER_LOCAL, userdir, PATH_MAX) == NULL) {
		MUTEX_UNLOCK(&lock);
		return NULL;
	    }
	    /*
	     * All dirhelper directories are now at the same level, so
	     * we need to remove the DIRHELPER_TOP_STR suffix to get the
	     * parent directory.
	     */
	    userdir[strlen(userdir) - (sizeof(DIRHELPER_TOP_STR) - 1)] = 0;
	    /*
	     * check if userdir exists, and if not, either do the work
	     * ourself if we are root, or call
	     * __dirhelper_create_user_local to create it (we have to
	     * check again afterwards).
	     */
	    if(stat(userdir, &sb) < 0) {
		mach_port_t mp;
		
		if(errno != ENOENT) { /* some unknown error */
		    *userdir = 0;
		    MUTEX_UNLOCK(&lock);
		    return NULL;
		}
		/*
		 * If we are root, lets do what dirhelper does for us.
		 */
		if (geteuid() == 0) {
		    if (__user_local_mkdir_p(userdir) == NULL) {
			*userdir = 0;
			MUTEX_UNLOCK(&lock);
			return NULL;
		    }
		} else {
		    if(bootstrap_look_up(bootstrap_port, DIRHELPER_BOOTSTRAP_NAME, &mp) != KERN_SUCCESS) {
			errno = EPERM;
		    server_error:
			mach_port_deallocate(mach_task_self(), mp);
			MUTEX_UNLOCK(&lock);
			return NULL;
		    }
		    if(__dirhelper_create_user_local(mp) != KERN_SUCCESS) {
			errno = EPERM;
			goto server_error;
		    }
		    /* double check that the directory really got created */
		    if(stat(userdir, &sb) < 0) {
			goto server_error;
		    }
		    mach_port_deallocate(mach_task_self(), mp);
		}
	    }
	}
	MUTEX_UNLOCK(&lock);
    }
    
    if(pathlen < strlen(userdir) + strlen(subdirs[which]) + 1) {
	errno = EINVAL;
	return NULL; /* buffer too small */
    }
    strcpy(path, userdir);
    strcat(path, subdirs[which]);

    /*
     * create the subdir with the appropriate permissions if it doesn't already
     * exist. On OS X, if we're under App Sandbox, we rely on the subdir having
     * been already created for us.
     */
#if !TARGET_OS_IPHONE
    if (!_xpc_runtime_is_app_sandboxed())
#endif
    if(mkdir(path, modes[which]) != 0 && errno != EEXIST)
        return NULL;

#if !TARGET_OS_IPHONE
    char *userdir_suffix = NULL;

    if (_xpc_runtime_is_app_sandboxed()) {
        /*
         * if the subdir wasn't made for us, bail since we probably don't have
         * permission to create it ourselves.
         */
        if(stat(path, &sb) < 0) {
            errno = EPERM;
            return NULL;
        }

        /*
         * sandboxed applications get per-application directories named
         * after the container
         */
        userdir_suffix = getenv(XPC_ENV_SANDBOX_CONTAINER_ID);
        if (!userdir_suffix) {
            errno = EINVAL;
            return NULL;
        }
    } else
        userdir_suffix = getenv(DIRHELPER_ENV_USER_DIR_SUFFIX);

    if (userdir_suffix) {
        /*
         * suffix (usually container ID) doesn't end in a slash, so +2 is for slash and \0
         */
        if (pathlen < strlen(path) + strlen(userdir_suffix) + 2) {
            errno = EINVAL;
            return NULL; /* buffer too small */
        }

        strcat(path, userdir_suffix);
        strcat(path, "/");

        /*
         * create suffix subdirectory with the appropriate permissions
         * if it doesn't already exist.
         */
        if (mkdir(path, modes[which]) != 0 && errno != EEXIST)
            return NULL;

        /*
         * update TMPDIR if necessary
         */
        if (which == DIRHELPER_USER_LOCAL_TEMP) {
            char *tmpdir = getenv("TMPDIR");
            if (!tmpdir || strncmp(tmpdir, path, strlen(path)))
                setenv("TMPDIR", path, 1);
        }
    }
#endif

    return path;
}

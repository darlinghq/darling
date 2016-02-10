/*
 * Copyright (c) 2007 Rob Braun
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Rob Braun nor the names of his contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * 03-Apr-2005
 * DRI: Rob Braun <bbraun@synack.net>
 */
/*
 * Portions Copyright 2003, Apple Computer, Inc.
 * filetype_name() and associated structure.
 * DRI: Kevin Van Vechten <kvv@apple.com>
 */
/*
 * Portions Copyright 2006, Apple Computer, Inc.
 * Christopher Ryan <ryanc@apple.com>
*/

#define _FILE_OFFSET_BITS 64

#include "config.h"
#ifndef HAVE_ASPRINTF
#include "asprintf.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <libxml/hash.h>
#include <libxml/xmlstring.h>
#ifdef HAVE_SYS_ACL_H
#include <sys/acl.h>
#endif
#include "xar.h"
#include "arcmod.h"
#include "archive.h"
#include "util.h"

#ifndef LLONG_MIN
#define LLONG_MIN LONG_LONG_MIN
#endif

#ifndef LLONG_MAX
#define LLONG_MAX LONG_LONG_MAX
#endif

static struct {
	const char *name;
	mode_t type;
} filetypes [] = {
	{ "file", S_IFREG },
	{ "directory", S_IFDIR },
	{ "symlink", S_IFLNK },
	{ "fifo", S_IFIFO },
	{ "character special", S_IFCHR },
	{ "block special", S_IFBLK },
	{ "socket", S_IFSOCK },
#ifdef S_IFWHT
	{ "whiteout", S_IFWHT },
#endif
	{ NULL, 0 }
};

static const char * filetype_name (mode_t mode) {
	unsigned int i;
	for (i = 0; filetypes[i].name; i++)
		if (mode == filetypes[i].type)
			return (filetypes[i].name);
	return ("unknown");
}

static xar_file_t xar_link_lookup(xar_t x, dev_t dev, ino_t ino, xar_file_t f) {
	char key[32];
	xar_file_t ret;

	memset(key, 0, sizeof(key));
	snprintf(key, sizeof(key)-1, "%08" DEV_HEXSTRING "%08" INO_HEXSTRING, DEV_CAST dev, INO_CAST ino);
	ret = xmlHashLookup(XAR(x)->ino_hash, BAD_CAST(key));
	if( ret == NULL ) {
		xmlHashAddEntry(XAR(x)->ino_hash, BAD_CAST(key), XAR_FILE(f));
		return NULL;
	}
	return ret;
}

static int32_t aacls(xar_t x, xar_file_t f, const char *file) {
#ifdef HAVE_SYS_ACL_H
#if !defined(__APPLE__)
	acl_t a;
	const char *type;

	xar_prop_get(f, "type", &type);
	if( !type || (strcmp(type, "symlink") == 0) )
		return 0;

	if( !xar_check_prop(x, "acl") )
		return 0;

	a = acl_get_file(file, ACL_TYPE_DEFAULT);
	if( a ) {
		char *t;
		acl_entry_t e;

		/* If the acl is empty, or not valid, skip it */
		if( acl_get_entry(a, ACL_FIRST_ENTRY, &e) != 1 )
			goto NEXT;

		t = acl_to_text(a, NULL);
		if( t ) {
			xar_prop_set(f, "acl/default", t);
			acl_free(t);
		}
		acl_free(a);
	}
NEXT:

	a = acl_get_file(file, ACL_TYPE_ACCESS);
	if( a ) {
		char *t;
		acl_entry_t e;

		/* If the acl is empty, or not valid, skip it */
		if( acl_get_entry(a, ACL_FIRST_ENTRY, &e) != 1 )
			goto DONE;

		t = acl_to_text(a, NULL);
		if( t ) {
			xar_prop_set(f, "acl/access", t);
			acl_free(t);
		}
		acl_free(a);
	}
DONE:
#else /* !__APPLE__ */
	acl_entry_t e = NULL;
	acl_t a;
	int i;

	if( !xar_check_prop(x, "acl") )
		return 0;

	a = acl_get_file(file, ACL_TYPE_EXTENDED);
	if( !a )
		return 0;

	for( i = 0; acl_get_entry(a, e == NULL ? ACL_FIRST_ENTRY : ACL_NEXT_ENTRY, &e) == 0; i++ ) {
		char *t;
		t = acl_to_text(a, NULL);
		if( t ) {
			xar_prop_set(f, "acl/appleextended", t);
			acl_free(t);
		}
		
	}
	acl_free(a);
#endif /* !__APPLE__ */
#endif
	return 0;
}

static int32_t eacls(xar_t x, xar_file_t f, const char *file) {
#ifdef HAVE_SYS_ACL_H
#if !defined(__APPLE__)
	const char *t;
	acl_t a;
	const char *type;

	xar_prop_get(f, "type", &type);
	if( !type || (strcmp(type, "symlink") == 0) )
		return 0;


	xar_prop_get(f, "acl/default", &t);
	if( t ) {
		a = acl_from_text(t);
		if( !a ) {
			xar_err_new(x);
			xar_err_set_errno(x, errno);
			xar_err_set_string(x, "Error extracting default acl from toc");
			xar_err_set_file(x, f);
			xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
		} else {
			if( acl_set_file(file, ACL_TYPE_DEFAULT, a) != 0 ) {
				xar_err_new(x);
				xar_err_set_errno(x, errno);
				xar_err_set_string(x, "Error setting default acl");
				xar_err_set_file(x, f);
				xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
			}
		}
	}

	xar_prop_get(f, "acl/access", &t);
	if( t ) {
		a = acl_from_text(t);
		if( !a ) {
			xar_err_new(x);
			xar_err_set_errno(x, errno);
			xar_err_set_string(x, "Error extracting access acl from toc");
			xar_err_set_file(x, f);
			xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
		} else {
			if( acl_set_file(file, ACL_TYPE_ACCESS, a) != 0 ) {
				xar_err_new(x);
				xar_err_set_errno(x, errno);
				xar_err_set_string(x, "Error setting access acl");
				xar_err_set_file(x, f);
				xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
			}
			acl_free(a);
		}
	}
#else /* !__APPLE__ */
	const char *t;
	acl_t a;

	xar_prop_get(f, "acl/appleextended", &t);
	if( t ) {
		a = acl_from_text(t);
		if( !a ) {
			xar_err_new(x);
			xar_err_set_errno(x, errno);
			xar_err_set_string(x, "Error extracting access acl from toc");
			xar_err_set_file(x, f);
			xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
		} else {
			if( acl_set_file(file, ACL_TYPE_ACCESS, a) != 0 ) {
				xar_err_new(x);
				xar_err_set_errno(x, errno);
				xar_err_set_string(x, "Error setting access acl");
				xar_err_set_file(x, f);
				xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
			}
			acl_free(a);
		}
	}
#endif /* !__APPLE__ */
#endif
	return 0;
}

#ifdef HAVE_STRUCT_STAT_ST_FLAGS
#define XAR_FLAG_FORK "flags"
static void x_addflag(xar_file_t f, const char *name) {
	char opt[1024];
	memset(opt, 0, sizeof(opt));
	snprintf(opt, sizeof(opt)-1, "%s/%s", XAR_FLAG_FORK, name);
	xar_prop_set(f, opt, NULL);
	return;
}
#endif

static int32_t flags_archive(xar_t x, xar_file_t f, const struct stat *sb) {
#ifdef HAVE_STRUCT_STAT_ST_FLAGS
	if( !sb->st_flags )
		return 0;
	
	if( !xar_check_prop(x, XAR_FLAG_FORK) )
		return 0;
#ifdef UF_NODUMP
	if( sb->st_flags & UF_NODUMP )
		x_addflag(f, "UserNoDump");
#endif
#ifdef UF_IMMUTABLE
	if( sb->st_flags & UF_IMMUTABLE )
		x_addflag(f, "UserImmutable");
#endif
#ifdef UF_APPEND
	if( sb->st_flags & UF_APPEND )
		x_addflag(f, "UserAppend");
#endif
#ifdef UF_OPAQUE
	if( sb->st_flags & UF_OPAQUE )
		x_addflag(f, "UserOpaque");
#endif
#ifdef SF_ARCHIVED
	if( sb->st_flags & SF_ARCHIVED )
		x_addflag(f, "SystemArchived");
#endif
#ifdef SF_IMMUTABLE
	if( sb->st_flags & SF_IMMUTABLE )
		x_addflag(f, "SystemImmutable");
#endif
#ifdef SF_APPEND
	if( sb->st_flags & SF_APPEND )
		x_addflag(f, "SystemAppend");
#endif

#endif /* HAVE_STRUCT_STAT_ST_FLAGS */
	return 0;
}

#ifdef HAVE_CHFLAGS
static int32_t x_getprop(xar_file_t f, const char *name, char **value) {
	char v[1024];
	memset(v, 0, sizeof(v));
	snprintf(v, sizeof(v)-1, "%s/%s", XAR_FLAG_FORK, name);
	return xar_prop_get(f, v, (const char **)value);
}
#endif

int32_t xar_flags_extract(xar_t x, xar_file_t f, const char *file, char *buffer, size_t len) {
#ifdef HAVE_CHFLAGS
	char *tmp;
	u_int flags = 0;

	if( xar_prop_get(f, XAR_FLAG_FORK, NULL) )
		return 0;

#ifdef UF_NODUMP
	if( x_getprop(f, "UserNoDump", (char **)&tmp) == 0 )
		flags |= UF_NODUMP;
#endif
#ifdef UF_IMMUTABLE
	if( x_getprop(f, "UserImmutable", (char **)&tmp) == 0 )
		flags |= UF_IMMUTABLE;
#endif
#ifdef UF_APPEND
	if( x_getprop(f, "UserAppend", (char **)&tmp) == 0 )
		flags |= UF_APPEND;
#endif
#ifdef UF_OPAQUE
	if( x_getprop(f, "UserOpaque", (char **)&tmp) == 0 )
		flags |= UF_OPAQUE;
#endif
#ifdef SF_ARCHIVED
	if( x_getprop(f, "SystemArchived", (char **)&tmp) == 0 )
		flags |= SF_ARCHIVED;
#endif
#ifdef SF_IMMUTABLE
	if( x_getprop(f, "SystemImmutable", (char **)&tmp) == 0 )
		flags |= SF_IMMUTABLE;
#endif
#ifdef SF_APPEND
	if( x_getprop(f, "SystemAppend", (char **)&tmp) == 0 )
		flags |= SF_APPEND;
#endif

	if( !flags )
		return 0;

	if( chflags(file, flags) != 0 ) {
		char e[1024];
		memset(e, 0, sizeof(e));
		snprintf(e, sizeof(e)-1, "chflags: %s", strerror(errno));
		xar_err_new(x);
		xar_err_set_file(x, f);
		xar_err_set_string(x, e);
		xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
		return -1;
	}
#endif
	
	return 0;
}

int32_t xar_stat_archive(xar_t x, xar_file_t f, const char *file, const char *buffer, size_t len) {
	char *tmpstr;
	struct passwd *pw;
	struct group *gr;
	char time[128];
	struct tm t;
	const char *type;

	/* no stat attributes for data from a buffer, it is just a file */
	if(len){
		if( xar_check_prop(x, "type") )
			xar_prop_set(f, "type", "file");
		return 0;
	}
	
	if( S_ISREG(XAR(x)->sbcache.st_mode) && (XAR(x)->sbcache.st_nlink > 1) ) {
		xar_file_t tmpf;
		const char *id = xar_attr_get(f, NULL, "id");
		if( !id ) {
			xar_err_new(x);
			xar_err_set_file(x, f);
			xar_err_set_string(x, "stat: No file id for file");
			xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_CREATION);
			return -1;
		}
		tmpf = xar_link_lookup(x, XAR(x)->sbcache.st_dev, XAR(x)->sbcache.st_ino, f);
		if( xar_check_prop(x, "type") ) {
			xar_prop_set(f, "type", "hardlink");
			if( tmpf ) {
				const char *id;
				id = xar_attr_get(tmpf, NULL, "id");
				xar_attr_set(f, "type", "link", id);
			} else {
				xar_attr_set(f, "type", "link", "original");
			}
		}
	} else {
		type = filetype_name(XAR(x)->sbcache.st_mode & S_IFMT);
		if( xar_check_prop(x, "type") )
			xar_prop_set(f, "type", type);
	}

	/* Record major/minor device node numbers */
	if( xar_check_prop(x, "device") && (S_ISBLK(XAR(x)->sbcache.st_mode) || S_ISCHR(XAR(x)->sbcache.st_mode))) {
		uint32_t major, minor;
		char tmpstr[12];
		xar_devmake(XAR(x)->sbcache.st_rdev, &major, &minor);
		memset(tmpstr, 0, sizeof(tmpstr));
		snprintf(tmpstr, sizeof(tmpstr)-1, "%u", major);
		xar_prop_set(f, "device/major", tmpstr);
		memset(tmpstr, 0, sizeof(tmpstr));
		snprintf(tmpstr, sizeof(tmpstr)-1, "%u", minor);
		xar_prop_set(f, "device/minor", tmpstr);
	}

	if( S_ISLNK(XAR(x)->sbcache.st_mode) ) {
		char link[4096];
		struct stat lsb;

		memset(link, 0, sizeof(link));
		readlink(file, link, sizeof(link)-1);
		xar_prop_set(f, "link", link);
		if( stat(file, &lsb) != 0 ) {
			xar_attr_set(f, "link", "type", "broken");
		} else {
			type = filetype_name(lsb.st_mode & S_IFMT);
			xar_attr_set(f, "link", "type", type);
		}
	}

	if( xar_check_prop(x, "inode") ) {
		asprintf(&tmpstr, "%"INO_STRING, XAR(x)->sbcache.st_ino);
		xar_prop_set(f, "inode", tmpstr);
		free(tmpstr);
	}

	if( xar_check_prop(x, "deviceno") ) {
		asprintf(&tmpstr, "%"DEV_STRING, XAR(x)->sbcache.st_dev);
		xar_prop_set(f, "deviceno", tmpstr);
		free(tmpstr);
	}

	if( xar_check_prop(x, "mode") ) {
		asprintf(&tmpstr, "%04o", XAR(x)->sbcache.st_mode & (~S_IFMT));
		xar_prop_set(f, "mode", tmpstr);
		free(tmpstr);
	}

	if( xar_check_prop(x, "uid") ) {
		asprintf(&tmpstr, "%"PRIu64, (uint64_t)XAR(x)->sbcache.st_uid);
		xar_prop_set(f, "uid", tmpstr);
		free(tmpstr);
	}

	if( xar_check_prop(x, "user") ) {
		pw = getpwuid(XAR(x)->sbcache.st_uid);
		if( pw )
			xar_prop_set(f, "user", pw->pw_name);
	}

	if( xar_check_prop(x, "gid") ) {
		asprintf(&tmpstr, "%"PRIu64, (uint64_t)XAR(x)->sbcache.st_gid);
		xar_prop_set(f, "gid", tmpstr);
		free(tmpstr);
	}

	if( xar_check_prop(x, "group") ) {
		gr = getgrgid(XAR(x)->sbcache.st_gid);
		if( gr )
			xar_prop_set(f, "group", gr->gr_name);
	}

	if( xar_check_prop(x, "atime") ) {
		gmtime_r(&XAR(x)->sbcache.st_atime, &t);
		memset(time, 0, sizeof(time));
		strftime(time, sizeof(time), "%FT%T", &t);
		strcat(time, "Z");
		xar_prop_set(f, "atime", time);
	}

	if( xar_check_prop(x, "mtime") ) {
		gmtime_r(&XAR(x)->sbcache.st_mtime, &t);
		memset(time, 0, sizeof(time));
		strftime(time, sizeof(time), "%FT%T", &t);
		strcat(time, "Z");
		xar_prop_set(f, "mtime", time);
	}

	if( xar_check_prop(x, "ctime") ) {
		gmtime_r(&XAR(x)->sbcache.st_ctime, &t);
		memset(time, 0, sizeof(time));
		strftime(time, sizeof(time), "%FT%T", &t);
		strcat(time, "Z");
		xar_prop_set(f, "ctime", time);
	}

	flags_archive(x, f, &(XAR(x)->sbcache));

	aacls(x, f, file);

	return 0;
}

int32_t xar_set_perm(xar_t x, xar_file_t f, const char *file, char *buffer, size_t len) {
	const char *opt;
	int32_t m=0, mset=0;
	uid_t u;
	gid_t g;
	const char *timestr;
	struct tm t;
	enum {ATIME=0, MTIME};
	struct timeval tv[2];
	char savesuid = 0;

	/* when writing to a buffer, there are no permissions to set */
	if ( len )
		return 0;
	
	/* in case we don't find anything useful in the archive */
	u = geteuid();
	g = getegid();

	opt = xar_opt_get(x, XAR_OPT_OWNERSHIP);
	if( opt && (strcmp(opt, XAR_OPT_VAL_SYMBOLIC) == 0) ) {
		struct passwd *pw;
		struct group *gr;

		xar_prop_get(f, "user", &opt);
		if( opt ) {
			pw = getpwnam(opt);
			if( pw ) {
				u = pw->pw_uid;
			}
		}
		xar_prop_get(f, "group", &opt);
		if( opt ) {
			gr = getgrnam(opt);
			if( gr ) {
				g = gr->gr_gid;
			}
		}
		savesuid = 1;
	}
	if( opt && (strcmp(opt, XAR_OPT_VAL_NUMERIC) == 0) ) {
		xar_prop_get(f, "uid", &opt);
		if( opt ) {
			long long tmp;
			tmp = strtol(opt, NULL, 10);
			if( ( (tmp == LLONG_MIN) || (tmp == LLONG_MAX) ) && (errno == ERANGE) ) {
				return -1;
			}
			u = (uid_t)tmp;
		}

		xar_prop_get(f, "gid", &opt);
		if( opt ) {
			long long tmp;
			tmp = strtol(opt, NULL, 10);
			if( ( (tmp == LLONG_MIN) || (tmp == LLONG_MAX) ) && (errno == ERANGE) ) {
				return -1;
			}
			g = (gid_t)tmp;
		}
		savesuid = 1;
	}

	opt = xar_opt_get(x, XAR_OPT_SAVESUID);
	if( opt && (strcmp(opt, XAR_OPT_VAL_TRUE) == 0) ) {
		savesuid = 1;
	}

	xar_prop_get(f, "mode", &opt);
	if( opt ) {
		long long tmp;
		tmp = strtoll(opt, NULL, 8);
		if( ( (tmp == LLONG_MIN) || (tmp == LLONG_MAX) ) && (errno == ERANGE) ) {
			return -1;
		}
		m = (mode_t)tmp;
		if( !savesuid ) {
			m &= ~S_ISUID;
			m &= ~S_ISGID;
		}
		mset = 1;
	}

	xar_prop_get(f, "type", &opt);
	if( opt && !mset ) {
		mode_t u = umask(0);
		umask(u);
		if( strcmp(opt, "directory") == 0 ) {
			m = (mode_t)(0777 & ~u);
		} else {
			m = (mode_t)(0666 & ~u);
		}
		mset = 1;
	}
	if( opt && (strcmp(opt, "symlink") == 0) ) {
#ifdef HAVE_LCHOWN
		if( lchown(file, u, g) ) {
			xar_err_new(x);
			xar_err_set_file(x, f);
			xar_err_set_string(x, "perm: could not lchown symlink");
			xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
			m &= ~S_ISUID;
			m &= ~S_ISGID;
		}
#ifdef HAVE_LCHMOD
		if( mset )
			if( lchmod(file, m) ) {
				xar_err_new(x);
				xar_err_set_file(x, f);
				xar_err_set_string(x, "perm: could not lchmod symlink");
				xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
			}
#endif
#endif 
	} else {
		if( chown(file, u, g) ) {
			xar_err_new(x);
			xar_err_set_file(x, f);
			xar_err_set_string(x, "perm: could not chown file");
			xar_err_set_errno(x, errno);
			xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
			m &= ~S_ISUID;
			m &= ~S_ISGID;
		}
		if( mset )
			if( chmod(file, m) ) {
				xar_err_new(x);
				xar_err_set_file(x, f);
				xar_err_set_string(x, "perm: could not chmod file");
				xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
			}
	}

	eacls(x, f, file);

	memset(tv, 0, sizeof(struct timeval) * 2);
	xar_prop_get(f, "atime", &timestr);
	if( timestr ) {
		memset(&t, 0, sizeof(t));
		strptime(timestr, "%FT%T", &t);
		tv[ATIME].tv_sec = timegm(&t);
	} else {
		tv[ATIME].tv_sec = time(NULL);
	}

	xar_prop_get(f, "mtime", &timestr);
	if( timestr ) {
		memset(&t, 0, sizeof(t));
		strptime(timestr, "%FT%T", &t);
		tv[MTIME].tv_sec = timegm(&t);
	} else {
		tv[MTIME].tv_sec = time(NULL);
	}
	utimes(file, tv);

	return 0;
}

int32_t xar_stat_extract(xar_t x, xar_file_t f, const char *file, char *buffer, size_t len) {
	const char *opt;
	int ret, fd;
	mode_t modet = 0;

	xar_prop_get(f, "type", &opt);
	if(opt && (strcmp(opt, "character special") == 0))
		modet = S_IFCHR;
	if(opt && (strcmp(opt, "block special") == 0))
		modet = S_IFBLK;

	if( modet ) {
		uint32_t major, minor;
		long long tmpll;
		dev_t devt;

		xar_prop_get(f, "device/major", &opt);
		tmpll = strtoll(opt, NULL, 10);
		if( ( (tmpll == LLONG_MIN) || (tmpll == LLONG_MAX) ) && (errno == ERANGE) )
			return -1;
		if( (tmpll < 0) || (tmpll > 255) )
			return -1;
		major = tmpll;

		xar_prop_get(f, "device/minor", &opt);
		tmpll = strtoll(opt, NULL, 10);
		if( ( (tmpll == LLONG_MIN) || (tmpll == LLONG_MAX) ) && (errno == ERANGE) )
			return -1;
		if( (tmpll < 0) || (tmpll > 255) )
			return -1;
		minor = tmpll;
		
		devt = xar_makedev(major, minor);
		unlink(file);
		if( mknod(file, modet, devt) ) {
			xar_err_new(x);
			xar_err_set_file(x, f);
			xar_err_set_string(x, "mknod: Could not create character device");
			xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
			return -1;
		}
		return 0;
	}
	if(opt && (strcmp(opt, "directory") == 0)) {
		ret = mkdir(file, 0700);
		if( (ret != 0) && (errno != EEXIST) ) {
			xar_err_new(x);
			xar_err_set_file(x, f);
			xar_err_set_string(x, "stat: Could not create directory");
			xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
			return ret;
		}
		return 0;
	}
	if(opt && (strcmp(opt, "symlink") == 0)) {
		xar_prop_get(f, "link", &opt);
		if( opt ) {
			unlink(file);
			ret = symlink(opt, file);
			if( ret != 0 ) {
				xar_err_new(x);
				xar_err_set_file(x, f);
				xar_err_set_string(x, "stat: Could not create symlink");
				xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
			}
			return ret;
		}
	}
	if(opt && (strcmp(opt, "hardlink") == 0)) {
		xar_file_t tmpf;
		opt = xar_attr_get(f, "type", "link");
		if( !opt )
			return 0;
		if( strcmp(opt, "original") == 0 )
			goto CREATEFILE;

		tmpf = xmlHashLookup(XAR(x)->link_hash, BAD_CAST(opt));
		if( !tmpf ) {
			xar_err_new(x);
			xar_err_set_file(x, f);
			xar_err_set_string(x, "stat: Encountered hardlink with no original");
			xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
			return -1;
		}

		unlink(file);
		if( link(XAR_FILE(tmpf)->fspath, file) != 0 ) {
			if( errno == ENOENT ) {
				xar_iter_t i;
				const char *ptr;
				i = xar_iter_new();
				for(ptr = xar_prop_first(tmpf, i); ptr; ptr = xar_prop_next(i)) {
					xar_iter_t a;
					const char *val = NULL;
					const char *akey, *aval;
					if( strncmp("data", ptr, 4) != 0 )
						continue;
	
					if( xar_prop_get(tmpf, ptr, &val) )
						continue;
	
					xar_prop_set(f, ptr, val);
					a = xar_iter_new();
					for(akey = xar_attr_first(tmpf, ptr, a); akey; akey = xar_attr_next(a)) {
						aval = xar_attr_get(tmpf, ptr, akey);
						xar_attr_set(f, ptr, akey, aval);
					}
					xar_iter_free(a);
				}
				xar_iter_free(i);
				xar_attr_set(f, "type", "link", "original");
				return 0;
			} else {
				xar_err_new(x);
				xar_err_set_file(x, f);
				xar_err_set_string(x, "stat: Could not link hardlink to original");
				xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
				return -1;
			}
		}
		return 0;
	}

	if(opt && (strcmp(opt, "fifo") == 0)) {
		unlink(file);
		if( mkfifo(file, 0) ) {
			xar_err_new(x);
			xar_err_set_file(x, f);
			xar_err_set_string(x, "mkfifo: Could not create fifo");
			xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
			return -1;
		}
		return 0;
	}

	/* skip sockets */
	if(opt && (strcmp(opt, "socket") == 0)) {
		return 0;
	}

CREATEFILE:
	if (!file)
        return 0;
	
	unlink(file);
	fd = open(file, O_RDWR|O_CREAT|O_TRUNC, 0600);
	if( fd > 0 )
		close(fd);
	return 0;
}

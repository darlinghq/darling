/*
 * Copyright (c) 2004 Rob Braun
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
 * 3. Neither the name of Rob Braun nor the names of its contributors
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
 * 26-Oct-2004
 * DRI: Rob Braun <bbraun@synack.net>
 */
/*
 * Portions Copyright 2006, Apple Computer, Inc.
 * Christopher Ryan <ryanc@apple.com>
*/

#include "config.h"
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include "xar.h"
#include "arcmod.h"
#include "b64.h"
#include <errno.h>
#include <string.h>
#include "util.h"
#include "linuxattr.h"
#include "io.h"

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_SYS_STATFS_H  /* Nonexistant future OS needs this */
#include <sys/statfs.h>
#endif

#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#ifndef EXT3_SUPER_MAGIC
#define EXT3_SUPER_MAGIC 0xEF53
#endif

#ifndef JFS_SUPER_MAGIC
#define JFS_SUPER_MAGIC 0x3153464a
#endif

#ifndef REISERFS_SUPER_MAGIC
#define REISERFS_SUPER_MAGIC 0x52654973
#endif

#ifndef XFS_SUPER_MAGIC
#define XFS_SUPER_MAGIC 0x58465342
#endif

#if defined(HAVE_SYS_XATTR_H) && defined(HAVE_LGETXATTR) && !defined(__APPLE__)

struct _linuxattr_context{
	const char *file;
	const char *attrname;
	xar_ea_t ea;
	void *buf;
	int off;
	int bufsz;
};

#define LINUXATTR_CONTEXT(x) ((struct _linuxattr_context *)(x))

int32_t xar_linuxattr_read(xar_t x, xar_file_t f, void * buf, size_t len, void *context) {
	
	if( !LINUXATTR_CONTEXT(context)->buf ) {
		int r;
		LINUXATTR_CONTEXT(context)->bufsz = 1024;
AGAIN2:
		LINUXATTR_CONTEXT(context)->buf = malloc(LINUXATTR_CONTEXT(context)->bufsz);
		if(!LINUXATTR_CONTEXT(context)->buf)
			goto AGAIN2;
		memset(LINUXATTR_CONTEXT(context)->buf, 0, LINUXATTR_CONTEXT(context)->bufsz);
		r = lgetxattr(LINUXATTR_CONTEXT(context)->file, LINUXATTR_CONTEXT(context)->attrname, LINUXATTR_CONTEXT(context)->buf, LINUXATTR_CONTEXT(context)->bufsz);
		if( r < 0 ) {
			switch(errno) {
			case ERANGE: LINUXATTR_CONTEXT(context)->bufsz *= 2; free(LINUXATTR_CONTEXT(context)->buf); goto AGAIN2;
			case ENOTSUP: free(LINUXATTR_CONTEXT(context)->buf); return 0;
			default: break;
			};
			return -1;
		}
		LINUXATTR_CONTEXT(context)->bufsz = r;
	}

	if( (LINUXATTR_CONTEXT(context)->bufsz-LINUXATTR_CONTEXT(context)->off) <= len ) {
		int32_t ret;
		ret = LINUXATTR_CONTEXT(context)->bufsz - LINUXATTR_CONTEXT(context)->off;
		memcpy(buf, ((char *)LINUXATTR_CONTEXT(context)->buf)+LINUXATTR_CONTEXT(context)->off, ret);
		LINUXATTR_CONTEXT(context)->off += ret;
		return(ret);
	} else {
		memcpy(buf, ((char *)LINUXATTR_CONTEXT(context)->buf)+LINUXATTR_CONTEXT(context)->off, len);
		LINUXATTR_CONTEXT(context)->buf = ((char *)LINUXATTR_CONTEXT(context)->buf) + len;
		return len;
	}
}

int32_t xar_linuxattr_write(xar_t x, xar_file_t f, void *buf, size_t len, void *context) {
	return lsetxattr(LINUXATTR_CONTEXT(context)->file, LINUXATTR_CONTEXT(context)->attrname, buf, len, 0);
}
#endif

int32_t xar_linuxattr_archive(xar_t x, xar_file_t f, const char* file, const char *buffer, size_t len)
{
#if defined(HAVE_SYS_XATTR_H) && defined(HAVE_LGETXATTR) && !defined(__APPLE__)
	char *i, *buf = NULL;
	int ret, retval=0, bufsz = 1024;
	struct statfs sfs;
	char *fsname = NULL;
	struct _linuxattr_context context;
	
	memset(&context,0,sizeof(struct _linuxattr_context));

	/* data from buffers don't have linuxattr */
	if(len)
		return 0;
	if( file == NULL )
		return 0;

	if( !xar_check_prop(x, "ea") )
		return 0;
	
TRYAGAIN:
	buf = malloc(bufsz);
	if(!buf)
		goto TRYAGAIN;
	ret = llistxattr(file, buf, bufsz);
	if( ret < 0 ) {
		switch(errno) {
		case ERANGE: bufsz = bufsz*2; free(buf); goto TRYAGAIN;
		case ENOTSUP: retval = 0; goto BAIL;
		default: retval = -1; goto BAIL;
		};
	}
	if( ret == 0 ) goto BAIL;

	memset(&sfs, 0, sizeof(sfs));
	statfs(file, &sfs);

	switch(sfs.f_type) {
	case EXT3_SUPER_MAGIC: fsname = "ext3"; break; /* assume ext3 */
	case JFS_SUPER_MAGIC:  fsname = "jfs" ; break;
	case REISERFS_SUPER_MAGIC:fsname = "reiser" ; break;
	case XFS_SUPER_MAGIC:  fsname = "xfs" ; break;
	default: retval=0; goto BAIL;
	};

	for( i=buf; (i-buf) < ret; i += strlen(i)+1 ) {
		xar_ea_t e;

		context.bufsz = 0;
		context.off = 0;
		context.buf = NULL;
		context.file = file;
		e = xar_ea_new(f, i);
		xar_ea_pset(f, e, "fstype", fsname);
		context.attrname = i;
		context.ea = e;
		XAR(x)->attrcopy_to_heap(x, f, xar_ea_root(e), xar_linuxattr_read,&context);
		free(context.buf);
		context.attrname = NULL;
	}

BAIL:
	free(buf);
	return retval;
#endif
	return 0;
}

int32_t xar_linuxattr_extract(xar_t x, xar_file_t f, const char* file, char *buffer, size_t len)
{
#if defined HAVE_SYS_XATTR_H && defined(HAVE_LSETXATTR) && !defined(__APPLE__)
	const char *fsname = "bogus";
	struct statfs sfs;
	int eaopt = 0;
	struct _linuxattr_context context;
	xar_prop_t p;
	
	memset(&context,0,sizeof(struct _linuxattr_context));
	
	/* data buffers, can't store linux attrs */
	if(len){
		return 0;
	}
	
	/* Check for EA extraction behavior */

	memset(&sfs, 0, sizeof(sfs));
	if( statfs(file, &sfs) != 0 ) {
		char *tmp, *bname;
		tmp = strdup(file);
		bname = dirname(tmp);
		statfs(bname, &sfs);
		free(tmp);
	}
	switch(sfs.f_type) {
	case EXT3_SUPER_MAGIC: fsname = "ext3"; break; /* assume ext3 */
	case JFS_SUPER_MAGIC:  fsname = "jfs" ; break;
	case REISERFS_SUPER_MAGIC:fsname = "reiser" ; break;
	case XFS_SUPER_MAGIC:  fsname = "xfs" ; break;
	};

	for(p = xar_prop_pfirst(f); p; p = xar_prop_pnext(p)) {
		const char *fs = NULL;
		const char *prop;
		const char *eaname = NULL;
		xar_prop_t tmpp;
		prop = xar_prop_getkey(p);

		if( strncmp(prop, XAR_EA_FORK, strlen(XAR_EA_FORK)) != 0 )
			continue;
		if( strlen(prop) != strlen(XAR_EA_FORK) )
			continue;

		tmpp = xar_prop_pget(p, "fstype");
		if( tmpp )
			fs = xar_prop_getvalue(tmpp);
		if( !eaopt && fs && strcmp(fs, fsname) != 0 ) {
			continue;
		}
		if( !fs )
			continue;

		tmpp = xar_prop_pget(p, "name");
		if( tmpp )
			eaname = xar_prop_getvalue(tmpp);

		context.file = file;
		context.attrname = eaname;
		XAR(x)->attrcopy_from_heap(x, f, p, xar_linuxattr_write, &context);

	}

#endif
	return 0;
}

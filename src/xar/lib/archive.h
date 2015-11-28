/*
 * Copyright (c) 2005-2007 Rob Braun
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
 * Portions Copyright 2006, Apple Computer, Inc.
 * Christopher Ryan <ryanc@apple.com>
 */


#ifndef _XAR_ARCHIVE_H_
#define _XAR_ARCHIVE_H_
#include <zlib.h>
#include <libxml/hash.h>
#ifdef __APPLE__
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonDigestSPI.h>
#else
#include <openssl/evp.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include "xar.h"
#include "filetree.h"
#include "hash.h"

typedef int (*read_callback)(xar_t, xar_file_t, void *, size_t, void *context);
typedef int (*write_callback)(xar_t, xar_file_t, void *, size_t, void *context);

struct errctx {
	const char *str;
	int         saved_errno;
	xar_file_t  file;
	void       *usrctx;
	xar_t       x;
};

struct __xar_t {
	xar_prop_t props;
	xar_attr_t attrs;      /* archive options, such as rsize */
	const char *prefix;
	const char *ns;
	const char *filler1;
	const char *filler2;
	xar_file_t files;       /* file forest */
	const char *filename;   /* name of the archive we are operating on */
	char *dirname;          /* directory of the archive, used in creation */
	int fd;                 /* open file descriptor for the archive */
	int heap_fd;            /* fd for tmp heap archive, used in creation */
	off_t heap_offset;      /* current offset within the heap */
	off_t heap_len;         /* current length of the heap */
	xar_header_t header;    /* header of the xar archive */
	void *readbuf;          /* buffer for reading/writing compressed toc */
	size_t readbuf_len;     /* length of readbuf */
	size_t offset;          /* offset into readbuf for keeping track
	                         * between callbacks. */
	size_t toc_count;       /* current bytes read of the toc */
	z_stream zs;            /* gz state for compressing/decompressing toc */
	char *path_prefix;      /* used for distinguishing absolute paths */
	err_handler ercallback; /* callback for errors/warnings */
	struct errctx errctx;   /* error callback context */
	xar_subdoc_t subdocs;   /* linked list of subdocs */
	xar_signature_t signatures; /* linked list of signatures */
	int32_t (*attrcopy_to_heap)(xar_t, xar_file_t, xar_prop_t, read_callback, void *);
	int32_t (*attrcopy_from_heap)(xar_t, xar_file_t, xar_prop_t, write_callback, void *);
	int32_t (*heap_to_archive)(xar_t);
	uint64_t last_fileid;       /* unique fileid's in the archive */
	xmlHashTablePtr ino_hash;   /* Hash for looking up hardlinked files (add)*/
	xmlHashTablePtr link_hash;  /* Hash for looking up hardlinked files (extract)*/
	xmlHashTablePtr csum_hash;  /* Hash for looking up checksums of files */
	xar_hash_t toc_hash_ctx;
	int skipwarn;
	struct stat sbcache;
};

#define XAR(x) ((struct __xar_t *)(x))

#endif /* _XAR_ARCHIVE_H_ */

/*
 * Copyright (c) 2005-2008 Rob Braun
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

#define _FILE_OFFSET_BITS 64

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <arpa/inet.h> /* for ntoh{l,s} */
#include <inttypes.h>  /* for PRIu64 */
#include <time.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlstring.h>
#ifndef HAVE_ASPRINTF
#include "asprintf.h"
#endif
#include "xar.h"
#include "filetree.h"
#include "archive.h"
#include "signature.h"
#include "arcmod.h"
#include "io.h"
#include "util.h"
#include "subdoc.h"
#include "darwinattr.h"

#ifndef O_EXLOCK
#define O_EXLOCK 0
#endif
#ifndef O_SHLOCK
#define O_SHLOCK 0
#endif

#ifndef LONG_MAX
#define LONG_MAX INT32_MAX
#endif
#ifndef LONG_MIN
#define LONG_MIN INT32_MIN
#endif

#if LIBXML_VERSION < 20618
#define xmlDictCleanup()	/* function doesn't exist in older API */
#endif

static int32_t xar_unserialize(xar_t x);
void xar_serialize(xar_t x, const char *file);

/* xar_new
 * Returns: newly allocated xar_t structure
 * Summary: just does basicallocation and initialization of 
 * xar_t structure.
 */
static xar_t xar_new() {
	xar_t ret;
	ret = malloc(sizeof(struct __xar_t));
	if(!ret) return NULL;
	memset(XAR(ret), 0, sizeof(struct __xar_t));
	XAR(ret)->readbuf_len = 4096;
	XAR(ret)->readbuf = malloc(XAR(ret)->readbuf_len);
	if(!XAR(ret)->readbuf) {
		free((void *)ret);
		return NULL;
	}
	XAR(ret)->offset = 0;

	XAR(ret)->zs.zalloc = Z_NULL;
	XAR(ret)->zs.zfree = Z_NULL;
	XAR(ret)->zs.opaque = Z_NULL;
	XAR(ret)->ino_hash = xmlHashCreate(0);
	XAR(ret)->link_hash = xmlHashCreate(0);
	XAR(ret)->csum_hash = xmlHashCreate(0);
	XAR(ret)->subdocs = NULL;
	
	XAR(ret)->attrcopy_to_heap = xar_attrcopy_to_heap;
	XAR(ret)->attrcopy_from_heap = xar_attrcopy_from_heap;
	XAR(ret)->heap_to_archive = xar_heap_to_archive;
	
	return ret;
}

/* xar_parse_header
 * x: archive to operate on.
 * Returns: 0 on success, -1 on failure
 * Summary: internal helper function to read in the xar header.
 */
static int32_t xar_parse_header(xar_t x) {
	ssize_t r;
	int off = 0;
	int sz2read = 0;

	/* read just the magic, verify it, read the header length,
	 * then read in the size of the header according to the
	 * recorded header length, or the length of the structure
	 * we expect, whichever is smaller.  Then seek forward
	 * if the recorded header length is greater than the 
	 * expected header length.
	 */
	r = xar_read_fd(XAR(x)->fd, (char *)&XAR(x)->header.magic+off, sizeof(XAR(x)->header.magic)-off);
	if ( r == -1 )
		return r;

	/* Verify the header.  If the header doesn't match, exit without
	 * attempting to read any more.
	 */
	XAR(x)->header.magic = ntohl(XAR(x)->header.magic);

	if( XAR(x)->header.magic != XAR_HEADER_MAGIC ) {
		return -1;
	}

	r = xar_read_fd(XAR(x)->fd, (char *)&XAR(x)->header.size+off, sizeof(XAR(x)->header.size)-off);
	if ( r == -1 )
		return r;

	XAR(x)->header.size = ntohs(XAR(x)->header.size);

	if( XAR(x)->header.size > sizeof(xar_header_t) )
		sz2read = sizeof(xar_header_t);
	else
		sz2read = XAR(x)->header.size;

	off = sizeof(XAR(x)->header.magic) + sizeof(XAR(x)->header.size);
	r = xar_read_fd(XAR(x)->fd, ((char *)&XAR(x)->header)+off, sizeof(xar_header_t)-off);
	if ( r == -1 )
		return r;

	XAR(x)->header.version = ntohs(XAR(x)->header.version);
	XAR(x)->header.toc_length_compressed = xar_ntoh64(XAR(x)->header.toc_length_compressed);
	XAR(x)->header.toc_length_uncompressed = xar_ntoh64(XAR(x)->header.toc_length_uncompressed);
	XAR(x)->header.cksum_alg = ntohl(XAR(x)->header.cksum_alg);

	off = XAR(x)->header.size - sz2read;
	if( off > 0 )
		r = lseek(XAR(x)->fd, (off_t)off, SEEK_CUR);

	if ( (r == -1) && (errno != ESPIPE) )
		/* Some fatal error here perhaps? */ ;

	return 0;
}

/* xar_open
 * file: filename to open
 * flags: flags on how to open the file.  0 for readonly, !0 for read/write
 * Returns: allocated and initialized xar structure with an open
 * file descriptor to the target xar file.  If the xarchive is opened
 * for writing, the file is created, and a heap file is opened.
 */
xar_t xar_open(const char *file, int32_t flags) {
	xar_t ret;

	ret = xar_new();
	if( !ret ) return NULL;
	if( !file )
		file = "-";
	XAR(ret)->filename = strdup(file);
	if( flags ) { // writing
		char *tmp1, *tmp2, *tmp3, *tmp4;
		tmp1 = tmp2 = strdup(file);
		tmp3 = dirname(tmp2);
		XAR(ret)->dirname = strdup(tmp3);
		/* Create the heap file in the directory which will contain
		 * the target archive.  /tmp or elsewhere may fill up.
		 */
		asprintf(&tmp4, "%s/xar.heap.XXXXXX", tmp3);
		free(tmp1);
		if( strcmp(file, "-") == 0 )
			XAR(ret)->fd = 1;
		else{
			XAR(ret)->fd = open(file, O_WRONLY | O_CREAT | O_TRUNC | O_EXLOCK, 0644);
			if( (-1 == XAR(ret)->fd ) && (ENOTSUP == errno) ){
				XAR(ret)->fd = open(file, O_WRONLY | O_CREAT | O_TRUNC , 0644);				
			}
		}
		XAR(ret)->heap_fd = mkstemp(tmp4);
		if( XAR(ret)->heap_fd < 0 ) {
			close(XAR(ret)->fd);
			free(XAR(ret));
			return NULL;
		}
		
		unlink(tmp4);
		free(tmp4);

		deflateInit(&XAR(ret)->zs, Z_BEST_COMPRESSION);

		if( XAR(ret)->fd < 0 ) {
			xar_close(ret);
			return NULL;
		}

		/* default to using sha1, if nothing else is
		 * specified.
		 */
		XAR(ret)->heap_offset += 20;
		XAR(ret)->heap_len += 20;
		
		xar_opt_set(ret, XAR_OPT_COMPRESSION, XAR_OPT_VAL_GZIP);
		xar_opt_set(ret, XAR_OPT_FILECKSUM, XAR_OPT_VAL_SHA1);
		xar_opt_set(ret, XAR_OPT_TOCCKSUM, XAR_OPT_VAL_SHA1);
	} else {
		if( strcmp(file, "-") == 0 )
			XAR(ret)->fd = 0;
		else{
			XAR(ret)->fd = open(file, O_RDONLY | O_SHLOCK);
			
			if( (-1 == XAR(ret)->fd ) && (ENOTSUP == errno) ){
				XAR(ret)->fd = open(file, O_RDONLY);
			}

		}
		XAR(ret)->heap_fd = -1;
		inflateInit(&XAR(ret)->zs);
		if( XAR(ret)->fd < 0 ) {
			xar_close(ret);
			return NULL;
		}

		if( xar_parse_header(ret) != 0 ) {
			xar_close(ret);
			return NULL;
		}

		switch(XAR(ret)->header.cksum_alg) {
		case XAR_CKSUM_NONE:
			break;
		case XAR_CKSUM_SHA1:
			XAR(ret)->toc_hash_ctx = xar_hash_new("sha1", (void *)ret);
			break;
		case XAR_CKSUM_SHA256:
			XAR(ret)->toc_hash_ctx = xar_hash_new("sha256", (void *)ret);
			break;
		case XAR_CKSUM_SHA512:
			XAR(ret)->toc_hash_ctx = xar_hash_new("sha512", (void *)ret);
			break;
		case XAR_CKSUM_MD5:
			XAR(ret)->toc_hash_ctx = xar_hash_new("md5", (void *)ret);
			break;
		default:
			fprintf(stderr, "Unknown hashing algorithm, skipping\n");
			break;
		};

		if( xar_unserialize(ret) != 0 ) {
			xar_close(ret);
			return NULL;
		}

		/* check for inconsistency between checksum style in header, 
		 * and the one described in the TOC; otherwise, you can flip 
		 * the header bit to XAR_CKSUM_NONE, and nothing will ever 
		 * verify that the TOC matches the checksum stored in the 
		 * heap, and the signature check will pass on a modified 
		 * file! <rdar://problem/6134714>
		 */
		int cksum_match = 0;
		const char* cksum_style = xar_attr_get(XAR_FILE(ret), "checksum", "style");
		switch(XAR(ret)->header.cksum_alg) {
			case XAR_CKSUM_NONE:
				cksum_match = (cksum_style == NULL || strcmp(cksum_style, XAR_OPT_VAL_NONE) == 0);
				break;
			case XAR_CKSUM_SHA1:
				cksum_match = (cksum_style != NULL && strcmp(cksum_style, XAR_OPT_VAL_SHA1) == 0);
				break;
			case XAR_CKSUM_SHA256:
				cksum_match = (cksum_style != NULL && strcmp(cksum_style, XAR_OPT_VAL_SHA256) == 0);
				break;
			case XAR_CKSUM_SHA512:
				cksum_match = (cksum_style != NULL && strcmp(cksum_style, XAR_OPT_VAL_SHA512) == 0);
				break;
			case XAR_CKSUM_MD5:
				cksum_match = (cksum_style != NULL && strcmp(cksum_style, XAR_OPT_VAL_MD5) == 0);
				break;
			default:
				cksum_match = 0;
				break;
		}
		if( !cksum_match ) {
			fprintf(stderr, "Checksum style mismatch!\n");
			xar_close(ret);
			return NULL;
		}
		
		/* also check for consistency between the checksum style and 
		 * the existence (or not) of signatures: since the signature 
		 * is signing the checksum, we must have a checksum to verify 
		 * that the TOC has not been modified <rdar://problem/6134714>
		 */
		if( xar_signature_first(ret) != NULL && XAR(ret)->header.cksum_alg == XAR_CKSUM_NONE ) {
			fprintf(stderr, "Checksum/signature mismatch!\n");
			xar_close(ret);
			return NULL;
		}
		
		/* If we aren't checksumming the TOC, we're done */
		if( !XAR(ret)->toc_hash_ctx )
			return ret;
        
        /* if TOC specifies a location for the checksum, make sure that
         * we read the checksum from there: this is required for an archive
         * with a signature, because the signature will be checked against
         * the checksum at the specified location <rdar://problem/7041949>
         */
		const char *value;
		uint64_t offset = 0;
		uint64_t length = 0;
		if( xar_prop_get( XAR_FILE(ret) , "checksum/offset", &value) == 0 ) {
			errno = 0;
			offset = strtoull( value, (char **)NULL, 10);
			if( errno != 0 ) {
				xar_close(ret);
				return NULL;
			}
		} else if( xar_signature_first(ret) != NULL ) {
			// All archives that have a signature also specify the location
			// of the checksum.  If the location isn't specified, error out.
			xar_close(ret);
			return NULL;
		}
        
		XAR(ret)->heap_offset = xar_get_heap_offset(ret) + offset;
		if( lseek(XAR(ret)->fd, XAR(ret)->heap_offset, SEEK_SET) == -1 ) {
			xar_close(ret);
			return NULL;
		}
		if( xar_prop_get( XAR_FILE(ret) , "checksum/size", &value) == 0 ) {
			errno = 0;
			length = strtoull( value, (char **)NULL, 10);
			if( errno != 0 ) {
				xar_close(ret);
				return NULL;
			}
		} else if( xar_signature_first(ret) != NULL ) {
			xar_close(ret);
			return NULL;
		}
		
		size_t tlen = 0;
		void *toccksum = xar_hash_finish(XAR(ret)->toc_hash_ctx, &tlen);
		
		if( length != tlen ) {
			free(toccksum);
			xar_close(ret);
			return NULL;
		}
		
		void *cval = calloc(1, tlen);
		if( ! cval ) {
			free(toccksum);
			xar_close(ret);
			return NULL;
		}
		
		xar_read_fd(XAR(ret)->fd, cval, tlen);
		XAR(ret)->heap_offset += tlen;
		if( memcmp(cval, toccksum, tlen) != 0 ) {
			fprintf(stderr, "Checksums do not match!\n");
			free(toccksum);
			free(cval);
			xar_close(ret);
			return NULL;
		}
		
		free(toccksum);
		free(cval);
	}

	return ret;
}

/* xar_close
 * x: the xar_t to close
 * Summary: closes all open file descriptors, frees all
 * file structures and options, deallocates the xar_t its self.
 * Returns 0 for success, -1 for failure.
 */
int xar_close(xar_t x) {
	xar_attr_t a;
	xar_file_t f;
	int ret, retval = 0;

	/* If we're creating an archive */
	if( XAR(x)->heap_fd != -1 ) {
		char *tmpser;
		void *rbuf, *wbuf = NULL;
		int fd, r, off, wbytes, rbytes;
		long rsize, wsize;
		z_stream zs;
		uint64_t ungztoc, gztoc;
		int tocfd;
		char timestr[128];
		struct tm tmptm;
		time_t t;

		tmpser = (char *)xar_opt_get(x, XAR_OPT_TOCCKSUM);
		/* If no checksum type is specified, default to sha1 */
		if( !tmpser ) tmpser = XAR_OPT_VAL_SHA1;

		if( (strcmp(tmpser, XAR_OPT_VAL_NONE) != 0) ) {
			xar_prop_set(XAR_FILE(x), "checksum", NULL);
			if( strcmp(tmpser, XAR_OPT_VAL_SHA1) == 0 ) {
				XAR(x)->toc_hash_ctx = xar_hash_new("sha1", (void *)x);
				XAR(x)->header.cksum_alg = htonl(XAR_CKSUM_SHA1);
				xar_attr_set(XAR_FILE(x), "checksum", "style", XAR_OPT_VAL_SHA1);
				xar_prop_set(XAR_FILE(x), "checksum/size", "20");
			}
			if( strcmp(tmpser, XAR_OPT_VAL_SHA256) == 0 ) {
				XAR(x)->toc_hash_ctx = xar_hash_new("sha256", (void *)x);
				XAR(x)->header.cksum_alg = htonl(XAR_CKSUM_SHA256);
				xar_attr_set(XAR_FILE(x), "checksum", "style", XAR_OPT_VAL_SHA256);
				xar_prop_set(XAR_FILE(x), "checksum/size", "32");
			}
			if( strcmp(tmpser, XAR_OPT_VAL_SHA512) == 0 ) {
				XAR(x)->toc_hash_ctx = xar_hash_new("sha512", (void *)x);
				XAR(x)->header.cksum_alg = htonl(XAR_CKSUM_SHA512);
				xar_attr_set(XAR_FILE(x), "checksum", "style", XAR_OPT_VAL_SHA512);
				xar_prop_set(XAR_FILE(x), "checksum/size", "64");
			}
			if( strcmp(tmpser, XAR_OPT_VAL_MD5) == 0 ) {
				XAR(x)->toc_hash_ctx = xar_hash_new("md5", (void *)x);
				XAR(x)->header.cksum_alg = htonl(XAR_CKSUM_MD5);
				xar_attr_set(XAR_FILE(x), "checksum", "style", XAR_OPT_VAL_MD5);
				xar_prop_set(XAR_FILE(x), "checksum/size", "16");
			}

			xar_prop_set(XAR_FILE(x), "checksum/offset", "0");
		} else {
			XAR(x)->header.cksum_alg = XAR_CKSUM_NONE;
		}

		t = time(NULL);
		gmtime_r(&t, &tmptm);
		memset(timestr, 0, sizeof(timestr));
		strftime(timestr, sizeof(timestr), "%FT%T", &tmptm);
		xar_prop_set(XAR_FILE(x), "creation-time", timestr);
		
		/* serialize the toc to a tmp file */
		asprintf(&tmpser, "%s/xar.toc.XXXXXX", XAR(x)->dirname);
		fd = mkstemp(tmpser);
		xar_serialize(x, tmpser);
		unlink(tmpser);
		free(tmpser);
		asprintf(&tmpser, "%s/xar.toc.XXXXXX", XAR(x)->dirname);
		tocfd = mkstemp(tmpser);
		unlink(tmpser);
		free(tmpser);
		
	
		/* read the toc from the tmp file, compress it, and write it
	 	* out to the archive.
	 	*/
		rsize = wsize = 4096;
		const char * opt = xar_opt_get(x, XAR_OPT_RSIZE);
		if ( opt ) {
		  rsize = strtol(opt, NULL, 0);
		  if ( ((rsize == LONG_MAX) || (rsize == LONG_MIN)) && (errno == ERANGE) ) {
		    rsize = wsize;
		  }
		}
		
		rbuf = malloc(rsize);
		if( !rbuf ) {
			retval = -1;
			close(fd);
			close(tocfd);
			goto CLOSE_BAIL;
		}
		zs.zalloc = Z_NULL;
		zs.zfree = Z_NULL;
		zs.opaque = Z_NULL;
		deflateInit(&zs, Z_BEST_COMPRESSION);
	
		ungztoc = gztoc = 0;
	
		while(1) {
			r = read(fd, rbuf, rsize);
			if( (r < 0) && (errno == EINTR) )
				continue;
			if( r == 0 )
				break;
	
			ungztoc += r;
	
			zs.avail_in = r;
			zs.next_in = (void *)rbuf;
			zs.next_out = NULL;
			zs.avail_out = 0;

			wsize = rsize/2;
	
			off = 0;
			while( zs.avail_in != 0 ) {
				wsize *= 2;
				wbuf = realloc(wbuf, wsize);

				zs.next_out = ((unsigned char *)wbuf) + off;
				zs.avail_out = wsize - off;

				ret = deflate(&zs, Z_SYNC_FLUSH);
				off = wsize - zs.avail_out;
			}
	
			wbytes = off;
			off = 0;
			do {
				r = write(tocfd, ((char *)wbuf)+off, wbytes-off);
				if( (r < 0) && (errno == EINTR) )
					continue;
				if( r < 0 ) {
					xar_err_new(x);
					xar_err_set_string(x, "Error closing xar archive");
					retval = -1;
					goto CLOSEEND;
				}
				if( XAR(x)->toc_hash_ctx )
					xar_hash_update(XAR(x)->toc_hash_ctx, ((char*)wbuf)+off, r);
				off += r;
				gztoc += r;
			} while( off < wbytes );

		}

		zs.next_in = NULL;
		zs.avail_in = 0;
		zs.next_out = wbuf;
		zs.avail_out = wsize;

		deflate(&zs, Z_FINISH);
		r = write(tocfd, wbuf, wsize - zs.avail_out);
		gztoc += r;
		if( XAR(x)->toc_hash_ctx )
			xar_hash_update(XAR(x)->toc_hash_ctx, wbuf, r);
		
		deflateEnd(&zs);

		/* populate the header and write it out */
		XAR(x)->header.magic = htonl(XAR_HEADER_MAGIC);
		XAR(x)->header.size = ntohs(sizeof(xar_header_t));
		XAR(x)->header.version = ntohs(1);
		XAR(x)->header.toc_length_uncompressed = xar_ntoh64(ungztoc);
		XAR(x)->header.toc_length_compressed = xar_ntoh64(gztoc);

		write(XAR(x)->fd, &XAR(x)->header, sizeof(xar_header_t));

		/* Copy the temp compressed toc file into the file */
		lseek(tocfd, (off_t)0, SEEK_SET);
		while(1) {
			r = read(tocfd, rbuf, rsize);
			if( (r < 0) && (errno == EINTR) )
				continue;
			if( r == 0 )
				break;

			wbytes = r;
			off = 0;
			do {
				r = write(XAR(x)->fd, ((char *)rbuf)+off, wbytes-off);
				if( (r < 0) && (errno == EINTR) )
					continue;
				if( r < 0 ) {
					xar_err_new(x);
					xar_err_set_string(x, "Error closing xar archive");
					retval = -1;
					goto CLOSEEND;
				}
					
				off += r;
			} while( off < wbytes );
		}

		if( XAR(x)->toc_hash_ctx ) {
			size_t chklen = 0;
			void *chkstr = xar_hash_finish(XAR(x)->toc_hash_ctx, &chklen);
			write(XAR(x)->fd, chkstr, chklen);

			/* If there are any signatures, get the signed data a sign it */
			if( XAR(x)->signatures ) {
				xar_signature_t sig;
				uint32_t signed_len = 0;
				uint8_t *signed_data = NULL;
				
				/* Loop through the signatures */
				for(sig = XAR(x)->signatures; sig; sig = XAR_SIGNATURE(sig)->next ){				
					signed_len = XAR_SIGNATURE(sig)->len;
					
					/* If callback returns something other then 0, bail */
					if( 0 != sig->signer_callback( sig, sig->callback_context, chkstr, chklen, &signed_data, &signed_len ) ){
						fprintf(stderr, "Error signing data.\n");
						free(chkstr);
						retval = -1;
						close(fd);
						close(tocfd);
						goto CLOSE_BAIL;					
					}
					
					if( signed_len != XAR_SIGNATURE(sig)->len ){
						fprintf(stderr, "Signed data not the proper length.  %i should be %i.\n",signed_len,XAR_SIGNATURE(sig)->len);
						free(chkstr);
						retval = -1;
						close(fd);
						close(tocfd);
						goto CLOSE_BAIL;										
					}
					
					/* Write the signed data to the heap */
					write(XAR(x)->fd, signed_data,XAR_SIGNATURE(sig)->len);
					
					free(signed_data);
				}
				free(chkstr);
			}
			
			xar_signature_remove( XAR(x)->signatures );
			XAR(x)->signatures = NULL;
		}
		
		/* copy the heap from the temporary heap into the archive */
		if( 0 > XAR(x)->heap_to_archive(x) ) {
			xar_err_new(x);
			xar_err_set_string(x, "Error while copying heap contents into archive");
			xar_err_set_errno(x, errno);
			xar_err_callback(x, XAR_SEVERITY_FATAL, XAR_ERR_ARCHIVE_CREATION);
			retval = -1;
			close(fd);
			close(tocfd);
			goto CLOSEEND;
		}
		
CLOSEEND:
		close(fd);
		close(tocfd);
		free(rbuf);
		free(wbuf);
		deflateEnd(&XAR(x)->zs);
	} else {
		
		xar_signature_remove( XAR(x)->signatures );
		XAR(x)->signatures = NULL;
		
		inflateEnd(&XAR(x)->zs);
	}
		
CLOSE_BAIL:
	/* continue deallocating the archive and return */
	while(XAR(x)->subdocs) {
		xar_subdoc_remove(XAR(x)->subdocs);
	}

	while(XAR(x)->attrs) {
		a = XAR(x)->attrs;
		XAR(x)->attrs = XAR_ATTR(a)->next;
		xar_attr_free(a);
	}

	while(XAR(x)->props) {
		xar_prop_t p;
		p = XAR(x)->props;
		XAR(x)->props = XAR_PROP(p)->next;
		xar_prop_free(p);
	}

	while(XAR(x)->files) {
		f = XAR(x)->files;
		XAR(x)->files = XAR_FILE(f)->next;
		xar_file_free(f);
	}

	xmlHashFree(XAR(x)->ino_hash, NULL);
	xmlHashFree(XAR(x)->link_hash, NULL);
	xmlHashFree(XAR(x)->csum_hash, NULL);
	close(XAR(x)->fd);
	if( XAR(x)->heap_fd >= 0 )
		close(XAR(x)->heap_fd);
	free((char *)XAR(x)->filename);
	free((char *)XAR(x)->dirname);
	free(XAR(x)->readbuf);
	free((void *)x);

	return retval;
}

/* xar_opt_get
 * x: archive to get the option from
 * option: name of the option
 * Returns: a pointer to the value of the option
 * In the case of more than one option with the same name, this will
 * return the first match.
 */
const char *xar_opt_get(xar_t x, const char *option) {
	xar_attr_t i;
	for(i = XAR(x)->attrs; i && XAR_ATTR(i)->next; i = XAR_ATTR(i)->next) {
		if(strcmp(XAR_ATTR(i)->key, option)==0)
			return XAR_ATTR(i)->value;
	}
	if( i && (strcmp(XAR_ATTR(i)->key, option)==0) )
		return XAR_ATTR(i)->value;
	return NULL;
}

/* xar_opt_set
 * x: the archive to set the option of
 * option: the name of the option to set the value of
 * value: the value to set the option to
 * Returns: 0 for sucess, -1 for failure
 */
int32_t xar_opt_set(xar_t x, const char *option, const char *value) {
	xar_attr_t currentAttr, a;

	if( (strcmp(option, XAR_OPT_TOCCKSUM) == 0) ) {
		XAR(x)->heap_offset = xar_io_get_toc_checksum_length_for_type(value);
	}
	
	/*	This was an edit from xar-1.4
		Looks like we would only allow one definition for a particular key in this list
		But xar_opt_unset is implemented to remove many pairs for the same key
	 
	// if the attribute is already defined, find it and free its value,
	// replace and return
	for(currentAttr = XAR(x)->attrs; currentAttr ; currentAttr = XAR_ATTR(currentAttr)->next) {
		if(strcmp(XAR_ATTR(currentAttr)->key, option)==0) {
			free((char*)XAR_ATTR(currentAttr)->value);
			XAR_ATTR(currentAttr)->value = strdup(value);
			return 0;
		}
	} */
	
	// otherwise create a new attribute
	a = xar_attr_new();
	XAR_ATTR(a)->key = strdup(option);
	XAR_ATTR(a)->value = strdup(value);
	// and prepend it to the attrs list for the archive
	XAR_ATTR(a)->next = XAR(x)->attrs;
	XAR(x)->attrs = a;
	return 0;
}

/* xar_opt_unset
 * x: the archive to set the option of
 * option: the name of the option to delete
 * This will delete ALL instances of the option name
 */
int32_t xar_opt_unset(xar_t x, const char *option) {
	xar_attr_t currentAttr, previousAttr = NULL;
	for(currentAttr = XAR(x)->attrs; 
		currentAttr ; 
		previousAttr = currentAttr, currentAttr = XAR_ATTR(currentAttr)->next) {
		if(strcmp(XAR_ATTR(currentAttr)->key, option)==0) {
			
			// if this attribute match is the head of the attrs list
			// promote the next list item to the head
			if( previousAttr == NULL )
				XAR(previousAttr)->attrs = XAR_ATTR(currentAttr)->next;
			
			// otherwise splice the list around this attr
			else
				XAR_ATTR(previousAttr)->next = XAR_ATTR(currentAttr)->next;
			xar_attr_free(currentAttr);
			
			// keep going to find other instances
			currentAttr = previousAttr;
		}
	}
	return 0;
}

/* xar_add_node
 * x: archive the file should belong to
 * f: parent node, possibly NULL
 * name: name of the node to add
 * realpath: real path to item, this is used if the item being archived is to be located at a different location in the tree
 * then it is on the real filesystem.
 * Returns: newly allocated and populated node
 * Summary: helper function which adds a child of f and populates
 * its properties.  If f is NULL, the node will be added as a top
 * level node of the archive, x.
 */
static xar_file_t xar_add_node(xar_t x, xar_file_t f, const char *name, const char *prefix, const char *realpath, int srcpath) {
	xar_file_t ret;
	const char *path; 
	char *tmp;
	char idstr[32];

	if( !f ) {
		if( realpath )
			asprintf(&tmp, "%s", realpath);
		else
			asprintf(&tmp, "%s%s%s", XAR(x)->path_prefix, prefix, name);

		if( lstat(tmp, &XAR(x)->sbcache) != 0 ) {
			free(tmp);
			return NULL;
		}

		ret = xar_file_new(NULL);
		if( !ret )
			return NULL;
		memset(idstr, 0, sizeof(idstr));
		snprintf(idstr, sizeof(idstr)-1, "%"PRIu64, ++XAR(x)->last_fileid);
		xar_attr_set(ret, NULL, "id", idstr);
		XAR_FILE(ret)->parent = NULL;
		XAR_FILE(ret)->fspath = tmp;
		if( XAR(x)->files == NULL )
			XAR(x)->files = ret;
		else {
			XAR_FILE(ret)->next = XAR(x)->files;
			XAR(x)->files = ret;
		}
	} else {
		path = XAR_FILE(f)->fspath;
		if( strcmp(prefix, "../") == 0 ) {
			int len1, len2;
			len1 = strlen(path);
			len2 = strlen(name);
			if( (len1>=len2) && (strcmp(path+(len1-len2), name) == 0) ) {
				return f;
			}
			
		}

		if( realpath ){
			asprintf(&tmp, "%s", realpath);
		}else
			asprintf(&tmp, "%s/%s%s", path, prefix, name);
		
		if( lstat(tmp, &XAR(x)->sbcache) != 0 ) {
			free(tmp);
			return NULL;
		}

		ret = xar_file_new(f);
		if( !ret )
			return NULL;
		memset(idstr, 0, sizeof(idstr));
		snprintf(idstr, sizeof(idstr)-1, "%"PRIu64, ++XAR(x)->last_fileid);
		xar_attr_set(ret, NULL, "id", idstr);
		XAR_FILE(ret)->fspath = tmp;
	}

	xar_prop_set(ret, "name", name);

	if( xar_arcmod_archive(x, ret, XAR_FILE(ret)->fspath, NULL, 0) < 0 ) {
		xar_file_t i = NULL;
		if( f ) {
			if( ret == XAR_FILE(f)->children )
				XAR_FILE(f)->children = XAR_FILE(ret)->next;
			else
				for( i = XAR_FILE(f)->children; i && (XAR_FILE(i)->next != ret); i = XAR_FILE(i)->next );
		} else {
			if( ret == XAR(x)->files )
				XAR(x)->files = XAR_FILE(ret)->next;
			else
				for( i = XAR(x)->files; i && (XAR_FILE(i)->next != ret); i = XAR_FILE(i)->next );
		}
		if( i )
			XAR_FILE(i)->next = XAR_FILE(ret)->next;
		xar_file_free(ret);
		return NULL;
	}

	return ret;
}

/* xar_add_pseudodir
 * Summary: Adds a placeholder directory when archiving a file prior
 * to archiving its path.
 */
static xar_file_t xar_add_pseudodir(xar_t x, xar_file_t f, const char *name, const char *prefix, const char *realpath)
{
	xar_file_t ret;
	const char *path; 
	char *tmp;
	char idstr[32];

	if( !f ) {
		if( realpath )
			asprintf(&tmp, "%s", realpath);
		else
			asprintf(&tmp, "%s%s%s", XAR(x)->path_prefix, prefix, name);

		if( lstat(tmp, &XAR(x)->sbcache) != 0 ) {
			free(tmp);
			return NULL;
		}

		ret = xar_file_new(NULL);
		if( !ret )
			return NULL;
		memset(idstr, 0, sizeof(idstr));
		snprintf(idstr, sizeof(idstr)-1, "%"PRIu64, ++XAR(x)->last_fileid);
		xar_attr_set(ret, NULL, "id", idstr);
		XAR_FILE(ret)->parent = NULL;
		XAR_FILE(ret)->fspath = tmp;
		if( XAR(x)->files == NULL )
			XAR(x)->files = ret;
		else {
			XAR_FILE(ret)->next = XAR(x)->files;
			XAR(x)->files = ret;
		}
	} else {
		path = XAR_FILE(f)->fspath;
		if( strcmp(prefix, "../") == 0 ) {
			int len1, len2;
			len1 = strlen(path);
			len2 = strlen(name);
			if( (len1>=len2) && (strcmp(path+(len1-len2), name) == 0) ) {
				return f;
			}
			
		}

		if( realpath ){
			asprintf(&tmp, "%s", realpath);
		}else
			asprintf(&tmp, "%s/%s%s", path, prefix, name);
		
		if( lstat(tmp, &XAR(x)->sbcache) != 0 ) {
			free(tmp);
			return NULL;
		}

		ret = xar_file_new(f);
		if( !ret )
			return NULL;
		memset(idstr, 0, sizeof(idstr));
		snprintf(idstr, sizeof(idstr)-1, "%"PRIu64, ++XAR(x)->last_fileid);
		xar_attr_set(ret, NULL, "id", idstr);
		XAR_FILE(ret)->fspath = tmp;
	}
	xar_prop_set(ret, "name", name);
	xar_prop_set(ret, "type", "directory");

	return ret;
}

/* xar_add_r
 * Summary: a recursive helper function for adding a node to the
 * tree.  This will search all children of node f, looking for
 * the path component.  If found, will recurse into it.  If not,
 * will add the path component to the tree, and recurse into it.
 * If f is NULL, will start with x->files.
 */
static xar_file_t xar_add_r(xar_t x, xar_file_t f, const char *path, const char *prefix) {
	xar_file_t i = NULL, ret, ret2, start = NULL;
	char *tmp1, *tmp2, *tmp3;

	if( path && (path[0] == '\0') ) {
		return f;
	}

	tmp1 = tmp2 = strdup(path);
	tmp3 = strsep(&tmp2, "/");

	if( tmp3 && tmp2 && (tmp3[0] == '\0') ) {
		ret2 = xar_add_r(x, f, tmp2, "");
		free(tmp1);
		return ret2;
	}

	if( strcmp(tmp3, "..") == 0 ) {
		char *prefixstr;
		if( !XAR(x)->skipwarn ) {
			xar_err_new(x);
			xar_err_set_string(x, "Skipping .. in path");
			xar_err_callback(x, XAR_SEVERITY_WARNING, XAR_ERR_ARCHIVE_CREATION);
			XAR(x)->skipwarn = 1;
		}
		asprintf(&prefixstr, "%s../", prefix);
		ret2 = xar_add_r(x, f, tmp2, prefixstr);
		free(prefixstr);
		free(tmp1);
		return ret2;
	}

	if( strcmp(tmp3, ".") == 0 ) {
		if( tmp2 )
			ret2 = xar_add_r(x, f, tmp2, prefix);
		else
			ret2 = NULL;
		free(tmp1);
		return ret2;
	}

	if( !f ) {
		start = XAR(x)->files;
	} else {
		start = XAR_FILE(f)->children;
	}

	/* Search all the siblings */
	for( i = start; i; i = XAR_FILE(i)->next ) {
		const char *n;
		xar_prop_get(i, "name", &n);
		if( strcmp(n, tmp3) == 0 ) {
			if( !tmp2 ) {
				/* Node already exists, and it is i */
				free(tmp1);
				return i;
			}
			ret2 = xar_add_r(x, i, tmp2, "");
			free(tmp1);
			return ret2;
		}
	}

	/* tmp3 was not found in children of start, so we add it */
	if( tmp2 ) {
		//ret = xar_add_node(x, f, tmp3, prefix, NULL,  1);
		ret = xar_add_pseudodir(x, f, tmp3, prefix, NULL);
	} else {
		ret = xar_add_node(x, f, tmp3, prefix, NULL,  0);
	}

	if( !ret ) {
		free(tmp1);
		return NULL;
	}

	if( !tmp2 ) {
		/* We've added the final piece, done, don't recurse */
		free(tmp1);
		return ret;
	}

	/* still more to add, recurse */
	ret2 = xar_add_r(x, ret, tmp2, "");
	free(tmp1);
	return ret2;
}

/* xar_add
 * x: archive to add the file to
 * path: path to file
 * Returns: allocated an populated xar_file_t representing the 
 * specified file.
 * Summary: if a full path "foo/bar/blah" is specified, then any
 * directories not already existing in the archive will be added
 * automagically.  The returned xar_file_t represents the file
 * specified, not the parent of the directory tree.
 * For instance, if "foo/bar/blah" is specified, the xar_file_t
 * representing "blah" will be returned.
 */
xar_file_t xar_add(xar_t x, const char *path) {
#ifdef __APPLE__
	xar_file_t ret;
	if( (ret = xar_underbar_check(x, NULL, path)) )
		return ret;
#endif

	if( path[0] == '/' ) {
		XAR(x)->path_prefix = "/";
		path++;
	} else
		XAR(x)->path_prefix = "";
	return xar_add_r(x, NULL, path, "");
}

/* xar_add_frombuffer
* x: archive to add the file to
* parent: parent node, possibly NULL
* name: name of file
* buffer: buffer for file contents
* length: length of buffer
* Returns: allocated an populated xar_file_t representing the 
* specified file.
* Summary: Use this to add chunks of named data to a xar without
* using the filesystem.
*/

xar_file_t xar_add_frombuffer(xar_t x, xar_file_t parent, const char *name, char *buffer, size_t length) {
	xar_file_t ret;
	char idstr[32];
	
	if( !parent ) {
		ret = xar_file_new(NULL);
		if( !ret )
			return NULL;
		memset(idstr, 0, sizeof(idstr));
		snprintf(idstr, sizeof(idstr)-1, "%"PRIu64, ++XAR(x)->last_fileid);
		xar_attr_set(ret, NULL, "id", idstr);
		XAR_FILE(ret)->parent = NULL;
		if( XAR(x)->files == NULL )
			XAR(x)->files = ret;
		else {
			XAR_FILE(ret)->next = XAR(x)->files;
			XAR(x)->files = ret;
		}
	} else {
		ret = xar_file_new(parent);
		if( !ret )
			return NULL;
		memset(idstr, 0, sizeof(idstr));
		snprintf(idstr, sizeof(idstr)-1, "%"PRIu64, ++XAR(x)->last_fileid);
		xar_attr_set(ret, NULL, "id", idstr);
		XAR_FILE(ret)->fspath = NULL;
	}
	
	xar_prop_set(ret, "name", name);
		
	//int32_t xar_arcmod_archive(xar_t x, xar_file_t f, const char *file, const char *buffer, size_t len) 
	if( xar_arcmod_archive(x, ret, NULL , buffer , length) < 0 ) {
		xar_file_t i;
		if( parent ) {
			for( i = XAR_FILE(parent)->children; i && (XAR_FILE(i)->next != ret); i = XAR_FILE(i)->next );
		} else {
			for( i = XAR(x)->files; i && (XAR_FILE(i)->next != ret); i = XAR_FILE(i)->next );
		}
		if( i )
			XAR_FILE(i)->next = XAR_FILE(ret)->next;
		xar_file_free(ret);
		return NULL;
	}
	
	return ret;
}

xar_file_t xar_add_folder(xar_t x, xar_file_t f, const char *name, struct stat *info)
{
	xar_file_t ret;
	char idstr[32];

	if( info )
		memcpy(&XAR(x)->sbcache,info,sizeof(struct stat));
	
	ret = xar_file_new(f);
	if( !ret )
		return NULL;
	
	memset(idstr, 0, sizeof(idstr));
	snprintf(idstr, sizeof(idstr)-1, "%"PRIu64, ++XAR(x)->last_fileid);
	xar_attr_set(ret, NULL, "id", idstr);
	XAR_FILE(ret)->fspath = NULL;
	
	if( !f ) {
		XAR_FILE(ret)->parent = NULL;
		
		if( XAR(x)->files == NULL )
			XAR(x)->files = ret;
		else {
			XAR_FILE(ret)->next = XAR(x)->files;
			XAR(x)->files = ret;
		}
	}
	
	xar_prop_set(ret, "name", name);

	if( xar_arcmod_archive(x, ret, XAR_FILE(ret)->fspath, NULL, 0) < 0 ) {
		xar_file_t i;
		if( f ) {
			for( i = XAR_FILE(f)->children; i && (XAR_FILE(i)->next != ret); i = XAR_FILE(i)->next );
		} else {
			for( i = XAR(x)->files; i && (XAR_FILE(i)->next != ret); i = XAR_FILE(i)->next );
		}
		if( i )
			XAR_FILE(i)->next = XAR_FILE(ret)->next;
		xar_file_free(ret);
		return NULL;
	}
	
	return ret;	
}

xar_file_t xar_add_frompath(xar_t x, xar_file_t parent, const char *name, const char *realpath)
{
	return xar_add_node(x, parent, name , "" , realpath,  1);
}

xar_file_t xar_add_from_archive(xar_t x, xar_file_t parent, const char *name, xar_t sourcearchive, xar_file_t sourcefile)
{
	xar_file_t ret;
	char idstr[32];
		
	ret = xar_file_replicate(sourcefile, parent);
	
	if( !ret )
		return NULL;
	
	memset(idstr, 0, sizeof(idstr));
	snprintf(idstr, sizeof(idstr)-1, "%"PRIu64, ++XAR(x)->last_fileid);
	xar_attr_set(ret, NULL, "id", idstr);
	XAR_FILE(ret)->fspath = NULL;
	
	if( !parent ) {
		XAR_FILE(ret)->parent = NULL;
		
		if( XAR(x)->files == NULL )
			XAR(x)->files = ret;
		else {
			XAR_FILE(ret)->next = XAR(x)->files;
			XAR(x)->files = ret;
		}
	}
		
	xar_prop_set(ret, "name", name);
		
	/* iterate through all the properties, see if any of them have an offset */
	xar_prop_t p = xar_prop_pfirst(ret);

	do{
		xar_prop_t tmpp;
		
		tmpp = xar_prop_pget(p, "offset");
		if(tmpp) {
			if( 0 != xar_attrcopy_from_heap_to_heap(sourcearchive, sourcefile, p, x, ret)){			
				xar_file_free(ret);
				ret = NULL;
				break;
			}
		}
		
	}while( (p = xar_prop_pnext(p)) );
	
	return ret;	
}

/* xar_extract_tofile
* x: archive to extract from
* f: file associated with x
* Returns 0 on success, -1 on failure
* Summary: This actually does the file extraction.
* No traversal is performed, it is assumed all directory paths
* leading up to f already exist.
*/
int32_t xar_extract_tofile(xar_t x, xar_file_t f, const char *path) {
	return xar_arcmod_extract(x, f, path,NULL, 0);
}


/* xar_extract_tobuffer
* x: archive to extract from
* buffer: buffer to extract to
* Returns 0 on success, -1 on failure.
* Summary: This is the entry point for extraction to a buffer.
* On success, a buffer is allocated with the contents of the file
* specified.  The caller is responsible for freeing the returend buffer.
* Example: xar_extract_tobuffer(x, "foo/bar/blah",&buffer)
*/
int32_t xar_extract_tobuffer(xar_t x, xar_file_t f, char **buffer) {
	size_t size;

	return xar_extract_tobuffersz(x, f, buffer, &size);
}

/* xar_extract_tobuffer
* x: archive to extract from
* buffer: buffer to extract to
* size: On return, this will contain the size of the memory pointed to by buffer
* Returns 0 on success, -1 on failure.
* Summary: This is the entry point for extraction to a buffer.
* On success, a buffer is allocated with the contents of the file
* specified.  The caller is responsible for freeing the returend buffer.
* Example: xar_extract_tobuffer(x, "foo/bar/blah",&buffer)
*/
int32_t xar_extract_tobuffersz(xar_t x, xar_file_t f, char **buffer, size_t *size) {
	const char *sizestring = NULL;
	int32_t ret;
	
	if(0 != xar_prop_get(f,"data/size",&sizestring)){
		if(0 != xar_prop_get(f, "type", &sizestring))
			return -1;
		if(strcmp(sizestring, "file") == 0) {
			*size = 0;
			return 0;
		}
		return -1;
	}

	*size = strtoull(sizestring, (char **)NULL, 10);
	*buffer = malloc(*size);
	
	if(!(*buffer)){
		return -1;
	}
	
	ret = xar_arcmod_extract(x,f,NULL,*buffer,*size);
	if( ret ) {
		*size = 0;
		free(*buffer);
		*buffer = NULL;
	}

	return ret;
}

int32_t xar_extract_tostream_init(xar_t x, xar_file_t f, xar_stream *stream) {
	xar_prop_t tmpp;

	if( !xar_check_prop(x, "data") )
		return XAR_STREAM_OK;

	tmpp = xar_prop_pfirst(f);
	if( tmpp )
		tmpp = xar_prop_find(tmpp, "data");
	if( !tmpp )
		return XAR_STREAM_OK;

	return xar_attrcopy_from_heap_to_stream_init(x, f, tmpp, stream);
}

int32_t xar_extract_tostream(xar_stream *stream) {
	return xar_attrcopy_from_heap_to_stream(stream);
}

int32_t xar_extract_tostream_end(xar_stream *stream) {
	return xar_attrcopy_from_heap_to_stream_end(stream);
}

/* xar_extract
 * x: archive to extract from
 * path: path to file to extract
 * Returns 0 on success, -1 on failure.
 * Summary: This is the entry point for extraction.  This will find
 * the file node described by path, extract any directories needed
 * to extract the node, and finally extract the file.
 * Example: xar_extract(x, "foo/bar/blah")
 * If foo does not exist, xar_extract will extract foo from the
 * archive, extract bar from the archive, and then extract blah.
 * Total extractions will be "foo", "foo/bar", and "foo/bar/blah".
 */
int32_t xar_extract(xar_t x, xar_file_t f) {
	struct stat sb;
	char *tmp1, *dname;
	xar_file_t tmpf;
	
	if( (strstr(XAR_FILE(f)->fspath, "/") != NULL) && (stat(XAR_FILE(f)->fspath, &sb)) && (XAR_FILE(f)->parent_extracted == 0) ) {
		tmp1 = strdup(XAR_FILE(f)->fspath);
		dname = dirname(tmp1);
		tmpf = xar_file_find(XAR(x)->files, dname);
		if( !tmpf ) {
			xar_err_set_string(x, "Unable to find file");
			xar_err_callback(x, XAR_SEVERITY_NONFATAL, XAR_ERR_ARCHIVE_EXTRACTION);
			return -1;
		}
		free(tmp1);
		XAR_FILE(f)->parent_extracted++;
		xar_extract(x, tmpf);
	}
	
	return xar_extract_tofile(x, f, XAR_FILE(f)->fspath);
}

/* xar_verify
* x: archive to extract from
* f: file to verify
* Returns 0 on success, -1 on failure.
* Summary: This function allows for verification of
* an entry without extraction.  If there is no checksum
* the verification will pass.
*/
int32_t xar_verify(xar_t x, xar_file_t f) {
	return xar_arcmod_verify(x,f);
}

/* toc_read_callback
 * context: context passed through from the reader
 * buffer: buffer to read into
 * len: size of buffer
 * Returns: number of bytes read or -1 in case of error
 * Summary: internal callback for xmlReaderForIO.
 */
static int toc_read_callback(void *context, char *buffer, int len) {
	xar_t x = (xar_t)context;
	int ret, off = 0;

	if ( ((!XAR(x)->offset) || (XAR(x)->offset == XAR(x)->readbuf_len)) && (XAR(x)->toc_count != XAR(x)->header.toc_length_compressed) ) {
		XAR(x)->offset = 0;
		if( (XAR(x)->readbuf_len - off) + XAR(x)->toc_count > XAR(x)->header.toc_length_compressed )
			ret = xar_read_fd(XAR(x)->fd, XAR(x)->readbuf, XAR(x)->header.toc_length_compressed - XAR(x)->toc_count);
		else
			ret = read(XAR(x)->fd, XAR(x)->readbuf, XAR(x)->readbuf_len);
		if ( ret == -1 )
			return ret;
		
		if ( XAR(x)->toc_hash_ctx )
			xar_hash_update(XAR(x)->toc_hash_ctx, XAR(x)->readbuf, ret);

		XAR(x)->toc_count += ret;
		off += ret;
	}

	if( off && (off < XAR(x)->readbuf_len) )
		XAR(x)->readbuf_len = off;
	XAR(x)->zs.next_in = ((unsigned char *)XAR(x)->readbuf) + XAR(x)->offset;
	XAR(x)->zs.avail_in = XAR(x)->readbuf_len - XAR(x)->offset;
	XAR(x)->zs.next_out = (void *)buffer;
	XAR(x)->zs.avail_out = len;

	ret = inflate(&XAR(x)->zs, Z_SYNC_FLUSH);
	if( ret < 0 )
		return -1;

	XAR(x)->offset = XAR(x)->readbuf_len - XAR(x)->zs.avail_in;

	return len - XAR(x)->zs.avail_out;
}

/* close_callback
 * context: this will be a xar_t
 * Returns: 0 or -1 in case of error
 * Summary: this is the callback for xmlTextReaderForIO to close the IO
 */
static int close_callback(void *context) {
	return 0;
}

/* xar_serialize
 * x: xar to serialize
 * file: file to serialize to
 * Summary: serializes the archive out to xml.
 */
void xar_serialize(xar_t x, const char *file) {
	xmlTextWriterPtr writer;
	xar_subdoc_t i;

	writer = xmlNewTextWriterFilename(file, 0);
	xmlTextWriterStartDocument(writer, "1.0", "UTF-8", NULL);
	xmlTextWriterSetIndent(writer, 4);
	xmlTextWriterStartElement(writer, BAD_CAST("xar"));

	for( i = XAR(x)->subdocs; i; i = xar_subdoc_next(i) )
		xar_subdoc_serialize(i, writer, 1);

	xmlTextWriterStartElement(writer, BAD_CAST("toc"));
			
	if( XAR(x)->props )
		xar_prop_serialize(XAR(x)->props, writer);

	if( XAR(x)->signatures )
		xar_signature_serialize(XAR(x)->signatures,writer);

	if( XAR(x)->files )
		xar_file_serialize(XAR(x)->files, writer);

	xmlTextWriterEndDocument(writer);
	xmlFreeTextWriter(writer);
	return;
}

/* xar_unserialize
 * x: xar archive to unserialize to.  Must have been allocated with xar_open
 * file: the xml filename to unserialize from
 * Summary: Takes the TOC representation from file and creates the
 * corresponding in-memory representation.
 */
static int32_t xar_unserialize(xar_t x) {
	xmlTextReaderPtr reader;
	xar_file_t f = NULL;
	const xmlChar *name, *prefix, *uri;
	int type, noattr, ret;

	reader = xmlReaderForIO(toc_read_callback, close_callback, XAR(x), NULL, NULL, 0);
	if( !reader ) return -1;

	while( (ret = xmlTextReaderRead(reader)) == 1 ) {
		type = xmlTextReaderNodeType(reader);
		noattr = xmlTextReaderAttributeCount(reader);
		name = xmlTextReaderConstLocalName(reader);
		if( type != XML_READER_TYPE_ELEMENT )
			continue;
		if(strcmp((const char*)name, "xar") != 0)
			continue;
		while( (ret = xmlTextReaderRead(reader)) == 1 ) {
			type = xmlTextReaderNodeType(reader);
			noattr = xmlTextReaderAttributeCount(reader);
			name = xmlTextReaderConstLocalName(reader);
			if( type == XML_READER_TYPE_ELEMENT ) {
				if(strcmp((const char*)name, "toc") == 0) {
					while( (ret = xmlTextReaderRead(reader)) == 1 ) {
						type = xmlTextReaderNodeType(reader);
						noattr = xmlTextReaderAttributeCount(reader);
						name = xmlTextReaderConstLocalName(reader);
						if( type == XML_READER_TYPE_ELEMENT ) {
							if(strcmp((const char*)name, "file") == 0) {
								f = xar_file_unserialize(x, NULL, reader);
								XAR_FILE(f)->next = XAR(x)->files;
								XAR(x)->files = f;
							} else if( strcmp((const char*)name, "signature") == 0
#ifdef __APPLE__
                                      || strcmp((const char*)name, "x-signature") == 0
#endif
                                      ){
								xar_signature_t sig = NULL;			
								sig = xar_signature_unserialize(x, reader );
								
								if( !sig ) {
									xmlFreeTextReader(reader);
									return -1;
								}
								
								if( XAR(x)->signatures )
									XAR_SIGNATURE(XAR(x)->signatures)->next = XAR_SIGNATURE(sig);
								else
									XAR(x)->signatures = sig;
									
							} else {
								xar_prop_unserialize(XAR_FILE(x), NULL, reader);
							}
						}
					}
					if( ret == -1 ) {
						xmlFreeTextReader(reader);
						return -1;
					}
				} else {
					xar_subdoc_t s;
					int i;

					prefix = xmlTextReaderPrefix(reader);
					uri = xmlTextReaderNamespaceUri(reader);

					i = xmlTextReaderAttributeCount(reader);
					if( i > 0 ) {
						for(i = xmlTextReaderMoveToFirstAttribute(reader); i == 1; i = xmlTextReaderMoveToNextAttribute(reader)) {
							xar_attr_t a;
							const char *aname = (const char *)xmlTextReaderConstLocalName(reader);
							const char *avalue = (const char *)xmlTextReaderConstValue(reader);
							
							if( aname && (strcmp("subdoc_name", aname) == 0) ) {
								name = (const unsigned char *)avalue;
							} else {
								a = xar_attr_new();
								XAR_ATTR(a)->key = strdup(aname);
								XAR_ATTR(a)->value = strdup(avalue);
								XAR_ATTR(a)->next = XAR_SUBDOC(s)->attrs;
								XAR_SUBDOC(s)->attrs = XAR_ATTR(a);
							}
						}
					}

					s = xar_subdoc_new(x, (const char *)name);
                    if(s){
                        xar_subdoc_unserialize(s, reader);
                    }else{
                        xmlFreeTextReader(reader);
                        return -1;
                    }
				}
			}
			if( (type == XML_READER_TYPE_END_ELEMENT) && (strcmp((const char *)name, "toc")==0) ) {
				break;
			}
		}
		if( ret == -1 ) {
			xmlFreeTextReader(reader);
			return -1;
		}
	}

	if( ret == -1 ) {
		xmlFreeTextReader(reader);
		return -1;
	}
		
	xmlFreeTextReader(reader);
	return 0;
}

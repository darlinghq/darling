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


#include "config.h"
#ifndef HAVE_ASPRINTF
#include "asprintf.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#ifdef HAVE_LIBBZ2
#include <bzlib.h>
#endif
#include "xar.h"
#include "filetree.h"
#include "io.h"

#ifdef HAVE_LIBBZ2

struct _bzip_context{
	uint8_t		bzipcompressed;
	bz_stream	bz;
};


#define BZIP2_CONTEXT(x) ((struct _bzip_context *)(*x))
#endif

int xar_bzip_fromheap_done(xar_t x, xar_file_t f, xar_prop_t p, void **context) {
#ifdef HAVE_LIBBZ2

	if( !context || !BZIP2_CONTEXT(context) )
		return 0;

	if( BZIP2_CONTEXT(context)->bzipcompressed){
		BZ2_bzDecompressEnd(&BZIP2_CONTEXT(context)->bz);
	}
	
	/* free the context */
	free(BZIP2_CONTEXT(context));
	*context = NULL;
	
#endif /* HAVE_LIBBZ2 */
	return 0;
}

int xar_bzip_fromheap_in(xar_t x, xar_file_t f, xar_prop_t p, void **in, size_t *inlen, void **context) {
	const char *opt;
	xar_prop_t tmpp;
#ifdef HAVE_LIBBZ2
	void *out = NULL;
	size_t outlen, offset = 0;
	int r;

	/* on first run, we init the context and check the compression type */
	if( !BZIP2_CONTEXT(context) ) {
		*context = calloc(1,sizeof(struct _bzip_context));
		
		opt = NULL;
		tmpp = xar_prop_pget(p, "encoding");
		if( tmpp )
			opt = xar_attr_pget(f, tmpp, "style");
		if( !opt ) return 0;
		if( strcmp(opt, "application/x-bzip2") != 0 ) return 0;
		
		BZ2_bzDecompressInit(&BZIP2_CONTEXT(context)->bz, 0, 0);
		BZIP2_CONTEXT(context)->bzipcompressed = 1;
		if( *inlen == 0 )
			return 0;
	}else if( !(BZIP2_CONTEXT(context)->bzipcompressed) ){
		/* once the context has been initialized, then we have already
		   checked the compression type, so we need only check if we
		   actually are compressed */
		return 0;
	}

	outlen = *inlen;

	BZIP2_CONTEXT(context)->bz.next_in = *in;
	BZIP2_CONTEXT(context)->bz.avail_in = *inlen;
	BZIP2_CONTEXT(context)->bz.next_out = out;
	BZIP2_CONTEXT(context)->bz.avail_out = 0;

	while( BZIP2_CONTEXT(context)->bz.avail_in != 0 ) {
		outlen = outlen * 2;
		out = realloc(out, outlen);
		if( out == NULL ) abort();

		BZIP2_CONTEXT(context)->bz.next_out = ((char *)out) + offset;
		BZIP2_CONTEXT(context)->bz.avail_out = outlen - offset;

		r = BZ2_bzDecompress(&BZIP2_CONTEXT(context)->bz);
		if( (r != BZ_OK) && (r != BZ_STREAM_END) ) {
			xar_err_new(x);
			xar_err_set_file(x, f);
			xar_err_set_string(x, "Error decompressing file");
			xar_err_callback(x, XAR_SEVERITY_FATAL, XAR_ERR_ARCHIVE_EXTRACTION);
			return -1;
		}
		offset += outlen - offset - BZIP2_CONTEXT(context)->bz.avail_out;
		if( (r == BZ_STREAM_END) && (offset == 0) )
			break;
	}

	free(*in);
	*in = out;
	*inlen = offset;
#else
	opt = NULL;
	tmpp = xar_prop_pget(p, "encoding");
	if( tmpp )
		opt = xar_attr_pget(f, tmpp, "style");
	if( !opt ) return 0;
	if( strcmp(opt, "application/x-bzip2") != 0 ) return 0;
	xar_err_new(x);
	xar_err_set_file(x, f);
	xar_err_set_errno(x, 0);
	xar_err_set_string(x, "bzip2 support not compiled in.");
	xar_err_callback(x, XAR_SEVERITY_FATAL, XAR_ERR_ARCHIVE_EXTRACTION);

#endif /* HAVE_LIBBZ2 */
	return 0;
}

int xar_bzip_toheap_done(xar_t x, xar_file_t f, xar_prop_t p, void **context) {
#ifdef HAVE_LIBBZ2
	xar_prop_t tmpp;

	if( BZIP2_CONTEXT(context)->bzipcompressed){
		BZ2_bzCompressEnd(&BZIP2_CONTEXT(context)->bz);

		tmpp = xar_prop_pset(f, p, "encoding", NULL);
		if( tmpp )
			xar_attr_pset(f, tmpp, "style", "application/x-bzip2");
	}
	
	/* free the context */
	free(BZIP2_CONTEXT(context));
	*context = NULL;
	
#endif /* HAVE_LIBBZ2 */
	return 0;
}

int32_t xar_bzip_toheap_in(xar_t x, xar_file_t f, xar_prop_t p, void **in, size_t *inlen, void **context) {
	const char *opt;
#ifdef HAVE_LIBBZ2
	void *out = NULL;
	size_t outlen, offset = 0;
	int r;

	/* on first run, we init the context and check the compression type */
	if( !BZIP2_CONTEXT(context) ) {
		int level = 9;
		*context = calloc(1,sizeof(struct _bzip_context));
		
		opt = xar_opt_get(x, XAR_OPT_COMPRESSION);
		if( !opt )
			return 0;
		
		if( strcmp(opt, XAR_OPT_VAL_BZIP) != 0 )
			return 0;

		opt = xar_opt_get(x, XAR_OPT_COMPRESSIONARG);
		if( opt ) {
			int tmp;
			errno = 0;
			tmp = strtol(opt, NULL, 10);
			if( errno == 0 ) {
				if( (level >= 0) && (level <= 9) )
					level = tmp;
			}
		}
		
		BZ2_bzCompressInit(&BZIP2_CONTEXT(context)->bz, level, 0, 30);
		BZIP2_CONTEXT(context)->bzipcompressed = 1;
	}else if( !BZIP2_CONTEXT(context)->bzipcompressed ){
		/* once the context has been initialized, then we have already
		checked the compression type, so we need only check if we
		actually are compressed */
		return 0;
	}
	
	outlen = *inlen/2;
	if(outlen == 0) outlen = 1024;
	BZIP2_CONTEXT(context)->bz.next_in = *in;
	BZIP2_CONTEXT(context)->bz.avail_in = *inlen;
	BZIP2_CONTEXT(context)->bz.next_out = out;
	BZIP2_CONTEXT(context)->bz.avail_out = 0;

	if( *inlen != 0 ) {
		do {
			outlen *= 2;
			out = realloc(out, outlen);
			if( out == NULL ) abort();

			BZIP2_CONTEXT(context)->bz.next_out = ((char *)out) + offset;
			BZIP2_CONTEXT(context)->bz.avail_out = outlen - offset;

			r = BZ2_bzCompress(&BZIP2_CONTEXT(context)->bz, BZ_RUN);
			offset = outlen - BZIP2_CONTEXT(context)->bz.avail_out;
		} while( r == BZ_RUN_OK && BZIP2_CONTEXT(context)->bz.avail_in != 0 );
	} else {
		do {
			outlen *= 2;
			out = realloc(out, outlen);
			if( out == NULL ) abort();

			BZIP2_CONTEXT(context)->bz.next_out = ((char *)out) + offset;
			BZIP2_CONTEXT(context)->bz.avail_out = outlen - offset;

			r = BZ2_bzCompress(&BZIP2_CONTEXT(context)->bz, BZ_FINISH);
			offset = outlen - BZIP2_CONTEXT(context)->bz.avail_out;
		} while( (r == BZ_FINISH_OK) && (r != BZ_STREAM_END /* no-op */) );
	}

	if( (r != BZ_RUN_OK && r != BZ_STREAM_END && r != BZ_SEQUENCE_ERROR) ) {
		xar_err_new(x);
		xar_err_set_file(x, f);
		xar_err_set_string(x, "Error compressing file");
		xar_err_set_errno(x, r);
		xar_err_callback(x, XAR_SEVERITY_FATAL, XAR_ERR_ARCHIVE_CREATION);
		return -1;
	}
	
	free(*in);
	*in = out;
	*inlen = offset;
#else
	opt = xar_opt_get(x, XAR_OPT_COMPRESSION);
	if( !opt )
		return 0;
	if( strcmp(opt, XAR_OPT_VAL_BZIP) != 0 )
		return 0;
	xar_err_new(x);
	xar_err_set_file(x, f);
	xar_err_set_errno(x, 0);
	xar_err_set_string(x, "bzip2 support not compiled in.");
	xar_err_callback(x, XAR_SEVERITY_FATAL, XAR_ERR_ARCHIVE_CREATION);
#endif /* HAVE_LIBBZ2 */
	return 0;
}

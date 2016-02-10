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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <zlib.h>
#ifdef __APPLE__
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonDigestSPI.h>
#else
#include <openssl/evp.h>
#endif

#include "xar.h"
#include "hash.h"
#include "config.h"
#ifndef HAVE_ASPRINTF
#include "asprintf.h"
#endif


#pragma mark Hash Wrapper Object

#ifdef __APPLE__

CCDigestRef digestRef_from_name(const char* name, unsigned int *outHashSize) {
    CCDigestRef result = NULL;
    
    if (NULL != outHashSize)
        *outHashSize = 0;
    
	if (0 == strcasecmp(name, "sha512")) {
        result = CCDigestCreate(kCCDigestSHA512);
        if (NULL != outHashSize)
            *outHashSize = CC_SHA512_DIGEST_LENGTH;
    } else if (0 == strcasecmp(name, "sha256")) {
        result = CCDigestCreate(kCCDigestSHA256);
        if (NULL != outHashSize)
            *outHashSize = CC_SHA256_DIGEST_LENGTH;
    } else if (0 == strcasecmp(name, "sha") || !strcasecmp(name, "sha1")) {
        result = CCDigestCreate(kCCDigestSHA1);
        if (NULL != outHashSize)
            *outHashSize = CC_SHA1_DIGEST_LENGTH;
    } else if (0 == strcasecmp(name, "md5")) {
        result = CCDigestCreate(kCCDigestMD5);
        if (NULL != outHashSize)
            *outHashSize = CC_MD5_DIGEST_LENGTH;
    } else if (0 == strcasecmp(name, "md2")) {
        result = CCDigestCreate(kCCDigestMD2);
        if (NULL != outHashSize)
            *outHashSize = CC_MD2_DIGEST_LENGTH;
    }
    
    return result;
	
}
#endif // __APPLE__


struct __xar_hash_t {
	const char *digest_name;
	void *context;
#ifdef __APPLE__
	CCDigestRef digest;
#else
	EVP_MD_CTX digest;
	const EVP_MD *type;
#endif
	unsigned int length;
};

#define HASH_CTX(x) ((struct __xar_hash_t *)(x))

xar_hash_t xar_hash_new(const char *digest_name, void *context) {
	struct __xar_hash_t *hash = calloc(1, sizeof(struct __xar_hash_t));
	if( ! hash )
		return NULL; // errno will already be set
	
	if( context )
		HASH_CTX(hash)->context = context;
	
#ifdef __APPLE__
	HASH_CTX(hash)->digest = digestRef_from_name(digest_name, &HASH_CTX(hash)->length);
#else
	OpenSSL_add_all_digests();
	HASH_CTX(hash)->type = EVP_get_digestbyname(digest_name);
	EVP_DigestInit(&HASH_CTX(hash)->digest, HASH_CTX(hash)->type);
#endif
	
	HASH_CTX(hash)->digest_name = strdup(digest_name);
	
	return hash;
}

void *xar_hash_get_context(xar_hash_t hash) {
	return HASH_CTX(hash)->context;
}

const char *xar_hash_get_digest_name(xar_hash_t hash) {
	return HASH_CTX(hash)->digest_name;
}

void xar_hash_update(xar_hash_t hash, void *buffer, size_t nbyte) {
#ifdef __APPLE__
	CCDigestUpdate(HASH_CTX(hash)->digest, buffer, nbyte);
#else
	EVP_DigestUpdate(&HASH_CTX(hash)->digest, buffer, nbyte);
#endif
}

void *xar_hash_finish(xar_hash_t hash, size_t *nbyte) {
#ifdef __APPLE__
	void *buffer = calloc(1, CC_SHA512_DIGEST_LENGTH); // current biggest digest size  This is what OpenSSL uses
#else
	void *buffer = calloc(1, EVP_MAX_MD_SIZE);
#endif
	if( ! buffer )
		return NULL;
	
#ifdef __APPLE__
	CCDigestFinal(HASH_CTX(hash)->digest, buffer);
	CCDigestDestroy(HASH_CTX(hash)->digest);
#else
	EVP_DigestFinal(&HASH_CTX(hash)->digest, buffer, &HASH_CTX(hash)->length);
#endif
	
	*nbyte = HASH_CTX(hash)->length;
	free((void *)HASH_CTX(hash)->digest_name);
	free((void *)hash);
	return buffer;
}

#undef HASH_CTX


#pragma mark datamod

struct _hash_context {
	xar_hash_t archived;
	xar_hash_t unarchived;
	uint64_t count;
};

#define CONTEXT(x) ((struct _hash_context *)(*x))

static char *_xar_format_hash(const unsigned char* m,unsigned int len) {
	char *result = malloc((2*len)+1);
	char hexValue[3];
	unsigned int itr = 0;
	
	result[0] = '\0';
	
	for(itr = 0;itr < len;itr++) {
		sprintf(hexValue,"%02x",m[itr]);
		strncat(result,hexValue,2);
	}
	
	return result;
}

int32_t xar_hash_toheap_in(xar_t x, xar_file_t f, xar_prop_t p, void **in, size_t *inlen, void **context) {
	return xar_hash_fromheap_out(x,f,p,*in,*inlen,context);
}

int32_t xar_hash_fromheap_out(xar_t x, xar_file_t f, xar_prop_t p, void *in, size_t inlen, void **context) {
	const char *opt;
	xar_prop_t tmpp;

	opt = NULL;
	tmpp = xar_prop_pget(p, "extracted-checksum");
	if( tmpp ) {
		opt = xar_attr_pget(f, tmpp, "style");
	} else {
		// The xar-1.7 release in OS X Yosemite accidentally wrote <unarchived-checksum>
		// instead of <extracted-checksum>. Since archives like this are now in the wild,
		// we check for both.
		tmpp = xar_prop_pget(p, "unarchived-checksum");
		if( tmpp ) {
			opt = xar_attr_pget(f, tmpp, "style");
		}
	}

	// If there's an <archived-checksum> and no <extracted-checksum> (or
	// <unarchived-checksum>), the archive is malformed.
	if ( !opt && xar_prop_pget(p, "archived-checksum") ) {
		xar_err_new(x);
		xar_err_set_string(x, "No extracted-checksum");
		xar_err_callback(x, XAR_SEVERITY_FATAL, XAR_ERR_ARCHIVE_EXTRACTION);
		return -1;
	}

	if( !opt )
		opt = xar_opt_get(x, XAR_OPT_FILECKSUM);
    
	if( !opt || (0 == strcmp(opt, XAR_OPT_VAL_NONE) ) )
		return 0;
	
	if(!CONTEXT(context)) {
		*context = calloc(1, sizeof(struct _hash_context));
		if( ! *context )
			return -1;
	}
	
	if( ! CONTEXT(context)->unarchived ) {
		CONTEXT(context)->unarchived = xar_hash_new(opt, NULL);
		if( ! CONTEXT(context)->unarchived ) {
			free(*context);
			*context = NULL;
			return -1;
		}
	}
		
	if( inlen == 0 )
		return 0;
	
	CONTEXT(context)->count += inlen;
	xar_hash_update(CONTEXT(context)->unarchived, in, inlen);
	return 0;
}

int32_t xar_hash_fromheap_in(xar_t x, xar_file_t f, xar_prop_t p, void **in, size_t *inlen, void **context) {
	return xar_hash_toheap_out(x,f,p,*in,*inlen,context);
}

int32_t xar_hash_toheap_out(xar_t x, xar_file_t f, xar_prop_t p, void *in, size_t inlen, void **context) {
	const char *opt;
	xar_prop_t tmpp;
	
	opt = NULL;
	tmpp = xar_prop_pget(p, "archived-checksum");
	if( tmpp )
		opt = xar_attr_pget(f, tmpp, "style");
	
	if( !opt ) 	
		opt = xar_opt_get(x, XAR_OPT_FILECKSUM);
	
	if( !opt || (0 == strcmp(opt, XAR_OPT_VAL_NONE) ) )
		return 0;
		
	if( ! CONTEXT(context) ) {
		*context = calloc(1, sizeof(struct _hash_context));
		if( ! *context )
			return -1;
	}
	
	if( ! CONTEXT(context)->archived ) {
		CONTEXT(context)->archived = xar_hash_new(opt, NULL);
		if( ! CONTEXT(context)->archived ) {
			free(*context);
			*context = NULL;
			return -1;
		}
	}
	
	if( inlen == 0 )
		return 0;
	
	CONTEXT(context)->count += inlen;
	xar_hash_update(CONTEXT(context)->archived, in, inlen);
	return 0;
}

int32_t xar_hash_toheap_done(xar_t x, xar_file_t f, xar_prop_t p, void **context) {
	const char *archived_style = NULL, *unarchived_style = NULL;
	size_t archived_length = -1, unarchived_length = -1;
	void *archived_hash = NULL, *unarchived_hash = NULL;
	
	if( ! CONTEXT(context) )
		return 0;
	else if( CONTEXT(context)->count == 0 )
		goto DONE;
	
	archived_style = strdup(xar_hash_get_digest_name(CONTEXT(context)->archived));
	unarchived_style = strdup(xar_hash_get_digest_name(CONTEXT(context)->unarchived));
	
	archived_hash = xar_hash_finish(CONTEXT(context)->archived, &archived_length);
	unarchived_hash = xar_hash_finish(CONTEXT(context)->unarchived, &unarchived_length);
	
	char *str;
	xar_prop_t tmpp;

	str = _xar_format_hash(archived_hash, archived_length);
	if( f ) {
		tmpp = xar_prop_pset(f, p, "archived-checksum", str);
		if( tmpp )
			xar_attr_pset(f, tmpp, "style", archived_style);
	}
	free(str);
		
	str = _xar_format_hash(unarchived_hash, unarchived_length);
	if( f ) {
		tmpp = xar_prop_pset(f, p, "extracted-checksum", str);
		if( tmpp )
			xar_attr_pset(f, tmpp, "style", unarchived_style);
	}
	free(str);
	
DONE:
	free((void *)archived_style);
	free((void *)unarchived_style);
	
	free(archived_hash);
	free(unarchived_hash);
	
	free(*context);
	*context = NULL;

	return 0;
}

int32_t xar_hash_fromheap_done(xar_t x, xar_file_t f, xar_prop_t p, void **context) {
	if(!CONTEXT(context))
		return 0;
	
	int32_t result = 0;
    const char *archived_hash = NULL, *archived_style = NULL;
	
	// Fetch the existing hash from the archive
	if( CONTEXT(context)->archived ) {
		xar_prop_t tmpp = xar_prop_pget(p, "archived-checksum");
		if( tmpp ) {
			archived_style = xar_attr_pget(f, tmpp, "style");
			archived_hash = xar_prop_getvalue(tmpp);
		}
		
		// We have the fetched hash; now get the calculated hash
		if( archived_hash && archived_style ) {
			size_t calculated_length = -1;
			const char *calculated_style = strdup(xar_hash_get_digest_name(CONTEXT(context)->archived));
			void *calculated_buffer = xar_hash_finish(CONTEXT(context)->archived, &calculated_length);
			char *calculated_hash = _xar_format_hash(calculated_buffer, calculated_length);
			free(calculated_buffer);
			
			// Compare
			int hash_match = ( strcmp(archived_hash, calculated_hash) == 0 );
			int style_match = (strcmp(archived_style, calculated_style) == 0 );
			
			if( ! hash_match || ! style_match ) {
				xar_err_new(x);
				xar_err_set_file(x, f);
				xar_err_set_formatted_string(x, "archived-checksum %s's do not match", archived_style);
				xar_err_callback(x, XAR_SEVERITY_FATAL, XAR_ERR_ARCHIVE_EXTRACTION);
				result = -1;
			}
			
			free((void *)calculated_style);
			free(calculated_hash);
		}
	}
	
	// Clean up the unarchived hash as well, if we have one
	if( CONTEXT(context)->unarchived ) {
		size_t length = -1;
		void *hash = xar_hash_finish(CONTEXT(context)->unarchived, &length);
		free(hash);
	}

	if(*context) {
		free(*context);
		*context = NULL;
	}

	return result;
}

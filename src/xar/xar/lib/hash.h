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

#ifndef _XAR_HASH_H_
#define _XAR_HASH_H_

#include "filetree.h"

#pragma mark Hash Wrapper Object

typedef struct __xar_hash_t *xar_hash_t;

xar_hash_t xar_hash_new(const char *digest_name, void *context);
void *xar_hash_get_context(xar_hash_t hash);
const char *xar_hash_get_digest_name(xar_hash_t hash); // returns inner pointer
void xar_hash_update(xar_hash_t hash, void *buffer, size_t nbyte);
void *xar_hash_finish(xar_hash_t hash, size_t *nbyte);

#pragma mark datamod

int32_t xar_hash_fromheap_in(xar_t x, xar_file_t f, xar_prop_t p, void **in, size_t *inlen, void **context);
int32_t xar_hash_fromheap_out(xar_t x, xar_file_t f, xar_prop_t p, void *in, size_t inlen, void **context);
int32_t xar_hash_fromheap_done(xar_t x, xar_file_t f, xar_prop_t p, void **context);

int32_t xar_hash_toheap_in(xar_t x, xar_file_t f, xar_prop_t p, void **in, size_t *inlen, void **context);
int32_t xar_hash_toheap_out(xar_t x, xar_file_t f, xar_prop_t p, void *in, size_t inlen, void **context);
int32_t xar_hash_toheap_done(xar_t x, xar_file_t f, xar_prop_t p, void **context);

#endif /* _XAR_HASH_H_ */

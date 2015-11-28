/*
 * Copyright (c) 2006 Apple Computer, Inc.
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
 * 3. Neither the name of Apple nor the names of its contributors
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
 * 6-July-2006
 * DRI: Christopher Ryan <ryanc@apple.com>
*/

#ifndef _XAR_SIGNATURE_H_
#define _XAR_SIGNATURE_H_

#include "xar.h"

struct __xar_x509cert_t{
	uint8_t *content;
	int32_t	len;
	struct __xar_x509cert_t *next;
};

struct __xar_signature_t {
	char *type;
	int32_t	len;
	off_t  offset;
	int32_t x509cert_count;
	struct __xar_x509cert_t *x509certs;
	struct __xar_signature_t *next;
	xar_signer_callback signer_callback;		/* callback for signing */
	void	*callback_context;					/* context for callback */
	xar_t x;
#ifdef __APPLE__
    int is_extended;
#endif
};

#define XAR_SIGNATURE(x) ((struct __xar_signature_t *)(x))

#ifdef __APPLE__
xar_signature_t xar_signature_new_internal(xar_t x, int is_extended, const char *type, int32_t length, xar_signer_callback callback, void *callback_context);
#endif

int32_t xar_signature_serialize(xar_signature_t sig, xmlTextWriterPtr writer);
xar_signature_t xar_signature_unserialize(xar_t x, xmlTextReaderPtr reader);


/* deallocates the link list of xar signatures */
void xar_signature_remove(xar_signature_t sig);

#endif /* _XAR_SIGNATURE_H_ */

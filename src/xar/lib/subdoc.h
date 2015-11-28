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

#ifndef _XAR_SUBDOC_H_
#define _XAR_SUBDOC_H_

#include "xar.h"
#include "filetree.h"

struct __xar_subdoc_t {
	struct __xar_prop_t  *props;
	struct __xar_attr_t  *attrs;
	const char *prefix;
	const char *ns;
	const char *blank1; /* filler for xar_file_t compatibility */
	const char *blank2; /* filler for xar_file_t compatibility */
	const char blank3; /* filler for xar_file_t compatibility */
	const char *name;
	struct __xar_subdoc_t *next;
	const char *value; /* a subdoc should very rarely have a value */
	xar_t x;
};

#define XAR_SUBDOC(x) ((struct __xar_subdoc_t *)(x))

void xar_subdoc_unserialize(xar_subdoc_t s, xmlTextReaderPtr reader);
void xar_subdoc_serialize(xar_subdoc_t s, xmlTextWriterPtr writer, int wrap);
void xar_subdoc_free(xar_subdoc_t s);
xar_subdoc_t xar_subdoc_find(xar_t x, const char *name);

#endif /* _XAR_SUBDOC_H_ */

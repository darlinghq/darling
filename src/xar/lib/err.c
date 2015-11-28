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

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "xar.h"
#include "archive.h"

#define ECTX(x) ((struct errctx *)(x))

void xar_register_errhandler(xar_t x, err_handler callback, void *usrctx) {
	ECTX(&XAR(x)->errctx)->x = x;
	ECTX(&XAR(x)->errctx)->usrctx = usrctx;
	XAR(x)->ercallback = callback;
	return;
}

xar_t xar_err_get_archive(xar_errctx_t ctx) {
	return ECTX(ctx)->x;
}

xar_file_t xar_err_get_file(xar_errctx_t ctx) {
	return ECTX(ctx)->file;
}

void  xar_err_set_file(xar_t x, xar_file_t f) {
	XAR(x)->errctx.file = f;
	return;
}

const char *xar_err_get_string(xar_errctx_t ctx) {
	return ECTX(ctx)->str;
}

void xar_err_set_string(xar_t x, const char *str) {
	XAR(x)->errctx.str = strdup(str); // this leaks right now, but it's safer than the alternative
	return;
}

void xar_err_set_formatted_string(xar_t x, const char *format, ...) {
	va_list arg;
	char *msg;
	va_start(arg, format);
	vasprintf(&msg, format, arg);
	va_end(arg);
	xar_err_set_string(x, msg);
	free(msg);
}


int xar_err_get_errno(xar_errctx_t ctx) {
	return ECTX(ctx)->saved_errno;
}

void  xar_err_set_errno(xar_t x, int e) {
	XAR(x)->errctx.saved_errno = e;
	return;
}

void xar_err_new(xar_t x) {
	memset(&XAR(x)->errctx, 0, sizeof(struct errctx));
	XAR(x)->errctx.saved_errno = errno;
	return;
}

int32_t xar_err_callback(xar_t x, int32_t sev, int32_t err) {
	if( XAR(x)->ercallback )
		return XAR(x)->ercallback(sev, err, &XAR(x)->errctx, ECTX(&XAR(x)->errctx)->usrctx);
	return 0;
}

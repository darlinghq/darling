/* 
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
 * 3. Neither the name of Apple nor the names of any contributors
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
 * Portions Copyright 2006, Apple Computer, Inc.
 * Christopher Ryan <ryanc@apple.com>
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>

#include "config.h"
#ifndef HAVE_ASPRINTF
#include "asprintf.h"
#endif
#include "xar.h"
#include "filetree.h"
#include "arcmod.h"

struct _script_context{
	int initted;
};

#define SCRIPT_CONTEXT(x) ((struct _script_context*)(*x))

int32_t xar_script_in(xar_t x, xar_file_t f, xar_prop_t p, void **in, size_t *inlen, void **context) {
	char *buf = *in;
	xar_prop_t tmpp;

	if(!SCRIPT_CONTEXT(context)){
		*context = calloc(1,sizeof(struct _script_context));
	}

	if( SCRIPT_CONTEXT(context)->initted )
		return 0;

	if( !xar_check_prop(x, "contents") )
		return 0;

	/* Sanity check *inlen, which really shouldn't be more than a 
	 * few kbytes...
	 */
	if( *inlen > INT_MAX )
		return 0;

	/*We only run on the begining of the file, so once we init, we don't run again*/
	SCRIPT_CONTEXT(context)->initted = 1;
	
	if( (*inlen > 2) && (buf[0] == '#') && (buf[1] == '!') ) {
		char *exe;
		int i;

		exe = malloc(*inlen);
		if( !exe )
			return -1;
		memset(exe, 0, *inlen);
		
		for(i = 2; (i < *inlen) && (buf[i] != '\0') && (buf[i] != '\n') && (buf[i] != ' '); ++i) {
			exe[i-2] = buf[i];
		}

		tmpp = xar_prop_pset(f, p, "contents", NULL);
		if( tmpp ) {
			xar_prop_pset(f, tmpp, "type", "script");
			xar_prop_pset(f, tmpp, "interpreter", exe);
		}
		free(exe);
	}
	return 0;
}

int32_t xar_script_done(xar_t x, xar_file_t f, xar_prop_t p, void **context) {

	if(!SCRIPT_CONTEXT(context)){
		return 0;
	}
		
	if( *context ){
		free(*context);
		*context = NULL;
	}

	return 0;
}

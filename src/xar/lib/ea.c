/*
 * Copyright (c) 2008 Rob Braun
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
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>
 
#ifndef HAVE_ASPRINTF
#include "asprintf.h"
#endif
#include "xar.h"
#include "filetree.h"
#include "archive.h"
#include "b64.h"
#include "ea.h"

struct __xar_ea_t {
	const struct __xar_prop_t *prop;
	const struct __xar_ea_t *next;
};

#define XAR_EA(x) ((struct __xar_ea_t *)(x))

xar_ea_t xar_ea_new(xar_file_t f, const char *name)
{
	xar_ea_t ret;

	ret = calloc(sizeof(struct __xar_ea_t), 1);
	 if( !ret )
		return NULL;

	XAR_EA(ret)->prop = xar_prop_new(f, NULL);
	if( !XAR_EA(ret)->prop ) {
		free(ret);
		return NULL;
	}

	xar_prop_setkey(XAR_EA(ret)->prop, "ea");
	xar_prop_setvalue(XAR_EA(ret)->prop, NULL);
	XAR_PROP(XAR_EA(ret)->prop)->attrs = xar_attr_new();
	XAR_ATTR(XAR_PROP(XAR_EA(ret)->prop)->attrs)->key = strdup("id");
	asprintf((char **)&XAR_ATTR(XAR_PROP(XAR_EA(ret)->prop)->attrs)->value, "%lld", XAR_FILE(f)->nexteaid++);

	xar_prop_pset(f, XAR_EA(ret)->prop, "name", name);
	
	return ret;
}

int32_t xar_ea_pset(xar_file_t f, xar_ea_t e, const char *key, const char *value){
	if( xar_prop_pset(f, XAR_EA(e)->prop, key, value) )
		return 0;
	return -1;
}

int32_t xar_ea_pget(xar_ea_t e, const char *key, const char **value) {
	xar_prop_t r = xar_prop_find(XAR_EA(e)->prop, key);
	if( !r ) {
		if( value )
			*value = NULL;
		return -1;
	}

	if(value)
		*value = XAR_PROP(r)->value;

	return 0;
}

xar_prop_t xar_ea_root(xar_ea_t e) {
	return XAR_EA(e)->prop;
}

xar_prop_t xar_ea_find(xar_file_t f, const char *name)
{
	xar_prop_t p;

	for(p = xar_prop_pfirst(f); p; p = xar_prop_pnext(p)) {
		const char *tmp;
		xar_prop_t tmpp;

		tmp = xar_prop_getkey(p);
		if( strncmp(tmp, XAR_EA_FORK, strlen(XAR_EA_FORK)) != 0 )
			continue;
		if( strlen(tmp) != strlen(XAR_EA_FORK) )
			continue;

		tmpp = xar_prop_pget(p, "name");
		if( !tmpp )
			continue;
		tmp = xar_prop_getvalue(tmpp);
		if( !tmp )
			continue;

		if( strcmp(tmp, name) == 0 )
			return p;
	}

	return NULL;
}

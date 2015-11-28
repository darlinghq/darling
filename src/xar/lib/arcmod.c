/*
 * Copyright (c) 2007 Rob Braun
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

#include "arcmod.h"
#include "archive.h"
#include "stat.h"
#include "data.h"
#include "linuxattr.h"
#include "fbsdattr.h"
#include "darwinattr.h"
#include "ext2.h"
#include "xar.h"
#include <string.h>

struct arcmod xar_arcmods[] = {
	{ xar_stat_archive, xar_stat_extract },      /* must be first */
	{ xar_linuxattr_archive, xar_linuxattr_extract },
	{ xar_fbsdattr_archive, xar_fbsdattr_extract },
	{ xar_darwinattr_archive, xar_darwinattr_extract },
	{ xar_ext2attr_archive, xar_ext2attr_extract },
	{ xar_data_archive, xar_data_extract },
	/* Add new modules here */
	{ NULL, xar_set_perm },
	{ NULL, xar_flags_extract }
};

/* xar_arcmod_archive
 * x: archive to add the file to
 * f: node representing the file
 * file: the filesystem path to the file
 * Returns: 0 on success
 * Summary: This is the entry point to actual file archival.
 */
int32_t xar_arcmod_archive(xar_t x, xar_file_t f, const char *file, const char *buffer, size_t len) {
	int i;
	int32_t ret;
	for(i = 0; i < (sizeof(xar_arcmods)/sizeof(struct arcmod)); i++) {
		if( xar_arcmods[i].archive ) {
			ret = xar_arcmods[i].archive(x, f, file, buffer, len);
			if( ret < 0 ) {
				return ret;
			}
			if( ret > 0 ) {
				return 0;
			}
		}
	}
	return 0;
}

/* xar_arcmod_extract
 * x: archive to extract the file from
 * f: node representing the file
 * file: the filesystem path to the target file
 * Returns: 0 on success
 * Summary: This is the entry point to actual file archival.
 */
int32_t xar_arcmod_extract(xar_t x, xar_file_t f, const char *file, char *buffer, size_t len) {
	int i;
	int32_t ret;
	for(i = 0; i < (sizeof(xar_arcmods)/sizeof(struct arcmod)); i++) {
		if( xar_arcmods[i].extract ) {
			ret = xar_arcmods[i].extract(x, f, file, buffer, len);
			if( ret < 0 ) {
				return ret;
			}
			if( ret > 0 ) {
				return 0;
			}
		}
	}
	return 0;
}


int32_t xar_arcmod_verify(xar_t x, xar_file_t f){
	return xar_data_verify(x,f);
}

/* xar_check_prop
 * x: xar archive
 * name: name of property to check
 * Description: If XAR_OPT_PROPINCLUDE is set at all, only properties
 * specified for inclusion will be added.
 * If XAR_OPT_PROPINCLUDE is not set, and XAR_OPT_PROPEXCLUDE is set,
 * properies specified by XAR_OPT_PROPEXCLUDE will be omitted.
 * Returns: 0 for not to include, 1 for include.
 */
int32_t xar_check_prop(xar_t x, const char *name) {
	xar_attr_t i;
	char includeset = 0;

	for(i = XAR(x)->attrs; i; i = XAR_ATTR(i)->next) {
		if( strcmp(XAR_ATTR(i)->key, XAR_OPT_PROPINCLUDE) == 0 ) {
			if( strcmp(XAR_ATTR(i)->value, name) == 0 )
				return 1;
			includeset = 1;
		}
	}

	if( includeset )
		return 0;

	for(i = XAR(x)->attrs; i; i = XAR_ATTR(i)->next) {
		if( strcmp(XAR_ATTR(i)->key, XAR_OPT_PROPEXCLUDE) == 0 ) {
			if( strcmp(XAR_ATTR(i)->value, name) == 0 )
				return 0;
		}
	}

	return 1;
}

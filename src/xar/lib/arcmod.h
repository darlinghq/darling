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


#ifndef _XAR_ARCMOD_H_
#define _XAR_ARCMOD_H_
#include "xar.h"
#include "filetree.h"


typedef int32_t (*arcmod_archive)(xar_t x, xar_file_t f, const char* file, const char *buffer, size_t len);
typedef int32_t (*arcmod_extract)(xar_t x, xar_file_t f, const char* file, char *buffer, size_t len);

struct arcmod {
	arcmod_archive archive;
	arcmod_extract extract;
};

int32_t xar_arcmod_archive(xar_t x, xar_file_t f, const char *file, const char *buffer, size_t len);
int32_t xar_arcmod_extract(xar_t x, xar_file_t f, const char *file, char *buffer, size_t len);

int32_t xar_arcmod_verify(xar_t x, xar_file_t f);
int32_t xar_check_prop(xar_t x, const char *name);

#endif /* _XAR_ARCMOD_H_ */

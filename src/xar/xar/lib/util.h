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
 * Steven Cento <cento@apple.com>
*/

#ifndef _XAR_UTIL_H_
#define _XAR_UTIL_H_

#include <stdint.h>
#include "xar.h"


uint64_t xar_ntoh64(uint64_t num);
uint32_t xar_swap32(uint32_t num);
char *xar_get_path(xar_file_t f);
off_t	xar_get_heap_offset(xar_t x);
ssize_t xar_read_fd(int fd, void * buffer, size_t nbyte);
ssize_t xar_pread_fd(int fd, void * buffer, size_t nbyte, off_t offset);
ssize_t xar_write_fd(int fd, void * buffer, size_t nbyte);
ssize_t xar_pwrite_fd( int fd, void * buffer, size_t nbyte, off_t offset );
dev_t xar_makedev(uint32_t major, uint32_t minor);
void xar_devmake(dev_t dev, uint32_t *major, uint32_t *minor);
char* xar_safe_dirname(const char* path);

// This is used to check to see if a given path escapes from
// the extraction root.
int xar_path_issane(char* path);

// Returns a string containing the name of the next path component in path_to_advance. 
// Path to advance also gets moved forward to the start of the next component in the path.
// The returned string must be released by the caller.
char* xar_path_nextcomponent(char** path_to_advance);


#endif /* _XAR_UTIL_H_ */

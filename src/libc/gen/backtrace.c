/*
 * Copyright (c) 2007, 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <mach/vm_types.h>
#include <sys/uio.h>

#include <dlfcn.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stack_logging.h"
#include "execinfo.h"

int backtrace(void** buffer, int size) {
	extern void _thread_stack_pcs(vm_address_t *buffer, unsigned max, unsigned *nb, unsigned skip);
	unsigned int num_frames;
	_thread_stack_pcs((vm_address_t*)buffer, size, &num_frames, 1);
	while (num_frames >= 1 && buffer[num_frames-1] == NULL) num_frames -= 1;
	return num_frames;
}

#if __LP64__
#define _BACKTRACE_FORMAT "%-4d%-35s 0x%016lx %s + %lu"
#define _BACKTRACE_FORMAT_SIZE 83 /* %lu can take up to 20, does not include %s, includes NUL */
#define _BACKTRACE_ADDRESS_LEN 18 /* 0x + 16 (no NUL) */
#else
#define _BACKTRACE_FORMAT "%-4d%-35s 0x%08lx %s + %lu"
#define _BACKTRACE_FORMAT_SIZE 65 /* %lu can take up to 10, does not include %s, includes NUL */
#define _BACKTRACE_ADDRESS_LEN 10 /* 0x + 8 (no NUL) */
#endif

static int _backtrace_snprintf(char* buf, size_t size, int frame, const void* addr, const Dl_info* info) {
	char symbuf[_BACKTRACE_ADDRESS_LEN + 1];
	const char* image = "???";
	const char* symbol = "0x0";
	uintptr_t symbol_offset = 0;

	if (info->dli_fname) {
		const char *tmp = strrchr(info->dli_fname, '/');
		if(tmp == NULL)
			image = info->dli_fname;
		else
			image = tmp + 1;
	}
	
	if (info->dli_sname) {
		symbol = info->dli_sname;
		symbol_offset = (uintptr_t)addr - (uintptr_t)info->dli_saddr;
	} else if(info->dli_fname) {
		symbol = image;
		symbol_offset = (uintptr_t)addr - (uintptr_t)info->dli_fbase;
	} else if(0 < snprintf(symbuf, sizeof(symbuf), "0x%lx", (uintptr_t)info->dli_saddr)) {
		symbol = symbuf;
		symbol_offset = (uintptr_t)addr - (uintptr_t)info->dli_saddr;
	} else {
		symbol_offset = (uintptr_t)addr;
	}

	return snprintf(buf, size,
			_BACKTRACE_FORMAT,
			frame,
			image,
			(uintptr_t)addr,
			symbol,
			symbol_offset) + 1;
}

char** backtrace_symbols(void* const* buffer, int size) {
	int i;
	size_t total_bytes;
	char** result;
	char** ptrs;
	intptr_t strs, end;
	Dl_info* info = calloc(size, sizeof (Dl_info));
	
	if (info == NULL) return NULL;
	
	// Compute the total size for the block that is returned.
	// The block will contain size number of pointers to the
	// symbol descriptions.

	total_bytes = sizeof(char*) * size;
	
	// Plus each symbol description
	for (i = 0 ; i < size; ++i) {
		dladdr(buffer[i], &info[i]);
		total_bytes += _BACKTRACE_FORMAT_SIZE;
		if (info[i].dli_sname) {
			total_bytes += strlen(info[i].dli_sname);
		} else if(info[i].dli_fname) {
			const char *tmp = strrchr(info->dli_fname, '/');
			if(tmp == NULL)
				total_bytes += strlen(info->dli_fname);
	                else
				total_bytes += strlen(tmp + 1);
		} else {
			total_bytes += _BACKTRACE_ADDRESS_LEN;
		}
	}
	
	result = (char**)malloc(total_bytes);
	if (result == NULL) {
		free(info);
		return NULL;
	}
	end = (intptr_t)result + total_bytes;
	
	// Fill in the array of pointers and append the strings for
	// each symbol description.
	
	ptrs = result;
	strs = ((intptr_t)result) + sizeof(char*) * size;

	for (i = 0; i < size; ++i) {
		int chk = _backtrace_snprintf((char*)strs, end - (intptr_t)strs, i, buffer[i], &info[i]);

		if(chk < 0) {
			free(info);
			return NULL;
		}

		ptrs[i] = (char*)strs;
		strs += chk;
	}
	
	free(info);
	
	return result;
}

void backtrace_symbols_fd(void* const* buffer, int size, int fd) {
	int i;
	char buf[BUFSIZ];
	Dl_info info;
	struct iovec iov[2];

	iov[0].iov_base = buf;

	iov[1].iov_base = "\n";
	iov[1].iov_len = 1;

	for (i = 0; i < size; ++i) {
		memset(&info, 0, sizeof(info));
		dladdr(buffer[i], &info);

		iov[0].iov_len = _backtrace_snprintf(buf, sizeof(buf), i, buffer[i], &info);
		
		writev(fd, iov, 2);
	}
}

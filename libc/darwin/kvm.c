/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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

#include <stdio.h>
#include <string.h>

int
kvm_close(void* kd)
{
	return (0);
}

char**
kvm_getargv(void* kd, const void* p, int nchr)
{
	return (0);
}

char**
kvm_getenvv(void* kd, const void* p, int nchr)
{
	return (0);
}

char*
kvm_geterr(void* kd)
{
	return (0);
}

int
kvm_getloadavg(void* kd, double loadagv[], int nelem)
{
	return (-1);
}

char*
kvm_getfiles(void* kd, int op, int arg, int* cnt)
{
	if (cnt) *cnt = 0;
	return (0);
}

void*
kvm_getprocs(void* kd, int op, int arg, int* cnt)
{
	if (cnt) *cnt = 0;
	return (0);
}

int
kvm_nlist(void* kd, void* nl)
{
	return (-1);
}

void*
kvm_open(const char* execfile, const char* corefile, const char* swapfile, int flags, const char* errstr)
{
	fprintf(stderr, "%s%s/dev/mem: No such file or directory", errstr ? errstr : "", errstr ? ": " : "");
	return (0);
}

void*
kvm_openfiles(const char* execfile, const char* corefile, const char* swapfile, int flags, char* errout)
{
	if (errout) strcpy(errout, "/dev/mem: No such file or directory");
	return (0);
}

int
kvm_read(void* kd, unsigned long addr, void* buf, unsigned int nbytes)
{
	return (-1);
}

int
kvm_uread(void* kd, void* p, unsigned long uva, void* buf, size_t len)
{
	return (0);
}

int
kvm_write(void* kd, unsigned long addr, const void* buf, unsigned int nbytes)
{
	return (0);
}

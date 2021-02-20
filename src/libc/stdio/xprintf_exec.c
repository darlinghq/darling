/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

#define	__va_list	__darwin_va_list

#include <printf.h>
#include <stdarg.h>
#include <errno.h>
#include <local.h>
#include <xprintf_private.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpointer-bool-conversion"

int
asxprintf_exec(char ** __restrict ret,
    printf_comp_t __restrict pc, ...)
{
    int iret;
    va_list ap;

    if (!pc) {
	errno = EINVAL;
	return -1;
    }

    va_start(ap, pc);
    iret = _vasprintf(pc, NULL, ret, NULL, NULL, ap);
    va_end(ap);
    return iret;
}

int
dxprintf_exec(int fd, printf_comp_t __restrict pc, ...)
{
    int ret;
    va_list ap;

    if (!pc) {
	errno = EINVAL;
	return -1;
    }

    va_start(ap, pc);
    ret = _vdprintf(pc, NULL, fd, NULL, NULL, ap);
    va_end(ap);
    return ret;
}

int
fxprintf_exec(FILE * __restrict stream,
    printf_comp_t __restrict pc, ...)
{
    int ret;
    va_list ap;

    if (!pc) {
	errno = EINVAL;
	return -1;
    }

    va_start(ap, pc);
    ret = __xvprintf(pc, NULL, stream, NULL, NULL, ap);
    va_end(ap);
    return ret;
}

int
sxprintf_exec(char * __restrict str, size_t size,
    printf_comp_t __restrict pc, ...)
{
    int ret;
    va_list ap;

    if (!pc) {
	errno = EINVAL;
	return -1;
    }

    va_start(ap, pc);
    ret = _vsnprintf(pc, NULL, str, size, NULL, NULL, ap);
    va_end(ap);
    return ret;
}

int
xprintf_exec(printf_comp_t __restrict pc, ...)
{
    int ret;
    va_list ap;

    if (!pc) {
	errno = EINVAL;
	return -1;
    }

    va_start(ap, pc);
    ret = __xvprintf(pc, NULL, stdout, NULL, NULL, ap);
    va_end(ap);
    return ret;
}

int
vasxprintf_exec(char ** __restrict ret,
    printf_comp_t __restrict pc, va_list ap)
{
    if (!pc) {
	errno = EINVAL;
	return -1;
    }

    return _vasprintf(pc, NULL, ret, NULL, NULL, ap);
}

int
vdxprintf_exec(int fd, printf_comp_t __restrict pc,
    va_list ap) 
{
    if (!pc) {
	errno = EINVAL;
	return -1;
    }

    return _vdprintf(pc, NULL, fd, NULL, NULL, ap);
}

int
vfxprintf_exec(FILE * __restrict stream,
    printf_comp_t __restrict pc, va_list ap)
{
    if (!pc) {
	errno = EINVAL;
	return -1;
    }

    return __xvprintf(pc, NULL, stream, NULL, NULL, ap);
}

int
vsxprintf_exec(char * __restrict str, size_t size,
    printf_comp_t __restrict pc, va_list ap)
{
    if (!pc) {
	errno = EINVAL;
	return -1;
    }

    return _vsnprintf(pc, NULL, str, size, NULL, NULL, ap);
}

int
vxprintf_exec(printf_comp_t __restrict pc, va_list ap)
{
    if (!pc) {
	errno = EINVAL;
	return -1;
    }

    return __xvprintf(pc, NULL, stdout, NULL, NULL, ap);
}
#pragma clang diagnostic pop

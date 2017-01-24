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
#include <local.h>
#include <xprintf_private.h>

int
asxprintf(char ** __restrict ret, printf_domain_t __restrict domain,
    locale_t __restrict loc, const char * __restrict format, ...)
{
    int iret;
    va_list ap;

    va_start(ap, format);
    iret = _vasprintf(NULL, domain, ret, loc, format, ap);
    va_end(ap);
    return iret;
}

int
dxprintf(int fd, printf_domain_t __restrict domain, locale_t __restrict loc,
    const char * __restrict format, ...)
{
    int ret;
    va_list ap;

    va_start(ap, format);
    ret = _vdprintf(NULL, domain, fd, loc, format, ap);
    va_end(ap);
    return ret;
}

int
fxprintf(FILE * __restrict stream, printf_domain_t __restrict domain,
    locale_t __restrict loc, const char * __restrict format, ...)
{
    int ret;
    va_list ap;

    va_start(ap, format);
    ret = __xvprintf(NULL, domain, stream, loc, format, ap);
    va_end(ap);
    return ret;
}

int
sxprintf(char * __restrict str, size_t size, printf_domain_t __restrict domain,
    locale_t __restrict loc, const char * __restrict format, ...)
{
    int ret;
    va_list ap;

    va_start(ap, format);
    ret = _vsnprintf(NULL, domain, str, size, loc, format, ap);
    va_end(ap);
    return ret;
}

int
xprintf(printf_domain_t __restrict domain, locale_t __restrict loc,
    const char * __restrict format, ...)
{
    int ret;
    va_list ap;

    va_start(ap, format);
    ret = __xvprintf(NULL, domain, stdout, loc, format, ap);
    va_end(ap);
    return ret;
}

int
vasxprintf(char ** __restrict ret, printf_domain_t __restrict domain,
    locale_t __restrict loc, const char * __restrict format, va_list ap)
{
    return _vasprintf(NULL, domain, ret, loc, format, ap);
}

int
vdxprintf(int fd, printf_domain_t __restrict domain, locale_t __restrict loc,
    const char * __restrict format, va_list ap)
{
    return _vdprintf(NULL, domain, fd, loc, format, ap);
}

int
vfxprintf(FILE * __restrict stream, printf_domain_t __restrict domain,
    locale_t __restrict loc, const char * __restrict format, va_list ap)
{
    return __xvprintf(NULL, domain, stream, loc, format, ap);
}

int
vsxprintf(char * __restrict str, size_t size, printf_domain_t __restrict domain,
    locale_t __restrict loc, const char * __restrict format, va_list ap)
{
    return _vsnprintf(NULL, domain, str, size, loc, format, ap);
}

int
vxprintf(printf_domain_t __restrict domain, locale_t __restrict loc,
    const char * __restrict format, va_list ap)
{
    return __xvprintf(NULL, domain, stdout, loc, format, ap);
}

/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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



#ifndef __DYLD_LOGGING_H__
#define __DYLD_LOGGING_H__

#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#define VIS_HIDDEN __attribute__((visibility("hidden")))


namespace dyld3 {


bool log_loads(const char* format, ...)         __attribute__((format(printf, 1, 2))) VIS_HIDDEN;
bool log_apis(const char* format, ...)          __attribute__((format(printf, 1, 2))) VIS_HIDDEN;
bool log_segments(const char* format, ...)      __attribute__((format(printf, 1, 2))) VIS_HIDDEN;
bool log_initializers(const char* format, ...)  __attribute__((format(printf, 1, 2))) VIS_HIDDEN;
bool log_fixups(const char* format, ...)        __attribute__((format(printf, 1, 2))) VIS_HIDDEN;
bool log_notifications(const char* format, ...) __attribute__((format(printf, 1, 2))) VIS_HIDDEN;
bool log_dofs(const char* format, ...)        __attribute__((format(printf, 1, 2))) VIS_HIDDEN;

void halt(const char* message) __attribute((noreturn)) VIS_HIDDEN ;


// only called during libdyld set up
void setLoggingFromEnvs(const char* envp[]) VIS_HIDDEN ;
void setLoggingFunction(void (*func)(const char* format, va_list list)) VIS_HIDDEN;
void setHaltFunction(void (*func)(const char* message) __attribute__((noreturn))) VIS_HIDDEN;




} // namespace dyld3

#endif // __DYLD_LOGGING_H__



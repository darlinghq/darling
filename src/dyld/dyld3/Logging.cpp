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


#include <string.h>
#include <stdint.h>
#include <_simple.h>

#include "Logging.h"


namespace dyld3 {


static bool sVerboseLoading         = false;
static bool sVerboseInitializers    = false;
static bool sVerboseSegments        = false;
static bool sVerboseAPIs            = false;
static bool sVerboseNotifications   = false;
static bool sVerboseFixups          = false;
static bool sVerboseDOFs          = false;

static void vlog_default(const char* format, va_list list)
{
    _simple_vdprintf(2, format, list);
}

static void (*sLogFunction)(const char* format, va_list list) = &vlog_default;
static void (*sHaltFunction)(const char* message) __attribute__((noreturn)) = nullptr;

void halt(const char* message)
{
    (*sHaltFunction)(message);
}

static void vlog(const char* format, va_list list)
{
    (*sLogFunction)(format, list);
}

bool log_loads(const char* format, ...)
{
    if ( !sVerboseLoading )
        return false;
    va_list  list;
    va_start(list, format);
    vlog(format, list);
    va_end(list);
    return true;
}

bool log_segments(const char* format, ...)
{
    if ( !sVerboseSegments )
        return false;
    va_list  list;
    va_start(list, format);
    vlog(format, list);
    va_end(list);
    return true;
}

bool log_fixups(const char* format, ...)
{
    if ( !sVerboseFixups )
        return false;
    va_list    list;
    va_start(list, format);
    vlog(format, list);
    va_end(list);
    return true;
}

bool log_initializers(const char* format, ...)
{
    if ( !sVerboseInitializers )
        return false;
    va_list  list;
    va_start(list, format);
    vlog(format, list);
    va_end(list);
    return true;
}

bool log_apis(const char* format, ...)
{
    if ( !sVerboseAPIs )
        return false;
    va_list  list;
    va_start(list, format);
    vlog(format, list);
    va_end(list);
    return true;
}

bool log_notifications(const char* format, ...)
{
    if ( !sVerboseNotifications )
        return false;
    va_list  list;
    va_start(list, format);
    vlog(format, list);
    va_end(list);
    return true;
}

bool log_dofs(const char* format, ...)
{
    if ( !sVerboseDOFs )
        return false;
    va_list    list;
    va_start(list, format);
    vlog(format, list);
    va_end(list);
    return true;
}



void setLoggingFromEnvs(const char* envp[])
{
    for(const char** p = envp; *p != NULL; p++) {
        const char* keyEqualsValue = *p;
        const char* equals = strchr(keyEqualsValue, '=');
        if ( equals != NULL ) {
            //const char* value = &equals[1];
            const size_t keyLen = equals-keyEqualsValue;
            char key[keyLen+1];
            strncpy(key, keyEqualsValue, keyLen);
            key[keyLen] = '\0';
            if ( strcmp(key, "DYLD_PRINT_LIBRARIES") == 0 ) {
                sVerboseLoading = true;
            }
            else if ( strcmp(key, "DYLD_PRINT_SEGMENTS") == 0 ) {
                sVerboseSegments = true;
            }
            else if ( strcmp(key, "DYLD_PRINT_INITIALIZERS") == 0 ) {
                sVerboseInitializers = true;
            }
            else if ( strcmp(key, "DYLD_PRINT_APIS") == 0 ) {
                sVerboseAPIs = true;
            }
            else if ( strcmp(key, "DYLD_PRINT_NOTIFICATIONS") == 0 ) {
                sVerboseNotifications = true;
            }
            else if ( strcmp(key, "DYLD_PRINT_BINDINGS") == 0 ) {
                sVerboseFixups = true;
            }
            else if ( strcmp(key, "DYLD_PRINT_DOFS") == 0 ) {
                sVerboseDOFs = true;
            }
        }
    }
}

void setLoggingFunction(void (*func)(const char* format, va_list list))
{
    sLogFunction = func;
}

void setHaltFunction(void (*func)(const char* message) __attribute__((noreturn)))
{
    sHaltFunction = func;
}


} // namespace dyld3


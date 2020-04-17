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


#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <uuid/uuid.h>
#include <fcntl.h>
#include <errno.h>
#include <_simple.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dirent.h>
#include <mach/mach.h>
#include <mach/machine.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/fat.h>
#include <pthread.h>
#include <libc_private.h>

#include "Diagnostics.h"

#if BUILDING_CACHE_BUILDER
  #include <dispatch/dispatch.h>
  dispatch_queue_t sWarningQueue = dispatch_queue_create("com.apple.dyld.cache-builder.warnings", NULL);
#endif

Diagnostics::Diagnostics(bool verbose)
#if BUILDING_CACHE_BUILDER
    : _verbose(verbose)
    , _prefix("")
#endif
{
}

#if BUILDING_CACHE_BUILDER
Diagnostics::Diagnostics(const std::string& prefix, bool verbose)
    : _verbose(verbose)
    , _prefix(prefix)
{
}
#endif

Diagnostics::~Diagnostics()
{
    clearError();
}

void Diagnostics::error(const char* format, ...)
{
    va_list    list;
    va_start(list, format);
    error(format, list);
    va_end(list);
}

void Diagnostics::error(const char* format, va_list list)
{
    //FIXME: this should be assertNoError(), but we currently overwrite some errors
    //assertNoError();
    _buffer = _simple_salloc();
    _simple_vsprintf(_buffer, format, list);

#if BUILDING_CACHE_BUILDER
    if ( !_verbose )
        return;

    if (_prefix.empty()) {
        fprintf(stderr, "%s", _simple_string(_buffer));
    } else {
        fprintf(stderr, "[%s] %s", _prefix.c_str(), _simple_string(_buffer));
    }
#endif
}

bool Diagnostics::hasError() const
{
    return _buffer != nullptr;
}

bool Diagnostics::noError() const
{
    return _buffer == nullptr;
}

void Diagnostics::clearError()
{
    if ( _buffer )
        _simple_sfree(_buffer);
    _buffer = nullptr;
}

void Diagnostics::assertNoError() const
{
    if ( _buffer != nullptr )
        abort_report_np("%s", _simple_string(_buffer));
}

#if !BUILDING_CACHE_BUILDER
const char* Diagnostics::errorMessage() const
{
    return _simple_string(_buffer);
}

#else
void Diagnostics::warning(const char* format, ...)
{
    _SIMPLE_STRING tmp = _simple_salloc();
    va_list    list;
    va_start(list, format);
    _simple_vsprintf(tmp, format, list);
    va_end(list);
#if BUILDING_CACHE_BUILDER
    dispatch_sync(sWarningQueue, ^{
        _warnings.insert(_simple_string(tmp));
    });
#else
    _warnings.insert(_simple_string(tmp));
#endif
    _simple_sfree(tmp);
}

void Diagnostics::verbose(const char* format, ...)
{
    if ( !_verbose )
        return;

    char*   output_string;
    va_list list;
    va_start(list, format);
    vasprintf(&output_string, format, list);
    va_end(list);

    if (_prefix.empty()) {
        fprintf(stderr, "%s", output_string);
    } else {
        fprintf(stderr, "[%s] %s", _prefix.c_str(), output_string);
    }
    free(output_string);
}

const std::string Diagnostics::prefix() const
{
    return _prefix;
}

void Diagnostics::copy(const Diagnostics& other)
{
    if ( other.hasError() )
        error("%s", other.errorMessage().c_str());
    for (const std::string& warn : other.warnings())
        warning("%s", warn.c_str());
}

std::string Diagnostics::errorMessage() const
{
    if ( _buffer != nullptr )
        return _simple_string(_buffer);
    else
        return "";
}

const std::set<std::string> Diagnostics::warnings() const
{
#if BUILDING_CACHE_BUILDER
    __block std::set<std::string> retval;
    dispatch_sync(sWarningQueue, ^{
        retval = _warnings;
    });
    return retval;
#else
    return _warnings;
#endif
}

void Diagnostics::clearWarnings()
{
#if BUILDING_CACHE_BUILDER
    dispatch_sync(sWarningQueue, ^{
        _warnings.clear();
    });
#else
    _warnings.clear();
#endif
}

#endif


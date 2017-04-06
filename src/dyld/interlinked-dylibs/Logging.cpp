/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
 *
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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

#include <dispatch/dispatch.h>
#include <string.h>
#include <stdlib.h>
#include <set>

#include <assert.h>
#include "mega-dylib-utils.h"

#include "Logging.h"

#define MAX_LOG_STR_LEN (512)
//const char* kDispatchQueueLogNameKey = "kDispatchQueueLogNameKey";
const char* kDispatchWarningArrayKey = "kDispatchWarningArrayKey";

static dispatch_queue_t log_queue;
static dispatch_once_t  logQueueInit = 0;
static dispatch_queue_t unique_queue;
static dispatch_once_t  uniqueQueueInit = 0;

static uint32_t verbose = 0;
static bool returnNonZeroIfTerminateCalled = false;
static bool terminateCalled = false;

static const char* warningPrefix = "WARNING: ";
static const char* errorPrefix   = "ERROR: ";

LoggingContext::LoggingContext(const std::string& N)
    : _name(N)
    , _tainted(false)
{
}

LoggingContext::LoggingContext(const std::string& N, WarningTargets T)
    : _name(N)
    , _warnings(T)
    , _tainted(false)
{
}

void LoggingContext::taint()
{
    _tainted = true;
}

bool LoggingContext::isTainted()
{
    return _tainted;
}

const std::string& LoggingContext::name()
{
    return _name;
}

const WarningTargets& LoggingContext::targets()
{
    return _warnings;
}



pthread_key_t getLoggingContextKey(void) {
    static pthread_key_t   logContextKey;
    static dispatch_once_t logContextToken;
    dispatch_once(&logContextToken, ^{
        pthread_key_create(&logContextKey, nullptr);
    });
    return logContextKey;
}


void setLoggingContext(std::shared_ptr<LoggingContext>& context)
{
    pthread_setspecific(getLoggingContextKey(), (void*)&context);

    if (context && !context->name().empty()) {
        pthread_setname_np(context->name().substr(0, MAXTHREADNAMESIZE-1).c_str());
    }
}

std::shared_ptr<LoggingContext> getLoggingContext()
{
    if (void* val = pthread_getspecific(getLoggingContextKey()))
        return *((std::shared_ptr<LoggingContext>*)val);
    return nullptr;
}

void runBody(void* Ctx)
{
    std::unique_ptr<std::function<void(void)>>
    Body(reinterpret_cast<std::function<void(void)>*>(Ctx));
    (*Body)();
}

static dispatch_queue_t getLogQueue()
{
   dispatch_once(&logQueueInit, ^{
        log_queue = dispatch_queue_create("com.apple.dyld.cache.logging", DISPATCH_QUEUE_SERIAL);
    });
    return log_queue;
}

void setVerbose(bool level)
{
    verbose = level;
}

void setWarnAnErrorPrefixes(const char* warn, const char* err)
{
    warningPrefix = warn;
    errorPrefix   = err;
}

void setReturnNonZeroOnTerminate()
{
    returnNonZeroIfTerminateCalled = true;
}

void queued_print(FILE* __restrict fd, const char* str)
{
    const char* qstr = strdup(str);

    dispatch_async(getLogQueue(), ^{
        (void)fprintf(fd, "%s", qstr);
        free((void*)qstr);
    });
}

#define VLOG(header)                                                                 \
    va_list list;                                                                    \
    va_start(list, format);                                                          \
    char temp[MAX_LOG_STR_LEN];                                                      \
    vsprintf(temp, format, list);                                                    \
    auto ctx = getLoggingContext();                                                  \
    char temp2[MAX_LOG_STR_LEN];                                                     \
    if (ctx && !ctx->name().empty()) {                                               \
        snprintf(temp2, MAX_LOG_STR_LEN, "[%s] %s%s\n", ctx->name().c_str(), header, \
            temp);                                                                   \
    } else {                                                                         \
        snprintf(temp2, MAX_LOG_STR_LEN, "%s%s\n", header, temp);                    \
    }                                                                                \
    queued_print(stderr, temp2);                                                     \
    va_end(list);

void log(const char* __restrict format, ...)
{
    VLOG("");
}

void verboseLog(const char* format, ...)
{
    if (verbose) {
        VLOG("");
    }
}

static std::set<std::string> warnings;

void warning(const char* format, ...)
{
    dispatch_once(&uniqueQueueInit, ^{
        unique_queue = dispatch_queue_create("com.apple.dyld.cache.logging", DISPATCH_QUEUE_SERIAL);
    });

    va_list list;
    va_start(list, format);
    char temp[MAX_LOG_STR_LEN];
    vsprintf(temp, format, list);
    char* blockTemp = strdup(temp);

    auto ctx = getLoggingContext();
    if (ctx) {
        for (auto& target : ctx->targets().second) {
            ctx->targets().first->configurations[target.first].architectures[target.second].results.warnings.push_back(blockTemp);
        }
    }

    dispatch_sync(unique_queue, ^{
        if (warnings.count(blockTemp) == 0) {
            warnings.insert(blockTemp);
        }

        free(blockTemp);
    });

    va_end(list);
}

void terminate(const char* format, ...)
{
    VLOG(errorPrefix);

    terminateCalled = true;

    if (ctx) {
        // We are a work in a logging context, throw
        throw std::string(temp);
    } else {
        // We are in general handing, let the loggging queue darain and exit
        dispatch_sync(getLogQueue(), ^{
            for (auto& warning : warnings) {
                (void)fprintf(stderr, "%s%s\n", warningPrefix, warning.c_str());
            }
            if ( returnNonZeroIfTerminateCalled ) {
                exit(1);
            }
            else {
                time_t endtime = time(0);
                (void)fprintf(stderr, "Finished: %s\n", asctime(localtime(&endtime)));
                (void)fprintf(stderr, "Exiting\n");
                exit(0);
            }
        });
    }

    // clang can't reason out that we won't hit this due to the dispatch_sync in the exit path
    __builtin_unreachable();
}

void dumpLogAndExit(bool logFinishTime)
{
    dispatch_async(getLogQueue(), ^{
        for (auto& warning : warnings) {
            (void)fprintf(stderr, "%s%s\n", warningPrefix, warning.c_str());
        }
        if ( logFinishTime ) {
            time_t endtime = time(0);
            (void)fprintf(stderr, "Finished: %s\n", asctime(localtime(&endtime)));
            (void)fprintf(stderr, "Exiting\n");
        }
        exit(returnNonZeroIfTerminateCalled && terminateCalled ? 1 : 0);
    });
}

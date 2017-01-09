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

#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <pthread.h>
#include "mega-dylib-utils.h"

void verboseLog(const char* format, ...) __printflike(1, 2);
void log(const char* __restrict format, ...) __printflike(1, 2);
void alwaysLog(const char* __restrict format, ...) __printflike(1, 2);

void warning(const char* format, ...) __printflike(1, 2);
void terminate(const char* format, ...) __printflike(1, 2) __attribute__((noreturn));
void dumpLogAndExit(bool logFinishTime=true);

void setVerbose(bool level);
void setReturnNonZeroOnTerminate();
void setWarnAnErrorPrefixes(const char* warn, const char* err);

struct LoggingContext {
    LoggingContext(const std::string& N);
    LoggingContext(const std::string& N, WarningTargets T);
    void                  taint();
    bool                  isTainted();
    const std::string&    name();
    const WarningTargets& targets();

private:
    const std::string    _name;
    const WarningTargets _warnings;
    bool                 _tainted;
};

void setLoggingContext(std::shared_ptr<LoggingContext>& context);
std::shared_ptr<LoggingContext> getLoggingContext();

/* Okay, so this gets tricky
 * Naively, what we are doing is stashing some information in pthread specific
 * variables so that the logging system can pick it up and tag messages with it,
 * but without us having to track it all through the entire cache builder code base.
 * Additionally, we use the presence of that information to determine if we are in
 * the root logging context (where terminate() calls are fatal), or a thread context
 * (where they should just throw so the thread fails, logs an error, and cleans up its
 * state.
 *
 * The problem is that we need that context to follow our blocks when we switch threads.
 * We achieve that by wrapping dispatch_(a)sync with our own calls that setup try{} blocks
 * around the executing lambda, that way it is always safe to throw in a named context
 * name. We also use those wrappers to copy the context between the threads using
 * the closures as glue. Finally, we set a taint variable that can back propgate to stop
 * the execution of any furthur blocks related to a context that has thrown.
 *
 * This is exposed in the header because we need it for the templates to work, but aside from
 * cacheBuilderDispatchAsync() and friends nothing here should be used directly.
 */

void runBody(void* Ctx);

pthread_key_t getLoggingContextKey(void);

template <typename BodyFtor>
std::function<void(void)>* heapSafe(BodyFtor&& Body, std::shared_ptr<LoggingContext> context)
{
    auto retval = new std::function<void(void)>([ B = std::move(Body), context ]() mutable {
        if (!context || !context->isTainted()) {

            void* oldCtx = pthread_getspecific(getLoggingContextKey());
            setLoggingContext(context);
            try {
                B();
            } catch (std::string exception) {
                WarningTargets warningTargets = context->targets();
                for (auto& target : warningTargets.second) {
                    warningTargets.first->configurations[target.first].architectures[target.second].results.failure = exception;
                }
                if (context) {
                    context->taint();
                }
            } catch (...) {
                if (context) {
                    context->taint();
                }
            }
            pthread_setspecific(getLoggingContextKey(), oldCtx);
        }
    });
    return retval;
}

template <typename BodyFtor>
void cacheBuilderDispatchAsync(dispatch_queue_t queue, BodyFtor&& Body)
{
    dispatch_async_f(queue, heapSafe(Body, getLoggingContext()), runBody);
}

template <typename BodyFtor>
void cacheBuilderDispatchGroupAsync(dispatch_group_t group, dispatch_queue_t queue, BodyFtor&& Body)
{
    dispatch_group_async_f(group, queue, heapSafe(Body, getLoggingContext()), runBody);
}

template <typename BodyFtor>
void cacheBuilderDispatchSync(dispatch_queue_t queue, BodyFtor&& Body)
{
    dispatch_sync_f(queue, heapSafe(Body, getLoggingContext()), runBody);
}

#endif /* LOGGING_H */

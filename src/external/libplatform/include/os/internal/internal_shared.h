/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __OS_INTERNAL_SHARED__
#define __OS_INTERNAL_SHARED__

#ifndef __OS_EXPOSE_INTERNALS__
/*
 * XXX                           /!\ WARNING /!\                           XXX
 *
 * This header file describes INTERNAL interfaces to libplatform used by other
 * libsystem targets, which are subject to change in future releases of Mac
 * OS X and iOS. Any applications relying on these interfaces WILL break.
 *
 * If you are not a libsystem target, you should NOT EVER use these headers.
 * Not even a little.
 *
 * XXX                           /!\ WARNING /!\                           XXX
 */
#error "these internals are not for general use outside of libsystem"
#else

#ifndef __OS_EXPOSE_INTERNALS_INDIRECT__
#define __OS_EXPOSE_INTERNALS_INDIRECT__
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#if defined(__arm__) || defined(__arm64__)
#include <arm/arch.h>
#endif

#include <os/base.h>
#include <os/base_private.h>
#include <os/internal/atomic.h>
#include <os/internal/crashlog.h>

#endif // __OS_EXPOSE_INTERNALS__

#endif // __OS_INTERNAL_SHARED__

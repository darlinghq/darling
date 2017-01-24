/*
 * Copyright (c) 2008-2013 Apple Inc. All rights reserved.
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

#ifndef __OS_BASE_PRIVATE__
#define __OS_BASE_PRIVATE__

#include <os/base.h>

#ifndef os_fastpath
#define os_fastpath(x) ((__typeof__(x))OS_EXPECT((long)(x), ~0l))
#endif
#ifndef os_slowpath
#define os_slowpath(x) ((__typeof__(x))OS_EXPECT((long)(x), 0l))
#endif
#ifndef os_likely
#define os_likely(x) OS_EXPECT(!!(x), 1)
#endif
#ifndef os_unlikely
#define os_unlikely(x) OS_EXPECT(!!(x), 0)
#endif

#endif // __OS_BASE_PRIVATE__

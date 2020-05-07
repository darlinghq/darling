/*
 * Copyright (c) 2015-2016 Apple Inc. All rights reserved.
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

#ifndef __FIREHOSE_IOCTL_PRIVATE__
#define __FIREHOSE_IOCTL_PRIVATE__

#include <sys/ioccom.h>
#include "firehose_types_private.h"

// Ioctls implemented by the oslog dev node

/* Flushed the log data. Return the updated pointers */
#ifndef LOGFLUSHED
#define LOGFLUSHED      _IOW('t', 81, firehose_push_reply_t)
#endif

/* <Reserved for later. Unused now> Map the kernel log buffers to logd's address space */
#ifndef LOGREGISTER
#define LOGREGISTER _IOR('t', 80, int)
#endif

/* Map the kernel log buffers to logd's address space */
#ifndef LOGBUFFERMAP
#define LOGBUFFERMAP _IOR('t', 79, firehose_buffer_map_info_t)
#endif

#endif // __FIREHOSE_IOCTL_PRIVATE__

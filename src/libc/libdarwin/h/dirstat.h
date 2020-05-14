/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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

#ifndef __DIRSTAT_H__
#define __DIRSTAT_H__

#include <Availability.h>
#include <sys/types.h>
#include <stdint.h>

__BEGIN_DECLS

/* Fail if the file system does not support fast directory sizing on the provided directory */
#define DIRSTAT_FAST_ONLY 0x1

/* Do not use filesystem support for directory sizing. */
#define DIRSTAT_FORCE_FALLBACK 0x2

/*
 * NOT ABI: the size of this structure may grow.  You must pass the current
 * size of this structure in as the dirstat_size argument to the functions
 * below.
 */
struct dirstat {
	off_t total_size;
	uint64_t descendants;
};

/* Returns -1 on error and sets errno.  Does not cross file-system boundaries. */
__API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
int dirstat_np(const char *path, int flags, struct dirstat *ds, size_t dirstat_size);

__API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
int dirstatat_np(int dfd, const char *path, int flags, struct dirstat *ds, size_t dirstat_size);

__END_DECLS

#endif // __DIRSTAT_H__

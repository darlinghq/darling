/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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

#ifndef __REBOOT2_H__
#define __REBOOT2_H__

#include <sys/cdefs.h>
#include <sys/reboot.h>
#include <stdint.h>

__BEGIN_DECLS

#define RB2_FULLREBOOT	0x8000000000000000llu

/* Returns NULL on success. Not NULL on failure */

__attribute__((visibility("default")))
void *reboot2(uint64_t flags);

__END_DECLS

#endif /* __REBOOT2_H__ */

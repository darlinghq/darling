/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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

#ifndef __LAUNCHD_H__
#define __LAUNCHD_H__

#include <mach/mach.h>
#include <mach/port.h>
#include "launch.h"
#include "bootstrap.h"
#include "runtime.h"

struct kevent;
struct conncb;

extern bool pid1_magic;
extern bool launchd_shutting_down;
extern bool fake_launchd_shutting_down;
extern bool network_up;
extern FILE *launchd_console;
extern uid_t launchd_uid;

void launchd_SessionCreate(void);
void launchd_shutdown(void);

enum {
	LAUNCHD_PERSISTENT_STORE_DB,
	LAUNCHD_PERSISTENT_STORE_LOGS,
};
char *launchd_copy_persistent_store(int type, const char *file);

int _fd(int fd);

#endif /* __LAUNCHD_H__ */

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

#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>

#include "kill2.h"

int
kill2(pid_t pid, int sig)
{
	/*
	 * POSIX defines consistency over correctness, and consequently
	 * kill/killpg now returns EPERM instead of ESRCH.
	 *
	 * I've filed 5487498 to get a non-portable kill().
	 * We'll regretfully take advantage of implementation details for now.
	 */
	return syscall(SYS_kill, pid, sig, 0);
}

int
killpg2(pid_t pgrp, int sig)
{
	return kill2(-pgrp, sig);
}

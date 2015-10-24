/*
 * Copyright (c) 2011 Mark Heily.
 * Copyright (c) 2008-2009 Apple Inc. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <dispatch/dispatch.h>

void
work(void *context __attribute__((unused)))
{
    puts("work complete");
	exit(0);
}

int main(void) {
	dispatch_queue_t q = dispatch_get_main_queue();
	dispatch_sync_f(dispatch_get_main_queue(), NULL, work);
	dispatch_main();
	return 0;
}

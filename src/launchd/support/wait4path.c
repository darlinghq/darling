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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int kq = kqueue();
	struct kevent kev;
	struct stat sb;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <object on mount point>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	EV_SET(&kev, 0, EVFILT_FS, EV_ADD, 0, 0, 0);

	if (kevent(kq, &kev, 1, NULL, 0, NULL) == -1) {
		fprintf(stderr, "adding EVFILT_FS to kqueue failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (stat(argv[1], &sb) == 0) {
		exit(EXIT_SUCCESS);
	}

	for (;;) {
		kevent(kq, NULL, 0, &kev, 1, NULL);
		if (stat(argv[1], &sb) == 0) {
			break;
		}
	}

	exit(EXIT_SUCCESS);
}

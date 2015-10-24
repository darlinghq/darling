/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

#include <stdlib.h>
#include <string.h>
#include <xpc/private.h>
#include <sys/stat.h>
#include <TargetConditionals.h>
#include "configuration_profile.h"

#define NOTIFY_PATH_SERVICE "com.apple.system.notify.service.path:0x87:"
#define CPROF_PATH "/Library/Managed Preferences/mobile"

char *
configuration_profile_create_notification_key(const char *ident)
{
	char *out = NULL;

	if (ident == NULL) return NULL;

	if (ident[0] == '/')
	{
		asprintf(&out, "%s%s", NOTIFY_PATH_SERVICE, ident);
		return out;
	}

#if TARGET_OS_EMBEDDED
	if (strchr(ident + 1, '/') != NULL) return NULL;
	asprintf(&out, "%s%s/%s.plist", NOTIFY_PATH_SERVICE, CPROF_PATH, ident);
#endif

	return out;
}

xpc_object_t
configuration_profile_copy_property_list(const char *ident)
{
	char path[MAXPATHLEN];
	void *data;
	int fd;
	struct stat sb;
	xpc_object_t out = NULL;

	if (ident == NULL) return NULL;

	path[0] = '\0';
	if (ident[0] == '/')
	{
		snprintf(path, sizeof(path), "%s", ident);
	}
#if TARGET_OS_EMBEDDED
	else
	{
		if (strchr(ident + 1, '/') != NULL) return NULL;
		snprintf(path, sizeof(path), "%s/%s.plist", CPROF_PATH, ident);
	}
#endif

	if (path[0] == '\0') return NULL;

	fd = open(path, O_RDONLY, 0);
	if (fd < 0) return NULL;

	memset(&sb, 0, sizeof(struct stat));
	if (fstat(fd, &sb) < 0)
	{
		close(fd);
		return NULL;
	}

	data = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (data != NULL) out = xpc_create_from_plist(data, sb.st_size);

	munmap(data, sb.st_size);
	close(fd);

	return out;
}

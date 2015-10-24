/*
 * Copyright (c) 2006, 2007, 2010 Apple Inc. All rights reserved.
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

#ifndef _DIRHELPER_PRIV_H_
#define _DIRHELPER_PRIV_H_

#include <sys/cdefs.h>
#include <sys/types.h>

#define VAR_FOLDERS_PATH		"/var/folders/"

#define DIRHELPER_BOOTSTRAP_NAME	"com.apple.bsd.dirhelper"
#define DIRHELPER_CACHE_STR		"C/"
#define DIRHELPER_TEMP_STR		"T/"
#define DIRHELPER_TOP_STR		"0/"
#define DIRHELPER_ENV_USER_DIR_SUFFIX	"DIRHELPER_USER_DIR_SUFFIX"

typedef enum {
    DIRHELPER_USER_LOCAL = 0,
    DIRHELPER_USER_LOCAL_TEMP,
    DIRHELPER_USER_LOCAL_CACHE,
    DIRHELPER_USER_LOCAL_LAST = DIRHELPER_USER_LOCAL_CACHE
} dirhelper_which_t;

__BEGIN_DECLS
char *__user_local_dirname(uid_t uid, dirhelper_which_t which, char *path,
	size_t pathlen);
char *__user_local_mkdir_p(char *path);
__END_DECLS

#endif /* _DIRHELPER_PRIV_H_ */

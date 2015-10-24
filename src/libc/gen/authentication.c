/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
#include <sys/param.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <err.h>
#include <errno.h>
#include <grp.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "authentication.h"

int isAuthenticatedAsAdministrator(void)
{
    if (isAuthenticatedAsRoot()) {
        return 1;
    }
    // otherwise ...
    return isAuthenticatedAsAdministratorForTask(0);
}

int isAuthenticatedAsAdministratorForTask(int taskNum)
{
    int admin = 0;
    uid_t ruid;

    if (isAuthenticatedAsRoot()) {
        return 1;
    }

    ruid = getuid();

    if (ruid) {
            gid_t groups[NGROUPS_MAX];
            int   numgroups;

            /*
             * Only allow those in group taskNum group (By default admin) to authenticate.
             */
            if ((numgroups = getgroups(NGROUPS_MAX, groups)) > 0) {
                    int i;
                    gid_t admingid = 0;
                    struct group *admingroup;

                    if ((admingroup = getgrnam(groupNameForTask(taskNum))) != NULL) {
                            admingid = admingroup->gr_gid;

                            for (i = 0; i < numgroups; i++) {
                                    if (groups[i] == admingid) {
                                            admin = 1;
                                            break;
                                    }
                            }
                    }

            }
    }
    // otherwise
    return admin;
}

int isAuthenticatedAsRoot(void)
{
    if (getuid() == 0) {
        return 1;
    }
    return 0;
}

char *groupNameForTask(int taskNum)
{
    if (taskNum == 0)
        return "admin";

    return "admin";
}


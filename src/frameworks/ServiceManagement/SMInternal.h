/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// Talking to launchd, shared by SMAppService (macOS 13 API) and the legacy SMJob* / SMJobBless calls.

#ifndef SM_INTERNAL_H
#define SM_INTERNAL_H

#import <Foundation/Foundation.h>
#include <launch.h>

// Returns NULL for values a launchd job dictionary cannot hold.
launch_data_t SMLaunchDataFromObject(id object);

// Converts a launchd reply into Foundation objects (nil for types without a counterpart).
id SMObjectFromLaunchData(launch_data_t data);

// Sends { key: value } to launchd (taking ownership of value), or the bare string key when value is NULL
// (how a GetJobs request is made), and returns the reply's errno, or -1 with
// *ipcErrno set when launchd could not be reached. A dictionary reply (GetJob) counts as success and is
// handed to the caller in *replyOut (to be freed with launch_data_free) when replyOut is not NULL.
int SMLaunchRequestWithReply(const char *key, launch_data_t value, int *ipcErrno, launch_data_t *replyOut);
int SMLaunchRequest(const char *key, launch_data_t value, int *ipcErrno);

#endif

/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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

#ifndef _NetFS_H_
#define _NetFS_H_

#include <sys/types.h>

void* GetCompleteMountURL(void);
void* GetURLFromURLRemountInfo(void);
void* GetURLRemountInfo(void);
void* GetURLRemountInfoSize(void);
void* MountCompleteURL(void);
void* MountServerURL(void);
void* MountURLWithAuthentication(void);
void* NetFSCFStringtoCString(void);
void* NetFSClearAllURLApprovals(void);
void* NetFSCopyHostAndPort(void);
void* NetFSCopyNormalizedURLString(void);
void* NetFSCopyURLForRemountingVolume(void);
void* NetFSGetMountInfo(void);
void* NetFSGetMountInfoByFSID(void);
void* NetFSGetSupportedSchemes(void);
void* NetFSGetURLApprovalState(void);
void* NetFSInterface_AddRef(void);
void* NetFSInterface_Release(void);
void* NetFSLogToMessageTracer(void);
void* NetFSMountURLAsync(void);
void* NetFSMountURLCancel(void);
void* NetFSMountURLProbe(void);
void* NetFSMountURLSync(void);
void* NetFSMountURLWithAuthenticationSync(void);
void* NetFSQueryInterface(void);
void* NetFSSetURLApprovalState(void);
void* NetFS_CreateInterface(void);
void* PremountHomeDirectoryWithAuthentication(void);
void* RemountInfoQueryInterface(void);
void* UnmountServerURL(void);
void* netfs_Cancel(void);
void* netfs_CancelAsyncMount(void);
void* netfs_CloseSession(void);
void* netfs_ConnectToServerAsync(void);
void* netfs_ConnectToServerSync(void);
void* netfs_CreateSessionRef(void);
void* netfs_CreateURL(void);
void* netfs_EnumerateShares(void);
void* netfs_GetServerInfo(void);
void* netfs_Mount(void);
void* netfs_MountURLWithAuthenticationAsync(void);
void* netfs_MountURLWithAuthenticationSync(void);
void* netfs_OpenSession(void);
void* netfs_ParseURL(void);
void* netfs_log_message_tracer(void);
void* netfs_log_message_tracer_auth_type(void);
void* netfs_log_message_tracer_automounted(void);
void* netfs_log_message_tracer_executable_name(void);
void* netfs_log_message_tracer_result(void);
void* netfs_log_message_tracer_result_on_fail(void);
void* netfs_log_message_tracer_scheme(void);

int NetFSUnmountHomeDirectory(const char* home, const char* path, uid_t uid, int flags /* unsure */);
int NetFSMountHomeDirectoryWithAuthentication(const char* url, const char* home, const char* path, uid_t uid, size_t mount_dir_length, const char* mount_dir, const char* username, const char* authenticator, int flags, unsigned int* out_was_remounted);

enum {
	// no clue what the real value is
	kNetFSAllowKerberos = 1,
};

#endif

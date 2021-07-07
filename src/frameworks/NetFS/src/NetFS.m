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


#include <NetFS/NetFS.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* GetCompleteMountURL(void)
{
    if (verbose) puts("STUB: GetCompleteMountURL called");
    return NULL;
}

void* GetURLFromURLRemountInfo(void)
{
    if (verbose) puts("STUB: GetURLFromURLRemountInfo called");
    return NULL;
}

void* GetURLRemountInfo(void)
{
    if (verbose) puts("STUB: GetURLRemountInfo called");
    return NULL;
}

void* GetURLRemountInfoSize(void)
{
    if (verbose) puts("STUB: GetURLRemountInfoSize called");
    return NULL;
}

void* MountCompleteURL(void)
{
    if (verbose) puts("STUB: MountCompleteURL called");
    return NULL;
}

void* MountServerURL(void)
{
    if (verbose) puts("STUB: MountServerURL called");
    return NULL;
}

void* MountURLWithAuthentication(void)
{
    if (verbose) puts("STUB: MountURLWithAuthentication called");
    return NULL;
}

void* NetFSCFStringtoCString(void)
{
    if (verbose) puts("STUB: NetFSCFStringtoCString called");
    return NULL;
}

void* NetFSClearAllURLApprovals(void)
{
    if (verbose) puts("STUB: NetFSClearAllURLApprovals called");
    return NULL;
}

void* NetFSCopyHostAndPort(void)
{
    if (verbose) puts("STUB: NetFSCopyHostAndPort called");
    return NULL;
}

void* NetFSCopyNormalizedURLString(void)
{
    if (verbose) puts("STUB: NetFSCopyNormalizedURLString called");
    return NULL;
}

void* NetFSCopyURLForRemountingVolume(void)
{
    if (verbose) puts("STUB: NetFSCopyURLForRemountingVolume called");
    return NULL;
}

void* NetFSGetMountInfo(void)
{
    if (verbose) puts("STUB: NetFSGetMountInfo called");
    return NULL;
}

void* NetFSGetMountInfoByFSID(void)
{
    if (verbose) puts("STUB: NetFSGetMountInfoByFSID called");
    return NULL;
}

void* NetFSGetSupportedSchemes(void)
{
    if (verbose) puts("STUB: NetFSGetSupportedSchemes called");
    return NULL;
}

void* NetFSGetURLApprovalState(void)
{
    if (verbose) puts("STUB: NetFSGetURLApprovalState called");
    return NULL;
}

void* NetFSInterface_AddRef(void)
{
    if (verbose) puts("STUB: NetFSInterface_AddRef called");
    return NULL;
}

void* NetFSInterface_Release(void)
{
    if (verbose) puts("STUB: NetFSInterface_Release called");
    return NULL;
}

void* NetFSLogToMessageTracer(void)
{
    if (verbose) puts("STUB: NetFSLogToMessageTracer called");
    return NULL;
}

void* NetFSMountURLAsync(void)
{
    if (verbose) puts("STUB: NetFSMountURLAsync called");
    return NULL;
}

void* NetFSMountURLCancel(void)
{
    if (verbose) puts("STUB: NetFSMountURLCancel called");
    return NULL;
}

void* NetFSMountURLProbe(void)
{
    if (verbose) puts("STUB: NetFSMountURLProbe called");
    return NULL;
}

void* NetFSMountURLSync(void)
{
    if (verbose) puts("STUB: NetFSMountURLSync called");
    return NULL;
}

void* NetFSMountURLWithAuthenticationSync(void)
{
    if (verbose) puts("STUB: NetFSMountURLWithAuthenticationSync called");
    return NULL;
}

void* NetFSQueryInterface(void)
{
    if (verbose) puts("STUB: NetFSQueryInterface called");
    return NULL;
}

void* NetFSSetURLApprovalState(void)
{
    if (verbose) puts("STUB: NetFSSetURLApprovalState called");
    return NULL;
}

void* NetFS_CreateInterface(void)
{
    if (verbose) puts("STUB: NetFS_CreateInterface called");
    return NULL;
}

void* PremountHomeDirectoryWithAuthentication(void)
{
    if (verbose) puts("STUB: PremountHomeDirectoryWithAuthentication called");
    return NULL;
}

void* RemountInfoQueryInterface(void)
{
    if (verbose) puts("STUB: RemountInfoQueryInterface called");
    return NULL;
}

void* UnmountServerURL(void)
{
    if (verbose) puts("STUB: UnmountServerURL called");
    return NULL;
}

void* netfs_Cancel(void)
{
    if (verbose) puts("STUB: netfs_Cancel called");
    return NULL;
}

void* netfs_CancelAsyncMount(void)
{
    if (verbose) puts("STUB: netfs_CancelAsyncMount called");
    return NULL;
}

void* netfs_CloseSession(void)
{
    if (verbose) puts("STUB: netfs_CloseSession called");
    return NULL;
}

void* netfs_ConnectToServerAsync(void)
{
    if (verbose) puts("STUB: netfs_ConnectToServerAsync called");
    return NULL;
}

void* netfs_ConnectToServerSync(void)
{
    if (verbose) puts("STUB: netfs_ConnectToServerSync called");
    return NULL;
}

void* netfs_CreateSessionRef(void)
{
    if (verbose) puts("STUB: netfs_CreateSessionRef called");
    return NULL;
}

void* netfs_CreateURL(void)
{
    if (verbose) puts("STUB: netfs_CreateURL called");
    return NULL;
}

void* netfs_EnumerateShares(void)
{
    if (verbose) puts("STUB: netfs_EnumerateShares called");
    return NULL;
}

void* netfs_GetServerInfo(void)
{
    if (verbose) puts("STUB: netfs_GetServerInfo called");
    return NULL;
}

void* netfs_Mount(void)
{
    if (verbose) puts("STUB: netfs_Mount called");
    return NULL;
}

void* netfs_MountURLWithAuthenticationAsync(void)
{
    if (verbose) puts("STUB: netfs_MountURLWithAuthenticationAsync called");
    return NULL;
}

void* netfs_MountURLWithAuthenticationSync(void)
{
    if (verbose) puts("STUB: netfs_MountURLWithAuthenticationSync called");
    return NULL;
}

void* netfs_OpenSession(void)
{
    if (verbose) puts("STUB: netfs_OpenSession called");
    return NULL;
}

void* netfs_ParseURL(void)
{
    if (verbose) puts("STUB: netfs_ParseURL called");
    return NULL;
}

void* netfs_log_message_tracer(void)
{
    if (verbose) puts("STUB: netfs_log_message_tracer called");
    return NULL;
}

void* netfs_log_message_tracer_auth_type(void)
{
    if (verbose) puts("STUB: netfs_log_message_tracer_auth_type called");
    return NULL;
}

void* netfs_log_message_tracer_automounted(void)
{
    if (verbose) puts("STUB: netfs_log_message_tracer_automounted called");
    return NULL;
}

void* netfs_log_message_tracer_executable_name(void)
{
    if (verbose) puts("STUB: netfs_log_message_tracer_executable_name called");
    return NULL;
}

void* netfs_log_message_tracer_result(void)
{
    if (verbose) puts("STUB: netfs_log_message_tracer_result called");
    return NULL;
}

void* netfs_log_message_tracer_result_on_fail(void)
{
    if (verbose) puts("STUB: netfs_log_message_tracer_result_on_fail called");
    return NULL;
}

void* netfs_log_message_tracer_scheme(void)
{
    if (verbose) puts("STUB: netfs_log_message_tracer_scheme called");
    return NULL;
}

int NetFSUnmountHomeDirectory(const char* home, const char* path, uid_t uid, int flags) {
	if (verbose) puts("STUB: NetFSUnmountHomeDirectory called");
	return -1;
};

int NetFSMountHomeDirectoryWithAuthentication(const char* url, const char* home, const char* path, uid_t uid, size_t mount_dir_length, const char* mount_dir, const char* username, const char* authenticator, int flags, unsigned int* out_was_remounted) {
	if (verbose) puts("STUB: NetFSMountHomeDirectoryWithAuthentication called");
	return -1;
};

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

#ifndef __LAUNCH_PRIVATE_H__
#define __LAUNCH_PRIVATE_H__

#include <mach/mach.h>
#include <sys/types.h>
#include <launch.h>
#include <unistd.h>
#include <paths.h>
#include <uuid/uuid.h>

#pragma GCC visibility push(default)

__BEGIN_DECLS

#define LAUNCH_EXITSTATUS_FAIRPLAY_FAIL (INT64_MAX)

#define LAUNCH_KEY_SETUSERENVIRONMENT "SetUserEnvironment"
#define LAUNCH_KEY_UNSETUSERENVIRONMENT "UnsetUserEnvironment"
#define LAUNCH_KEY_SHUTDOWN "Shutdown"
#define LAUNCH_KEY_SINGLEUSER "SingleUser"
#define LAUNCH_KEY_GETRESOURCELIMITS "GetResourceLimits"
#define LAUNCH_KEY_SETRESOURCELIMITS "SetResourceLimits"
#define LAUNCH_KEY_GETRUSAGESELF "GetResourceUsageSelf"
#define LAUNCH_KEY_GETRUSAGECHILDREN "GetResourceUsageChildren"

#define LAUNCHD_SOCKET_ENV "LAUNCHD_SOCKET"
#define LAUNCHD_SOCK_PREFIX _PATH_VARTMP "launchd"
#define LAUNCHD_TRUSTED_FD_ENV "__LAUNCHD_FD"
#define LAUNCHD_ASYNC_MSG_KEY "_AsyncMessage"
#define LAUNCH_KEY_BATCHCONTROL "BatchControl"
#define LAUNCH_KEY_BATCHQUERY "BatchQuery"

#define LAUNCH_JOBKEY_TRANSACTIONCOUNT "TransactionCount"
#define LAUNCH_JOBKEY_QUARANTINEDATA "QuarantineData"
#define LAUNCH_JOBKEY_SANDBOXPROFILE "SandboxProfile"
#define LAUNCH_JOBKEY_SANDBOXFLAGS "SandboxFlags"
#define LAUNCH_JOBKEY_SANDBOX_NAMED "Named"
#define	LAUNCH_JOBKEY_SANDBOXCONTAINER "SandboxContainer"
#define LAUNCH_JOBKEY_JETSAMPROPERTIES "JetsamProperties"
#define LAUNCH_JOBKEY_JETSAMPRIORITY "JetsamPriority"
#define LAUNCH_JOBKEY_JETSAMMEMORYLIMIT "JetsamMemoryLimit"
#define LAUNCH_JOBKEY_JETSAMMEMORYLIMITBACKGROUND "JetsamMemoryLimitBackground"
#define LAUNCH_JOBKEY_SECURITYSESSIONUUID "SecuritySessionUUID"
#define LAUNCH_JOBKEY_DISABLEASLR "DisableASLR"
#define LAUNCH_JOBKEY_XPCDOMAIN "XPCDomain"
#define LAUNCH_JOBKEY_POSIXSPAWNTYPE "POSIXSpawnType"

#define LAUNCH_KEY_JETSAMLABEL "JetsamLabel"
#define LAUNCH_KEY_JETSAMFRONTMOST "JetsamFrontmost"
#define LAUNCH_KEY_JETSAMACTIVE "JetsamActive"
#define LAUNCH_KEY_JETSAMPRIORITY LAUNCH_JOBKEY_JETSAMPRIORITY
#define LAUNCH_KEY_JETSAMMEMORYLIMIT LAUNCH_JOBKEY_JETSAMMEMORYLIMIT

#define LAUNCH_KEY_POSIXSPAWNTYPE_APP LAUNCH_KEY_PROCESSTYPE_APP
#define LAUNCH_KEY_POSIXSPAWNTYPE_SYSTEMAPP "SystemApp"
#define LAUNCH_KEY_POSIXSPAWNTYPE_STANDARD LAUNCH_KEY_PROCESSTYPE_STANDARD
#define LAUNCH_KEY_POSIXSPAWNTYPE_BACKGROUND LAUNCH_KEY_PROCESSTYPE_BACKGROUND
#define LAUNCH_KEY_POSIXSPAWNTYPE_INTERACTIVE LAUNCH_KEY_PROCESSTYPE_INTERACTIVE
#define LAUNCH_KEY_POSIXSPAWNTYPE_ADAPTIVE LAUNCH_KEY_PROCESSTYPE_ADAPTIVE
#define LAUNCH_KEY_POSIXSPAWNTYPE_TALAPP "TALApp"

#define LAUNCH_JOBKEY_EMBEDDEDPRIVILEGEDISPENSATION "EmbeddedPrivilegeDispensation"
#define LAUNCH_JOBKEY_EMBEDDEDHOMESCREEN "EmbeddedHomeScreen"
#define LAUNCH_JOBKEY_EMBEDDEDMAINTHREADPRIORITY "EmbeddedMainThreadPriority"

#define LAUNCH_JOBKEY_ENTERKERNELDEBUGGERBEFOREKILL "EnterKernelDebuggerBeforeKill"
#define LAUNCH_JOBKEY_PERJOBMACHSERVICES "PerJobMachServices"
#define LAUNCH_JOBKEY_SERVICEIPC "ServiceIPC"
#define LAUNCH_JOBKEY_BINARYORDERPREFERENCE "BinaryOrderPreference"
#define LAUNCH_JOBKEY_MACHEXCEPTIONHANDLER "MachExceptionHandler"
#define LAUNCH_JOBKEY_MULTIPLEINSTANCES "MultipleInstances"
#define LAUNCH_JOBKEY_EVENTMONITOR "EventMonitor"
#define LAUNCH_JOBKEY_SHUTDOWNMONITOR "ShutdownMonitor"
#define LAUNCH_JOBKEY_BEGINTRANSACTIONATSHUTDOWN "BeginTransactionAtShutdown"
#define LAUNCH_JOBKEY_XPCDOMAINBOOTSTRAPPER "XPCDomainBootstrapper"
#define LAUNCH_JOBKEY_ASID "AuditSessionID"
#define LAUNCH_JOBKEY_JOINGUISESSION "JoinGUISession"

#define LAUNCH_JOBKEY_MACH_KUNCSERVER "kUNCServer"
#define LAUNCH_JOBKEY_MACH_EXCEPTIONSERVER "ExceptionServer"
#define LAUNCH_JOBKEY_MACH_TASKSPECIALPORT "TaskSpecialPort"
#define LAUNCH_JOBKEY_MACH_HOSTSPECIALPORT "HostSpecialPort"
#define LAUNCH_JOBKEY_MACH_ENTERKERNELDEBUGGERONCLOSE "EnterKernelDebuggerOnClose"
#define LAUNCH_JOBKEY_LOWPRIORITYBACKGROUNDIO "LowPriorityBackgroundIO"
#define LAUNCH_JOBKEY_LEGACYTIMERS "LegacyTimers"

#define LAUNCH_ENV_INSTANCEID "LaunchInstanceID"

#define JETSAM_PROPERTY_PRIORITY "Priority"
#define JETSAM_PROPERTY_MEMORYLIMIT "MemoryLimitMB"

/* For LoginWindow.
 *
 * After this call, the task's bootstrap port is set to the per session launchd.
 *
 * This returns 1 on success (it used to return otherwise), and -1 on failure.
 */
#define	LOAD_ONLY_SAFEMODE_LAUNCHAGENTS (1 << 0)
#define LAUNCH_GLOBAL_ON_DEMAND (1 << 1)
pid_t
create_and_switch_to_per_session_launchd(const char *, int flags, ...);

/* Also for LoginWindow.
 *
 * This is will load jobs at the LoginWindow prompt.
 */
void
load_launchd_jobs_at_loginwindow_prompt(int flags, ...);

/* For CoreProcesses */
#define SPAWN_VIA_LAUNCHD_STOPPED 0x0001
#define SPAWN_VIA_LAUNCHD_TALAPP 0x0002
#define SPAWN_VIA_LAUNCHD_WIDGET 0x0004
#define SPAWN_VIA_LAUNCHD_DISABLE_ASLR 0x0008

struct spawn_via_launchd_attr {
	uint64_t spawn_flags;
	const char *spawn_path;
	const char *spawn_chdir;
 	const char * const * spawn_env;
 	const mode_t *spawn_umask;
 	mach_port_t *spawn_observer_port;
 	const cpu_type_t *spawn_binpref;
	size_t spawn_binpref_cnt;
	void * spawn_quarantine;
	const char *spawn_seatbelt_profile;
	const uint64_t *spawn_seatbelt_flags;
};

#define spawn_via_launchd(a, b, c) _spawn_via_launchd(a, b, c, 3)
pid_t
_spawn_via_launchd(const char *label, const char * const *argv,
	const struct spawn_via_launchd_attr *spawn_attrs, int struct_version);

int
launch_wait(mach_port_t port);

/* The mpm_*() APIs no longer do anything. */
kern_return_t
mpm_wait(mach_port_t ajob, int *wstatus);

kern_return_t
mpm_uncork_fork(mach_port_t ajob);

launch_data_t
launch_socket_service_check_in(void);

__END_DECLS

#pragma GCC visibility pop


#endif /* __LAUNCH_PRIVATE_H__ */
